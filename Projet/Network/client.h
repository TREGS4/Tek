#ifndef CLIENT_H
#define CLIENT_H

#include "server.h"
#include "network_tools.h"


int SendMessage(struct clientInfo *client, char *message);
int addClient(struct sockaddr_in IP, struct clientInfo *clientList);

#endif
