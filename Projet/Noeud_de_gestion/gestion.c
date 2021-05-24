#include "../Block/block.h"
#include "../Block/transactions.h"
#include "../Block/blockchain.h"
#include "../Block/account.h"
#include "../Hash/sha256.h"

#include "../Network/network.h"

#include <pthread.h>

typedef struct {
    BLOCKCHAIN bc;
    pthread_mutex_t mutex;
} BLOCKCHAIN_M;

typedef struct {
    TRANSACTIONS_LIST tl;
    pthread_mutex_t mutex;
} TL_M;

void* gestion(void *arg){

    /*
        args
    */
    struct server *network = (struct server*)arg;
    /*if (network->status != ONLINE){
        return NULL;
    }*/
    


    BLOCKCHAIN_M bc_m;
    pthread_mutex_init(&bc_m.mutex, NULL);
    bc_m.bc = initBlockchain();

    TL_M txs_temp_m;
    pthread_mutex_init(&txs_temp_m.mutex, NULL);
    txs_temp_m.tl = initListTxs();

    /*pthread_mutex_lock(&bc_m.mutex);
    pthread_mutex_unlock(&bc_m.mutex);*/

    pthread_mutex_destroy(&bc_m.mutex);
    pthread_mutex_destroy(&txs_temp_m.mutex);
    clearTxsList(&txs_temp_m.tl);
    freeBlockchain(&bc_m.bc);
}