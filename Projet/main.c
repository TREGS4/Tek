#include "Block/block.h"
#include "Block/transactions.h"
#include "Block/blockchain.h"
#include "Hash/sha256.h"
#include <stdio.h>
#include <stdlib.h>
#include "Network/network.h"
#include "Network/server.h"
#include <signal.h>


struct test
{
	int fd[2];
	char *IP;
	char *firstserver;
};

void *moncul(void *arg)
{
	struct test *tst = arg;

	network(&tst->fd[0], &tst->fd[1], tst->IP, tst->firstserver);

	return NULL;
}

void *mabite(void *arg)
{
	struct test *tst = arg;
	int fdin = tst->fd[0];
    unsigned long long sizeData = 0;
    int type = 0;
    char headerBuff[HEADER_SIZE];
    char *messageBuff;
    int problem;
    size_t nbCharToRead;
    size_t offset;
    int r;

    //peut y avoir un souci si la taille de data depasse la taille du buffer du file descriptor
    //comportement inconnu dans ce cas la

    while (1)
    {
		fdin = tst->fd[0];
        offset = 0;
        r = 0;
        nbCharToRead = HEADER_SIZE;
        problem = 0;

        while (problem == 0 && nbCharToRead)
        {
            r = read(fdin, headerBuff + offset, nbCharToRead);
            nbCharToRead -= r;
            if (r <= 0)
                problem = 1;
        }

        memcpy(&type, headerBuff, SIZE_TYPE_MSG);
        memcpy(&sizeData, headerBuff + SIZE_TYPE_MSG, SIZE_DATA_LEN_HEADER);
        messageBuff = malloc(sizeof(char) * (HEADER_SIZE + sizeData));
        memcpy(messageBuff, headerBuff, HEADER_SIZE);

        nbCharToRead = sizeData;

        while (problem == 0 && nbCharToRead)
        {
            r = read(fdin, messageBuff + offset, nbCharToRead);
            nbCharToRead -= r;

            if (r > 0)
                offset += r;
            else
                problem = 1;
        }

        if (problem == 0)
            printData(type, sizeData, messageBuff);
        else
            printf("Error while receinving data in Main\nError with function read or not enough bytes received\n");

        free(messageBuff);
    }

    return NULL;

	return NULL;
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

int grosTest(int argc, char **argv)
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
	//pthread_create(&readfd, NULL, mabite, (void *)&tst);

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
	//printf("Number of blocks in the blockchain : %lu.\n\n", newBlockchain.blocksNumber);
	//printBlockchain(newBlockchain);
	//printf("\n");

	/*BLOCKCHAIN*/
	//printf("\n\n");

	BLOCKCHAIN_BIN bcbin = blockchainToBin(&newBlockchain);

	for (size_t i = 0; i < bcbin.nbBytes; i++)
	{
		if (i % 20 == 0)
		{
			//printf("\n");
		}
		//printf("%02x ", bcbin.bin[i]);
	}
	//printf("\n\n");

	char *data = "cing elit. Proin eros eros, dictum et ligula sit amet, placerat placerat diam. Curabitur id urna quis tellus accumsan molestie. Ut elementum congue eleifend. Donec auctor purus vulputate tortor faucibus rutrum. Nam pretium blandit purus a vulputate. Duis dolor neque, blandit id neque quis, dictum interdum est. Nam sit amet varius orci, id sollicitudin odio. Mauris dignissim ultrices sem, in ullamcorper mi eleifend id.Suspendisse non ligula tortor. Fusce blandit odio eget urna iaculis euismod. Nulla sollicitudin rutrum imperdiet. Mauris laoreet convallis quam a feugiat. Nullam vitae facilisis nulla. Praesent quis ligula risus. Pellentesque vel velit ipsum. Duis convallis non mauris sit amet lacinia. Morbi non tellus ac felis tempus pellentesque. Donec ultrices lobortis enim in egestas. Cras non dapibus lectus, at eleifend neque. Morbi sit amet mi ipsum.Donec feugiat nisi sagittis ultrices scelerisque. Phasellus augue ligula, bibendum ut efficitur molestie, sodales at nisi. Donec placerat justo iaculis odio dapibus, in euismod neque interdum. Donec fringilla sem et eros ornare porttitor ac at risus. Sed maximus nec nunc at tempor. Aliquam non egestas lectus, et ullamcorper arcu. Orci varius natoque penatibus et magnis dis parturient montes, nascetur ridiculus mus. Vestibulum malesuada tempor tellus eu dictum. Integer ex nibh, ultricies et sapien ac, elementum mollis elit. Morbi sit amet mi feugiat, efficitur sapien et, tincidunt arcu. Integer eu nunc tincidunt nulla tincidunt vehicula. Nunc dui felis, mollis quis arcu at, fermentum vulputate dui. Quisque et tellus non elit gravida imperdiet. Aenean tortor velit, viverra sit amet rhoncus in, tincidunt vitae nisl.Pellentesque nec vehicula orci, egestas consequat ipsum. Phasellus blandit lorem sed quam malesuada bibendum. In ornare varius sagittis. Mauris metus tortor, rutrum sit amet semper non, auctor et libero. Nam eleifend massa quis neque scelerisque vestibulum. Etiam velit urna, placerat vel aliquet ac, varius sed diam. Ut sit amet libero eu ligula.\n";
	int type = 2;
	unsigned long long len = strlen(data);
	char buff[HEADER_SIZE + len];
	
	
	memcpy(buff, &type, SIZE_TYPE_MSG);
	memcpy(buff + SIZE_TYPE_MSG, &len, SIZE_DATA_LEN_HEADER);
	memcpy(buff + HEADER_SIZE, data, len);

	while (1)
	{
		sleep(1);
		printData(type, len + HEADER_SIZE, buff);
		write(tst.fd[1], buff, HEADER_SIZE + len);
		//SendMessage("Hello world !\n", tst.fd[1], 15, 2);
		//SendMessage((char *)bcbin.bin, tst.fd[1], (unsigned long long)bcbin.nbBytes, 2);
		//printf("Message send\n");
		BLOCKCHAIN bc = binToBlockchain(bcbin.bin);
		free(bc.blocks);
	}


	free(bcbin.bin);

	free(newBlockchain.blocks);
	pthread_join(thread, NULL);
	pthread_join(readfd, NULL);
}


int main(int argc, char **argv)
{
	grosTest(argc, argv);
}
