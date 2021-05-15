#include "server.h"
#include "network_tools.h"
#include "client.h"

void *SendForAllClients(void *arg)
{
    struct server *server = arg;
    unsigned long long sizeData = 0;
    int type = 0;
    char headerBuff[HEADER_SIZE];
    char *messageBuff;
    int problem;
    size_t nbCharToRead;
    size_t offset;
    int r;

    //peut y avoir un souci si la taille de data depasse la taille du buffer du file descriptor
    //comportement inconnu dans ce cas la

    while (server->status == ONLINE)
    {
        offset = 0;
        r = 0;
        nbCharToRead = HEADER_SIZE;
        problem = 0;

        while (problem == 0 && nbCharToRead)
        {
            r = read(server->fdinExtern, headerBuff + offset, nbCharToRead);
            nbCharToRead -= r;
            if (r <= 0)
                problem = 1;
        }

        memcpy(&type, headerBuff, SIZE_TYPE_MSG);
        memcpy(&sizeData, headerBuff + SIZE_TYPE_MSG, SIZE_DATA_LEN_HEADER);
        messageBuff = malloc(sizeof(char) * (HEADER_SIZE + sizeData));
        memcpy(messageBuff, headerBuff, HEADER_SIZE);

        nbCharToRead = sizeData;

        while (problem == 0 && nbCharToRead)
        {
            r = read(server->fdinExtern, messageBuff + HEADER_SIZE + offset, nbCharToRead);
            nbCharToRead -= r;

            if (r > 0)
                offset += r;
            else
                problem = 1;
        }

        if (problem == 0)
            SendMessage(server->KnownServers, messageBuff);
        else
            printf("Error while receinving data in SendForAllClients\nError with function read or not enough bytes received\n");

        free(messageBuff);
    }

    return NULL;
}

void *printList(void *arg)
{
    struct server *server = arg;
    struct clientInfo *client = server->KnownServers;
    while (server->status == ONLINE)
    {
        for (client = client->sentinel->next; client->isSentinel == FALSE; client = client->next)
            printIP(&client->IP);
        printf("\n\n");
        sleep(2);
    }
    return NULL;
}

struct server *initServer(int fdinExtern, int fdoutExtern, char *IP)
{
    int fdIntern[2];
    pipe(fdIntern);

    struct server *server = malloc(sizeof(struct server));
    pthread_mutex_init(&server->lockKnownServers, NULL);
    pthread_mutex_init(&server->lockReadGlobalExtern, NULL);
    pthread_mutex_init(&server->lockReadGlobalIntern, NULL);

    server->fdinIntern = fdIntern[0];
    server->fdoutIntern = fdIntern[1];
    server->fdinExtern = fdinExtern;
    server->fdoutExtern = fdoutExtern;

    server->KnownServers = initClientList(&server->lockKnownServers, &server->lockReadGlobalIntern, &server->lockReadGlobalExtern);

    struct sockaddr_in serverIP;
    inet_pton(AF_INET, IP, &serverIP.sin_addr);
    serverIP.sin_port = htons(atoi(PORT));
    serverIP.sin_family = AF_INET;
    addClient(server->KnownServers, serverIP);

    server->status = ONLINE;

    return server;
}

void freeServer(struct server *server)
{
    freeClientList(server->KnownServers);
    pthread_mutex_destroy(&server->lockKnownServers);
    pthread_mutex_destroy(&server->lockReadGlobalIntern);
    pthread_mutex_destroy(&server->lockReadGlobalExtern);
    close(server->fdinIntern);
    close(server->fdoutIntern);
    free(server);
}

void *internComms(void *arg)
{
    struct server *server = arg;
    unsigned long long sizeData = 0;
    int type = 0;
    char headerBuff[HEADER_SIZE];
    char *messageBuff;
    int problem;
    size_t nbCharToRead;
    size_t offset;
    int r;
    size_t nbstruct;
    size_t sizeStructSockaddr_in = sizeof(struct sockaddr_in);
    struct sockaddr_in temp;

    //peut y avoir un souci si la taille de data depasse la taille du buffer du file descriptor
    //comportement inconnu dans ce cas la

    while (server->status == ONLINE)
    {
        offset = 0;
        r = 0;
        nbCharToRead = HEADER_SIZE;
        problem = 0;
        nbstruct = 0;

        while (problem == 0 && nbCharToRead)
        {
            r = read(server->fdinExtern, headerBuff + offset, nbCharToRead);
            nbCharToRead -= r;
            if (r <= 0)
                problem = 1;
        }

        memcpy(&type, headerBuff, SIZE_TYPE_MSG);
        memcpy(&sizeData, headerBuff + SIZE_TYPE_MSG, SIZE_DATA_LEN_HEADER);
        messageBuff = malloc(sizeof(char) * (sizeData));
        memcpy(messageBuff, headerBuff, HEADER_SIZE);

        nbCharToRead = sizeData;

        while (problem == 0 && nbCharToRead)
        {
            r = read(server->fdinExtern, messageBuff, nbCharToRead);
            nbCharToRead -= r;

            if (r <= 0)
                problem = 1;
        }

        if (problem == 1)
        {
            printf("Error while receinving data in InternComms\nError with function read or not enough bytes received\n");
            continue;
        }

        nbstruct = sizeData / sizeStructSockaddr_in;

        for (size_t i = 0; i < nbstruct; i++)
        {
            offset = sizeStructSockaddr_in * i;
            memcpy(&temp, messageBuff + offset, sizeStructSockaddr_in);
            if (FindClient(&temp, server->KnownServers) == NULL)
                addClient(server->KnownServers, temp);
        }

        free(messageBuff);
    }

    return NULL;
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
        messageBuff = malloc(sizeof(char) * (dataSize + HEADER_SIZE));

        memcpy(messageBuff, &type, SIZE_TYPE_MSG);
        memcpy(messageBuff + SIZE_TYPE_MSG, &dataSize, SIZE_DATA_LEN_HEADER);

        for (client = client->sentinel->next; client->isSentinel == FALSE; client = client->next)
        {
            memcpy(messageBuff + HEADER_SIZE + offset, &client->IP, sizeStructSockaddr_in);
            offset += sizeStructSockaddr_in;
        }
            

        pthread_mutex_unlock(&server->lockKnownServers);
        SendMessage(client, messageBuff);
        free(messageBuff);
        sleep(2);
    }

    return NULL;
}

int network(int *fdin, int *fdout, char *IP, char *firstserver)
{
    int printListTerm = 0;
    int fd1[2];
    int fd2[2];
    pipe(fd1);
    pipe(fd2);

    struct server *server = initServer(fd2[0], fd1[1], IP);
    *fdin = fd1[0];
    *fdout = fd2[1];

    pthread_t serverThread;
    pthread_t sendThread;
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
        addClient(server->KnownServers, firstser);
    }

    pthread_create(&serverThread, NULL, Server, (void *)server);
    pthread_create(&sendThread, NULL, SendForAllClients, (void *)server);
    pthread_create(&internCommsThread, NULL, internComms, (void *)server);
    pthread_create(&sendNetworkThread, NULL, sendNetwork, (void *)server);
    if (printListTerm == 1)
        pthread_create(&printListThread, NULL, printList, (void *)server);

    pthread_join(serverThread, NULL);
    pthread_join(sendThread, NULL);
    pthread_join(internCommsThread, NULL);
    pthread_join(sendNetworkThread, NULL);
    if (printListTerm == 1)
        pthread_join(printListThread, NULL);

    freeServer(server);

    return EXIT_SUCCESS;
}
