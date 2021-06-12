#ifndef SERVER_H
#define SERVER_H

#include <fcntl.h>

#include "network_tools.h"

struct connection
{
    int socket;
    struct sockaddr_in IP;
    struct server *server;
};

void *Server(void *arg);

#endif