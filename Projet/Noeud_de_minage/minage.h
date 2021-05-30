#ifndef MINAGE_H
#define MINAGE_H

int moncuq();

unsigned int mine_from_string(char *strin, int nbthread);

void *mining(MINING_THREAD_ARG);

int mininglobby(int fdin, int fdout, int nbthread);

typedef struct {
    BLOCKCHAIN_M *bc_m;
    TL_M * tl_m;
    shared_queue * exq;
    int nb_thread;
    int difficulty;
    int * status;
} MINING_THREAD_ARG;
#endif
