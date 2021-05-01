#include "./Network/network.h"

struct test
{
	int fd[2];
	pthread_mutex_t mutext;
	char *IP;
	char *firstserver;
};

void *moncul(void *arg)
{
	struct test *tst = arg;

	network(&tst->fd[0], &tst->fd[1], &tst->mutext, tst->IP, tst->firstserver);

	return NULL;
}



int reseau(int argc, char **argv)
{
	if(argc > 3)
		return -1;

	struct test tst;
	tst.fd[1] = -1;
	tst.fd[0] = -1;
	tst.IP = argv[1];
	tst.firstserver = argv[2];

	
	pthread_t thread;
	pthread_create(&thread, NULL, moncul, (void *)&tst);

	char *data = "Lorem ipsum do sodales lorem. Fusce.";



	char str[9];
	char type = 69;
	unsigned long long size = strlen(data);

	memcpy(str, &type, 1);
	memcpy(str + 1, &size, 8);
	
	sleep(2);
	/*
	pthread_mutex_lock(&tst.mutext);
	write(tst.fd[1], str, 10);
	write(tst.fd[1], data, size);
	pthread_mutex_unlock(&tst.mutext);
	printf("Message send\n");
*/
	pthread_join(thread, NULL);
	return 0;
}




// Main gestion

#include "Block/block.h"
#include "Block/blockchain.h"
#include "Hash/sha256.h"
#include <stdio.h>
#include <stdlib.h>

void printTransaction(TRANSACTION t) 
{
	printf("Amount : %14d$. From %s to %s.\n", t.amount, t.sender, t.receiver);
}

void printBlock(BLOCK block)
{
	printf("prevHash : ");
	for (int i = 0; i < SHA256_BLOCK_SIZE; i++)
	{
		printf("%02x", block.previusHash[i]);
	}
	printf("\n");
	
	printf("currHash : ");
	for (int i = 0; i < SHA256_BLOCK_SIZE; i++)
	{
		printf("%02x", block.blockHash[i]);
	}
	printf("\n");
	
	for (int i = 0; i < NB_TRANSACTIONS_PER_BLOCK; i++)
	{
		printTransaction(block.transactions[i]);
	}	
}

void printBlockchain(BLOCKCHAIN blockchain)
{
	printf("-------------------BLOCKCHAIN-------------------\n\n");
	printf("----Genesis----\n");
	
	printf("Hash : ");
	for (int i = 0; i < SHA256_BLOCK_SIZE; i++)
	{
		printf("%02x", blockchain.blocks[0].blockHash[i]);
	}
	printf("\n");
	printf("---------------\n\n");
	
	for(size_t i = 1; i < blockchain.blocksNumber; i++)
	{
		printf("---------------BLOCK %02ld---------------\n", i);
		printBlock(blockchain.blocks[i]);
		printf("--------------------------------------\n\n");
	}
	printf("------------------------------------------------\n");
}

int main(){
	TRANSACTION t = 
	{
		.sender = "Adrien",
		.receiver = "Paul",
		.amount = 104
	};
	
	TRANSACTION t2 = 
	{
		.sender = "Thimot",
		.receiver = "Margaux",
		.amount = 358
	};
	
	TRANSACTION t3 = 
	{
		.sender = "Pierre",
		.receiver = "Hugo",
		.amount = 226
	};
	
	BLOCK b;
	for (int i = 0; i < NB_TRANSACTIONS_PER_BLOCK; i++)
	{
		t.amount = rand() % 3000 + 1;
		b.transactions[i] = t;
	}
	BLOCK b2;
	for (int i = 0; i < NB_TRANSACTIONS_PER_BLOCK; i++)
	{
		t2.amount = rand() % 3000 + 1;
		b2.transactions[i] = t2;
	}
	BLOCK b3;
	for (int i = 0; i < NB_TRANSACTIONS_PER_BLOCK; i++)
	{
		t3.amount = rand() % 3000 + 1;
		b3.transactions[i] = t3;
	}
	/*for (int i = 0; i < NB_TRANSACTIONS_PER_BLOCK; i++){
		printf("txs %d: %d de %s a %s\n",i, b.transactions[i].amount, b.transactions[i].sender, b.transactions[i].receiver);
	}
	
	BYTE tmp[] = {"margauxcavalie"};
	sha256(tmp, b.previusHash);
	
	for (int i = 0; i < SHA256_BLOCK_SIZE; i++)
	{
		printf("%02x", b.previusHash[i]);
		
	}
	printf("\n");
	
	BYTE hash[SHA256_BLOCK_SIZE];
	getHash(&b, hash);
	
	for (int i = 0; i < SHA256_BLOCK_SIZE; i++)
	{
		printf("%02x", hash[i]);
	}
	
	printf("\n");*/
	
	BLOCKCHAIN newBlockchain = initBlockchain();
	addBlock(&newBlockchain, b);
	addBlock(&newBlockchain, b2);
	addBlock(&newBlockchain, b3);
	printf("Number of blocks in the blockchain : %lu.\n\n", newBlockchain.blocksNumber);
	printBlockchain(newBlockchain);

	return 1;
}
