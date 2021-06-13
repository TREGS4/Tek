#include "gestion.h"
#include "../API/API.h"
#include "../Noeud_de_minage/minage.h"

#include <pthread.h>

typedef struct
{
    BLOCKCHAIN_M *bc_m;
    TL_M *tl_m;
    shared_queue *exq;
    int nb_thread;
    int difficulty;
    int *status;
} MINING_THREAD_ARG;
//main function : execute this to launch a mining node
//qrguments :
//blockachain : the current blockchain,
//tlq : a queue with the transaction lists,
//exq : the queue where we push the new blocks,
//nb_trhead = number of thread we will use to mine,
//difficulty : difficulty of the mining
//status : to end the prgrm if needed
void *mining(void *arg)
{
    MINING_THREAD_ARG *m_a = (MINING_THREAD_ARG *)arg;
    BLOCKCHAIN_M *blockchain = m_a->bc_m;
    TL_M *txl = m_a->tl_m;
    shared_queue *exq = m_a->exq;
    int nb_thread = m_a->nb_thread;
    int difficulty = m_a->difficulty;
    int *status = m_a->status;

    printf("Mining thread launched.\n");

    while (*status)
    {
        //Get a transaction list's hash
        //build a hash
        if (txl->tl.size != 0)
        {
            pthread_mutex_lock(&blockchain->mutex);
            BLOCK *last_block = getLastBlock(&blockchain->bc);
            pthread_mutex_unlock(&blockchain->mutex);
            BYTE prev_hash[SHA256_BLOCK_SIZE];
            memcpy(prev_hash, last_block->blockHash, SHA256_BLOCK_SIZE);

            //create a new block
            BLOCK *block = malloc(sizeof(BLOCK));
            *block = initBlock();
            memcpy(block->previusHash, prev_hash, SHA256_BLOCK_SIZE);

            pthread_mutex_lock(&txl->mutex);
            ull_t nb_transactions = txl->tl.size;
            for (ull_t i = 0; i < nb_transactions; i++)
            {
                addTx(&block->tl, &txl->tl.transactions[i]);
            }
            pthread_mutex_unlock(&txl->mutex);

            BYTE merkle_hash[SHA256_BLOCK_SIZE];
            getMerkleHash(block, merkle_hash);

            //Both hash need to be in ascii form
            char Aprev_hash[2 * SHA256_BLOCK_SIZE + 1];
            char Amerkle_hash[2 * SHA256_BLOCK_SIZE + 1];

            sha256ToAscii(prev_hash, Aprev_hash);
            sha256ToAscii(merkle_hash, Amerkle_hash);

            Amerkle_hash[2 * SHA256_BLOCK_SIZE] = '\0';
            Aprev_hash[2 * SHA256_BLOCK_SIZE] = '\0';
            //mining a proof
            BYTE sum[4 * SHA256_BLOCK_SIZE + 11 + 1];
            sprintf((char *)sum, "%011llu%s%s", (ull_t)block->time, (char *)Aprev_hash, (char *)Amerkle_hash);
            ull_t proof = mine_from_string((char *)sum, nb_thread, difficulty);

            pthread_mutex_lock(&txl->mutex);
            removeTxsList(&txl->tl, 0, nb_transactions);
            pthread_mutex_unlock(&txl->mutex);

            //sha(proof/prev_hash/merkle_hash) = blockhash
            block->proof = proof;

            //hash with sha256
            BYTE new_hash[SHA256_BLOCK_SIZE];
            getHash(block, new_hash);

            memcpy(block->blockHash, new_hash, SHA256_BLOCK_SIZE);

            //return proof;
            shared_queue_push(exq, block);
        }
        sleep(5);
    }
    return NULL;
}

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
    printf("Blockchain init.\n");

    TL_M txs_temp_m;
    pthread_mutex_init(&txs_temp_m.mutex, NULL);
    txs_temp_m.tl = initListTxs();

    int isAPI = 0;
    int isMINING = 1;

    pthread_t api_thread;
    shared_queue *api_txs;

    API_THREAD_ARG args_api = {
        .server = network,
        .bc_m = &bc_m,
        .tl_m = &txs_temp_m,
    };

    if (isAPI)
    {
        api_txs = shared_queue_new();
        args_api.outgoingTxs = api_txs;
        pthread_create(&api_thread, NULL, API, (void *)&args_api);
    }

    pthread_t mining_thread;
    shared_queue *mining_blocks;
    int mining_status = 1;
    int nb_mining_thread = 1;
    int difficulty = 2;

    MINING_THREAD_ARG args_mining = {
        .bc_m = &bc_m,
        .tl_m = &txs_temp_m,
        .nb_thread = nb_mining_thread,
        .difficulty = difficulty,
        .status = &mining_status,
    };

    if (isMINING)
    {
        mining_blocks = shared_queue_new();
        args_mining.exq = mining_blocks;
        pthread_create(&mining_thread, NULL, mining, (void *)&args_mining);
    }

    while (1)
    {
        // reading the network messages
        if (!shared_queue_isEmpty(network->IncomingMessages))
        {
            MESSAGE *message = shared_queue_pop(network->IncomingMessages);

            if (message->type == TYPE_TXS)
            { //transactions
                TRANSACTION txs = binToTxs((BYTE *)message->data);
                pthread_mutex_lock(&txs_temp_m.mutex);
                int hassended = hasSendedTxs(txs.sender, &txs_temp_m.tl);

                if (hassended)
                {
                    printf("FROM NETWORK: A transaction has been refused because the sender has already an uncheck transaction.\n");
                }
                else
                {
                    pthread_mutex_lock(&bc_m.mutex);
                    int hasenough = enoughMoney(txs.sender, txs.amount, &bc_m.bc);
                    if (!hasenough)
                    {
                        printf("FROM NETWORK: A transaction has been refused because the sender has not enough TEK.\n");
                    }
                    else
                    {
                        addTx(&txs_temp_m.tl, &txs);

                        TRANSACTION_BIN txsbin = txsToBin(&txs);
                        MESSAGE *msg = CreateMessage(TYPE_TXS, txsbin.nbBytes, txsbin.bin);
                        shared_queue_push(network->OutgoingMessages, msg);

                        printf("FROM NETWORK: A acceptable transaction has been added.\n");
                    }
                    pthread_mutex_unlock(&bc_m.mutex);
                }
                pthread_mutex_unlock(&txs_temp_m.mutex);
            }
            else if (message->type == TYPE_BLOCKCHAIN)
            { // blockchain
                BLOCKCHAIN bc = binToBlockchain(message->data);
                

                // verifications
                int res = checkBlockchain(&bc);
                if (!res)
                {
                    pthread_mutex_lock(&bc_m.mutex);
                    ull_t current_bc_size = bc_m.bc.blocksNumber;
                    if (current_bc_size == bc.blocksNumber)
                    {
                        BLOCK *last_cur_b = getLastBlock(&bc_m.bc);
                        BLOCK *last_new_b = getLastBlock(&bc);
                        if (last_new_b->time < last_cur_b->time)
                        {
                            freeBlockchain(&bc_m.bc);
                            bc_m.bc = bc;
                            pthread_mutex_lock(&txs_temp_m.mutex);
                            updateTlWithBc(&txs_temp_m.tl, &bc_m.bc);
                            pthread_mutex_unlock(&txs_temp_m.mutex);
                            printf("FROM NETWORK: the blockchain has been updated.\n");
                        }
                        else
                        {
                            freeBlockchain(&bc);
                            printf("FROM NETWORK: A blockchain has been refused (size equal / timestamp inf).\n");
                        }
                    }
                    else if (current_bc_size < bc.blocksNumber)
                    {
                        freeBlockchain(&bc_m.bc);
                        bc_m.bc = bc;
                        pthread_mutex_lock(&txs_temp_m.mutex);
                        updateTlWithBc(&txs_temp_m.tl, &bc_m.bc);
                        pthread_mutex_unlock(&txs_temp_m.mutex);
                        printf("FROM NETWORK: the blockchain has been updated.\n");
                    }
                    else
                    {
                        freeBlockchain(&bc);
                        printf("FROM NETWORK: A blockchain has been refused (shortest).\n");
                    }
                    pthread_mutex_unlock(&bc_m.mutex);
                }
                else
                {
                    printf("FROM NETWORK: A blockchain has been refused (invalid).\n");
                }
            }

            DestroyMessage(message);
        }

        // reading the api transactions
        if (isAPI && !shared_queue_isEmpty(api_txs))
        {
            TRANSACTION *txs = shared_queue_pop(api_txs);
            // verification of the transaction
            pthread_mutex_lock(&txs_temp_m.mutex);
            int hassended = hasSendedTxs(txs->sender, &txs_temp_m.tl);

            if (hassended)
            {
                printf("FROM API: A transaction has been refused because the sender has already an uncheck transaction.\n");
            }
            else
            {
                pthread_mutex_lock(&bc_m.mutex);
                int hasenough = enoughMoney(txs->sender, txs->amount, &bc_m.bc);
                if (!hasenough)
                {
                    printf("FROM API: A transaction has been refused because the sender has not enough TEK.\n");
                }
                else
                {
                    addTx(&txs_temp_m.tl, txs);
                    TRANSACTION_BIN txsbin = txsToBin(txs);
                    MESSAGE *msg = CreateMessage(TYPE_TXS, txsbin.nbBytes, txsbin.bin);
                    shared_queue_push(network->OutgoingMessages, msg);

                    printf("FROM API: A acceptable transaction has been added.\n");
                }
                pthread_mutex_unlock(&bc_m.mutex);
            }
            pthread_mutex_unlock(&txs_temp_m.mutex);
            free(txs);
        }

        // reading the blocks mined
        if (isMINING && !shared_queue_isEmpty(mining_blocks))
        {
            BLOCK *b = shared_queue_pop(mining_blocks);

            pthread_mutex_lock(&bc_m.mutex);
            int success = addBlock(&bc_m.bc, *b);

            if (success)
            {
                BLOCKCHAIN_BIN bcbin = blockchainToBin(&bc_m.bc);
                MESSAGE *msg = CreateMessage(TYPE_BLOCKCHAIN, bcbin.nbBytes, bcbin.bin);
                shared_queue_push(network->OutgoingMessages, msg);
                printf("FROM MINING: correct Block received.\n");
            }
            else
            {
                printf("FROM MINING: incorrect Block received.\n");
            }
            pthread_mutex_unlock(&bc_m.mutex);
            free(b);
        }
        sleep(0.05);
    }

    pthread_mutex_destroy(&bc_m.mutex);
    pthread_mutex_destroy(&txs_temp_m.mutex);
    freeTxsList(&txs_temp_m.tl);
    freeBlockchain(&bc_m.bc);
}
