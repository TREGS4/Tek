#ifndef GESTION_H
#define GESTION_H

#include "../Block/block.h"
#include "../Block/transactions.h"
#include "../Block/blockchain.h"
#include "../Block/account.h"
#include "../Hash/sha256.h"
#include "../Network/network.h"

typedef struct {
    BLOCKCHAIN bc;
    pthread_mutex_t mutex;
} BLOCKCHAIN_M;

typedef struct {
    TRANSACTIONS_LIST tl;
    pthread_mutex_t mutex;
} TL_M;

void* gestion(void *arg);

#endif