#include "gestion.h"

#include <pthread.h>

void* gestion(void *arg){

    /*
        args
    */
    struct server *network = (struct server*)arg;
    if (network->status != ONLINE){
        return NULL;
    }
    
    BLOCKCHAIN_M bc_m;
    pthread_mutex_init(&bc_m.mutex, NULL);
    bc_m.bc = initBlockchain();

    TL_M txs_temp_m;
    pthread_mutex_init(&txs_temp_m.mutex, NULL);
    txs_temp_m.tl = initListTxs();


    int isAPI = 1;
    shared_queue *api_txs = shared_queue_new();
    if (isAPI){
        ;// TODO
    }

    while (1){


        // reading the network messages
        if (!shared_queue_isEmpty(network->IncomingMessages)){
            MESSAGE *message = shared_queue_pop(network->IncomingMessages);
            if (message->type == 2){ // transaction
                printf("message type 2 received\n");
            }else if (message->type == 3){ // blockchain
                printf("message type 3 received\n");
            }
            DestroyMessage(message);
        }

        // reading the api transactions
        if (!shared_queue_isEmpty(api_txs)){

        }
        
    
    
    // thimot la pute

    
    }

    pthread_mutex_destroy(&bc_m.mutex);
    pthread_mutex_destroy(&txs_temp_m.mutex);
    clearTxsList(&txs_temp_m.tl);
    freeBlockchain(&bc_m.bc);
}