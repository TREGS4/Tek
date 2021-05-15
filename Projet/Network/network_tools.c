#include "network_tools.h"

struct clientInfo *initClientList()
{
    struct clientInfo *sentinel = malloc(sizeof(struct clientInfo));
    memset(&sentinel->IP, 0, sizeof(struct sockaddr_in));
    sentinel->sentinel = sentinel;
    sentinel->prev = NULL;
    sentinel->next = sentinel;
    sentinel->isSentinel = TRUE;

    return sentinel;
}

void freeClientList(struct clientInfo *clientList)
{
    clientList = clientList->sentinel;

    while (clientList->next->isSentinel == FALSE)
        removeClient(clientList->next);

    free(clientList);
}

struct clientInfo *addClient(struct clientInfo *list, struct sockaddr_in IP)
{
    struct clientInfo *client = malloc(sizeof(struct clientInfo));

    client->isSentinel = FALSE;
    client->IP = IP;

    client->next = list->next;
    client->prev = list;
    list->next = client;
    if (client->next != NULL)
        client->next->prev = client;

    client->sentinel = list->sentinel;

    return client;
}

int removeClient(struct clientInfo *client)
{
    if (client->isSentinel == TRUE)
    {
        printf("Can't remove the sentinel\n");
        return EXIT_FAILURE;
    }

    client->prev->next = client->next;
    client->next->prev = client->prev;

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
    int me = FALSE;
    
    if(memcmp(&first->sin_addr, &second->sin_addr, sizeof(struct in_addr)) == 0)
        me = TRUE;

    return me;
}

struct clientInfo *FindClient(struct sockaddr_in *tab, struct clientInfo *list)
{
    struct clientInfo *res = NULL;
    list = list->sentinel->next;

    while(res == NULL && list->isSentinel == FALSE)
    {
        if(sameIP(tab, &list->IP) == TRUE)
            res = list;
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

void addServerFromMessage(MESSAGE message, struct server *server)
{
    size_t offset = 0;
    size_t nbstruct;
    size_t sizeStructSockaddr_in = sizeof(struct sockaddr_in);
    struct sockaddr_in temp;

    //peut y avoir un souci si la taille de data depasse la taille du buffer du file descriptor
    //comportement inconnu dans ce cas la

    nbstruct = message.sizeData / sizeStructSockaddr_in;

    for (size_t i = 0; i < nbstruct; i++)
    {
        memcpy(&temp, message.data + offset, sizeStructSockaddr_in);
        temp.sin_port = htons(atoi(PORT));

        pthread_mutex_lock(&server->lockKnownServers);
        if (FindClient(&temp, server->KnownServers) == NULL)
            addClient(server->KnownServers, temp);
        pthread_mutex_unlock(&server->lockKnownServers);
        offset += sizeStructSockaddr_in;
    }
}

void *sendNetwork(void *arg)
{
    struct server *server = arg;
    struct clientInfo *client = server->KnownServers;
    size_t sizeStructSockaddr_in = sizeof(struct sockaddr_in);
    char type = 1;
    unsigned long long dataSize = 0;
    char *messageBuff;
    size_t offset = 0;

    while (server->status == ONLINE)
    {
        dataSize = 0;
        offset = 0;

        pthread_mutex_lock(&server->lockKnownServers);
        dataSize = listLen(server->KnownServers) * sizeStructSockaddr_in;
        messageBuff = malloc(sizeof(char) * dataSize);

        for (client = client->sentinel->next; client->isSentinel == FALSE; client = client->next)
        {
            memcpy(messageBuff + offset, &client->IP, sizeStructSockaddr_in);
            offset += sizeStructSockaddr_in;
        }

        pthread_mutex_unlock(&server->lockKnownServers);

        MESSAGE message = CreateMessage(type, dataSize, messageBuff);
        shared_queue_push(server->OutgoingMessages, message);

        free(messageBuff);
        sleep(2);
    }

    return NULL;
}