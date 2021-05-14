#include "server.h"
#include "network_tools.h"
#include "client.h"



void *ReWriteForAllThreads(void *arg)
{
    struct serverInfo *server = arg;
    struct clientInfo *client = server->listClients;
    unsigned long long len = 0;
    char buffh[HEADER_SIZE];
    char *buff;
    int r = 1;
    unsigned long long nbToRead = 0;
    unsigned long long nbchr = 0;
printf("here");
    while ((r = read(client->sentinel->fdinThread, &buffh, HEADER_SIZE)) > 0 && server->status == ONLINE)
    {
        memcpy(buffh + SIZE_TYPE_MSG, &len, SIZE_DATA_LEN_HEADER);
        memcpy(buff, buffh, HEADER_SIZE);
        buff = malloc(sizeof(char) * (HEADER_SIZE + len));

        
        while (server->status == ONLINE && nbToRead > 0)
        {
            r = read(client->sentinel->fdinThread, buff + HEADER_SIZE + nbchr, nbToRead);
			nbToRead -= r;
			nbchr += r;
        }

        pthread_mutex_lock(&server->mutextmessage);
        SendMessage(client, buff);
        pthread_mutex_unlock(&server->mutextmessage);
    }

    return NULL;
}

void *removeClientThread(void *arg)
{
    struct serverInfo *server = arg;
    struct clientInfo *client = server->listClients;
    struct clientInfo *toRemove;

    while(server->status != EXITING)
    {
        for (client = client->sentinel->next; client != client->sentinel; client = client->next)
        {
            if (client->status == ERROR)
            {
                toRemove = client;
                client = client->next;
                removeClient(toRemove);
            }
        }
        sleep(0.5);
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
            case NOTCONNECTED:
                printf("%lu is not connected\n", client->ID);
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
            printIP(&client->IPandPort);
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
    pthread_mutex_init(&server->mutextmessage, NULL);

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
    pthread_mutex_destroy(&server->mutextmessage);
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

            //printf("IP: %s\n", buffIP);
            //printf("%d\n", me);
            //printf("%d\n", inlist);

            if (itsme(&client, &server->IPandPort) == 0 &&  isInList(&client, server->listClients) == NULL)
            {
                addClient(client, server->listClients);
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
    size_t buffsize = HEADER_SIZE + datasize;
    char buff[buffsize];

    memcpy(buff, &type, SIZE_TYPE_MSG);
    memcpy(buff + SIZE_TYPE_MSG, &datasize, SIZE_DATA_LEN_HEADER);

    while (server->status == ONLINE)
    {
        pthread_mutex_lock(&server->lockinfo);
        memcpy(buff + HEADER_SIZE, &server->IPandPort, datasize);
        pthread_mutex_unlock(&server->lockinfo);

        pthread_mutex_lock(&server->mutexfdtemp);
        SendMessage(server->listClients, buff);
        pthread_mutex_unlock(&server->mutexfdtemp);

        for (client = client->sentinel->next; client->next != client->sentinel; client = client->next)
        {
            pthread_mutex_lock(&client->lockInfo);
            memcpy(buff + HEADER_SIZE, &client->IPandPort, datasize);
            pthread_mutex_unlock(&client->lockInfo);

            pthread_mutex_lock(&server->mutextmessage);
            SendMessage(server->listClients, buff);
            pthread_mutex_unlock(&server->mutextmessage);
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

    pthread_t serverThread;
    pthread_t removeThread;
    pthread_t reWriteThread;
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
        addClient(firstser, serverInf->listClients);
    }

    pthread_create(&serverThread, NULL, server, (void *)serverInf);
    pthread_create(&removeThread, NULL, removeClientThread, (void *)serverInf);
    pthread_create(&reWriteThread, NULL, ReWriteForAllThreads, (void *)serverInf);
    pthread_create(&internCommsThread, NULL, internComms, (void *)serverInf);
    pthread_create(&sendNetworkThread, NULL, sendNetwork, (void *)serverInf);
    if (printListTerm == 1)
        pthread_create(&printListThread, NULL, printList, (void *)serverInf->listClients);

    pthread_join(serverThread, NULL);
    pthread_join(removeThread, NULL);
    pthread_join(reWriteThread, NULL);
    pthread_join(internCommsThread, NULL);
    pthread_join(sendNetworkThread, NULL);
    if (printListTerm == 1)
        pthread_join(printListThread, NULL);

    freeServer(serverInf);

    return EXIT_SUCCESS;
}
