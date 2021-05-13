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
#include <arpa/inet.h>

#define PORT "6969"


// Message part
#define BUFFER_SIZE_SOCKET 512
#define SIZE_DATA_LEN_HEADER 8
#define SIZE_TYPE_MSG 1
#define HEADER_SIZE SIZE_DATA_LEN_HEADER + SIZE_TYPE_MSG

//Status part
#define ERROR -1
#define ENDED 0
#define CONNECTED 1
#define CONNECTING 3
#define NOTCONNECTED 4
#define SENTINEL 5
#define ONLINE 6
#define OFFLINE 7
#define EXITING 8
#define NOTUSED 255
#define DEAD 254

struct clientInfo
{
    size_t ID;
    struct sockaddr_in IPandPort;
    socklen_t IPLen;

    pthread_mutex_t lockInfo;              //lock when modifing everything except file descriptors
    pthread_mutex_t lockWrite;             //lock when modifing clientSocket
    pthread_mutex_t lockRead;              //lock when modifing fdInThread or fdTofdin
    pthread_mutex_t *lockReadGlobalExtern; //lock when writing on fdoutExtern, we need send the whole message before an other thread can write
    pthread_mutex_t *lockReadGlobalIntern; //lock when writing on fdoutIntern, we need send the whole message before an other thread can write

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
    pthread_mutex_t mutexfdtemp;

    struct sockaddr_in IPandPort;
    socklen_t IPLen;

    struct clientInfo *listClients;
};

void *server(void *arg);

#endif