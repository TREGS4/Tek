#include "API.h"


#define BUFFER_SIZE 500


char *serverToJson(struct *server_list server_list)
{
	//TODO
}

char *ttxsToJson(*TRANSACTIONS_LIST transaction_list)
{
 	//TODO
}
void resend(int fd, const void *buf, size_t count,int flag)
{
   ssize_t send1 = send(fd,buf,count,flag);
   if (send1 == -1) 
   	return err(1, "Error writing");
   
   while (send1 < (ssize_t) count) 
	{
	     send1 += send(fd,buf+send, count-send,flag);
	}
}


void temptransaction_cmd(int client_socket_id, *TRANSACTIONS_LIST transaction _list)
{
	//TODO
}

void server_cmd(int client_socket_id, struct *server server_list)
{
	//TODO
}

void blockchain_cmd(int client_socket_id, *BLOCKCHAIN block_list)
{
	char *txt = blockchainToJson(block_list);
	resend(client_socket_id,txt,strlen(txt),0);
}

void add_transaction_cmd(int client_socket_id, gchar* resource,*TRANSACTIONS_LIST transaction _list)
{
	char *transaction = binToTxs( (BYTE*) resource);
	addTx(transaction _list,transaction);
	resend(client_socket_id,"ok",2,0);
}

// Define the thread function.
void* worker(void* arg)
{
    struct *WORK_ARG work_arg = (struct WORK_ARG) arg;
    int client_socket_id = work_arg->client_socket_id;
    *BLOCKCHAIN block_list= work_arg->block_list;
    struct *server server_list = work_arg->server_list;
    *TRANSACTIONS_LIST transaction _list = work_arg->transaction_list;
    
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
        char message[] = "HTTP/1.1 200 OK\r\n\r\n";
        send(client_socket_id, message, strlen(message), MSG_MORE);
        
        //Compute and send content message depending on requested resource
        
        //Treat update command
        if(strcmp(resource, "send_transaction_list") == 0) temptransaction_cmd(client_socket_id,transaction_list);
        else
        {
            // Treat set command
            if(g_str_has_prefix(resource, "send_server_list") == TRUE) server_cmd(client_socket_id, server_list);
            else
            {
                // Treat grid command
                if(g_str_has_prefix(resource, "send_blockchain_list") == TRUE) blockchain_cmd(client_socket_id, block_list);
                else
                {
                    // Treat restart command
                    if(strcmp(resource, "add_transaction") == 0) add_transaction_cmd(client_socket_id,resource);
                    else //TODO message error;
                }
            }
        }
	
        g_free(resource);

        //Close client sockets
        close(client_socket_id);
	
    }
    free(work_arg);
    g_string_free(full_request, TRUE);
    
    return NULL;
}

int API(struct *BLOCKCHAIN block_list, struct *server server_list , struct *TRANSACTIONS_LIST transaction_list  )
{
    struct addrinfo hints;
    struct addrinfo *addr_list, *addr;
    int socket_id, client_socket_id;
    int res;

    // Init with a value of 1
    // first 0 is for unused option, keep it this way
    if ( sem_init(&lock, 0, 1) == -1)
        err(1, "Fail to initialized semaphore");
    
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

    //Socket waiting for connections on port 2048
    printf("Tic-Tac-Toe Server\nListening to port 2048...\n");

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
	struct *WORK_ARG work_arg = malloc(sizeof(struct WORK_ARG));
	work_arg->client_socket_id = client_socket_id;
	work_arg->block_list = block_list;
	work_arg->server_list = server_list;
	work_Arg->transaction_list = transaction_list; 

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
    
    sem_destroy(&lock);
}


int main()
{
}
