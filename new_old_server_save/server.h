#ifndef SERVER_H
#define SERVER_H

#include "network_tools.h"

void *server(void *arg);
void printData(int type, unsigned long long len, char *buff);

#endif