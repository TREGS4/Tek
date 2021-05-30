#include "gestion.h"
#include "../API/API.h"
#include "../Noeud_de_minage/minage.h"

#include <pthread.h>


typedef struct {
    BLOCKCHAIN_M *bc_m;
    TL_M * tl_m;
    shared_queue * exq;
    int nb_thread;
    int difficulty;
    int * status;
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
	TL_M * txl = m_a->tl_m;
	shared_queue * exq = m_a->exq;
	int nb_thread = m_a->nb_thread;
	int difficulty = m_a->difficulty;
	int * status = m_a->status;
	while(*status)
	{
		//Get a transaction list's hash
		//build a hash
		if(txl->tl.size != 0)
		{
			BYTE *prev_hash = getLastBlock(&blockchain->bc)->blockHash;
			//create a new block
			BLOCK *block = malloc(sizeof(BLOCK));
			memcpy(block->previusHash, prev_hash, SHA256_BLOCK_SIZE);
			block->tl = initListTxs();


			pthread_mutex_lock(&txl->mutex);
			for(size_t i = 0; i < txl->tl.size; i++){
				addTx(&block->tl, &txl->tl.transactions[i]);
			}
			clearTxsList(&txl->tl);

			//empty the list

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
			BYTE sum[4 * SHA256_BLOCK_SIZE + 1];
			sprintf((char *)sum,"%s%s", (char *)Aprev_hash, (char *)Amerkle_hash);
			unsigned long proof = mine_from_string((char *)sum, nb_thread, difficulty);

			//sha(proof/prev_hash/merkle_hash) = blockhash
			block->proof = proof;

			//hash with sha256
			BYTE new_hash[SHA256_BLOCK_SIZE];
			getHash(block, new_hash);

			memcpy(block->blockHash, new_hash, SHA256_BLOCK_SIZE);

			//return proof;
			shared_queue_push(exq, block);
		}
		sleep(10);
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

    TL_M txs_temp_m;
    pthread_mutex_init(&txs_temp_m.mutex, NULL);
    txs_temp_m.tl = initListTxs();

    int isAPI = 1;
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
    int nb_mining_thread = 2;
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

    size_t amount = amountMoney("MFwwDQYJKoZIhvcNAQEBBQADSwAwSAJBAIVdUtUR9QG0wQl2jf00+0NiTOusk69PGFuHBEAoy7NIIzM7As81H1lGYUIg5pXVrWQ9ACt99trhVWNGRo3VMicCAwEAAQ==", &bc_m.bc);
    printf("amùountrzffzezv = %ld\n", amount);
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

            printf("sender of transaction = %s\n", txs->sender);

            // verification of the transaction
            pthread_mutex_lock(&bc_m.mutex);
            int hasenough = enoughMoney(txs->sender, txs->amount, &bc_m.bc);
            pthread_mutex_unlock(&bc_m.mutex);
            if (!hasenough){
                printf("A transaction have been refused because the sender has not enough TEK.\n");
            }else{
                pthread_mutex_lock(&txs_temp_m.mutex);
                addTx(&txs_temp_m.tl, txs);
                pthread_mutex_unlock(&txs_temp_m.mutex);
                printf("A acceptable transaction have been added.\n");
            }
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