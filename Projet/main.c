#include "Block/block.h"
#include "Block/transactions.h"
#include "Block/blockchain.h"
#include "Hash/sha256.h"
#include <stdio.h>
#include <stdlib.h>
#include "Network/network.h"
#include "Network/server.h"
#include <signal.h>

typedef struct
{
	char *IP;
	char *firstserverIP;
	struct server *server;
}NETWORK;

void *NetworkThread(void *arg)
{
	NETWORK *network = arg;
	Network(network->server, network->IP, network->firstserverIP);

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

void *PrintMessage(void *arg)
{
	struct server *server = arg;
	
	while (server->status == ONLINE)
	{
		MESSAGE message = shared_queue_pop(server->IncomingMessages);
		if(message.type == 3)
		{
			BLOCKCHAIN bc = binToBlockchain((BYTE *)message.data);
			printBlockchain(bc);
			printf("La taille du message est de %llu\n", message.sizeData);
			free(bc.blocks);
		}
		else
			write(STDOUT_FILENO, message.data, message.sizeData);
		DestroyMessage(message);
	}
	
	return NULL;
}



int grosTest(int argc, char **argv)
{
	if (argc > 3)
		return -1;

	struct server *server = initServer();
	NETWORK network;
	network.server = server;
	network.IP = argv[1];
	network.firstserverIP = argv[2];
	pthread_t networkthread;
	pthread_t thread;
	pthread_create(&networkthread, NULL, NetworkThread, (void *)&network);
	pthread_create(&thread, NULL, PrintMessage, (void *)server);

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

	//char *data = "cing elit. Prcing elit. Proin eros eros, dictum et ligula sit amet, placerat placerat diam. Curabitur id urna quis tellus accumsan molestie. Ut elementum congue eleifend. Donec auctor purus vulputate tortor faucibus rutrum. Nam pretium blandit purus a vulputate. Duis dolor neque, blandit id neque quis, dictum interdum est. Nam sit amet varius orci, id sollicitudin odio. Mauris dignissim ultrices sem, in ullamcorper mi eleifend id.Suspendisse non ligula tortor. Fusce blandit odio eget urna iaculis euismod. Nulla sollicitudin rutrum imperdiet. Mauris laoreet convallis quam a feugiat. Nullam vitae facilisis nulla. Praesent quis ligula risus. Pellentesque vel velit ipsum. Duis convallis non mauris sit amet lacinia. Morbi non tellus ac felis tempus pellentesque. Donec ultrices lobortis enim in egestas. Cras non dapibus lectus, at eleifend neque. Morbi sit amet mi ipsum.Donec feugiat nisi sagittis ultrices scelerisque. Phasellus augue ligula, bibendum ut efficitur molestie, sodales at nisi. Donec placerat justo iaculis odio dapibus, in euismod neque interdum. Donec fringilla sem et eros ornare porttitor ac at risus. Sed maximus nec nunc at tempor. Aliquam non egestas lectus, et ullamcorper arcu. Orci varius natoque penatibus et magnis dis parturient montes, nascetur ridiculus mus. Vestibulum malesuada tempor tellus eu dictum. Integer ex nibh, ultricies et sapien ac, elementum mollis elit. Morbi sit amet mi feugiat, efficitur sapien et, tincidunt arcu. Integer eu nunc tincidunt nulla tincidunt vehicula. Nunc dui felis, mollis quis arcu at, fermentum vulputate dui. Quisque et tellus non elit gravida imperdiet. Aenean tortor velit, viverra sit amet rhoncus in, tincidunt vitae nisl.Pellentesque nec vehicula orci, egestas consequat ipsum. Phasellus blandit lorem sed quam malesuada bibendum. In ornare varius sagittis. Mauricing elit. Proin eros eros, dictum et ligula sit amet, placerat placerat diam. Curabitur id urna quis tellus accumsan molestie. Ut elementum congue eleifend. Donec auctor purus vulputate tortor faucibus rutrum. Nam pretium blandit purus a vulputate. Duis dolor neque, blandit id neque quis, dictum interdum est. Nam sit amet varius orci, id sollicitudin odio. Mauris dignissim ultrices sem, in ullamcorper mi eleifend id.Suspendisse non ligula tortor. Fusce blandit odio eget urna iaculis euismod. Nulla sollicitudin rutrum imperdiet. Mauris laoreet convallis quam a feugiat. Nullam vitae facilisis nulla. Praesent quis ligula risus. Pellentesque vel velit ipsum. Duis convallis non mauris sit amet lacinia. Morbi non tellus ac felis tempus pellentesque. Donec ultrices lobortis enim in egestas. Cras non dapibus lectus, at eleifend neque. Morbi sit amet mi ipsum.Donec feugiat nisi sagittis ultrices scelerisque. Phasellus augue ligula, bibendum ut efficitur molestie, sodales at nisi. Donec placerat justo iaculis odio dapibus, in euismod neque interdum. Donec fringilla sem et eros ornare porttitor ac at risus. Sed maximus nec nunc at tempor. Aliquam non egestas lectus, et ullamcorper arcu. Orci varius natoque penatibus et magnis dis parturient montes, nascetur ridiculus mus. Vestibulum malesuada tempor tellus eu dictum. Integer ex nibh, ultricies et sapien ac, elementum mollis elit. Morbi sit amet mi feugiat, efficitur sapien et, tincidunt arcu. Integer eu nunc tincidunt nulla tincidunt vehicula. Nunc dui felis, mollis quis arcu at, fermentum vulputate dui. Quisque et tellus non elit gravida imperdiet. Aenean tortor velit, viverra sit amet rhoncus in, tincidunt vitae nisl.Pellentesque nec vehicula orci, egestas consequat ipsum. Phasellus blandit lorem sed quam malesuada bibendum. In ornare varius sagittis. Mauris metus tortor, rutrum sit amet semper non, auctor et libero. Nam eleifend massa quis neque scelerisque vestibulum. Etiam velit urna, placerat vel aliquet ac, varius sed diam. Ut sit amet libero eu ligulacing elit. Proin eros eros, dictum et ligula sit amet, placerat placerat diam. Curabitur id urna quis tellus accumsan molestie. Ut elementum congue eleifend. Donec auctor purus vulputate tortor faucibus rutrum. Nam pretium blandit purus a vulputate. Duis dolor neque, blandit id neque quis, dictum interdum est. Nam sit amet varius orci, id sollicitudin odio. Mauris dignissim ultrices sem, in ullamcorper mi eleifend id.Suspendisse non ligula tortor. Fusce blandit odio eget urna iaculis euismod. Nulla sollicitudin rutrum imperdiet. Mauris laoreet convallis quam a feugiat. Nullam vitae facilisis nulla. Praesent quis ligula risus. Pellentesque vel velit ipsum. Duis convallis non mauris sit amet lacinia. Morbi non tellus ac felis tempus pellentesque. Donec ultrices lobortis enim in egestas. Cras non dapibus lectus, at eleifend neque. Morbi sit amet mi ipsum.Donec feugiat nisi sagittis ultrices scelerisque. Phasellus augue ligula, bibendum ut efficitur molestie, sodales at nisi. Donec placerat justo iaculis odio dapibus, in euismod neque interdum. Donec fringilla sem et eros ornare porttitor ac at risus. Sed maximus nec nunc at tempor. Aliquam non egestas lectus, et ullamcorper arcu. Orci varius natoque penatibus et magnis dis parturient montes, nascetur ridiculus mus. Vestibulum malesuada tempor tellus eu dictum. Integer ex nibh, ultricies et sapien ac, elementum mollis elit. Morbi sit amet mi feugiat, efficitur sapien et, tincidunt arcu. Integer eu nunc tincidunt nulla tincidunt vehicula. Nunc dui felis, mollis quis arcu at, fermentum vulputate dui. Quisque et tellus non elit gravida imperdiet. Aenean tortor velit, viverra sit amet rhoncus in, tincidunt vitae nisl.Pellentesque nec vehicula orci, egestas consequat ipsum. Phasellus blandit lorem sed quam malesuada bibendum. In ornare varius sagittis. Mauris metus tortor, rutrum sit amet semper non, auctor et libero. Nam eleifend massa quis neque scelerisque vestibulum. Etiam velit urna, placerat vel aliquet ac, varius sed diam. Ut sit amet libero eu ligulacing elit. Proin eros eros, dictum et ligula sit amet, placerat placerat diam. Curabitur id urna quis tellus accumsan molestie. Ut elementum congue eleifend. Donec auctor purus vulputate tortor faucibus rutrum. Nam pretium blandit purus a vulputate. Duis dolor neque, blandit id neque quis, dictum interdum est. Nam sit amet varius orci, id sollicitudin odio. Mauris dignissim ultrices sem, in ullamcorper mi eleifend id.Suspendisse non ligula tortor. Fusce blandit odio eget urna iaculis euismod. Nulla sollicitudin rutrum imperdiet. Mauris laoreet convallis quam a feugiat. Nullam vitae facilisis nulla. Praesent quis ligula risus. Pellentesque vel velit ipsum. Duis convallis non mauris sit amet lacinia. Morbi non tellus ac felis tempus pellentesque. Donec ultrices lobortis enim in egestas. Cras non dapibus lectus, at eleifend neque. Morbi sit amet mi ipsum.Donec feugiat nisi sagittis ultrices scelerisque. Phasellus augue ligula, bibendum ut efficitur molestie, sodales at nisi. Donec placerat justo iaculis odio dapibus, in euismod neque interdum. Donec fringilla sem et eros ornare porttitor ac at risus. Sed maximus nec nunc at tempor. Aliquam non egestas lectus, et ullamcorper arcu. Orci varius natoque penatibus et magnis dis parturient montes, nascetur ridiculus mus. Vestibulum malesuada tempor tellus eu dictum. Integer ex nibh, ultricies et sapien ac, elementum mollis elit. Morbi sit amet mi feugiat, efficitur sapien et, tincidunt arcu. Integer eu nunc tincidunt nulla tincidunt vehicula. Nunc dui felis, mollis quis arcu at, fermentum vulputate dui. Quisque et tellus non elit gravida imperdiet. Aenean tortor velit, viverra sit amet rhoncus in, tincidunt vitae nisl.Pellentesque nec vehicula orci, egestas consequat ipsum. Phasellus blandit lorem sed quam malesuada bibendum. In ornare varius sagittis. Mauris metus tortor, rutrum sit amet semper non, auctor et libero. Nam eleifend massa quis neque scelerisque vestibulum. Etiam velit urna, placerat vel aliquet ac, varius sed diam. Ut sit amet libero eu ligulas metus tortor, rutrum sit amet semper non, auctor et libero. Nam eleifend massa quis neque scelerisque vestibulum. Etiam velit urna, placerat vel aliquet ac, varius sed diam. Ut sit amet libero eu ligulaoin eros eros, dictum et ligula sit amet, placerat placerat cing elit. Proin eros eros, dictum et ligula sit amet, placerat placerat diam. Curabitur id urna quis tellus accumsan molestie. Ut elementum congue eleifend. Donec auctor purus vulputate tortor faucibus rutrum. Nam pretium blandit purus a vulputate. Duis dolor neque, blandit id neque quis, dictum interdum est. Nam sit amet varius orci, id sollicitudin odio. Mauris dignissim ultrices sem, in ullamcorper mi eleifend id.Suspendisse non ligula tortor. Fusce blandit odio eget urna iaculis euismod. Nulla sollicitudin rutrum imperdiet. Mauris laoreet convallis quam a feugiat. Nullam vitae facilisis nulla. Praesent quis ligula risus. Pellentesque vel velit ipsum. Duis convallis non mauris sit amet lacinia. Morbi non tellus ac felis tempus pellentesque. Donec ultrices lobortis enim in egestas. Cras non dapibus lectus, at eleifend neque. Morbi sit amet mi ipsum.Donec feugiat nisi sagittis ultrices scelerisque. Phasellus augue ligula, bibendum ut efficitur molestie, sodales at nisi. Donec placerat justo iaculis odio dapibus, in euismod neque interdum. Donec fringilla sem et eros ornare porttitor ac at risus. Sed maximus nec nunc at tempor. Aliquam non egestas lectus, et ullamcorper arcu. Orci varius natoque penatibus et magnis dis parturient montes, nascetur ridiculus mus. Vestibulum malesuada tempor tellus eu dictum. Integer ex nibh, ultricies et sapien ac, elementum mollis elit. Morbi sit amet mi feugiat, efficitur sapien et, tincidunt arcu. Integer eu nunc tincidunt nulla tincidunt vehicula. Nunc dui felis, mollis quis arcu at, fermentum vulputate dui. Quisque et tellus non elit gravida imperdiet. Aenean tortor velit, viverra sit amet rhoncus in, tincidunt vitae nisl.Pellentesque nec vehicula orci, egestas consequat ipsum. Phasellus blandit lorem sed quam malesuada bibendum. In ornare varius sagittis. Mauris metus tortor, rutrum sit amet semper non, auctor et libero. Nam eleifend massa quis neque scelerisque vestibulum. Etiam velit urna, placerat vel aliquet ac, varius sed diam. Ut sit amet libero eu liguladiam. Curabitur id urna quis tellus accumsan molestie. Ut elementum congue eleifend. Donec auctor purus vulputate tortor faucibus rutrum. Nam pretium blandit purus a vulputate. Duis dolor neque, blandit id neque quis, dictum interdum est. Nam sit amet varius orci, id sollicitudin odio. Mauris dignissim ultrices sem, in ullamcorper mi eleifend id.Suspendisse non ligula tortor. Fusce blandit odio eget urna iaculis euismod. Nulla sollicitudin rutrum imperdiet. Mauris laoreet convallis quam a feugiat. Nullam vitae facilisis nulla. Praesent quis ligula risus. Pellentesque vel velit ipsum. Duis convallis non mauris sit amet lacinia. Morbi non tellus ac felis tempus pellentesque. Donec ultrices lobortis enim in egestas. Cras non dapibus lectus, at eleifend neque. Morbi sit amet mi ipsum.Donec feugiat nisi sagittis ultrices scelerisque. Phasellus augue ligula, bibendum ut efficitur molestie, sodales at nisi. Donec placerat justo iaculis odio dapibus, in euismod neque interdum. Donec fringilla sem et eros ornare porttitor ac at risus. Sed maximus nec nunc at tempor. Aliquam non egestas lectus, et ullamcorper arcu. Orci varius natoque penatibus et magnis dis parturient montes, nascetur ridiculus mus. Vestibulum malesuada tempor tellus eu dictum. Integer ex nibh, ultricies et sapien ac, elementum mollis elit. Morbi sit amet mi feugiat, efficitur sapien et, tincidunt arcu. Integer eu nunc tincidunt nulla tincidunt vehicula. Nunc dui felis, mollis quis arcu at, fermentum vulputate dui. Quisque et tellus non elit gravida imperdiet. Aenean tortor velit, viverra sit amet rhoncus in, tincidunt vitae nisl.Pellentesque nec vehicula orci, egestas consequat ipsum. Phasellus blandit lorem sed quam malesuada bibendum. In ornare varius sagittis. Mauris metus tortor, rutrum sit amet semper non, auctor et libero. Nam eleifend massa quis neque scelerisque vestibulum. Etiam velit urna, placerat vel aliquet ac, varius sed diam. Ut sit amet libero eu ligula.\n";
	
	
	char *datatest = "Je suis la pute de Pierre\n";
	int type = 2;
	
	char *data3 = blockchainToJson(&newBlockchain);

	MESSAGE message = CreateMessage(1, strlen(data3), data3);
	shared_queue_push(server->OutgoingMessages, message);
	while (1)
	{
		sleep(1);
		MESSAGE message = CreateMessage(3, strlen(data3), data3);
		shared_queue_push(server->OutgoingMessages, message);
	}

	free(data3);
	free(bcbin.bin);
	free(newBlockchain.blocks);
	pthread_join(networkthread, NULL);
	pthread_join(thread, NULL);
	freeServer(network.server);
}


int main(int argc, char **argv)
{
	grosTest(argc, argv);
}
