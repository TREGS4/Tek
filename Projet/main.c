#include "Block/block.h"
#include "Block/transactions.h"
#include "Block/blockchain.h"
#include "Hash/sha256.h"
#include "Noeud_de_gestion/gestion.h"
#include <stdio.h>
#include <stdlib.h>
#include "API/API.h"
#include "Network/network.h"

int main(int argc, char **argv)
{
	if (argc > 5)
		return 0;

	struct server *server = initServer();

	if (server != NULL)
	{
		Network(server, argv[1], argv[2], argv[3], argv[4]);
		free(server);
	}

	return 0;
}