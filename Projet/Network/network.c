#include "client.h"

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

struct server *initServer()
{
    struct server *server = malloc(sizeof(struct server));
    pthread_mutex_init(&server->lockKnownServers, NULL);
    server->IncomingMessages = shared_queue_new();
    server->OutgoingMessages = shared_queue_new();

    server->KnownServers = initClientList(&server->lockKnownServers);

    server->status = ONLINE;

    return server;
}

void freeServer(struct server *server)
{
    pthread_mutex_lock(&server->lockKnownServers);
    freeClientList(server->KnownServers);
    pthread_mutex_lock(&server->lockKnownServers);
    pthread_mutex_destroy(&server->lockKnownServers);
    shared_queue_destroy(server->IncomingMessages);
    shared_queue_destroy(server->OutgoingMessages);
    free(server);
}

void *SendOutgoinMessages(void *arg)
{
    struct server *server = arg;

    while (server->status == ONLINE)
    {
        MESSAGE messsage = shared_queue_pop(server->OutgoingMessages);
        SendMessage(server->KnownServers, messsage);
        DestroyMessage(messsage);
    }

    return NULL;
}

int Network(struct server *server, char *IP, char *firstserver)
{
    int printListTerm = TRUE;

    pthread_t serverThread;
    pthread_t sendOutgoingMessageThread;
    pthread_t sendNetworkThread;
    pthread_t printListThread;

    struct sockaddr_in serverIP;
    memset(&serverIP, 0, sizeof(struct sockaddr_in));
    inet_pton(AF_INET, IP, &serverIP.sin_addr);
    serverIP.sin_family = AF_INET;
    serverIP.sin_port = htons(atoi(PORT));

    pthread_mutex_lock(&server->lockKnownServers);
    addClient(server->KnownServers, serverIP);
    pthread_mutex_unlock(&server->lockKnownServers);

    if (firstserver != NULL)
    {
        struct sockaddr_in firstser;
        memset(&firstser, 0, sizeof(struct sockaddr_in));
        inet_pton(AF_INET, firstserver, &firstser.sin_addr);
        firstser.sin_family = AF_INET;
        firstser.sin_port = htons(atoi(PORT));
        pthread_mutex_lock(&server->lockKnownServers);
        addClient(server->KnownServers, firstser);
        pthread_mutex_unlock(&server->lockKnownServers);
    }
    else
        printf("No server to connect to...\n");

    pthread_create(&serverThread, NULL, Server, (void *)server);
    pthread_create(&sendOutgoingMessageThread, NULL, SendOutgoinMessages, (void *)server);
    pthread_create(&sendNetworkThread, NULL, sendNetwork, (void *)server);

    if (printListTerm == TRUE)
        pthread_create(&printListThread, NULL, printList, (void *)server);

    pthread_join(serverThread, NULL);
    pthread_join(sendOutgoingMessageThread, NULL);
    pthread_join(sendNetworkThread, NULL);
    if (printListTerm == 1)
        pthread_join(printListThread, NULL);

    freeServer(server);

    return EXIT_SUCCESS;
}
