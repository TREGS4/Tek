#ifndef SERVER_H
#define SERVER_H

#include "network_tools.h"

struct connection
{
    int socket;
    struct server *server;
};

void *Server(void *arg);
void printData(int type, unsigned long long len, char *buff);

#endif