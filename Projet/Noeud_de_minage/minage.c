#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <err.h>
#include <sys/wait.h>
#include <signal.h>
#include "../Hash/sha256.h"

//read stdin "prevhash\ntimestamp\nindex\ntransaction"
//read : string(hash infos)
//write stdout "prevhash\ntimestamp\nindex\ntransaction\nproof"
//write : string(either : "occupied" or "proof")
//

//string to sum : takes the incoming string and sum it's component splitted with \n
unsigned int char_to_sum(char str[]){
	char delim[] = "-";
	char *token = strtok(str, delim);
	unsigned int res = 0;
	while(token != NULL)
	{
		res += atoi(token);
		token = strtok(NULL, delim);
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
unsigned int testproof(int diff, char strin[] ,unsigned long proof){
	char *str = malloc(sizeof(char) * strlen(strin));
	strcpy(str,strin);
	unsigned int res = __testproof(diff, char_to_sum(str), proof);
	free(str);
	return res;
}

//find the proof from the string
unsigned int mine_from_string(int diff, char *strin){
	char *str = malloc(sizeof(char) * strlen(strin));
	strcpy(str,strin);
	unsigned int sum = char_to_sum(str);
	free(str);
	unsigned int proof = mine(diff,sum);
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
	unsigned long len = len_of_proof(proof);
	char strproof[len];
	sprintf(strproof, "%ld", proof);
	size_t i = 0;
	while(str[i] != '\0'){
		ptr[i] = str[i];
		i++;
	}
	ptr[i] = '-';
	i++;
	size_t j = 0;
	while(strproof[j] != '\0'){
		ptr[i + j] = strproof[j];
		j++;
	}
	ptr[i+j] = '\0';
}


/*
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
void mine_and_write(int diff, char str[], int fdout){
	unsigned long proof = mine_from_string(diff,str);
	char strout[len_of_proof(proof) + strlen(str) + 2];
	rebuild(strout, str, proof);
	rewrite(fdout, &strout, strlen(strout));
}
*/
/* beta test
int occupied = 0;

void handler(){
	wait(NULL);
	occupied = 0;
}


//basic server wich read on fdin fork a node and send the res on fdout, if occupied send "occupied"
int mininglobby(int fdin, int fdout){
	ssize_t r = 0;
	int diff = 3;
	//int occupied = 0;
	size_t lenbuff = 512;
	char buff[] = "111-222-333-555";
	pid_t p = getpid();
	while(p != 0){
		while(r == 0){
			//write(fdout, "111-222-333-555", 15);
			r = read(fdin, buff, lenbuff);
			//r=1;
			//sleep(1);
		}
		if(r > 0){
			if(occupied == 1){
				write(fdout, "occupied", 8);
			}
			else{
				signal(SIGCHLD,handler);
				if((p = fork()) == 0){//son
					mine_and_write(diff, buff, fdout);
				}
				else{//father
					occupied = 1;
				}
			}
			r = 0;
		}
	}
	return 0;
}
*/


//int moncuq()
//{
	//mininglobby(STDIN_FILENO, STDOUT_FILENO);
	//char str[] = "prevhash\ntimestamp\nindex\ntransaction";
	//char nbstr[] = "111-222-333-555";
	/*char *str = malloc(sizeof(char) * strlen(nbstr));
	  strcpy(str,nbstr);
	  unsigned int sum = char_to_sum(str);
	  free(str);
	  printf("sum : %d\n",sum);

	  unsigned int proof = mine(difficulty,sum);*/
	//int diff = 1;
	//mine_and_write(diff, nbstr, STDOUT_FILENO);
	//printf("\n");
	//unsigned long proof = mine_from_string(diff,nbstr);

	/*char *str = malloc(sizeof(char) * strlen(nbstr));
	strcpy(str,nbstr);*/
	/*unsigned int res = testproof(diff, nbstr, proof);
	//free(str);
	if(res == 0)
		printf("proof is true\n");
	else
		printf("proof is false\n");
	printf("proof : %ld\n", proof);*/
	//char strout[len_of_proof(proof) + strlen(nbstr) + 2];
	//rebuild(strout, nbstr, proof);
	/*printf("str finale : \n%s\n", strout);
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
	printf("\n");
*/
	/*
	   BYTE hash1[SHA256_BLOCK_SIZE] = {0x24,0x8d,0x6a,0x61,0xd2,0x06,0x38,0xb8,0xe5,0xc0,0x26,0x93,0x0c,0x3e,0x60,0x39,0xa3,0x3c,0xe4,0x59,0x64,0xff,0x21,0x67,0xf6,0xec,0xed,0xd4,0x19,0xdb,0x06,0xc1};
	   printf("hash :\n%s\n", hash1);
	   BYTE str[] = {"abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq"};
	   BYTE buf[SHA256_BLOCK_SIZE];
	   sha256(str, buf);
	   printf("buf :\n");
	   for(int i = 0; i<32; i++)
	   printf("%02x", buf[i]);
	   printf("\n");*/
	//return 0;
//}
