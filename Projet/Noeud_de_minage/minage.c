#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <err.h>
#include <pthread.h>
#include <sys/wait.h>
#include <signal.h>
#include "../Hash/sha256.h"

//read stdin "difficulty-index-prevhashmerklehash"
//read : string(hash infos)
//write stdout "difficulty-index-prevhashmerklehash-proof"
//write : string(either : "occupied" or "proof")
//
int occupied = 0;
int ismining = 0;
//struct for threads
struct mining_arg{
	char * sum;
	int diff;
	unsigned long nbthread;
	int start;
};

//input string splitting : takes the incoming string and split it's component splitted with -
char *string_split(char str[], int *diff){
	char delim[] = "-";
	char *token = strtok(str, delim);
	int count = 0;
	//printf("string split token :\n");
	while(token != NULL)
	{
		//printf("count %d :%s\n", count, token);
		if(count == 0)
			*diff = atoi(token);
		else if(count == 2){
			char *res = malloc(sizeof(char) * strlen(token));
			for(size_t i = 0; i < strlen(token); i++)
				res[i] = token[i];
			return res;
		}
		token = strtok(NULL, delim);
		if(count > 2)
			err(3,"mining.c : char to sum : Wrong format");
		count++;
	}
	return NULL;
}

//mine with the string from string_splitting
void *thread_mining(void *arg){
	struct mining_arg *m_a = arg;
	int diff = m_a->diff;
	char * sum = m_a->sum;
	unsigned long nbthread = m_a->nbthread;
	unsigned long *proof = malloc(sizeof(unsigned long));
	*proof = m_a->start;
	free(m_a);
	//printf("start : %d\n nbthread : %ld\n", m_a->start, nbthread);
	//printf("sum : %s\n",sum);
	unsigned int res = 1;
	char str[5*SHA256_BLOCK_SIZE];
	while(res == 1 && ismining == 0){
		//str = sum + proof
		sprintf((char *)str, "%ld%s", *proof, sum);
		//printf("proof : %ld\nstring : %s\n",*proof, str);

		//hash with sha256
		BYTE buf[SHA256_BLOCK_SIZE];
		sha256((BYTE *)str, buf);
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
			*proof += nbthread;
		else
			ismining = 1;
	}
	return (void *)proof;
}

//return 0 if proof is correct, 1 if not
unsigned int __testproof(int diff, char * sum, unsigned long proof){
	unsigned int res = 0;
	BYTE str[5*SHA256_BLOCK_SIZE];
	//str = sum + proof
	sprintf((char *)str, "%ld%s", proof, sum);

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
unsigned int testproof(char *strin ,unsigned long proof){
	int diff = 0;
	char *str = malloc(sizeof(char) * strlen(strin));
	strcpy(str,strin);
	char * sum = string_split(str, &diff);
	unsigned int res = __testproof(diff, sum, proof);
	free(str);
	return res;
}

//find the proof from the string
unsigned int mine_from_string(char *strin, int nbthread){
	char *str = malloc(sizeof(char) * strlen(strin));
	strcpy(str,strin);
	int diff = 0;
	char *sum = string_split(str, &diff);
	if(sum == NULL)
		err(3,"mining.c : mine_from_string : error while creating the sum");
	free(str);

	//uncomment to debug
	//printf("mine from string : \nsum : %s\n", sum);

	//threads for the mining
	pthread_t thr[nbthread];
	ismining = 0;
	unsigned int proof = 0;
	unsigned int *output = NULL;
	for(int n = 0; n < nbthread; n++){
		//struct for arg
		struct mining_arg *m_a = malloc(sizeof(struct mining_arg));
		m_a->diff = diff;
		m_a->nbthread = nbthread;
		m_a->start = n;
		m_a->sum = sum;

		if(pthread_create(thr + n, NULL, thread_mining, (void *)m_a) != 0)
			err(3,"mining.c: mine_from_string: error while creating the threads");
	}
	for(int n = 0; n < nbthread; n++){
		pthread_join(thr[n], (void **)&output);
		//printf("output thread %d :%d\n", n, *output);
		if(testproof(strin, *output) == 0){
			proof = *output;
		}
	}
	free(output);
	free(sum);
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
	char strout[len_of_proof(proof) + 4*SHA256_BLOCK_SIZE + 2];
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
			err(3,"mining.c : mininglobby: error while reading");
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
	char *str = "Pierre a donné 145 euros a thimot";
	char *str2 = "pouet";
	char *nbstr = "1-3-";
	//unsigned long proof = mine_from_string(nbstr,4);
	char str3[6*SHA256_BLOCK_SIZE];
	BYTE buf[SHA256_BLOCK_SIZE];
	BYTE buf2[SHA256_BLOCK_SIZE];
	sha256((BYTE *)str, buf);
	sha256((BYTE *)str2, buf2);

	char buff[SHA256_BLOCK_SIZE * 2];
	char buff2[SHA256_BLOCK_SIZE * 2];

	sha256ToAscii(buf, buff);
	sha256ToAscii(buf2, buff2);

	sprintf(str3, "%s%s%s", nbstr, buff, buff2);  				//stack smashing here
	/*printf("buf :");
	for(int i = 0; i < 4 * SHA256_BLOCK_SIZE + 4; i++)
		printf("%02x", str3[i]);
	printf("\n");*/

	//printf("buf :");
	char str4[4*SHA256_BLOCK_SIZE + 5];
	for(int i = 0; i < 4*SHA256_BLOCK_SIZE + 4; i++){
		//printf("%c", str3[i]);
		str4[i] = str3[i];
	}
	str4[4*SHA256_BLOCK_SIZE+4] = '\0';
	//printf("\n");

	//printf("%s\n",str4);
	mine_and_write(str4, STDOUT_FILENO, 4);
	printf("\n");
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
