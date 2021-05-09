#include "Block/block.h"
#include "Block/transactions.h"
#include "Block/blockchain.h"
#include "Hash/sha256.h"
#include <stdio.h>
#include <stdlib.h>
#include "Network/network.h"
#include "Network/server.h"

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

void *mabite(void *arg)
{
	struct test *tst = arg;
	while (tst->fd[0] == -1)
	{
		;
	}
	
	int fdin = tst->fd[0]; 
	char buffLen[SIZE_DATA_LEN_HEADER + SIZE_TYPE_MSG + 1];
	char buffType[SIZE_TYPE_MSG + 1];
	char *buff;
	int r = 1;
	int type;
	unsigned long long size;

	while (1)
	{

		memset(buffLen, 0, SIZE_DATA_LEN_HEADER + 1);
		memset(buffType, 0, SIZE_TYPE_MSG + 1);

		size_t nbToRead = SIZE_DATA_LEN_HEADER + SIZE_TYPE_MSG;
		size_t nbchr = 0;

		/*Header part*/

		while (nbToRead > 0)
		{
			r = read(fdin, &buffLen + nbchr, nbToRead);
			nbToRead -= r;
			nbchr += r;
		}

		for (size_t i = 0; i < SIZE_TYPE_MSG; i++)
			buffType[i] = buffLen[i];

		type = buffLen[0];
		memcpy(&size, &buffLen[SIZE_TYPE_MSG], 8);
		buff = malloc(sizeof(char) * size);
		nbToRead = size;
		nbchr = 0;

		while (nbToRead > 0)
		{
			r = read(fdin, &buff + nbchr, nbToRead);
			nbToRead -= r;
			nbchr += r;
		}

		for (size_t i = 0; i < size; i++)
		{
			if (i % 20 == 0)
			{
				printf("\n");
			}
			printf("%02x ", buff[i]);
		}
		printf("\n\n");
	}
}

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

	for (size_t i = 1; i < blockchain.blocksNumber; i++)
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

int main(int argc, char **argv)
{
	if (argc > 3)
		return -1;

	struct test tst;
	tst.fd[1] = -1;
	tst.fd[0] = -1;
	tst.IP = argv[1];
	tst.firstserver = argv[2];
	pthread_t thread;
	pthread_t readfd;
	pthread_create(&thread, NULL, moncul, (void *)&tst);
	pthread_create(&readfd, NULL, mabite, (void *)&tst);

	TRANSACTION t =
		{
			.sender = "Adrien",
			.receiver = "Paul",
			.amount = 104};

	TRANSACTION t2 =
		{
			.sender = "Thimot",
			.receiver = "Margaux",
			.amount = 358};

	TRANSACTION t3 =
		{
			.sender = "Pierre",
			.receiver = "Hugo",
			.amount = 226};

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

	/*BLOCKCHAIN*/
	printf("\n\n");

	BLOCKCHAIN_BIN bcbin = blockchainToBin(&newBlockchain);

	

	for (size_t i = 0; i < bcbin.nbBytes; i++)
	{
		if (i % 20 == 0)
		{
			printf("\n");
		}
		printf("%02x ", bcbin.bin[i]);
	}
	printf("\n\n");

	sleep(10);
	pthread_mutex_lock(&tst.mutext);
	//SendMessage("Hello world !\n", tst.fd[1], 15, 2);
	//SendMessage((char *)bcbin.bin, tst.fd[1], (unsigned long long)bcbin.nbBytes, 2);
	printf("Message send\n");
	pthread_mutex_unlock(&tst.mutext);
	BLOCKCHAIN bc = binToBlockchain(bcbin.bin);

	free(bcbin.bin);
	free(bc.blocks);

	free(newBlockchain.blocks);
	pthread_join(thread, NULL);
	pthread_join(readfd, NULL);
}
