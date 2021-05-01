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

void verifBlockchain(BLOCKCHAIN blockchain)
{
	printf("-------------Veryfing blockchain-------------\n\n");
	
	int checked = checkBlockchain(&blockchain);
	if (checked == 0)
		printf("This blockchain is legit.\n\n");
	else if (checked == 1)
		printf("There is something wrong with this blockchain.\n\n");
	
	printf("-------------End of verification-------------\n");
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
	printf("\n");
	
	verifBlockchain(newBlockchain);
	char *json = blockchainToJson(&newBlockchain);
	printf("%s\n", json);
	free(json);
}
