#ifndef NETWORK_H
#define NETWORK_H

#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>

void SendMessage(char *str /*Must be \0 terminated*/, int fd, unsigned long long len, char type);
int network(int *fdin, int *fdout, pthread_mutex_t *mutexfd, char *IP, char *firstserver);

/*
Potentiels problèmes / choses à régler:

- etre sur que tous les clients soient bien fini avant de les free, actuellement ca ne vérifie rien
- idem pour le serveur, mais je pense que ca pose moins de soucis


*/


#endif
