#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "../Hash/sha256.h"

//read stdin "prevhash\ntimestamp\nindex\ntransaction"
//read : string(hash infos)
//write stdout "prevhash\ntimestamp\nindex\ntransaction\nproof"
//write : string(either : "occupied" or "proof")
//
//(infos of the current block)
//char *info[prevhash,timestamp,index,transaction,proof]

//global var
int nbinfo = 4;
int difficulty = 2;

//string to sum : 
unsigned int char_to_sum(char str[]){
	char delim[] = "\n";
	char *token = strtok(str, delim);
	//char **res = malloc(nbinfo * sizeof(char *));
	unsigned int res = 0;
	while(token != NULL)
	{
		res += atoi(token);
		token = strtok(NULL, delim);
	}
	return res;
}

unsigned int my_pow(unsigned int x, unsigned int power){
	int tmp = x;
	x = 1;
	for(unsigned int i = 0; i<power; i++){
		x *= tmp;
	}
	return x;
}

unsigned int mine(int diff, unsigned int sum){
	unsigned int proof = 0;
	unsigned int res = 1;
	unsigned int tmp;
	BYTE str[SHA256_BLOCK_SIZE];
	while(res % my_pow(10,diff) != 0){
		//int to str
		tmp = sum + proof;
		sprintf(str, "%d", tmp);

		//hash with sha256
		BYTE buf[SHA256_BLOCK_SIZE];
		sha256(str, buf);
		printf("buf : %s\n",buf);

		//str to int
		res = atoi(buf);
		printf("res : %d\n",res);
		proof++;
	}
	return proof;
}

int moncuq()
{
	//char str[] = "prevhash\ntimestamp\nindex\ntransaction";
	/*char nbstr[] = "1\n2\n3\n5";
	  char *str = malloc(sizeof(char) * strlen(nbstr));
	  strcpy(str,nbstr);
	  unsigned int sum = char_to_sum(str);
	  free(str);
	  printf("%s\n",nbstr);
	  printf("sum : %d\n",sum);
	  unsigned int proof = mine(difficulty,sum);
	  printf("%d\n",proof);*/
	BYTE hash1[SHA256_BLOCK_SIZE] = {0x24,0x8d,0x6a,0x61,0xd2,0x06,0x38,0xb8,0xe5,0xc0,0x26,0x93,0x0c,0x3e,0x60,0x39,0xa3,0x3c,0xe4,0x59,0x64,0xff,0x21,0x67,0xf6,0xec,0xed,0xd4,0x19,0xdb,0x06,0xc1};
	printf("hash :\n%s\n", hash1);
	BYTE str[] = {"abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq"};
	BYTE buf[SHA256_BLOCK_SIZE];
	sha256(str, buf);
	printf("buf :\n%s\n", buf);
	return 0;
}
