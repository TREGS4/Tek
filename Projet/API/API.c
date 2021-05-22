#include "API.h"


#define BUFFER_SIZE 500



void resend(int fd, const void *buf, size_t count,int flag)
{
   ssize_t send1 = send(fd,buf,count,flag);
   if (send1 == -1) 
   	return err(1, "Error writing");
   
   while (send1 < (ssize_t) count) 
	{
	     send1 += send(fd,buf+send1, count-send1,flag);
	}
}


void temptransaction_cmd(int client_socket_id, TRANSACTIONS_LIST* transaction_list)
{
	char *message = tlToJson (transaction_list);
	resend (client_socket_id,message,strlen(message),0);
	free(message);
}

void server_cmd(int client_socket_id, struct server* server_list)
{
	size_t len = listLen (server_list->KnownServers->sentinel);
	char *str1 = "{\"size\":%ld}";
	char *message = malloc(sizeof(char)*(strlen(str1)+10));
	sprintf(message,str1,len);	
	resend (client_socket_id,message,strlen(message),0);
	free(message);
}

void blockchain_cmd(int client_socket_id, BLOCKCHAIN* block_list)
{
	char *message = blockchainToJson(block_list);
	resend(client_socket_id,message,strlen(message),0);
	free(message);
}

void add_transaction_cmd(int client_socket_id, gchar* resource,TRANSACTIONS_LIST* transaction_list)
{
	TRANSACTION transaction = binToTxs( (BYTE*) resource);
	addTx(transaction_list,&transaction);
	char *message = "{\"success\":\"ok\"}";
	resend(client_socket_id,message,strlen(message),0);
	free(message);
}

// Define the thread function.
void* worker(void* arg)
{
    struct WORK_ARG* work_arg = arg;
    int client_socket_id = work_arg->client_socket_id;
    BLOCKCHAIN* block_list= work_arg->block_list;
    struct server* server_list = work_arg->server_list;
    TRANSACTIONS_LIST* transaction_list = work_arg->transaction_list;
    
    ssize_t request_size;
    char request[BUFFER_SIZE];

    //Get the request from the web client
    //Loop until full message is read
    GString *full_request = g_string_new("");
    do
    {
        request_size = read(client_socket_id, request, BUFFER_SIZE-1);
        if (request_size == -1)
        {
            perror("read");
            exit(0);
        }
        request[request_size] = '\0';
        full_request = g_string_append(full_request, request);
    } while (request_size > 0 &&
             g_str_has_suffix(full_request->str, "\r\n\r\n") == FALSE);

    //Get resource from the request
    if (g_str_has_prefix(full_request->str, "GET ") == TRUE)
    {
        gchar* resource = g_strndup(full_request->str+5, g_strstr_len(full_request->str, -1, " HTTP/")-full_request->str-5);
         
        //Print resource and free full_request and resource
        printf("%d: %s\n", client_socket_id, resource);

        //Send message status
        char message200[] = "HTTP/1.1 200 OK\r\n\r\n";
	char message404[] = "HTTP/1.1 404 Not Found\r\n\r\n";

        
        //Compute and send content message depending on requested resource
        
        //Treat update command
        if(strcmp(resource, "send_transaction_list") == 0) 
	{
		send(client_socket_id, message200, strlen(message200), MSG_MORE);
		temptransaction_cmd(client_socket_id,transaction_list);
	}
        else if (strcmp(resource, "send_server_count") == 0)
        {
		send(client_socket_id, message200, strlen(message200), MSG_MORE);
		server_cmd(client_socket_id, server_list);
	}
	else if (strcmp(resource, "send_blockchain_list") == 0)
	{
		send(client_socket_id, message200, strlen(message200), MSG_MORE);
		blockchain_cmd(client_socket_id, block_list);
	}
	else if (strcmp(resource, "add_transaction") == 0)
	{
		send(client_socket_id, message200, strlen(message200), MSG_MORE);
		add_transaction_cmd(client_socket_id,resource,transaction_list);
	}
        else
	{
		send(client_socket_id, message404, strlen(message404), 0);   
	}
        g_free(resource);

        //Close client sockets
        close(client_socket_id);
	
    }
    free(work_arg);
    g_string_free(full_request, TRUE);
    
    return NULL;
}

int API(BLOCKCHAIN* block_list, struct server* server_list ,TRANSACTIONS_LIST* transaction_list  )
{
    struct addrinfo hints;
    struct addrinfo *addr_list, *addr;
    int socket_id, client_socket_id;
    int res;

  
    //Get addresses list
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    res = getaddrinfo(NULL, "2048", &hints, &addr_list);

    //If error, exit the program
    if (res != 0)
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(res));
        exit(0);
    }


    //Try to connect to each adress returned by getaddrinfo()
    for (addr = addr_list; addr != NULL; addr = addr->ai_next)
    {
        //Socket creation
        socket_id = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);

        //If error, try next adress
        if (socket_id == -1)
            continue;

        //Set options on socket
        int enable = 1;
        if (setsockopt(socket_id, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) == -1)
            perror("setsockopt(SO_REUSEADDR) failed");

        //Bind a name to a socket, exit if no error
        if (bind(socket_id, addr->ai_addr, addr->ai_addrlen) == 0)
            break;

        //Close current not connected socket
        close(socket_id);
    }

    //addr_list freed
    freeaddrinfo(addr_list);

    //If no address works, exit the program
    if (addr == NULL)
    {
        fprintf(stderr, "Could not bind\n");
        exit(0);
    }

    //Specify that the socket can be used to accept incoming connections
    if(listen(socket_id, 5) == -1)
    {
        fprintf(stderr, "Cannot wait\n");
        exit(0);
    }



    //Allow multiple connections
    while(1)
    {
        //Accept connection from a client and exit the program in case of error
        client_socket_id = accept(socket_id, addr->ai_addr, &(addr->ai_addrlen));
        if(client_socket_id == -1)
        {
            fprintf(stderr, "Cannot connect\n");
            exit(0);
        }

        int thread;
        pthread_t thread_id;
	struct WORK_ARG* work_arg = malloc(sizeof(struct WORK_ARG));
	work_arg->client_socket_id = client_socket_id;
	work_arg->block_list = block_list;
	work_arg->server_list = server_list;
	work_arg->transaction_list = transaction_list; 

        // - Create and execute the thread.
        thread = pthread_create(&thread_id, NULL, &(worker), (void*) work_arg);
        if(thread != 0)
        {
            fprintf(stderr, "hello: Can't create thread.\n");
            exit(0);
        }
    }

    //Close server sockets
    close(socket_id);
    
}



