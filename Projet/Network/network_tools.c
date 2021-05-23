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

    if (address.hostname == NULL)
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
    memcpy(&client->address, &address, sizeof(struct address));

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

int sameIP(struct address addr1, struct address addr2)
{
    int me = FALSE;

    if (strlen(addr1.hostname) == strlen(addr2.hostname))
        me = TRUE;
    if (me == TRUE && memcmp(&addr1.hostname, &addr2.hostname, strlen(addr1.hostname)) == 0)
        me = TRUE;

    return me;
}

struct clientInfo *FindClient(struct address addr, struct clientInfo *list)
{
    struct clientInfo *res = NULL;
    list = list->sentinel->next;

    printf("Len: %lu\n", strlen(addr.hostname));
    printf("Addr: %s\nPort: %s\n", addr.hostname, addr.port);
    while (res == NULL && list->isSentinel == FALSE)
    {
        if (sameIP(addr, list->address) == TRUE)
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

    //peut y avoir un souci si la taille de data depasse la taille du buffer du file descriptor
    //comportement inconnu dans ce cas la
    printf("sizeData: %llu\n", message.sizeData);

    while (offset < message.sizeData)
    {
        struct address temp;
        uint16_t size = 0;
        uint16_t sizeHostname;

        memcpy(&size, message.data + offset, HEADER_HOSTNAME_SIZE);
        printf("Size: %u\n", size);
        sizeHostname = size - PORT_SIZE - 1;

        temp.hostname = calloc(1, sizeof(char) * sizeHostname);
        offset += HEADER_HOSTNAME_SIZE;

        printf("DATA RECEIVE:\n");
        for (size_t i = 0; i < message.sizeData; i++)
        {
            printf("%u  ", message.data[i]);
        }
        printf("\n\n");

        memcpy(&temp.hostname, message.data + offset, sizeHostname);
        memcpy(&temp.port, message.data + offset + sizeHostname, PORT_SIZE + 1);
        offset += size;

        printf("Apres memecpy DATA:\n");
        for (size_t i = 0; i < sizeHostname; i++)
        {
            ; //printf("%u  ", temp.hostname[0]);
        }
        printf("\n\n");

        printf("Addr: %s\nPort: %s\n", "test", temp.port);
        /*pthread_mutex_lock(&server->lockKnownServers);
        if (FindClient(temp, server->KnownServers) == NULL)
        ;   //addClient(server->KnownServers, temp); 
        pthread_mutex_unlock(&server->lockKnownServers);
        */
        printf("offset: %lu\n", offset);
    }

    printf("\n\n\n");
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

        for (struct clientInfo *temp = server->KnownServers->sentinel->next; temp->isSentinel == FALSE; temp = temp->next)
            dataSize += strlen(temp->address.hostname) + 1 + PORT_SIZE + 1 + HEADER_HOSTNAME_SIZE;

        messageBuff = malloc(sizeof(char) * dataSize);

        for (client = client->sentinel->next; client->isSentinel == FALSE; client = client->next)
        {
            uint16_t size = strlen(client->address.hostname) + 1 + PORT_SIZE + 1;
            memcpy(messageBuff + offset, &size, HEADER_HOSTNAME_SIZE);
            offset += HEADER_HOSTNAME_SIZE;

            memcpy(messageBuff + offset, &client->address, size);
            offset += size;
        }

        pthread_mutex_unlock(&server->lockKnownServers);
        printf("SizeData before sending: %llu\n", dataSize);
        
        MESSAGE message = CreateMessage(type, dataSize, messageBuff);
        shared_queue_push(server->OutgoingMessages, message);
        printf("DATA SEND:\n");
        for (size_t i = 0; i < message.sizeData; i++)
        {
            printf("%u  ", message.data[i]);
        }
        printf("\n\n");


        free(messageBuff);
        sleep(2);
    }

    return NULL;
}

struct sockaddr_in GetIPfromHostname(struct address address)
{
    struct sockaddr_in *resIP;
    struct addrinfo hints, *res;
    memset(&res, 0, sizeof(struct sockaddr_in));
    memset(&hints, 0, sizeof(struct addrinfo));

    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    getaddrinfo(address.hostname, address.port, &hints, &res);
    resIP = (struct sockaddr_in *)res->ai_addr;
    freeaddrinfo(res);

    return *resIP;
}