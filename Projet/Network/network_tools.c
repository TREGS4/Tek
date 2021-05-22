#include "network_tools.h"

struct clientInfo *initClientList()
{
    struct clientInfo *sentinel = malloc(sizeof(struct clientInfo));
    memset(&sentinel->address.port, 0, PORT_SIZE + 1);
    sentinel->address.hostname = NULL;
    sentinel->sentinel = sentinel;
    sentinel->prev = NULL;
    sentinel->next = sentinel;
    sentinel->isSentinel = TRUE;

    return sentinel;
}

void freeClientList(struct clientInfo *clientList)
{
    clientList = clientList->sentinel;

    while (clientList->next->isSentinel == FALSE)
        removeClient(clientList->next);

    free(clientList);
}

struct clientInfo *addClient(struct clientInfo *list, struct address address)
{
    int skt = -1;

    if(address.hostname == NULL)
        return NULL;

    if ((skt = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Can't create the socket, while trying to test the client before add it");
        return NULL;
    }

    struct sockaddr_in IP = GetIPfromHostname(address);

    if (connect(skt, (struct sockaddr *)&IP, sizeof(struct sockaddr_in)) < 0)
    {
        perror("Can't add the client");
        return NULL;
    }

    close(skt);
    struct clientInfo *client = malloc(sizeof(struct clientInfo));

    client->isSentinel = FALSE;
    client->address = address;

    client->next = list->next;
    client->prev = list;
    list->next = client;
    if (client->next != NULL)
        client->next->prev = client;

    client->sentinel = list->sentinel;

    return NULL;
}

int removeClient(struct clientInfo *client)
{
    if (client->isSentinel == TRUE)
    {
        fprintf(stderr, "Can't remove the sentinel\n");
        return EXIT_FAILURE;
    }

    client->prev->next = client->next;
    client->next->prev = client->prev;

    free(client->address.hostname);
    free(client);

    return EXIT_SUCCESS;
}

size_t listLen(struct clientInfo *client)
{
    size_t res = 0;

    for (client = client->sentinel->next; client->isSentinel == FALSE; client = client->next)
    {
        res++;
    }

    return res;
}

struct clientInfo *last(struct clientInfo *client)
{
    while (client->next->isSentinel == FALSE)
        client = client->next;

    return client;
}

int sameIP(struct address addr1, struct sockaddr_in *second)
{
    int me = FALSE;

    if (memcmp(&first->sin_addr, &second->sin_addr, sizeof(struct in_addr)) == 0)
        me = TRUE;

    return me;
}

struct clientInfo *FindClient(struct sockaddr_in *tab, struct clientInfo *list)
{
    struct clientInfo *res = NULL;
    list = list->sentinel->next;

    while (res == NULL && list->isSentinel == FALSE)
    {
        if (sameIP(tab, &list->IP) == TRUE)
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

void addServerFromMessage(MESSAGE message, struct server *server)
{
    size_t offset = 0;
    size_t nbstruct = 1;
    struct adress temp;

    //peut y avoir un souci si la taille de data depasse la taille du buffer du file descriptor
    //comportement inconnu dans ce cas la

    for (size_t i = 0; i < nbstruct; i++)
    {
        unsigned int16 size = 0;
        memcpy(&size, message.data + offset, HEADER_HOSTNAME_SIZE);
        temp.hostname = malloc(sizeof(char) * (size - PORT_SIZE - 1);
        memcpy(temp, message.sizeData + offset + HEADER_HOSTNAME_SIZE, size);

        pthread_mutex_lock(&server->lockKnownServers);
        if (FindClient(&temp, server->KnownServers) == NULL)
            addClient(server->KnownServers, temp);
        pthread_mutex_unlock(&server->lockKnownServers);
        
        offset += size;
    }
}

void *sendNetwork(void *arg)
{
    struct server *server = arg;
    struct clientInfo *client = server->KnownServers;
    char type = TYPE_NETWORK;
    unsigned long long dataSize = 0;
    char *messageBuff;
    size_t offset = 0;

    while (server->status != EXITING)
    {
        dataSize = 0;
        offset = 0;

        pthread_mutex_lock(&server->lockKnownServers);

        for(struct clientInfo *temp = server->KnownServers; temp != NULL; temp = temp->next)
            dataSize += strlen(temp->address.hostname) + 1 + PORT_SIZE + 1 + HEADER_HOSTNAME_SIZE;

        messageBuff = malloc(sizeof(char) * dataSize);

        for (client = client->sentinel->next; client->isSentinel == FALSE; client = client->next)
        {
            unsigned int16 size = strlen(client->address.hostname) + 1 + PORT_SIZE + 1;
            memcpy(messageBuff + offset, &size, HEADER_HOSTNAME_SIZE);

            memcpy(messageBuff + offset + HEADER_HOSTNAME_SIZE, &client->address, size);
            offset += size;
        }

        pthread_mutex_unlock(&server->lockKnownServers);

        MESSAGE message = CreateMessage(type, dataSize, messageBuff);
        shared_queue_push(server->OutgoingMessages, message);

        free(messageBuff);
        sleep(2);
    }

    return NULL;
}

struct sockaddr_in GetIPfromHostname(struct address address)
{
    struct sockaddr_in resIP;
    struct addrinfo hints, *res;
    memset(&res, 0, sizeof(struct sockaddr_in));
	memset(&hints, 0, sizeof(struct addrinfo));

	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	getaddrinfo(address.hostname, server->port, &hints, &res);
    resIP = (struct sockaddr_in)*res->ai_addr;
    freeaddrinfo(res);

    return resIP;
}