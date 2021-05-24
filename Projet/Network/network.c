#include "client.h"
#include "server.h"

/*
*   Initalise a server structure and return its pointer.
*   If any error occurs return a null pointer.
*/
struct server *initServer()
{
    struct server *server = malloc(sizeof(struct server));
    if (server != NULL)
    {
        int problemM1 = 0;
        int problemM2 = 0;
        server->KnownServers = NULL;
        server->IncomingMessages = NULL;
        server->OutgoingMessages = NULL;
        problemM1 = pthread_mutex_init(&server->lockKnownServers, NULL);
        problemM2 = pthread_mutex_init(&server->lockStatus, NULL);

        if (problemM1 == 0 && problemM2 == 0 && (server->KnownServers = initClientList(&server->lockKnownServers)) != NULL && (server->IncomingMessages = shared_queue_new()) != NULL && (server->OutgoingMessages = shared_queue_new()) != NULL)
        {
            pthread_mutex_lock(&server->lockStatus);
            server->status = STARTING;
            pthread_mutex_unlock(&server->lockStatus);
        }
        else
        {
            if (server->KnownServers != NULL)
                freeClientList(server->KnownServers);

            if (server->IncomingMessages != NULL)
                shared_queue_destroy(server->IncomingMessages);

            if (server->OutgoingMessages != NULL)
                shared_queue_destroy(server->OutgoingMessages);

            if (problemM1 != 0)
                pthread_mutex_destroy(&server->lockKnownServers);

            if (problemM2 != 0)
                pthread_mutex_destroy(&server->lockStatus);
        }
    }

    return server;
}

/*
*   Free properly the structure server pass in argument and all dynamic memory allocated in it. 
*/
void freeServer(struct server *server)
{
    //list
    pthread_mutex_lock(&server->lockKnownServers);
    freeClientList(server->KnownServers);
    pthread_mutex_unlock(&server->lockKnownServers);

    //mutex
    pthread_mutex_destroy(&server->lockKnownServers);
    pthread_mutex_destroy(&server->lockStatus);

    //shared_queue
    shared_queue_destroy(server->IncomingMessages);
    shared_queue_destroy(server->OutgoingMessages);

    //malloc
    free(server->address.hostname);
    free(server);
}

/*
*   Function always trying to pop message and send them to the clients.
*   This function is running in a thread.
*/
void *SendOutGoinMessages(void *arg)
{
    struct server *server = arg;

    while (server->status != EXITING)
    {
        MESSAGE *messsage = shared_queue_pop(server->OutgoingMessages);
        SendMessage(server->KnownServers, messsage);
        DestroyMessage(messsage);
    }

    return NULL;
}

/*
*   Main function of the peer to peer network
*/
int Network(struct server *server, char *hostname, char *port, char *hostnameFirstServer, char *portFirstServer)
{
    pthread_t serverThread;
    pthread_t sendOutGoingMessageThread;
    pthread_t sendNetworkThread;
    int problem = 0;
    struct address addressFirstServer;

    /*
    *   Perform test on arguments to avoids problem as much as possible
    */

    /*
    *=========================================TEST==================================
    */
    if (server == NULL)
    {
        fprintf(stderr, "Error while starting network: hostname is NULL\n");
        return EXIT_FAILURE;
    }
    if (hostname == NULL)
    {
        fprintf(stderr, "Error while starting network: server is NULL\n");
        return EXIT_FAILURE;
    }
    if (port != NULL && strlen(port) > PORT_SIZE)
    {
        fprintf(stderr, "Error while starting network: port is too long\n");
        return EXIT_FAILURE;
    }
    if (portFirstServer != NULL && strlen(portFirstServer) > PORT_SIZE)
    {
        fprintf(stderr, "Error while starting network: port for the first server is too long\n");
        return EXIT_FAILURE;
    }

    /*
    *==============================================================================
    */

    /*
    *   Setting up the hostname and port of the server, it's a copie of the args so they can be freed
    */
    /*
    *=============================SETTING UP SERVER AND FIRST SERVER================
    */
    server->address.hostname = calloc(1, strlen(hostname) + 1);
    if (server->address.hostname == NULL)
    {
        fprintf(stderr, "Error when calloc for server's hostname in Network function\n");
        return EXIT_FAILURE;
    }
    memcpy(server->address.hostname, hostname, strlen(hostname));
    memset(server->address.port, 0, PORT_SIZE + 1);

    if (port != NULL)
        memcpy(server->address.port, port, strlen(port));
    else
        memcpy(server->address.port, DEFAULT_PORT, strlen(DEFAULT_PORT));

    /*
    *   Create the first server to connect to if the pointer is not null
    */

    if (hostnameFirstServer != NULL)
    {
        addressFirstServer.hostname = calloc(1, strlen(hostnameFirstServer) + 1);
        memcpy(addressFirstServer.hostname, hostnameFirstServer, strlen(hostnameFirstServer));
        memset(addressFirstServer.port, 0, PORT_SIZE + 1);

        if (portFirstServer != NULL)
            memcpy(addressFirstServer.port, portFirstServer, strlen(portFirstServer));
        else
            memcpy(addressFirstServer.port, DEFAULT_PORT, strlen(DEFAULT_PORT));
    }
    else
        printf("No server to connect to...\n");

    /*
    *==============================================================================
    */

    /*
    * Launch our three main thread
    *   - Server is accepting all incomming connection
    *   - SendOutGoingMessages pop any element of the the queue outgoingmessages and send it
    *   - SendNetwork send regularly to all the known servers our list of known servers.
    */
    /*
    *====================================STARTIGN THREAD============================
    */
    problem = pthread_create(&serverThread, NULL, Server, (void *)server);
    if (problem != 0)
        problem = -1;

    if (problem == 0)
    {
        problem = pthread_create(&sendOutGoingMessageThread, NULL, SendOutGoinMessages, (void *)server);
        if (problem != 0)
            problem = -2;
    }

    if (problem == 0)
    {
        problem = pthread_create(&sendNetworkThread, NULL, sendNetwork, (void *)server);
        if (problem != 0)
            problem = -3;
    }

    /*
    *==============================================================================
    */

    /*
    *==========================ADDING SERVER AND FIRST SERVER======================
    */
    if (problem == 0)
    {
        /*
        *   Wait for the server thread to be ONLINE before adding any serving.
        *   A test is perform to see if the server is online, if not the server is not
        *   added to the list. The first server we are contaction is ourselves so we need to
        *   wait.
        */
        while (server->status != ONLINE)
            sleep(0.01);

        pthread_mutex_lock(&server->lockKnownServers);
        if (addClient(server->KnownServers, server->address) == NULL)
            problem = 1;
        if (problem == 0 && hostnameFirstServer != NULL)
        {
            if (addClient(server->KnownServers, addressFirstServer) == NULL)
                problem = 2;
        }

        pthread_mutex_unlock(&server->lockKnownServers);
    }

    /*
    *==============================================================================
    */

    /*
    *   Wainting for our main threads before ending
    */
    /*
    *====================================ENDING====================================
    */
    if (problem != 0)
    {
        pthread_mutex_lock(&server->lockStatus);
        server->status = EXITING;
        pthread_mutex_unlock(&server->lockStatus);
    }

    if (problem >= 0)
    {
        pthread_join(serverThread, NULL);
        pthread_join(sendOutGoingMessageThread, NULL);
        pthread_join(sendNetworkThread, NULL);
    }
    else if (problem < -1)
    {
        pthread_join(serverThread, NULL);
        if (problem < -2)
            pthread_join(sendOutGoingMessageThread, NULL);

        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
    /*
    *==============================================================================
    */
}
