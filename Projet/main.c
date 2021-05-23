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
	char *port;
	char *firstserverIP;
	char *portFirstServer;
	struct server *server;
} NETWORK;

void *NetworkThread(void *arg)
{
	NETWORK *network = arg;
	Network(network->server, network->IP, network->port, network->firstserverIP, network->portFirstServer);

	return NULL;
}

void *PrintMessage(void *arg)
{
	struct server *server = arg;

	while (server->status != EXITING)
	{
		MESSAGE *message = shared_queue_pop(server->IncomingMessages);
		write(STDOUT_FILENO, message->data, message->sizeData);
		DestroyMessage(message);
	}

	return NULL;
}

int grosTest(int argc, char **argv)
{
	if (argc > 5 || argc < 2)
	{
		printf("Too few or to many arguments\n");
		return -1;
	}

	struct server *server = initServer();
	NETWORK network;
	network.server = server;
	network.IP = argv[1];
	if (argc > 2)
		network.port = argv[2];
	else
		network.port = NULL;
	if (argc > 3)
		network.firstserverIP = argv[3];
	else
		network.firstserverIP = NULL;
	if (argc > 4)
		network.portFirstServer = argv[4];
	else
		network.portFirstServer = NULL;
	pthread_t networkthread;
	//pthread_t thread;
	pthread_create(&networkthread, NULL, NetworkThread, (void *)&network);
	//pthread_create(&thread, NULL, PrintMessage, (void *)server);

	//char *data = "cing elit. Prcing elit. Proin eros eros, dictum et ligula sit amet, placerat placerat diam. Curabitur id urna quis tellus accumsan molestie. Ut elementum congue eleifend. Donec auctor purus vulputate tortor faucibus rutrum. Nam pretium blandit purus a vulputate. Duis dolor neque, blandit id neque quis, dictum interdum est. Nam sit amet varius orci, id sollicitudin odio. Mauris dignissim ultrices sem, in ullamcorper mi eleifend id.Suspendisse non ligula tortor. Fusce blandit odio eget urna iaculis euismod. Nulla sollicitudin rutrum imperdiet. Mauris laoreet convallis quam a feugiat. Nullam vitae facilisis nulla. Praesent quis ligula risus. Pellentesque vel velit ipsum. Duis convallis non mauris sit amet lacinia. Morbi non tellus ac felis tempus pellentesque. Donec ultrices lobortis enim in egestas. Cras non dapibus lectus, at eleifend neque. Morbi sit amet mi ipsum.Donec feugiat nisi sagittis ultrices scelerisque. Phasellus augue ligula, bibendum ut efficitur molestie, sodales at nisi. Donec placerat justo iaculis odio dapibus, in euismod neque interdum. Donec fringilla sem et eros ornare porttitor ac at risus. Sed maximus nec nunc at tempor. Aliquam non egestas lectus, et ullamcorper arcu. Orci varius natoque penatibus et magnis dis parturient montes, nascetur ridiculus mus. Vestibulum malesuada tempor tellus eu dictum. Integer ex nibh, ultricies et sapien ac, elementum mollis elit. Morbi sit amet mi feugiat, efficitur sapien et, tincidunt arcu. Integer eu nunc tincidunt nulla tincidunt vehicula. Nunc dui felis, mollis quis arcu at, fermentum vulputate dui. Quisque et tellus non elit gravida imperdiet. Aenean tortor velit, viverra sit amet rhoncus in, tincidunt vitae nisl.Pellentesque nec vehicula orci, egestas consequat ipsum. Phasellus blandit lorem sed quam malesuada bibendum. In ornare varius sagittis. Mauricing elit. Proin eros eros, dictum et ligula sit amet, placerat placerat diam. Curabitur id urna quis tellus accumsan molestie. Ut elementum congue eleifend. Donec auctor purus vulputate tortor faucibus rutrum. Nam pretium blandit purus a vulputate. Duis dolor neque, blandit id neque quis, dictum interdum est. Nam sit amet varius orci, id sollicitudin odio. Mauris dignissim ultrices sem, in ullamcorper mi eleifend id.Suspendisse non ligula tortor. Fusce blandit odio eget urna iaculis euismod. Nulla sollicitudin rutrum imperdiet. Mauris laoreet convallis quam a feugiat. Nullam vitae facilisis nulla. Praesent quis ligula risus. Pellentesque vel velit ipsum. Duis convallis non mauris sit amet lacinia. Morbi non tellus ac felis tempus pellentesque. Donec ultrices lobortis enim in egestas. Cras non dapibus lectus, at eleifend neque. Morbi sit amet mi ipsum.Donec feugiat nisi sagittis ultrices scelerisque. Phasellus augue ligula, bibendum ut efficitur molestie, sodales at nisi. Donec placerat justo iaculis odio dapibus, in euismod neque interdum. Donec fringilla sem et eros ornare porttitor ac at risus. Sed maximus nec nunc at tempor. Aliquam non egestas lectus, et ullamcorper arcu. Orci varius natoque penatibus et magnis dis parturient montes, nascetur ridiculus mus. Vestibulum malesuada tempor tellus eu dictum. Integer ex nibh, ultricies et sapien ac, elementum mollis elit. Morbi sit amet mi feugiat, efficitur sapien et, tincidunt arcu. Integer eu nunc tincidunt nulla tincidunt vehicula. Nunc dui felis, mollis quis arcu at, fermentum vulputate dui. Quisque et tellus non elit gravida imperdiet. Aenean tortor velit, viverra sit amet rhoncus in, tincidunt vitae nisl.Pellentesque nec vehicula orci, egestas consequat ipsum. Phasellus blandit lorem sed quam malesuada bibendum. In ornare varius sagittis. Mauris metus tortor, rutrum sit amet semper non, auctor et libero. Nam eleifend massa quis neque scelerisque vestibulum. Etiam velit urna, placerat vel aliquet ac, varius sed diam. Ut sit amet libero eu ligulacing elit. Proin eros eros, dictum et ligula sit amet, placerat placerat diam. Curabitur id urna quis tellus accumsan molestie. Ut elementum congue eleifend. Donec auctor purus vulputate tortor faucibus rutrum. Nam pretium blandit purus a vulputate. Duis dolor neque, blandit id neque quis, dictum interdum est. Nam sit amet varius orci, id sollicitudin odio. Mauris dignissim ultrices sem, in ullamcorper mi eleifend id.Suspendisse non ligula tortor. Fusce blandit odio eget urna iaculis euismod. Nulla sollicitudin rutrum imperdiet. Mauris laoreet convallis quam a feugiat. Nullam vitae facilisis nulla. Praesent quis ligula risus. Pellentesque vel velit ipsum. Duis convallis non mauris sit amet lacinia. Morbi non tellus ac felis tempus pellentesque. Donec ultrices lobortis enim in egestas. Cras non dapibus lectus, at eleifend neque. Morbi sit amet mi ipsum.Donec feugiat nisi sagittis ultrices scelerisque. Phasellus augue ligula, bibendum ut efficitur molestie, sodales at nisi. Donec placerat justo iaculis odio dapibus, in euismod neque interdum. Donec fringilla sem et eros ornare porttitor ac at risus. Sed maximus nec nunc at tempor. Aliquam non egestas lectus, et ullamcorper arcu. Orci varius natoque penatibus et magnis dis parturient montes, nascetur ridiculus mus. Vestibulum malesuada tempor tellus eu dictum. Integer ex nibh, ultricies et sapien ac, elementum mollis elit. Morbi sit amet mi feugiat, efficitur sapien et, tincidunt arcu. Integer eu nunc tincidunt nulla tincidunt vehicula. Nunc dui felis, mollis quis arcu at, fermentum vulputate dui. Quisque et tellus non elit gravida imperdiet. Aenean tortor velit, viverra sit amet rhoncus in, tincidunt vitae nisl.Pellentesque nec vehicula orci, egestas consequat ipsum. Phasellus blandit lorem sed quam malesuada bibendum. In ornare varius sagittis. Mauris metus tortor, rutrum sit amet semper non, auctor et libero. Nam eleifend massa quis neque scelerisque vestibulum. Etiam velit urna, placerat vel aliquet ac, varius sed diam. Ut sit amet libero eu ligulacing elit. Proin eros eros, dictum et ligula sit amet, placerat placerat diam. Curabitur id urna quis tellus accumsan molestie. Ut elementum congue eleifend. Donec auctor purus vulputate tortor faucibus rutrum. Nam pretium blandit purus a vulputate. Duis dolor neque, blandit id neque quis, dictum interdum est. Nam sit amet varius orci, id sollicitudin odio. Mauris dignissim ultrices sem, in ullamcorper mi eleifend id.Suspendisse non ligula tortor. Fusce blandit odio eget urna iaculis euismod. Nulla sollicitudin rutrum imperdiet. Mauris laoreet convallis quam a feugiat. Nullam vitae facilisis nulla. Praesent quis ligula risus. Pellentesque vel velit ipsum. Duis convallis non mauris sit amet lacinia. Morbi non tellus ac felis tempus pellentesque. Donec ultrices lobortis enim in egestas. Cras non dapibus lectus, at eleifend neque. Morbi sit amet mi ipsum.Donec feugiat nisi sagittis ultrices scelerisque. Phasellus augue ligula, bibendum ut efficitur molestie, sodales at nisi. Donec placerat justo iaculis odio dapibus, in euismod neque interdum. Donec fringilla sem et eros ornare porttitor ac at risus. Sed maximus nec nunc at tempor. Aliquam non egestas lectus, et ullamcorper arcu. Orci varius natoque penatibus et magnis dis parturient montes, nascetur ridiculus mus. Vestibulum malesuada tempor tellus eu dictum. Integer ex nibh, ultricies et sapien ac, elementum mollis elit. Morbi sit amet mi feugiat, efficitur sapien et, tincidunt arcu. Integer eu nunc tincidunt nulla tincidunt vehicula. Nunc dui felis, mollis quis arcu at, fermentum vulputate dui. Quisque et tellus non elit gravida imperdiet. Aenean tortor velit, viverra sit amet rhoncus in, tincidunt vitae nisl.Pellentesque nec vehicula orci, egestas consequat ipsum. Phasellus blandit lorem sed quam malesuada bibendum. In ornare varius sagittis. Mauris metus tortor, rutrum sit amet semper non, auctor et libero. Nam eleifend massa quis neque scelerisque vestibulum. Etiam velit urna, placerat vel aliquet ac, varius sed diam. Ut sit amet libero eu ligulas metus tortor, rutrum sit amet semper non, auctor et libero. Nam eleifend massa quis neque scelerisque vestibulum. Etiam velit urna, placerat vel aliquet ac, varius sed diam. Ut sit amet libero eu ligulaoin eros eros, dictum et ligula sit amet, placerat placerat cing elit. Proin eros eros, dictum et ligula sit amet, placerat placerat diam. Curabitur id urna quis tellus accumsan molestie. Ut elementum congue eleifend. Donec auctor purus vulputate tortor faucibus rutrum. Nam pretium blandit purus a vulputate. Duis dolor neque, blandit id neque quis, dictum interdum est. Nam sit amet varius orci, id sollicitudin odio. Mauris dignissim ultrices sem, in ullamcorper mi eleifend id.Suspendisse non ligula tortor. Fusce blandit odio eget urna iaculis euismod. Nulla sollicitudin rutrum imperdiet. Mauris laoreet convallis quam a feugiat. Nullam vitae facilisis nulla. Praesent quis ligula risus. Pellentesque vel velit ipsum. Duis convallis non mauris sit amet lacinia. Morbi non tellus ac felis tempus pellentesque. Donec ultrices lobortis enim in egestas. Cras non dapibus lectus, at eleifend neque. Morbi sit amet mi ipsum.Donec feugiat nisi sagittis ultrices scelerisque. Phasellus augue ligula, bibendum ut efficitur molestie, sodales at nisi. Donec placerat justo iaculis odio dapibus, in euismod neque interdum. Donec fringilla sem et eros ornare porttitor ac at risus. Sed maximus nec nunc at tempor. Aliquam non egestas lectus, et ullamcorper arcu. Orci varius natoque penatibus et magnis dis parturient montes, nascetur ridiculus mus. Vestibulum malesuada tempor tellus eu dictum. Integer ex nibh, ultricies et sapien ac, elementum mollis elit. Morbi sit amet mi feugiat, efficitur sapien et, tincidunt arcu. Integer eu nunc tincidunt nulla tincidunt vehicula. Nunc dui felis, mollis quis arcu at, fermentum vulputate dui. Quisque et tellus non elit gravida imperdiet. Aenean tortor velit, viverra sit amet rhoncus in, tincidunt vitae nisl.Pellentesque nec vehicula orci, egestas consequat ipsum. Phasellus blandit lorem sed quam malesuada bibendum. In ornare varius sagittis. Mauris metus tortor, rutrum sit amet semper non, auctor et libero. Nam eleifend massa quis neque scelerisque vestibulum. Etiam velit urna, placerat vel aliquet ac, varius sed diam. Ut sit amet libero eu liguladiam. Curabitur id urna quis tellus accumsan molestie. Ut elementum congue eleifend. Donec auctor purus vulputate tortor faucibus rutrum. Nam pretium blandit purus a vulputate. Duis dolor neque, blandit id neque quis, dictum interdum est. Nam sit amet varius orci, id sollicitudin odio. Mauris dignissim ultrices sem, in ullamcorper mi eleifend id.Suspendisse non ligula tortor. Fusce blandit odio eget urna iaculis euismod. Nulla sollicitudin rutrum imperdiet. Mauris laoreet convallis quam a feugiat. Nullam vitae facilisis nulla. Praesent quis ligula risus. Pellentesque vel velit ipsum. Duis convallis non mauris sit amet lacinia. Morbi non tellus ac felis tempus pellentesque. Donec ultrices lobortis enim in egestas. Cras non dapibus lectus, at eleifend neque. Morbi sit amet mi ipsum.Donec feugiat nisi sagittis ultrices scelerisque. Phasellus augue ligula, bibendum ut efficitur molestie, sodales at nisi. Donec placerat justo iaculis odio dapibus, in euismod neque interdum. Donec fringilla sem et eros ornare porttitor ac at risus. Sed maximus nec nunc at tempor. Aliquam non egestas lectus, et ullamcorper arcu. Orci varius natoque penatibus et magnis dis parturient montes, nascetur ridiculus mus. Vestibulum malesuada tempor tellus eu dictum. Integer ex nibh, ultricies et sapien ac, elementum mollis elit. Morbi sit amet mi feugiat, efficitur sapien et, tincidunt arcu. Integer eu nunc tincidunt nulla tincidunt vehicula. Nunc dui felis, mollis quis arcu at, fermentum vulputate dui. Quisque et tellus non elit gravida imperdiet. Aenean tortor velit, viverra sit amet rhoncus in, tincidunt vitae nisl.Pellentesque nec vehicula orci, egestas consequat ipsum. Phasellus blandit lorem sed quam malesuada bibendum. In ornare varius sagittis. Mauris metus tortor, rutrum sit amet semper non, auctor et libero. Nam eleifend massa quis neque scelerisque vestibulum. Etiam velit urna, placerat vel aliquet ac, varius sed diam. Ut sit amet libero eu ligula.\n";

	//char *datatest = "Je suis la pute de Pierre\n";

	while (0)
	{
		//MESSAGE message = CreateMessage(2, strlen(datatest), datatest);
		//shared_queue_push(server->OutgoingMessages, message);
		sleep(1);
	}

	pthread_join(networkthread, NULL);
	//pthread_join(thread, NULL);
	freeServer(network.server);

	return EXIT_SUCCESS;
}

int main(int argc, char **argv)
{
	grosTest(argc, argv);
}
