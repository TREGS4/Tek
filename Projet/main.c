#include "Block/block.h"
#include "Block/transactions.h"
#include "Block/blockchain.h"
#include "Block/account.h"
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
	
	size_t nbTxs = block.tl.size;
	for (size_t i = 0; i < nbTxs; i++)
	{
		printTransaction(block.tl.transactions[i]);
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
		.receiver = "Tom",
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
	b.tl = initListTxs();
	for (int i = 0; i < 3; i++)
	{
		t.amount = rand() % 3000 + 1;
		addTx(&b.tl, &t);
	}

	BLOCK b2;
	b2.tl = initListTxs();
	for (int i = 0; i < 5; i++)
	{
		t2.amount = rand() % 3000 + 1;
		addTx(&b2.tl, &t2);
	}
	
	BLOCK b3;
	b3.tl = initListTxs();
	for (int i = 0; i < 1; i++)
	{
		t3.amount = rand() % 3000 + 1;
		addTx(&b3.tl, &t3);
	}

	BLOCKCHAIN newBlockchain = initBlockchain();
	addBlock(&newBlockchain, b);
	addBlock(&newBlockchain, b2);
	addBlock(&newBlockchain, b3);
	printf("Number of blocks in the blockchain : %lu.\n\n", newBlockchain.blocksNumber);
	printBlockchain(newBlockchain);
	printf("\n");


	verifBlockchain(newBlockchain);

	char *json = blockchainToJson(&newBlockchain);
	printf("\nBlockchain en format JSON:\n\n%s\n\n\n", json);
	free(json);


	printf("\nBlockchain en format BIN:\n\n");
	BLOCKCHAIN_BIN bcbin = blockchainToBin(&newBlockchain);
	for (size_t i = 0; i < bcbin.nbBytes; i++)
	{
		if (i % 20 == 0){
			printf("\n");
		}
		printf("%02x ", bcbin.bin[i]);
	}
	printf("\n\n");
	
	free(bcbin.bin);

	
	printf("\nTests :\n\n");


	printTransaction(newBlockchain.blocks[1].tl.transactions[0]);
	TRANSACTION_BIN txsbin = txsToBin(&newBlockchain.blocks[1].tl.transactions[0]);
	for (size_t i = 0; i < txsbin.nbBytes; i++)
	{
		if (i % 20 == 0){
			printf("\n");
		}
		printf("%02x ", txsbin.bin[i]);
	}
	printf("\n\n");

	TRANSACTION t4 = binToTxs(txsbin.bin);
	printTransaction(t4);
	free(txsbin.bin);
	

	printf("\n\n");
	printBlock(newBlockchain.blocks[1]);
	BLOCK_BIN blockbin = blockToBin(&newBlockchain.blocks[1]);
	for (size_t i = 0; i < blockbin.nbBytes; i++)
	{
		if (i % 20 == 0){
			printf("\n");
		}
		printf("%02x ", blockbin.bin[i]);
	}
	printf("\n\n");

	BLOCK b4 = binToBlock(blockbin.bin);
	printBlock(b4);
	free(blockbin.bin);



	printf("\n\n");
	printBlockchain(newBlockchain);
	bcbin = blockchainToBin(&newBlockchain);
	for (size_t i = 0; i < bcbin.nbBytes; i++)
	{
		if (i % 20 == 0){
			printf("\n");
		}
		printf("%02x ", bcbin.bin[i]);
	}
	printf("\n\n");

	BLOCKCHAIN bc = binToBlockchain(bcbin.bin);
	printBlockchain(bc);
	free(bcbin.bin);
	free(bc.blocks);


	ACCOUNT acc = generate_account();
	printf("acc key size: %ld\n", acc.size);
	printf("pem = \n%s\n", acc.publicKey_pem);


	char *jsffon = tlToJson(&newBlockchain.blocks[0].tl);
	printf("\n%s\n", jsffon);
	free(jsffon);


	free(newBlockchain.blocks);
}
