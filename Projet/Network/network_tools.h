#ifndef NETWORK_TOOLS_H
#define NETWORK_TOOLS_H

#include "server.h"


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

//Search the tab in the list, return 1 if find, 0 otherwise
int isInList(struct sockaddr_in *tab, struct clientInfo *list);

//Print the IP and port in the terminal
void printIP(struct sockaddr_in *IP);

#endif