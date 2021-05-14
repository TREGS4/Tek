#include "network_tools.h"

struct clientInfo *initClientList(pthread_mutex_t *lockKnownServers, pthread_mutex_t *lockReadGlobalIntern, pthread_mutex_t *lockReadGlobalExtern)
{
    struct clientInfo *sentinel = malloc(sizeof(struct clientInfo));
    sentinel->sentinel = sentinel;
    sentinel->prev = NULL;
    sentinel->next = sentinel;
    sentinel->isSentinel = TRUE;
    sentinel->lockList = lockKnownServers;
    sentinel->lockReadGlobalIntern = lockReadGlobalIntern;
    sentinel->lockReadGlobalExtern = lockReadGlobalExtern;

    return sentinel;
}

void freeClientList(struct clientInfo *clientList)
{
    clientList = clientList->sentinel;

    pthread_mutex_lock(clientList->lockList);
    while (clientList->next->isSentinel == FALSE)
        removeClient(clientList->next);
    pthread_mutex_unlock(clientList->lockList);

    free(clientList);
}

struct clientInfo *addClient(struct clientInfo *list, struct sockaddr_in IP)
{
    struct clientInfo *client = malloc(sizeof(struct clientInfo));

    client->isSentinel = FALSE;
    client->IP = IP;

    pthread_mutex_lock(list->lockList);
    client->next = list->next;
    client->prev = list;
    list->next = client;
    if (client->next != NULL)
        client->next->prev = client;

    client->sentinel = list->sentinel;
    client->lockList = list->lockList;
    client->lockReadGlobalIntern = list->lockReadGlobalIntern;
    client->lockReadGlobalExtern = list->lockReadGlobalExtern;
    pthread_mutex_unlock(list->lockList);

    return client;
}

int removeClient(struct clientInfo *client)
{
    if (client->isSentinel == TRUE)
    {
        printf("Can't remove the sentinel\n");
        return EXIT_FAILURE;
    }

    pthread_mutex_lock(client->lockList);
    client->prev->next = client->next;
    client->next->prev = client->prev;
    pthread_mutex_unlock(client->lockList);

    free(client);

    return EXIT_SUCCESS;
}

size_t listLen(struct clientInfo *client)
{
    size_t res = 0;

    for (client = client->sentinel->next; client->isSentinel == FALSE; client = client->next)
    {
        res++;
    }

    return res;
}

struct clientInfo *last(struct clientInfo *client)
{
    while (client->next->isSentinel == FALSE)
        client = client->next;

    return client;
}

int sameIP(struct sockaddr_in *first, struct sockaddr_in *second)
{
    int me = TRUE;
    char buff[16];
    char buff2[16];
    memset(buff, 0, 16);
    memset(buff, 0, 16);
    inet_ntop(AF_INET, &first->sin_addr, buff, 16);
    inet_ntop(AF_INET, &second->sin_addr, buff2, 16);

    for (size_t i = 0; i <= 15 && me == 1; i++)
        if (((buff[i] >= '0' && buff[i] <= '9') || buff[i] == '.') && ((buff2[i] >= '0' && buff2[i] <= '9') || buff2[i] == '.'))
            me = buff[i] == buff2[i];

    return me;
}

struct clientInfo *FindClient(struct sockaddr_in *tab, struct clientInfo *list)
{
    for (list = list->sentinel->next; list->isSentinel == FALSE && sameIP(tab, &list->IP) == FALSE; list = list->next)
    {
        ;
    }

    return list;
}

void printIP(struct sockaddr_in *IP)
{
    char *buff = malloc(16 * sizeof(char));

    unsigned int port = ntohs(IP->sin_port);
    inet_ntop(AF_INET, &IP->sin_addr, buff, 16);

    printf("%s:%u\n", buff, port);
    free(buff);
}