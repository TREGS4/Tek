#include "network.h"

void *ReWriteForAllThreads(void *arg)
{
    struct clientInfo *client = arg;
    char buff[BUFFER_SIZE_SOCKET];
    int r = 1;

    while ((r = read(client->sentinel->fdinThread, &buff, BUFFER_SIZE_SOCKET)) > 0)
    {
        for (client = client->sentinel->next; client != client->sentinel; client = client->next)
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
    struct serverInfo *server = arg;
    struct clientInfo *client = server->listClients;
    while (1)
    {
        for (client = client->sentinel->next; client != client->sentinel; client = client->next)
        {
            if (client->status == ENDED)
            {
                pthread_mutex_lock(&client->lockWrite);
                close(client->fdTofdin);
                pthread_mutex_unlock(&client->lockWrite);
            }
        }
        sleep(1);
    }

    return NULL;
}

void *printList(void *arg)
{
    struct clientInfo *client = arg;
    while (1)
    {
        for (client = client->sentinel->next; client != client->sentinel; client = client->next)
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

struct serverInfo *initServer(int fdin, int fdoutExtern, char *IP)
{
    //List part
    struct serverInfo *server = malloc(sizeof(struct serverInfo));
    struct clientInfo *client = malloc(sizeof(struct clientInfo));
    client->lockReadGlobalExtern = malloc(sizeof(pthread_mutex_t));
    client->lockReadGlobalIntern = malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init(&client->lockInfo, NULL);
    pthread_mutex_init(&client->lockRead, NULL);
    pthread_mutex_init(&client->lockWrite, NULL);
    pthread_mutex_init(client->lockReadGlobalExtern, NULL);
    pthread_mutex_init(client->lockReadGlobalIntern, NULL);
    int fdIntern[2];
    pipe(fdIntern);

    pthread_mutex_lock(&client->lockInfo);
    client->ID = 0;
    client->server = server;
    client->sentinel = client;
    client->status = SENTINEL;
    client->next = client;
    client->IPandPort.sin_family = AF_INET;
    client->prev = NULL;

    client->clientSocket = -1;
    client->fdTofdin = -1;
    client->fdinThread = fdin;
    client->fdoutExtern = fdoutExtern;
    client->fdoutIntern = fdIntern[1];

    pthread_mutex_unlock(&client->lockInfo);

    //Server part
    pthread_mutex_init(&server->lockinfo, NULL);
    struct sockaddr_in temp;
    struct sockaddr *tempbis = (struct sockaddr *)&temp;
    memset(&temp.sin_addr, 0, sizeof(temp.sin_addr));

    pthread_mutex_lock(&server->lockinfo);
    server->listClients = client;
    server->fdInInternComm = fdIntern[0];
    inet_pton(AF_INET, IP, &temp.sin_addr);
    server->IPandPort = *tempbis;
    server->IPLen = sizeof(server->IPandPort.sa_data);
    server->status = ONLINE;
    pthread_mutex_unlock(&server->lockinfo);

    return server;
}

void freeServer(struct serverInfo *server)
{
    struct clientInfo *sentinel = server->listClients->sentinel;
    struct clientInfo *temp = sentinel->next;
    struct clientInfo *temp2 = temp->next;

    while (temp != sentinel)
    {
        removeClient(temp);
        temp = temp2;
        temp2 = temp2->next;
    }

    close(sentinel->fdoutIntern);

    pthread_mutex_destroy(&sentinel->lockInfo);
    pthread_mutex_destroy(&sentinel->lockWrite);
    pthread_mutex_destroy(&sentinel->lockRead);
    pthread_mutex_destroy(sentinel->lockReadGlobalExtern);
    pthread_mutex_destroy(sentinel->lockReadGlobalIntern);
    pthread_mutex_destroy(&server->lockinfo);

    free(sentinel->lockReadGlobalExtern);
    free(sentinel->lockReadGlobalIntern);
    free(sentinel);
    free(server);
}

void *internComms(void *arg)
{
    struct serverInfo *server = arg;

    char buff[BUFFER_SIZE_SOCKET];
    char buffIP[16];
    struct sockaddr info;
    size_t nbclient = 0;
    size_t sizeclient = 14;
    unsigned long long size = 0;

    size_t nbToRead = SIZE_ULONGLONG + SIZE_TYPE_MSG;
    size_t nbchr = 0;
    int r = 1;

    while (server->status == ONLINE)
    {
        /*Header part*/

        nbToRead = SIZE_ULONGLONG + SIZE_TYPE_MSG;
        nbchr = 0;
        r = 1;

        while (server->status != EXITING && nbToRead > 0 && r > 0)
        {
            r = read(server->fdInInternComm, &buff + nbchr, nbToRead);
            nbToRead -= r;
            nbchr += r;
        }


 

		memcpy(&size, &buff[SIZE_TYPE_MSG], 8);
        
        //size = (unsigned)atoll(&buff[SIZE_TYPE_MSG]); //not working number above 9 999 999 999
        nbclient = size / sizeclient;

        //message part

        for (size_t i = 0; i < nbclient; i++)
        {
            nbToRead = 14;
            nbchr = 0;

            while (server->status != ENDED && nbToRead > 0 && r > 0)
            {
                r = read(server->fdInInternComm, &info.sa_data + nbchr, nbToRead);
                nbToRead -= r;
                nbchr += r;
            }

            nbToRead = 5;
            nbchr = 0;
            while (server->status != ENDED && nbToRead > 0 && r > 0)
            {
                r = read(server->fdInInternComm, &buff + nbchr, nbToRead);
                nbToRead -= r;
                nbchr += r;
            }

            buff[nbchr] = '\0';
            info.sa_family = (unsigned)atoi(&buff[0]);
            memset(buffIP, 0, 16);

            struct sockaddr_in *test = (struct sockaddr_in *)&info;
            inet_ntop(AF_INET, &test->sin_addr, buffIP, 16);
            int me = isInList((struct sockaddr_in *)&info, server->listClients);
            int inlist = itsme((struct sockaddr_in *)&info, (struct sockaddr_in *)&server->IPandPort);
            //printf("IP: %s\n", buffIP);
            //printf("%d\n", me);
            //printf("%d\n", inlist);

            if (inlist == 0 && me == 0)
            {

                connectClient(buffIP, server->listClients);
            }
        }
    }
    return NULL;
}

void *sendNetwork(void *arg)
{
    struct serverInfo *server = arg;
    struct clientInfo *client = server->listClients->sentinel->next;
    unsigned long long size = 0;
    int type = 1;
    size_t headersize = SIZE_ULONGLONG + SIZE_TYPE_MSG;
    size_t datasize = 14 + 5;
    char buffh[headersize];
    char buff[datasize];
    memset(buff, 0, datasize);
    memset(buffh, 0, headersize);

    while (server->status == ONLINE)
    {
        pthread_mutex_lock(&server->mutexfdtemp);
        size = datasize * (listLen(server->listClients) + 1);
        memcpy(buffh, &type, SIZE_TYPE_MSG);
        memcpy(buffh + 1, &size, SIZE_ULONGLONG);
        //srintf(buffh, "%03d%019llu", type, size);
        //printf("%s", buffh);
        write(server->fdtemp, buffh, headersize);
        client = client->sentinel->next;
        for (size_t i = 0; i < listLen(server->listClients); i++)
        {
            if (client->status == CONNECTED)
            {
                write(server->fdtemp, &((struct sockaddr *)&client->IPandPort)->sa_data, 14);
                //printf("%s", client->IPandPort.sa_data);
                sprintf(buff, "%05u", ((struct sockaddr *)&client->IPandPort)->sa_family);
                //printf("%s", buff);
                write(server->fdtemp, buff, 5);
                client = client->next;
            }
        }
        write(server->fdtemp, &server->IPandPort.sa_data, 14);
        //printf("%s", server->IPandPort.sa_data);
        sprintf(buff, "%05u", server->IPandPort.sa_family);
        write(server->fdtemp, buff, 5);
        //printf("%s\n", buff);
        pthread_mutex_unlock(&server->mutexfdtemp);
        sleep(2);
    }

    return NULL;
}

int network(int *fdin, int *fdout, pthread_mutex_t *mutexfd, char *IP, char *firstserver)
{
    int fd1[2];
    int fd2[2];
    pipe(fd1);
    pipe(fd2);

    struct serverInfo *serverInf = initServer(fd2[0], fd1[1], IP);
    serverInf->fdtemp = fd2[1];

    pthread_mutex_init(&serverInf->mutexfdtemp, NULL);
    mutexfd = &serverInf->mutexfdtemp;
    fdin = &fd1[0];
    fdout = &fd2[1];

    fdin += 1;
    fdout += 1;
    mutexfd += 1;




    pthread_t serverThread;
    pthread_t maintenerThread;
    pthread_t reWriteThread;
    pthread_t closeConnectionThread;
    pthread_t internCommsThread;
    pthread_t sendNetworkThread;
    //pthread_t printListThread;

    connectClient(firstserver, serverInf->listClients);

    pthread_create(&serverThread, NULL, server, (void *)serverInf);
    pthread_create(&maintenerThread, NULL, connectionMaintener, (void *)serverInf);
    pthread_create(&reWriteThread, NULL, ReWriteForAllThreads, (void *)serverInf->listClients);
    pthread_create(&closeConnectionThread, NULL, closeConnection, (void *)serverInf);
    pthread_create(&internCommsThread, NULL, internComms, (void *)serverInf);
    pthread_create(&sendNetworkThread, NULL, sendNetwork, (void *)serverInf);
    //pthread_create(&printListThread, NULL, printList, (void *)serverInf->listClients);

    pthread_join(serverThread, NULL);
    pthread_join(maintenerThread, NULL);
    pthread_join(reWriteThread, NULL);
    pthread_join(closeConnectionThread, NULL);
    pthread_join(internCommsThread, NULL);
    pthread_join(sendNetworkThread, NULL);
    //pthread_join(printListThread, NULL);

    freeServer(serverInf);

    return EXIT_SUCCESS;
}

void printIP(struct sockaddr_in *IP)
{
    char *buff = malloc(16 * sizeof(char));

    unsigned int port = ntohs(IP->sin_port);
    inet_ntop(AF_INET, &IP->sin_addr, buff, 16);

    printf("%s:%u\n", buff, port);
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

    while (client != client->sentinel)
    {
        if (client->status == CONNECTED)
            res++;
        client = client->next;
    }

    return res;
}

int isInList(struct sockaddr_in *tab, struct clientInfo *list)
{
    int find = 0;
    list = list->sentinel->next;

    while (find == 0 && list != list->sentinel)
    {
        find = itsme(tab, &list->IPandPort);
        list = list->next;
    }

    return find;
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

    terminator(client);

    pthread_mutex_destroy(&client->lockInfo);
    pthread_mutex_destroy(&client->lockWrite);
    pthread_mutex_destroy(&client->lockRead);

    free(client);

    return EXIT_SUCCESS;
}
