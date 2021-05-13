#include "network_tools.h"

struct clientInfo *initClientList(int fdin, int fdoutExtern, int fdoutIntern)
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
    client->IPandPort.sin_family = AF_INET;
    client->prev = NULL;

    client->clientSocket = -1;
    client->fdTofdin = -1;
    client->fdinThread = fdin;
    client->fdoutExtern = fdoutExtern;
    client->fdoutIntern = fdoutIntern;

    pthread_mutex_unlock(&client->lockInfo);

    return client;
}

void freeClientList(struct clientInfo *clientList)
{
    struct clientInfo *sentinel = clientList->sentinel;
    struct clientInfo *temp = sentinel->next;
    struct clientInfo *temp2 = temp->next;

    while (temp != sentinel)
    {
        removeClient(temp);
        temp = temp2;
        temp2 = temp2->next;
    }

    close(sentinel->fdoutIntern);
    close(sentinel->fdoutExtern);
    close(sentinel->fdinThread);

    pthread_mutex_destroy(&sentinel->lockInfo);
    pthread_mutex_destroy(&sentinel->lockWrite);
    pthread_mutex_destroy(&sentinel->lockRead);
    pthread_mutex_destroy(sentinel->lockReadGlobalExtern);
    pthread_mutex_destroy(sentinel->lockReadGlobalIntern);

    free(sentinel->lockReadGlobalExtern);
    free(sentinel->lockReadGlobalIntern);
    free(sentinel);
}

struct clientInfo *initClient(struct clientInfo *prev)
{
    prev = last(prev);
    struct clientInfo *client = malloc(sizeof(struct clientInfo));

    pthread_mutex_init(&client->lockInfo, NULL);
    pthread_mutex_init(&client->lockWrite, NULL);
    pthread_mutex_init(&client->lockRead, NULL);

    pthread_mutex_lock(&client->lockInfo);

    client->ID = (rand() + 1) * (rand() + 1);
    client->sentinel = prev->sentinel;
    client->server = client->sentinel->server;
    client->lockReadGlobalExtern = client->sentinel->lockReadGlobalExtern;
    client->lockReadGlobalIntern = client->sentinel->lockReadGlobalIntern;
    client->status = NOTUSED;
    client->next = client->sentinel;
    client->IPandPort.sin_family = AF_INET;
    client->prev = prev;

    client->clientSocket = -1;

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

int removeClient(struct clientInfo *client)
{
    if (client->ID == 0)
    {
        printf("Can't remove the sentinel\n");
        return EXIT_FAILURE;
    }

    pthread_mutex_lock(&client->lockInfo);
    client->status = DEAD;
    pthread_mutex_unlock(&client->lockInfo);

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

    pthread_join(client->clientThread, NULL);

    close(client->fdTofdin);
    close(client->fdinThread);
    close(client->clientSocket);

    pthread_mutex_destroy(&client->lockInfo);
    pthread_mutex_destroy(&client->lockWrite);
    pthread_mutex_destroy(&client->lockRead);

    free(client);

    return EXIT_SUCCESS;
}

size_t listLen(struct clientInfo *client)
{
    size_t res = 0;
    client = client->sentinel->next;

    while (client != client->sentinel)
    {
        if (client->status == CONNECTED)
            res++;
        client = client->next;
    }

    return res;
}

struct clientInfo *last(struct clientInfo *client)
{
    while (client->next != client->sentinel)
        client = client->next;

    return client;
}

int itsme(struct sockaddr_in *first, struct sockaddr_in *second)
{
    int me = 1;
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

struct clientInfo *isInList(struct sockaddr_in *tab, struct clientInfo *list)
{
    int find = 0;
    struct clientInfo *res = NULL;
    list = list->sentinel->next;

    while (find == 0 && list != list->sentinel)
    {
        find = itsme(tab, &list->IPandPort);
        if (find)
            res = list;
        list = list->next;
    }

    return res;
}

void printIP(struct sockaddr_in *IP)
{
    char *buff = malloc(16 * sizeof(char));

    unsigned int port = ntohs(IP->sin_port);
    inet_ntop(AF_INET, &IP->sin_addr, buff, 16);

    printf("%s:%u\n", buff, port);
    free(buff);
}

struct clientInfo *addClient(struct sockaddr_in IP, struct clientInfo *clientList)
{
    struct clientInfo *client = isInList(&IP, clientList);
    if (itsme(&IP, &clientList->server->IPandPort) == 0 && client == NULL)
    {

        client = initClient(clientList->sentinel);
        pthread_mutex_lock(&client->lockInfo);

        client->IPandPort = IP;
        client->IPLen = sizeof(struct sockaddr_in);
        client->status = NOTCONNECTED;

        pthread_mutex_unlock(&client->lockInfo);
    }

    return client;
}