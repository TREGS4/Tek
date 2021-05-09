#include "server.h"
#include "network_tools.h"

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
    struct sockaddr_in temp;
    struct sockaddr *tempbis = (struct sockaddr *)&temp;
    memset(&temp.sin_addr, 0, sizeof(temp.sin_addr));


    pthread_mutex_lock(&server->lockinfo);
    pthread_mutex_lock(&server->listClients->lockInfo);
    server->listClients->server = server;
    pthread_mutex_unlock(&server->listClients->lockInfo);
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
    freeClientList(server->listClients);
    pthread_mutex_destroy(&server->lockinfo);
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

    size_t nbToRead = SIZE_DATA_LEN_HEADER + SIZE_TYPE_MSG;
    size_t nbchr = 0;
    int r = 1;

    while (server->status == ONLINE)
    {
        /*Header part*/

        nbToRead = SIZE_DATA_LEN_HEADER + SIZE_TYPE_MSG;
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
    size_t headersize = SIZE_DATA_LEN_HEADER + SIZE_TYPE_MSG;
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
        memcpy(buffh + 1, &size, SIZE_DATA_LEN_HEADER);
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
    *mutexfd = serverInf->mutexfdtemp;
    *fdin = fd1[0];
    *fdout = fd2[1];

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
