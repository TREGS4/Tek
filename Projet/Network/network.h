#ifndef NETWORK_H
#define NETWORK_H

#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>

#include "server.h"
#include "network_tools.h"

int Network(struct server *server, char *IP, char *firstserver);
struct server *initServer();
void freeServer(struct server *server);

/*
Potentiels problèmes / choses à régler:

- etre sur que tous les clients soient bien fini avant de les free, actuellement ca ne vérifie rien
- idem pour le serveur, mais je pense que ca pose moins de soucis


*/


#endif
