#include "server.h"
#include "network_tools.h"

void Send(int fd, const void *buf, size_t count, int flag)
{
    ssize_t r = 0;
    size_t reste = count;

    while (r >= 0 && (r = send(fd, buf + count - reste, count, flag)) < (long int)reste)
        reste -= r;

    if (r < 0)
        err(3, "Error while rewriting");
}


void SendMessage(char *str, int fd, unsigned long long len, char type)
{
    unsigned long long datasize = len;
    size_t headersize = SIZE_DATA_LEN_HEADER + SIZE_TYPE_MSG;
    char buffh[headersize];

    memcpy(buffh, &type, SIZE_TYPE_MSG);
    memcpy(buffh + SIZE_TYPE_MSG, &datasize, SIZE_DATA_LEN_HEADER);

    write(fd, buffh, headersize);
    write(fd, str, datasize);
}



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
    int fdIntern[2];
    pipe(fdIntern);

    struct serverInfo *server = malloc(sizeof(struct serverInfo));
    server->listClients = initClientList(fdin, fdoutExtern, fdIntern[1]);

    pthread_mutex_init(&server->lockinfo, NULL);

    pthread_mutex_lock(&server->lockinfo);
    pthread_mutex_lock(&server->listClients->lockInfo);
    server->listClients->server = server;
    pthread_mutex_unlock(&server->listClients->lockInfo);

    server->fdInInternComm = fdIntern[0];
    inet_pton(AF_INET, IP, &server->IPandPort.sin_addr);
    server->IPandPort.sin_port = htons(atoi(PORT));
    server->IPLen = sizeof(server->IPandPort);
    server->status = ONLINE;
    pthread_mutex_unlock(&server->lockinfo);

    return server;
}

void freeServer(struct serverInfo *server)
{
    freeClientList(server->listClients);
    pthread_mutex_destroy(&server->lockinfo);
    free(server);
}

void *internComms(void *arg)
{
    struct serverInfo *server = arg;

    char buff[BUFFER_SIZE_SOCKET];
    struct sockaddr_in client;
    size_t nbclient = 0;
    size_t sizeclient = sizeof(struct sockaddr_in);
    unsigned long long size = 0;

    size_t nbToRead;
    size_t nbchr;
    int r;

    while (server->status == ONLINE)
    {
        /*Header part*/

        nbToRead = HEADER_SIZE;
        nbchr = 0;
        r = 1;

        while (server->status != EXITING && nbToRead > 0 && r > 0)
        {
            r = read(server->fdInInternComm, &buff + nbchr, nbToRead);
            nbToRead -= r;
            nbchr += r;
        }

        memcpy(&size, &buff[SIZE_TYPE_MSG], 8);
        nbclient = size / sizeclient;

        //message part

        for (size_t i = 0; i < nbclient; i++)
        {
            nbToRead = sizeclient;
            nbchr = 0;

            while (server->status != EXITING && nbToRead > 0 && r > 0)
            {
                r = read(server->fdInInternComm, &client + nbchr, nbToRead);
                nbToRead -= r;
                nbchr += r;
            }

            client.sin_port = htons(atoi(PORT));

            int me = isInList(&client, server->listClients);
            int inlist = itsme(&client, &server->IPandPort);
            //printf("IP: %s\n", buffIP);
            //printf("%d\n", me);
            //printf("%d\n", inlist);

            if (inlist == 0 && me == 0)
            {
                connectClient(&client, server->listClients);
            }
        }
    }
    return NULL;
}

void *sendNetwork(void *arg)
{
    struct serverInfo *server = arg;
    struct clientInfo *client = server->listClients->sentinel->next;
    unsigned long long datasize = sizeof(struct sockaddr_in);
    char type = 1;
    size_t headersize = SIZE_DATA_LEN_HEADER + SIZE_TYPE_MSG;
    char buffh[headersize];
    char buff[datasize];

    memcpy(buffh, &type, SIZE_TYPE_MSG);
    memcpy(buffh + SIZE_TYPE_MSG, &datasize, SIZE_DATA_LEN_HEADER);

    while (server->status == ONLINE)
    {
        pthread_mutex_lock(&server->lockinfo);
        memcpy(buff, &server->IPandPort, datasize);
        pthread_mutex_unlock(&server->lockinfo);

        pthread_mutex_lock(&server->mutexfdtemp);
        write(server->fdtemp, buffh, headersize);
        write(server->fdtemp, buff, datasize);
        pthread_mutex_unlock(&server->mutexfdtemp);

        for (client = client->sentinel->next; client->next != client->sentinel; client = client->next)
        {
            pthread_mutex_lock(&client->lockInfo);
            if (client->status == CONNECTED)
            {
                memcpy(buff, &client->IPandPort, datasize);

                pthread_mutex_lock(&server->mutexfdtemp);
                write(server->fdtemp, buffh, headersize);
                write(server->fdtemp, buff, datasize);
                pthread_mutex_unlock(&server->mutexfdtemp);
            }
            pthread_mutex_unlock(&client->lockInfo);
        }
        sleep(2);
    }

    return NULL;
}

int network(int *fdin, int *fdout, pthread_mutex_t *mutexfd, char *IP, char *firstserver)
{
    int printListTerm = 0;
    int fd1[2];
    int fd2[2];
    pipe(fd1);
    pipe(fd2);

    struct serverInfo *serverInf = initServer(fd2[0], fd1[1], IP);
    serverInf->fdtemp = fd2[1];

    pthread_mutex_init(&serverInf->mutexfdtemp, NULL);
    *mutexfd = serverInf->mutexfdtemp;
    *fdin = fd1[0];
    *fdout = fd2[1];

    //write(fd1[1], "test\n", 5);
    printf("file descriptor network: %d\n", fd1[1]);

    pthread_t serverThread;
    pthread_t maintenerThread;
    pthread_t reWriteThread;
    pthread_t closeConnectionThread;
    pthread_t internCommsThread;
    pthread_t sendNetworkThread;
    pthread_t printListThread;

    if (firstserver != NULL)
    {
        struct sockaddr_in firstser;
        memset(&firstser, 0, sizeof(struct sockaddr_in));
        inet_pton(AF_INET, firstserver, &firstser.sin_addr);
        firstser.sin_family = AF_INET;
        firstser.sin_port = htons(atoi(PORT));
        connectClient(&firstser, serverInf->listClients);
    }

    pthread_create(&serverThread, NULL, server, (void *)serverInf);
    pthread_create(&maintenerThread, NULL, connectionMaintener, (void *)serverInf);
    pthread_create(&reWriteThread, NULL, ReWriteForAllThreads, (void *)serverInf->listClients);
    pthread_create(&closeConnectionThread, NULL, closeConnection, (void *)serverInf);
    pthread_create(&internCommsThread, NULL, internComms, (void *)serverInf);
    pthread_create(&sendNetworkThread, NULL, sendNetwork, (void *)serverInf);
    if (printListTerm == 1)
        pthread_create(&printListThread, NULL, printList, (void *)serverInf->listClients);

    pthread_join(serverThread, NULL);
    pthread_join(maintenerThread, NULL);
    pthread_join(reWriteThread, NULL);
    pthread_join(closeConnectionThread, NULL);
    pthread_join(internCommsThread, NULL);
    pthread_join(sendNetworkThread, NULL);
    if (printListTerm == 1)
        pthread_join(printListThread, NULL);

    freeServer(serverInf);

    return EXIT_SUCCESS;
}
