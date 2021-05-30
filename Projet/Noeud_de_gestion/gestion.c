#include "gestion.h"
#include "../API/API.h"
#include "../Noeud_de_minage/minage.h"

#include <pthread.h>

void *gestion(void *arg)
{

    /*
        args
    */
    struct server *network = (struct server *)arg;
    if (network->status != ONLINE)
    {
        return NULL;
    }

    BLOCKCHAIN_M bc_m;
    pthread_mutex_init(&bc_m.mutex, NULL);
    bc_m.bc = initBlockchain();

    TL_M txs_temp_m;
    pthread_mutex_init(&txs_temp_m.mutex, NULL);
    txs_temp_m.tl = initListTxs();

    int isAPI = 1;
    int isMINING = 1;

    pthread_t api_thread;
    shared_queue *api_txs;
    if (isAPI)
    {
        api_txs = shared_queue_new();
        API_THREAD_ARG args = {
            .server = network,
            .bc_m = &bc_m,
            .tl_m = &txs_temp_m,
            .outgoingTxs = api_txs
        };
        pthread_create(&api_thread, NULL, API, (void *)&args);
    }

    pthread_t mining_thread;
    shared_queue *mining_blocks;
    int mining_status = 1;
    int nb_mining_thread = 1;
    int difficulty = 3;
    if (isMINING)
    {
        mining_blocks = shared_queue_new();
        MINING_THREAD_ARG args = {
            .bc_m = &bc_m,
            .tl_m = &txs_temp_m,
            .exp = mining_blocks,
            .nb_thread = nb_mining_thread,
            .difficulty = difficulty,
            .status = &mining_status,
        };
        pthread_create(&mining_thread, NULL, mining, (void *)&args);
    }

    while (1)
    {

        // reading the network messages
        if (!shared_queue_isEmpty(network->IncomingMessages))
        {
            MESSAGE *message = shared_queue_pop(network->IncomingMessages);
            if (message->type == 2)
            { // transaction
                printf("message type 2 received\n");
            }
            else if (message->type == 3)
            { // blockchain
                printf("message type 3 received\n");
            }
            DestroyMessage(message);
        }

        // reading the api transactions
        if (isAPI && !shared_queue_isEmpty(api_txs))
        {
            TRANSACTION *txs = shared_queue_pop(api_txs);

            pthread_mutex_lock(&txs_temp_m.mutex);
            addTx(&txs_temp_m.tl, txs);
            pthread_mutex_unlock(&txs_temp_m.mutex);

            printf("une transaction depuis l'api a été reçu.\n");
            free(txs);
        }

        // reading the blocks mined
        if (isMINING && !shared_queue_isEmpty(mining_blocks))
        {
            BLOCK *b = shared_queue_pop(mining_blocks);

            pthread_mutex_lock(&bc_m.mutex);
            addBlock(&bc_m.bc, *b);
            pthread_mutex_unlock(&bc_m.mutex);

            printf("un block depuis le minage a été reçu.\n");
            free(b);
        }

        sleep(0.05);
    }

    pthread_mutex_destroy(&bc_m.mutex);
    pthread_mutex_destroy(&txs_temp_m.mutex);
    freeTxsList(&txs_temp_m.tl);
    freeBlockchain(&bc_m.bc);
}