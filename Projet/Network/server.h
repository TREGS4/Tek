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

#define PORT "6969"
#define BUFFER_SIZE_SOCKET 512



void * server(void *arg);
void * connectionMaintener(void *arg);

#endif
