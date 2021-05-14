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

//A new structure is created for each new known IP adress
struct clientInfo
{
    //IP adress of the element, can also contain family and port
    struct sockaddr_in IP;

    //TRUE is the it's the sentinel, FALSE otherwise
    short isSentinel;

    //Pointer to the next, previous and sentinel element
    //prev of sentinel is always NULL
    //next of the last element is always NULL
    struct clientInfo *next;
    struct clientInfo *prev;
    struct clientInfo *sentinel;

    //Pointer to the server's mutex
    pthread_mutex_t *lockList;
    pthread_mutex_t *lockReadGlobalIntern;
    pthread_mutex_t *lockReadGlobalExtern;
};

//One structure is initialised when starting, it contains all the informations for each part of the network code
struct server
{
    //Status of the server
    int status;

    //The two write files descriptor for intern and extern communications
    int fdoutExtern;
    int fdoutIntern;

    //The two read files descriptor for intern and extern communications
    int fdinExtern;
    int fdinIntern;

    //The sentinel to the list of known server
    struct clientInfo *KnownServers;

    //lock when writing on fdoutExtern, we need send the whole message before an other thread can write
    pthread_mutex_t lockReadGlobalExtern;

    //lock when writing on fdoutIntern, we need send the whole message before an other thread can write
    pthread_mutex_t lockReadGlobalIntern;

    //lock when using/modifing the list of known servers
    pthread_mutex_t lockKnownServers;     
};

//Create a new list of clientInfo, return the sentinel of the list
struct clientInfo *initClientList(pthread_mutex_t *lockKnownServers, pthread_mutex_t *lockReadGlobalIntern, pthread_mutex_t *lockReadGlobalExtern);

//Remove all the client and free the sentinel
void freeClientList(struct clientInfo *clientList);

//add the new element is the list, not necessarily at the end
struct clientInfo *addClient(struct clientInfo *list, struct sockaddr_in IP);

//Remove the client from the list
int removeClient(struct clientInfo *client);

//Return the size of the list (sentinel not included)
size_t listLen(struct clientInfo *client);

//Return a pointer to the last client of the list
struct clientInfo *last(struct clientInfo *client);

//Compare to sockaddr_in, if there are equals return TRUE, FALSE otherwise
int sameIP(struct sockaddr_in *first, struct sockaddr_in *second);

//Search the tab in the list, return his pointer if find, Null pointer otherwise
struct clientInfo *FindClient(struct sockaddr_in *tab, struct clientInfo *list);

//Print the IP and port in the terminal
void printIP(struct sockaddr_in *IP);



#endif