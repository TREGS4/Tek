#include "client.h"

void *printList(void *arg)
{
    struct server *server = arg;
    struct clientInfo *client = server->KnownServers;
    while (server->status != EXITING)
    {
        for (client = client->sentinel->next; client->isSentinel == FALSE; client = client->next)
        {
            ; //printHostname(client->address);
        }

        printf("\n\n");
        sleep(2);
    }
    return NULL;
}

struct server *initServer()
{
    struct server *server = malloc(sizeof(struct server));
    pthread_mutex_init(&server->lockKnownServers, NULL);
    pthread_mutex_init(&server->lockStatus, NULL);
    server->IncomingMessages = shared_queue_new();
    server->OutgoingMessages = shared_queue_new();

    server->KnownServers = initClientList(&server->lockKnownServers);

    pthread_mutex_lock(&server->lockStatus);
    server->status = STARTING;
    pthread_mutex_unlock(&server->lockStatus);

    return server;
}

void freeServer(struct server *server)
{
    pthread_mutex_lock(&server->lockKnownServers);
    freeClientList(server->KnownServers);
    pthread_mutex_unlock(&server->lockKnownServers);
    pthread_mutex_destroy(&server->lockKnownServers);
    pthread_mutex_destroy(&server->lockStatus);
    shared_queue_destroy(server->IncomingMessages);
    shared_queue_destroy(server->OutgoingMessages);
    free(server);
}

void *SendOutgoinMessages(void *arg)
{
    struct server *server = arg;

    while (server->status != EXITING)
    {
        MESSAGE messsage = shared_queue_pop(server->OutgoingMessages);
        SendMessage(server->KnownServers, messsage);
        DestroyMessage(messsage);
    }

    return NULL;
}

int Network(struct server *server, char *hostname, char *port, char *hostnameFirstServer, char *portFirstServer)
{
    int printListTerm = FALSE;

    pthread_t serverThread;
    pthread_t sendOutgoingMessageThread;
    pthread_t sendNetworkThread;
    pthread_t printListThread;

    server->address.hostname = hostname;
    memset(server->address.port, 0, PORT_SIZE + 1);

    if (port != NULL)
        memcpy(server->address.port, port, strlen(port));
    else
        memcpy(server->address.port, DEFAULT_PORT, strlen(DEFAULT_PORT));

    pthread_create(&serverThread, NULL, Server, (void *)server);
    pthread_create(&sendOutgoingMessageThread, NULL, SendOutgoinMessages, (void *)server);
    pthread_create(&sendNetworkThread, NULL, sendNetwork, (void *)server);

    if (printListTerm == TRUE)
        pthread_create(&printListThread, NULL, printList, (void *)server);

    while (server->status != ONLINE)
        sleep(0.01);

    pthread_mutex_lock(&server->lockKnownServers);
    addClient(server->KnownServers, server->address);
    pthread_mutex_unlock(&server->lockKnownServers);

    if (hostnameFirstServer != NULL)
    {
        struct address address;
        address.hostname = hostnameFirstServer;
        memset(address.port, 0, PORT_SIZE + 1);

        if (portFirstServer != NULL)
            memcpy(address.port, portFirstServer, strlen(portFirstServer));
        else
            memcpy(address.port, DEFAULT_PORT, strlen(DEFAULT_PORT));

        pthread_mutex_lock(&server->lockKnownServers);
        addClient(server->KnownServers, address);
        pthread_mutex_unlock(&server->lockKnownServers);
    }
    else
        printf("No server to connect to...\n");

    pthread_join(serverThread, NULL);
    pthread_join(sendOutgoingMessageThread, NULL);
    pthread_join(sendNetworkThread, NULL);
    if (printListTerm == 1)
        pthread_join(printListThread, NULL);

    return EXIT_SUCCESS;
}
