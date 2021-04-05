#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <err.h>
#include <pthread.h>
#include <sys/wait.h>
#include <signal.h>
#include "../Hash/sha256.h"

//read stdin "difficulty-index-prevhash-merklehash"
//read : string(hash infos)
//write stdout "difficulty-index-prevhash-merklehash-proof"
//write : string(either : "occupied" or "proof")
//
int occupied = 0;
int ismining = 0;

//string to sum : takes the incoming string and sum it's component splitted with \n
unsigned int char_to_sum(char str[], int *diff){
	char delim[] = "-";
	char *token = strtok(str, delim);
	unsigned int res = 0;
	int count = 0;
	while(token != NULL)
	{
		if(count == 0)
			*diff = atoi(token);
		else if(count != 1)
			res += atol(token);
		token = strtok(NULL, delim);
		if(count > 3)
			err(3,"minage : char to sum : Wrong format");
		count++;
	}
	return res;
}

//mine with the sum from char_to_sum
unsigned long mine(int diff, unsigned int sum){
	unsigned long proof = 0;
	unsigned int res = 1;
	unsigned int tmp;
	BYTE str[SHA256_BLOCK_SIZE];
	while(res == 1){
		//int to str
		tmp = sum + proof;
		sprintf((char *)str, "%d", tmp);

		//hash with sha256
		BYTE buf[SHA256_BLOCK_SIZE];
		sha256(str, buf);
		//uncomment if you want to print the hash
		/*printf("buf :");
		  for(int i = 0; i < 32; i++)
		  printf("%02x", buf[i]);
		  printf("\n");*/
		res = 0;
		for(int i = SHA256_BLOCK_SIZE - diff; i < SHA256_BLOCK_SIZE; i++)
		{
			if(buf[i] != 0)
				res = 1;
		}
		if(res == 1)
			proof++;
	}
	return proof;
}

struct mining_arg{
	unsigned int sum;
	int diff;
	unsigned long nbthread;
};

//mine with the sum from char_to_sum
void *thread_mining(void *arg){
	struct mining_arg *m_a = arg;
	int diff = m_a->diff;
	unsigned int sum = m_a->sum;
	unsigned long nbthread = m_a->nbthread;
	unsigned long proof = 0;
	unsigned int res = 1;
	unsigned int tmp;
	BYTE str[SHA256_BLOCK_SIZE];
	while(res == 1 && ismining == 0){
		//int to str
		tmp = sum + proof;
		sprintf((char *)str, "%d", tmp);

		//hash with sha256
		BYTE buf[SHA256_BLOCK_SIZE];
		sha256(str, buf);
		//uncomment if you want to print the hash
		/*printf("buf :");
		  for(int i = 0; i < 32; i++)
		  printf("%02x", buf[i]);
		  printf("\n");*/
		res = 0;
		for(int i = SHA256_BLOCK_SIZE - diff; i < SHA256_BLOCK_SIZE; i++)
		{
			if(buf[i] != 0)
				res = 1;
		}
		if(res == 1)
			proof += nbthread;
		else
			ismining = 1;
	}
	return (void *)proof;
}

//return 0 if proof is correct, 1 if not
unsigned int __testproof(int diff, unsigned int sum, unsigned long proof){
	unsigned int res = 0;
	unsigned int tmp;
	BYTE str[SHA256_BLOCK_SIZE];
	//int to str
	tmp = sum + proof;
	sprintf((char *)str, "%d", tmp);

	//hash with sha256
	BYTE buf[SHA256_BLOCK_SIZE];
	sha256(str, buf);
	for(int i = SHA256_BLOCK_SIZE - diff; i < SHA256_BLOCK_SIZE; i++)
	{
		if(buf[i] != 0)
			res = 1;
	}
	return res;
}

//test the proof witout splitting strin return 0 if proof is correct, 1 if not
unsigned int testproof(char strin[] ,unsigned long proof){
	int diff = 0;
	char *str = malloc(sizeof(char) * strlen(strin));
	strcpy(str,strin);
	unsigned int sum = char_to_sum(str, &diff);
	unsigned int res = __testproof(diff, sum, proof);
	free(str);
	return res;
}

//find the proof from the string
unsigned int mine_from_string(char *strin, int nbthread){
	char *str = malloc(sizeof(char) * strlen(strin));
	strcpy(str,strin);
	int diff = 0;
	unsigned int sum = char_to_sum(str, &diff);
	free(str);

	//struct for arg
	struct mining_arg *m_a = malloc(sizeof(struct mining_arg));
	m_a->diff = diff;
	m_a->nbthread = nbthread;
	m_a->sum = sum;

	//threads for the mining
	pthread_t thr[nbthread];
	ismining = 0;
	unsigned int proof = 0;
	unsigned int *output = 0;
	for(int n = 0; n < nbthread; n++){
		if(pthread_create(thr + n, NULL, thread_mining, (void *)m_a) != 0)
			err(3,"mining.c: mine_from_string: error while creating the threads");
	}
	for(int n = 0; n < nbthread; n++){
		pthread_join(thr[n], (void *)output);
		if(testproof(strin, *output) == 0)
			proof = *output;
	}
	//unsigned int proof = mine(diff,sum);
	free(m_a);
	return proof;
}

//len of the proof if it was a string
size_t len_of_proof(unsigned long proof){
	size_t len = 0;
	unsigned long tmp = proof;
	while(tmp != 0){
		tmp /= 10;
		len++;
	}
	return len;
}

//rebuild an output string using former string and proof
void rebuild(char* ptr, char *str, unsigned long proof){
	sprintf(ptr, "%s-%ld", str, proof);
}

//write on fd
void rewrite(int fd, const void *buf, size_t count)
{
	ssize_t res = 0;
	ssize_t tmp = 0;
	while((res = write(fd, buf + tmp, count)) < (ssize_t)count - tmp){
		if(res == -1)
			err(3,"error while writting");
		tmp += res;
	}
}

//take a string and send it with the proof on fdout
void mine_and_write(char str[], int fdout, int nbthread){
	unsigned long proof = mine_from_string(str, nbthread);
	char strout[len_of_proof(proof) + strlen(str) + 2];
	rebuild(strout, str, proof);
	rewrite(fdout, &strout, strlen(strout));
}

//handler for mining lobby
void handler(){
	wait(NULL);
	occupied = 0;
}

//basic fonction wich read on fdin and send the res on fdout, if occupied send "occupied"
int mininglobby(int fdin, int fdout, int nbthread){
	size_t lenbuff = 1024;
	char buff[lenbuff];
	pid_t p = getpid();
	while(p != 0){
		signal(SIGCHLD, handler);
		if(read(fdin, &buff, lenbuff) == 0)
			err(3,"minage : mininglobby: error while reading");
		if(occupied == 0){
			if((p = fork()) == 0){//son
				occupied = 1;
				mine_and_write(buff, fdout, nbthread);
			}
			else
				occupied = 1;
		}
		else{
			write(fdout, "occupied", 8);
		}
	}
	return 0;
}

int moncuq()
{
	//mininglobby(STDIN_FILENO, STDOUT_FILENO);
	char nbstr[] = "1-2-3-5";
	//unsigned long proof = mine_from_string(nbstr);
	mine_and_write(nbstr, STDOUT_FILENO, 4);
	/*char *str = malloc(sizeof(char) * strlen(nbstr));
	strcpy(str,nbstr);
	unsigned int res = testproof(nbstr, proof);
	free(str);
	if(res == 0)
		printf("proof is true\n");
	else
		printf("proof is false\n");
	printf("proof : %ld\n", proof);
	char strout[len_of_proof(proof) + strlen(nbstr) + 2];
	rebuild(strout, nbstr, proof);
	printf("str finale : \n%s\n", strout);
	printf("str finale v2 : \n");
	for(size_t i = 0; i < len_of_proof(proof) + strlen(nbstr) + 2; i++){
		if(strout[i] == '\n')
			printf("\n%d\n",strout[i]);
		else{
			if(strout[i] == '\0')
				printf("\n0");
			else
				printf("%d",strout[i]);
		}
	}
	printf("\n");*/
	return 0;
}
