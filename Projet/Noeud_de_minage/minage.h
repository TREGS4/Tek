#ifndef MINAGE_H
#define MINAGE_H

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <err.h>
#include <pthread.h>
#include <sys/wait.h>
#include <signal.h>
#include "../Hash/sha256.h"
#include "../Network/network.h"
#include "../Block/transactions.h"
#include "../Block/blockchain.h"
#include "../Block/block.h"
#include "../Tools/queue/shared_queue.h"

void *mining(void * arg);

typedef struct {
    BLOCKCHAIN_M *bc_m;
    TL_M * tl_m;
    shared_queue * exq;
    int nb_thread;
    int difficulty;
    int * status;
} MINING_THREAD_ARG;
#endif
