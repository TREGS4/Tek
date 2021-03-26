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

//string to sum : takes the incoming string and sum it's component splitted with \n
unsigned int char_to_sum(char str[]){
	char delim[] = "\n";
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
unsigned int testproof(int diff, unsigned int sum, unsigned long proof){
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
	ptr[i] = '\n';
	i++;
	size_t j = 0;
	while(strproof[j] != '\0'){
		ptr[i + j] = strproof[j];
		j++;
	}
	ptr[i+j] = '\0';
}

//read on fdin and send the proff on fdout
//void Node(int fdin, int fdout){

//}

int moncuq()
{
	//char str[] = "prevhash\ntimestamp\nindex\ntransaction";
	  char nbstr[] = "1\n2\n3\n5";
	  /*char *str = malloc(sizeof(char) * strlen(nbstr));
	  strcpy(str,nbstr);
	  unsigned int sum = char_to_sum(str);
	  free(str);
	  printf("sum : %d\n",sum);

	  unsigned int proof = mine(difficulty,sum);*/
	  int diff = 2;
	  unsigned long proof = mine_from_string(diff,nbstr);

	  /*char *str = malloc(sizeof(char) * strlen(nbstr));
	  strcpy(str,nbstr);
	  unsigned int res = testproof(diff, char_to_sum(str), proof);
	  free(str);
	  if(res == 0)
		  printf("true\n");
	  else
		  printf("false\n");*/
	  printf("proof : %ld\n", proof);
	  char strout[len_of_proof(proof) + strlen(nbstr) + 2];
	  rebuild(strout, nbstr, proof);
	  printf("str finale : \n%s\n", strout);
	  printf("str finale v2 : \n");
	  for(size_t i = 0; i < len_of_proof(proof) + strlen(nbstr) + 2; i++)
		  printf("%d\n",strout[i]);
	  printf("\n");

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
	return 0;
}
