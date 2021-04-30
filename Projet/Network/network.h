#ifndef NETWORK_H
#define NETWORK_H

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
#include <semaphore.h>
#include <arpa/inet.h>


#define ERROR -1
#define ENDED 0
#define CONNECTED 1
#define CONNECTING 3
#define SENTINEL 4
#define ONLINE 5
#define OFFLINE 6
#define EXITING 7
#define NOTUSED 255
#define DEAD 254




struct clientInfo
{
    size_t ID;
    struct sockaddr_in IPandPort;
    socklen_t IPLen;

    pthread_mutex_t lockInfo;                       //lock when modifing everything except file descriptors
    pthread_mutex_t lockWrite;                      //lock when modifing clientSocket
    pthread_mutex_t lockRead;                       //lock when modifing fdInThread or fdTofdin
    pthread_mutex_t *lockReadGlobalExtern;          //lock when writing on fdoutExtern, we need send the whole message before an other thread can write
    pthread_mutex_t *lockReadGlobalIntern;          //lock when writing on fdoutIntern, we need send the whole message before an other thread can write

    int clientSocket;
    int fdTofdin;
    int fdinThread;
    int fdoutExtern;
    int fdoutIntern;

    int status;

    pthread_t clientThread;
    pthread_t readThread;
    pthread_t writeThread;

    struct clientInfo *next;
    struct clientInfo *prev;
    struct clientInfo *sentinel;
    struct serverInfo *server;
};

struct serverInfo
{
    int status;

    int fdInInternComm;
    int fdtemp;

    pthread_mutex_t lockinfo;

    struct sockaddr IPandPort;
    socklen_t IPLen;

    struct clientInfo *listClients;
};


void printIP(struct sockaddr_in *IP);
int network(int *fdin, int *fdout, char *IP, char *firstserver);
struct clientInfo *initClient(struct clientInfo *clients);
struct clientInfo *last(struct clientInfo *client);
size_t listLen(struct clientInfo *client);
int removeClient(struct clientInfo *client);
int isInList(struct sockaddr_in *tab, struct clientInfo *list);
int itsme(struct sockaddr_in *first, struct sockaddr_in *second);


//Here because server.h needs the declaration of the struct clientInfo
#include "client.h"
#include "server.h"
#include "../Hash/sha256.h"


#endif
