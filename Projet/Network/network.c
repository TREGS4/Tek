#include "network.h"

pthread_mutex_t mutexListClients = PTHREAD_MUTEX_INITIALIZER;

struct descripAndClients
{
    struct listClientInfo *clients;
    int fdin;
    int fdout;
};

void *test(void *arg)
{
    while (1)
    {

        struct listClientInfo *clients = arg;
        for (size_t i = 0; i < clients->size; i++)
        {
            if (clients->list[i].status == NOTUSED)
                printf("%ld is not used\n", i);
            else if (clients->list[i].status == CONNECTED)
            {
                printf("%ld is connected\n", i);
                //printf("%ld IP is %s\n", i, clients->list[i].IP->sa_data);
            }
            else
                printf("%ld is %d\n", i, clients->list[i].status);
        }
        printf("\n\n");
        sleep(2);
    }
    return NULL;
}

int network(int fdin, int fdout)
{
    struct listClientInfo clients;
    clients.size = 3;
    clients.list = malloc(clients.size * sizeof(struct clientInfo));
    pthread_t ServerThread;
    pthread_t MaintenerThread;
    pthread_t transmitThread;
    struct descripAndClients fdCli;
    fdCli.clients = &clients;
    fdCli.fdin = fdin;
    fdCli.fdout = fdout;

    for (size_t i = 0; i < clients.size; i++)
    {
        clients.list[i].status = NOTUSED;
        clients.list[i].IP = malloc(sizeof(struct sockaddr));
    }
    //connect()

    pthread_create(&ServerThread, NULL, server, (void *)&clients);
    pthread_create(&MaintenerThread, NULL, connectionMaintener, (void *)&clients);
    pthread_create(&transmitThread, NULL, test, (void *)&clients);

    pthread_join(ServerThread, NULL);
    pthread_join(MaintenerThread, NULL);
    pthread_join(transmitThread, NULL);

    for (size_t i = 0; i < clients.size; i++)
        free(clients.list[i].IP);
    free(clients.list);

    return EXIT_SUCCESS;
}

int compareString(char str1[14], char *str2)
{
    int res = 1;
    size_t pos = 0;

    while (res && str1[pos] != '\0' && str2[pos] != '\0')
    {
        if (str1[pos] != str2[pos])
            res = 0;
        pos++;
    }

    if (str1[pos] != str2[pos])
        res = 0;

    return res;
}

size_t findClient(struct listClientInfo *clients, char *buff)
{
    size_t res = 0;
    int find = 0;
    size_t i = 0;

    while (i < clients->size && !find)
    {
        if (clients->list[i].status == CONNECTED && compareString(clients->list[i].IP->sa_data, buff))
            find = 1;
        else
            i++;
    }

    if (!find)
        res = clients->size * 2;

    return res;
}

void *transmit(void *arg)
{
    struct descripAndClients *fds = arg;
    char buff[4];
    char buff2[512];
    int r = 0;
    int pos2 = 0;
    int correct = 0;
    int sizeIPV4 = 4;
    int reste = sizeIPV4 + 1;
    size_t pos;
    while (1)
    {
        correct = 0;
        r = 0;
        while (correct == 0)
        {
            r = read(STDIN_FILENO, buff + pos2, reste);
            reste -= r;
            pos2 += r;
            if (reste == 0 && buff[pos2] != '\n')
                correct = 1;
            else
            {
                pos2 = 0;
                reste = sizeIPV4 + 1;
            }
        }

        if (r != 0)
        {
            pos = findClient(fds->clients, buff);
            if (pos != fds->clients->size * 2)
            {
                int stop = 0;
                while (!stop)
                {
                    r = read(fds->fdin, buff2, 512);
                    write(fds->clients->list[pos].fdout, buff, r);
                    if (buff[r - 1] == '\n')
                        stop = 1;
                }
            }
            else
                printf("The server is not in the list of connected server\n");
        }
        else
            printf("Error while receiving the IP of the server\n");
    }
}

/*
Double the size of the list
*/

void extendList(struct listClientInfo *ptr)
{
    pthread_mutex_lock(&mutexListClients);
    ptr->size *= 2;

    if (realloc(ptr->list, ptr->size * sizeof(struct clientInfo)) == NULL)
        err(EXIT_FAILURE, "Error while increasing the list of client network.c");
    for (size_t i = ptr->size / 2; i < ptr->size; i++)
    {
        ptr->list[i].status = NOTUSED;
        ptr->list[i].IP = malloc(sizeof(struct sockaddr));
    } 
        
    pthread_mutex_unlock(&mutexListClients);
}

size_t findNextNotUsed(struct listClientInfo *clients)
{
    size_t res = 0;
    while (res < clients->size && clients->list[res].status != NOTUSED)
    {
        res++;
    }
    
    if(res == clients->size && clients->list[res].status != NOTUSED)
    {
        extendList(clients);
    } 

    return res;
}



/*
Creat un ptr for a struct clientInfo, set config to IPV4
*/

struct clientInfo * initClient(struct listClientInfo *clients)
{
    struct clientInfo *client = &clients->list[findNextNotUsed(clients)];
    client->status = NOTUSED;
    client->IP->sa_family = INET_ADDRSTRLEN;
    client->ID = (rand() + 1) * (rand() + 1);
    client->fd = -1;
    int tab[2];
    int tab2[2];
    pipe(tab);
    pipe(tab2);

    client->fdin = tab[0];
    client->fdoutThread = tab[1];
    client->fdinThread = tab[0];
    client->fdout = tab[1];

    return client;
}



/*
Free the client and remove it from the list of clients
*/

void freeClient(struct clientInfo client, struct listClientInfo *clients)
{
    size_t i = 0;
    pthread_mutex_lock(&mutexListClients);
    for (; i < clients->size && clients->list[i].ID != client.ID; i++)
    {
        ;
    }

    if (clients->list[i].ID == client.ID)
    {
        free(client.IP->sa_data);
        free(client.IP);
    }
    else
    {
        printf("Error while freeing, this client does not exist in the list of client\n");
    }
    pthread_mutex_unlock(&mutexListClients);
}

/*
    Remove a client without freeing it, it just change the status to NOUSED
*/

void removeClient(struct clientInfo client, struct listClientInfo *clients)
{
    size_t i = 0;
    pthread_mutex_lock(&mutexListClients);
    for (; i < clients->size && clients->list[i].ID != client.ID; i++)
    {
        ;
    }

    if (clients->list[i].ID == client.ID)
    {
        clients->list[i].status = NOTUSED;
    }
    else
    {
        printf("Error while removing, this client does not exist in the list of client\n");
    }

    pthread_mutex_unlock(&mutexListClients);
}
