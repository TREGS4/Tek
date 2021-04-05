#include "network.h"

void *ReWriteForAllThreads(void *arg)
{
    struct clientInfo *client = arg;
    char buff[BUFFER_SIZE_SOCKET];
    int r = 1;

    while ((r = read(client->sentinel->fdinThread, &buff, BUFFER_SIZE_SOCKET)) > 0)
    {
        for(client = client->sentinel->next; client != client->sentinel; client = client->next)
        {
            if (client->status == CONNECTED)
            {
                pthread_mutex_lock(&client->lockWrite);
                write(client->fdTofdin, buff, r);
                pthread_mutex_unlock(&client->lockWrite);
            }
        }
    }

    return NULL;
}

void *closeConnection(void *arg)
{
    struct clientInfo *client = arg;
    while (1)
    {
        for(client = client->sentinel->next; client != client->sentinel; client = client->next)
        {
            if (client->status == ENDED)
            {
                pthread_mutex_lock(&client->lockWrite);
                close(client->fdTofdin);
                pthread_mutex_unlock(&client->lockWrite);
            }
        }
    }

    return NULL;
}

void *printList(void *arg)
{
    struct clientInfo *client = arg;
    while (1)
    {
        for(client = client->sentinel->next; client != client->sentinel; client = client->next)
        {
            switch (client->status)
            {
            case NOTUSED:
                printf("%lu is not used\n", client->ID);
                break;
            case CONNECTED:
                printf("%lu is connected\n", client->ID);
                break;
            case CONNECTING:
                printf("%lu is connecting\n", client->ID);
                break;
            case ENDED:
                printf("%lu is ended\n", client->ID);
                break;
            case ERROR:
                printf("%lu is error\n", client->ID);
                break;
            case SENTINEL:
                printf("%lu is the sentinel\n", client->ID);
                break;
            case DEAD:
                printf("%lu is dead\n", client->ID);
                break;
            default:
                printf("%lu is in an unknown state\n", client->ID);
                break;
            }
        }
        printf("\n\n");
        sleep(2);
    }
    return NULL;
}


struct clientInfo *initList(int fdin, int fdoutExtern, int fdoutIntern)
{
    struct clientInfo *client = malloc(sizeof(struct clientInfo));
    client->lockReadGlobalExtern = malloc(sizeof(pthread_mutex_t));
    client->lockReadGlobalIntern = malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init(&client->lockInfo, NULL);
    pthread_mutex_init(&client->lockRead, NULL);
    pthread_mutex_init(&client->lockWrite, NULL);
    pthread_mutex_init(client->lockReadGlobalExtern, NULL);
    pthread_mutex_init(client->lockReadGlobalIntern, NULL);

    pthread_mutex_lock(&client->lockInfo);
    client->ID = 0;
    client->sentinel = client;
    client->status = SENTINEL;
    client->next = client;
    client->IP.sa_family = AF_INET;
    client->prev = NULL;

    client->fd = -1;
    client->fdTofdin = -1;
    client->fdinThread = fdin;
    client->fdoutExtern = fdoutExtern;
    client->fdoutExtern = fdoutExtern;
    //client->fdoutIntern = fdoutIntern;

    pthread_mutex_unlock(&client->lockInfo);

    return client;
}

void freeList(struct clientInfo *sentinel)
{
    struct clientInfo *temp = sentinel->next;
    struct clientInfo *temp2 = temp->next;

    while(temp != sentinel)
    {
        removeClient(temp);
        temp = temp2;
        temp2 = temp2->next;
    }

    pthread_mutex_destroy(&sentinel->lockInfo);
    pthread_mutex_destroy(&sentinel->lockWrite);
    pthread_mutex_destroy(&sentinel->lockRead);
    pthread_mutex_destroy(sentinel->lockReadGlobalExtern);
    pthread_mutex_destroy(sentinel->lockReadGlobalIntern);

    free(sentinel->lockReadGlobalExtern);
    free(sentinel->lockReadGlobalIntern);
    free(sentinel);
}

void * interComms(void *arg)
{
    struct clientInfo *sentinel = arg;
    int r = 1;
    char buff[BUFFER_SIZE_SOCKET];

    /*while((r = read( , buff, BUFFER_SIZE_SOCKET) > 0))
    {

    } */

    return NULL;
}


int network(int fdin, int fdout)
{
    int fdIntern[2];
    pipe(fdIntern);

    struct clientInfo *listClients = initList(fdin, fdout, fdIntern[1]);

    pthread_t serverThread;
    pthread_t maintenerThread;
    pthread_t reWriteThread;
    pthread_t closeConnectionThread;
    //pthread_t printListThread;

    //connect()

    pthread_create(&serverThread, NULL, server, (void *)listClients);
    pthread_create(&maintenerThread, NULL, connectionMaintener, (void *)listClients);
    pthread_create(&reWriteThread, NULL, ReWriteForAllThreads, (void *)listClients);
    pthread_create(&closeConnectionThread, NULL, closeConnection, (void *)listClients);
    //pthread_create(&printListThread, NULL, printList, (void *)listClients);

    pthread_join(serverThread, NULL);
    pthread_join(maintenerThread, NULL);
    pthread_join(reWriteThread, NULL);
    pthread_join(closeConnectionThread, NULL);
    //pthread_join(printListThread, NULL);

    freeList(listClients);

    return EXIT_SUCCESS;
}

void printIP(struct sockaddr *IP)
{
    char *buff = malloc(16 * sizeof(char));
    inet_ntop(AF_INET, &(((struct sockaddr_in *)IP)->sin_addr), buff, 16);
    printf("%s\n", buff);
    free(buff);
}

struct clientInfo *last(struct clientInfo *client)
{
    while (client->next != client->sentinel)
        client = client->next;
    
    return client;
}

size_t listLen(struct clientInfo *client)
{
    size_t res = 0;
    client = client->sentinel->next;

    while(client != client->sentinel)
    {
        res++;
        client = client->next;
    } 

    return res;
}



/*
Creat un ptr for a struct clientInfo, set config to IPV4
*/

struct clientInfo *initClient(struct clientInfo *prev)
{
    prev = last(prev);
    struct clientInfo *client = malloc(sizeof(struct clientInfo));

    pthread_mutex_init(&client->lockInfo, NULL);
    pthread_mutex_init(&client->lockWrite, NULL);
    pthread_mutex_init(&client->lockRead, NULL);

    pthread_mutex_lock(&client->lockInfo);

    client->ID = rand() * rand() + 1;
    client->sentinel = prev->sentinel;
    client->lockReadGlobalExtern = client->sentinel->lockReadGlobalExtern;
    client->lockReadGlobalIntern = client->sentinel->lockReadGlobalIntern;
    client->status = NOTUSED;
    client->next = client->sentinel;
    client->IP.sa_family = AF_INET;
    client->prev = prev;

    client->fd = -1;

    int tab[2];
    pipe(tab);

    client->fdTofdin = tab[1];
    client->fdinThread = tab[0];
    client->fdoutExtern = client->sentinel->fdoutExtern;
    client->fdoutIntern = client->sentinel->fdoutIntern;
    pthread_mutex_unlock(&client->lockInfo);

    pthread_mutex_lock(&prev->lockInfo);
    prev->next = client;
    pthread_mutex_unlock(&prev->lockInfo);

    return client;
}



/*
    Remove a client without freeing it, it just change the status to NOUSED
*/

void terminator(struct clientInfo *client)
{
    pthread_join(client->clientThread, NULL);
}

int removeClient(struct clientInfo *client)
{
    if (client->ID == 0)
    {
        printf("Can't remove the sentinel\n");
        return EXIT_FAILURE;
    }

    pthread_mutex_lock(&client->lockInfo);
    pthread_mutex_lock(&client->lockRead);
    pthread_mutex_lock(&client->lockWrite);
    pthread_mutex_lock(&client->prev->lockInfo);
    if (client->next != client->sentinel)
    {
        pthread_mutex_lock(&client->next->lockInfo);
        client->next->prev = client->prev;
        pthread_mutex_unlock(&client->next->lockInfo);
    }

    client->prev->next = client->next;
    pthread_mutex_unlock(&client->prev->lockInfo);
    pthread_mutex_unlock(&client->lockInfo);
    pthread_mutex_unlock(&client->lockRead);
    pthread_mutex_unlock(&client->lockWrite);

    terminator(client);

    pthread_mutex_destroy(&client->lockInfo);
    pthread_mutex_destroy(&client->lockWrite);
    pthread_mutex_destroy(&client->lockRead);

    free(client);

    return EXIT_SUCCESS;
}
