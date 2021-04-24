#ifndef SERVER_H
#define SERVER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <unistd.h>
#include <err.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pthread.h>
#include <sys/socket.h>

#include "network.h"

#define PORT "6969"
#define BUFFER_SIZE_SOCKET 512
#define SIZE_ULONGLONG 19
#define SIZE_TYPE_MSG 3




void *server(void *arg);
void *connectionMaintener(void *arg);
int connectClient(struct sockaddr *infoClient, struct clientInfo *list);

#endif