#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <err.h>
#include <pthread.h>
#include <sys/wait.h>
#include <signal.h>
#include "../Hash/sha256.h"
#include "../Network/network.h"
#include "../Block/transactions.h"
#include "../Block/blockchain.h"
#include "../Block/block.h"
#include "../Tools/queue/shared_queue.h"

//read stdin "difficulty-index-prevhashmerklehash"
//read : string(hash infos)
//write stdout "difficulty-index-prevhashmerklehash-proof"
//write : string(either : "occupied" or "proof")
//
//struct for threads
struct mining_arg{
	char * sum;
	int diff;
	unsigned long nbthread;
	int start;
	int *ismining;
};

//input string splitting : takes the incoming string and split it's component splitted with -
/*char *string_split(char str[], int *diff){
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
			size_t size = strlen(token);
			char *res = malloc(sizeof(char) * (size+1));
			res[size] = 0;
			for(size_t i = 0; i < size; i++)
				res[i] = token[i];
			return res;
		}
		token = strtok(NULL, delim);
		if(count > 2)
			err(3,"mining.c : char to sum : Wrong format");
		count++;
	}
	return NULL;
}*/

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

//mine with the string from string_splitting
void *thread_mining(void *arg){
	struct mining_arg *m_a = arg;
	int diff = m_a->diff;
	char * sum = m_a->sum;
	unsigned long nbthread = m_a->nbthread;
	unsigned long *proof = malloc(sizeof(unsigned long));
	*proof = m_a->start;
	int *ismining = m_a->ismining;
	free(m_a);
	//printf("start : %d\n nbthread : %ld\n", m_a->start, nbthread);
	//printf("sum : %s\n",sum);
	unsigned int res = 1;
	char str[5*SHA256_BLOCK_SIZE];
	while(res == 1 && *ismining == 0){
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
			*ismining = 1;
	}
	return (void *)proof;
}

//return 0 if proof is correct, 1 if not
unsigned int testproof(int diff, char * sum, unsigned long proof){
	unsigned int res = 0;
	BYTE str[SHA256_BLOCK_SIZE + len_of_proof(proof) + 1];
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
/*unsigned int testproof(char *strin ,unsigned long proof){
	int diff = 0;
	char *str = malloc(sizeof(char) * strlen(strin));
	strcpy(str,strin);
	char * sum = string_split(str, &diff);
	unsigned int res = __testproof(diff, sum, proof);
	free(str);
	return res;
}*/

//find the proof from the string
unsigned int mine_from_string(char *sum, int nbthread, int diff){
	//char *str = malloc(sizeof(char) * strlen(strin));
	//strcpy(str,strin);
	//char *sum = string_split(str, &diff);
	//if(sum == NULL)
	//	err(3,"mining.c : mine_from_string : error while creating the sum");
	//free(str);

	//uncomment to debug
	//printf("mine from string : \nsum : %s\n", sum);

	//threads for the mining
	pthread_t thr[nbthread];
	unsigned int proof = 0;
	unsigned int *output = NULL;
	int ismining = 0;
	for(int n = 0; n < nbthread; n++){
		//struct for arg
		struct mining_arg *m_a = malloc(sizeof(struct mining_arg));
		m_a->diff = diff;
		m_a->nbthread = nbthread;
		m_a->start = n;
		m_a->sum = sum;
		m_a->ismining = &ismining;

		if(pthread_create(thr + n, NULL, thread_mining, (void *)m_a) != 0)
			err(3,"mining.c: mine_from_string: error while creating the threads");
	}
	for(int n = 0; n < nbthread; n++){
		pthread_join(thr[n], (void **)&output);
		//printf("output thread %d :%d\n", n, *output);
		if(testproof(diff, sum, *output) == 0){
			proof = *output;
		}
	}
	free(output);
	free(sum);
	return proof;
}

//rebuild an output string using former string and proof
/*void rebuild(char* ptr, char *str, unsigned long proof){
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
}*/

//take a string and send it with the proof on fdout
/*void mine_and_write(char str[], int fdout, int nbthread){	
	unsigned long proof = mine_from_string(str, nbthread);
	char strout[len_of_proof(proof) + 4*SHA256_BLOCK_SIZE + 2];
	rebuild(strout, str, proof);
	rewrite(fdout, &strout, strlen(strout));
}*/

//main function : execute this to launch a mining node
//qrguments : 
//blockachain : the current blockchain, 
//tlq : a queue with the transaction lists, 
//exq : the queue where we push the new blocks, 
//nb_trhead = number of thread we will use to mine,
//difficulty : difficulty of the mining
//status : to end the prgrm if needed
int mining(BLOCKCHAIN_M *blockchain, TL_M * txl, shared_queue * exq, int nb_thread, int difficulty, int * status)
{
	while(*status)
	{
		//Get a transaction list's hash
		//build a hash
		char *tljson = tlToJson(&txl->tl);
		BYTE merkle_hash[SHA256_BLOCK_SIZE];
		sha256((BYTE *)tljson, merkle_hash);

		//mining a proof
		BYTE hash[2 * SHA256_BLOCK_SIZE];
		BYTE *prev_hash = getLastBlock(&blockchain->bc)->blockHash;
		sprintf((char *)hash,"%s%s", (char *)prev_hash, (char *)merkle_hash);
		unsigned long proof = mine_from_string((char *)hash, nb_thread, difficulty);
		
		//create the mekle hahs with the proof
		BYTE buff[SHA256_BLOCK_SIZE + len_of_proof(proof) + 1];
		//str = sum + proof
		sprintf((char *)buff, "%ld%s", proof, hash);

		//hash with sha256
		BYTE new_merkle_hash[SHA256_BLOCK_SIZE];
		sha256(buff, new_merkle_hash);

		//create a new block
		BLOCK *block = malloc(sizeof(BLOCK));
		block->tl = txl->tl;
		block->previusHash = prev_hash;
		block->blockHash = new_merkle_hash;
		//BLOCK->proof = adrien ca existe pas mais fait le stp;

		//return proof;
		shared_queue_push(exq, block);

		sleep(10);
	}
	return 1;
}

/*int moncuq()
{
	//mininglobby(STDIN_FILENO, STDOUT_FILENO);
	char *str = "Pierre a donné 145 euros a thimot";
	char *str2 = "pouet";
	char *nbstr = "2-3-";
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
	*//*printf("buf :");
	for(int i = 0; i < 4 * SHA256_BLOCK_SIZE + 4; i++)
		printf("%02x", str3[i]);
	printf("\n");*/

	//printf("buf :");
	/*char str4[4*SHA256_BLOCK_SIZE + 5];
	for(int i = 0; i < 4*SHA256_BLOCK_SIZE + 4; i++){
		//printf("%c", str3[i]);
		str4[i] = str3[i];
	}
	str4[4*SHA256_BLOCK_SIZE+4] = '\0';
	//printf("\n");

	//printf("%s\n",str4);
	mine_and_write(str4, STDOUT_FILENO, 4);
	printf("\n");
	*//*char *str = malloc(sizeof(char) * strlen(nbstr));
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
	/*return 0;
}*/
