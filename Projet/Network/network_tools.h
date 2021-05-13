#ifndef NETWORK_TOOLS_H
#define NETWORK_TOOLS_H

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

#include "informations.h"

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
    pthread_mutex_t mutextmessage;

    struct sockaddr_in IPandPort;
    socklen_t IPLen;

    struct clientInfo *listClients;
};

//Create a new list of clientInfo, return the sentinel of the list
//fdin is the read side of the pipe to receive message from the management node
//fdoutExtern is the write side of the pipe to send message to the management node
//fdoutIntern is the write side of the pipe to network (internal ones) communications
struct clientInfo *initClientList(int fdin, int fdoutExtern, int fdoutIntern);

//Remove all the client and free the list create with initClientList
void freeClientList(struct clientInfo *clientList);

//Create a new client, return it's pointer and add it to the list, the list must not NULL
//Set everything correctly except the socket's file descriptor (set to -1)
//The status of this new client is set to NOTUSED
struct clientInfo *initClient(struct clientInfo *clients);

//Remove the client from the list
int removeClient(struct clientInfo *client);

//Return the size of the list (sentinel not included)
size_t listLen(struct clientInfo *client);

//Return a pointer to the last client of the list
struct clientInfo *last(struct clientInfo *client);

//Compare to sockaddr_in, if there are equals return 1, 0 otherwise
int itsme(struct sockaddr_in *first, struct sockaddr_in *second);

//Search the tab in the list, return his pointer if find, Null pointer otherwise
struct clientInfo *isInList(struct sockaddr_in *tab, struct clientInfo *list);

//Print the IP and port in the terminal
void printIP(struct sockaddr_in *IP);

struct clientInfo *addClient(struct sockaddr_in IP, struct clientInfo *clientList);

#endif