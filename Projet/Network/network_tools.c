#include "network_tools.h"

/*
*   Create the sentinel of a clientInfo list
*   The pointer to the sentinel is returned
*   If malloc failed null is returned
*/
struct clientInfo *initClientList()
{
    struct clientInfo *sentinel = malloc(sizeof(struct clientInfo));
    if (sentinel != NULL)
    {
        memset(&sentinel->address.port, 0, PORT_SIZE + 1);
        sentinel->address.hostname = NULL;
        sentinel->sentinel = sentinel;
        sentinel->prev = NULL;
        sentinel->next = sentinel;
        sentinel->isSentinel = TRUE;
    }

    return sentinel;
}

/*
*   Free the clientInfo list pass in argument
*   the pointer could be on any element of the list
*/
void freeClientList(struct clientInfo *clientList)
{
    clientList = clientList->sentinel;

    while (clientList->next->isSentinel == FALSE)
        removeClient(clientList->next);

    free(clientList);
}

/*
*   You need to block the mutex before calling the function !!!
*
*   Create un clientInfo element from the address pass in argument and add it to
*   the list. The element could be added anywhere in the list.
*   The function return a pointer to the new element is returned.
*   If any problems occurs null is returned
*
*   Before anything the function try to connect to the client to be sure it's a valid client.
*/
struct clientInfo *addClient(struct clientInfo *list, struct address address)
{
    int skt;

    if (address.hostname == NULL)
    {
        fprintf(stderr, "Error can't add client: invalid hostname");
        return NULL;
    }

    if ((skt = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        fprintf(stderr, "Can't create the socket, while trying to test %s:%s before adding it\n", address.hostname, address.port);
        perror(NULL);
        return NULL;
    }

    struct sockaddr_in IP = GetIPfromHostname(address);

    if (connect(skt, (struct sockaddr *)&IP, sizeof(struct sockaddr_in)) < 0)
    {
        fprintf(stderr, "Can't add the client: %s:%s\n", address.hostname, address.port);
        perror(NULL);
        return NULL;
    }

    close(skt);
    struct clientInfo *client = malloc(sizeof(struct clientInfo));

    if (client != NULL)
    {
        client->isSentinel = FALSE;
        memcpy(&client->address, &address, sizeof(struct address));

        client->next = list->next;
        client->prev = list;
        list->next = client;
        if (client->next != NULL)
            client->next->prev = client;

        client->sentinel = list->sentinel;

        printf("Client: %s:%s sucessfully added\n\n\n", address.hostname, address.port);
    }
    else
        fprintf(stderr, "Error during the allocation of memory for %s:%s in addClient()\n", address.hostname, address.port);

    return client;
}

/*
*   You need to block the mutex before calling the function !!!
*
*   Remove properly the client from the list
*/
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

/*
*   You need to block the mutex before calling the function !!!
*
*   Take a pointer to any element of the list.
*   Return the size of the list (sentinel not included).
*/
size_t listLen(struct clientInfo *client)
{
    size_t res = 0;

    for (client = client->sentinel->next; client->isSentinel == FALSE; client = client->next)
    {
        res++;
    }

    return res;
}

/*
*   Take a pointer to any element of the list.
*   Return the last element of the list.
*/
struct clientInfo *last(struct clientInfo *client)
{
    while (client->next->isSentinel == FALSE)
        client = client->next;

    return client;
}

int sameIP(struct address addr1, struct address addr2)
{
    int me = FALSE;

    if (strlen(addr1.hostname) == strlen(addr2.hostname) && memcmp(addr1.hostname, addr2.hostname, strlen(addr1.hostname)) == 0)
        me = TRUE;

    if (memcmp(addr1.port, addr2.port, PORT_SIZE) == 0)
        me = me && TRUE;

    return me;
}

struct clientInfo *FindClient(struct address addr, struct clientInfo *list)
{
    struct clientInfo *res = NULL;
    list = list->sentinel->next;

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

void addServerFromMessage(MESSAGE *message, struct server *server)
{
    size_t offset = 0;

    //peut y avoir un souci si la taille de data depasse la taille du buffer du file descriptor
    //comportement inconnu dans ce cas la

    while (offset < message->sizeData)
    {
        struct address temp;
        uint16_t size = 0;
        uint16_t sizeHostname;

        memcpy(&size, message->data + offset, HEADER_HOSTNAME_SIZE);
        sizeHostname = size - PORT_SIZE - 1;

        temp.hostname = calloc(1, sizeof(char) * sizeHostname);
        offset += HEADER_HOSTNAME_SIZE;

        memcpy(temp.hostname, message->data + offset, sizeHostname);
        memcpy(temp.port, message->data + offset + sizeHostname, PORT_SIZE + 1);
        offset += size;

        pthread_mutex_lock(&server->lockKnownServers);
        if (FindClient(temp, server->KnownServers) == NULL)
            addClient(server->KnownServers, temp);
        pthread_mutex_unlock(&server->lockKnownServers);
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

        for (struct clientInfo *temp = server->KnownServers->sentinel->next; temp->isSentinel == FALSE; temp = temp->next)
            dataSize += strlen(temp->address.hostname) + 1 + PORT_SIZE + 1 + HEADER_HOSTNAME_SIZE;

        messageBuff = malloc(sizeof(char) * dataSize);

        for (client = client->sentinel->next; client->isSentinel == FALSE; client = client->next)
        {
            uint16_t sizeHostname = strlen(client->address.hostname) + 1;
            uint16_t size = sizeHostname + PORT_SIZE + 1;

            memcpy(messageBuff + offset, &size, HEADER_HOSTNAME_SIZE);
            offset += HEADER_HOSTNAME_SIZE;

            memcpy(messageBuff + offset, client->address.hostname, sizeHostname);
            offset += sizeHostname;
            memcpy(messageBuff + offset, client->address.port, PORT_SIZE + 1);
            offset += PORT_SIZE + 1;
        }

        pthread_mutex_unlock(&server->lockKnownServers);

        MESSAGE *message = CreateMessage(type, dataSize, messageBuff);

        if (message != NULL)
            shared_queue_push(server->OutgoingMessages, message);

        free(messageBuff);
        sleep(2);
    }

    return NULL;
}

struct sockaddr_in GetIPfromHostname(struct address address)
{
    struct sockaddr_in resIP, *temp;
    struct addrinfo hints, *res;
    memset(&resIP, 0, sizeof(struct sockaddr_in));
    memset(&hints, 0, sizeof(struct addrinfo));

    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    int r = getaddrinfo(address.hostname, address.port, &hints, &res);
    temp = (struct sockaddr_in *)res->ai_addr;
    if (temp != NULL && r == 0)
        resIP = *temp;
    else
        printf("An error as occured while resolving %s:%s", address.hostname, address.port);
    freeaddrinfo(res);

    return resIP;
}

char* ServerToJSON(struct clientInfo *client)
{
    char *base = "{\"hostname\":\"%s\", \"port\":\"%s\"}";
    size_t nbChar = strlen(client->address.hostname) + strlen(client->address.port) + strlen(base);
    char *res = calloc(1, nbChar + 1);

    sprintf(res, base, client->address.hostname, client->address.port);

    return res;
}

char *AllServerToJSON(struct clientInfo *client)
{
    char *res = NULL;
    size_t offset = 0;

    client = client->sentinel->next;

    if(client->isSentinel == FALSE)
    {
        char *temp = ServerToJSON(client);
        res = calloc(1, strlen(temp) + 1);
        sprintf(res, "%s", temp);
        free(temp);
        client = client->next;
        offset = strlen(res);
    }


    for(; client->isSentinel == FALSE; client = client->next)
    {
        char *temp = ServerToJSON(client);
        res = realloc(res, strlen(res) + strlen(temp) + 2);
        sprintf(res + offset, ",%s", temp);
        offset = strlen(res);
        free(temp);
    }

    return res;
}

char *ServerListToJSON(struct server *server)
{
    char *str = "{\"server_list\":[%s]}";

    pthread_mutex_lock(&server->lockKnownServers);

    char *temp = AllServerToJSON(server->KnownServers);
    char *res = calloc(1, sizeof(char) * (strlen(str) + strlen(temp) + 1));
    sprintf(res, str, temp);
    free(temp);
    
    pthread_mutex_unlock(&server->lockKnownServers);

    printf("%s\n", res);

    return res;
}