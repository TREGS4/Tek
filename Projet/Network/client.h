#ifndef CLIENT_H
#define CLIENT_H

#include "network_tools.h"

/*
*   Send the message to all client present in the list.
*   If there is a problem with the client, it will remove of the list
*/
void SendMessage(struct clientInfo *clientList, MESSAGE *message);

#endif
