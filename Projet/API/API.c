#include "API.h"

#define BUFFER_SIZE 500

// return full_request on problem
char *string_append(char *full_request, char *request)
{
	char *new_request = malloc(sizeof(char) * (strlen(full_request) + strlen(request) + 1));
	if (new_request == NULL)
	{
		return full_request;
	}
	sprintf(new_request, "%s%s", full_request, request);
	free(full_request);
	return new_request;
}

char *findPath(char *str)
{
	char *res;
	size_t start = 0;
	size_t len;

	for (; (char)str[start] != ' '; start++)
	{
		;
	}
	start += 2;
	for (len = start; (char)str[len] != ' '; len++)
	{
		;
	}
	len -= start;

	res = malloc(sizeof(char) * (len + 1));
	if (res == NULL)
	{
		return NULL;
	}

	memset(res, 0, len + 1);

	for (size_t i = 0; i < len; i++)
		res[i] = str[start + i];

	return res;
}

int findInfoTxs(size_t *amount, char **sender, char **receiver, char *ressource)
{
	size_t nbarg = 6;
	char *infos[nbarg];

	//?sender=LESENDER&receiver=LERECEIVER&amount=15

	size_t start = 0;
	size_t end = 0;
	size_t n = 0;
	int para = FALSE;

	while (ressource[start] != '?' && ressource[start] != '\0')
	{
		start++
	}
	start++;
	para = TRUE;

	while (ressource[start] != '\0' && n < nbarg)
	{
		end = start;

		if (para == TRUE)
		{
			while (ressource[end] != '=' && ressource[end] != '\0')
			{
				end++;
			}
		}
		else
		{
			while (ressource[end] != '&' && ressource[end] != '\0')
			{
				end++;
			}
		}

		infos[n] = calloc(1, sizeof(char) * (end - start + 1));
		memcpy(infos[n], ressource + start, end - start);

		n++;
		start = end + 1;

		para = !para;
	}

	for (size_t i = 0; i < n; i += 2)
	{
		if (memcmp(infos[i], "sender", 7) == 0)
		{
			*sender = infos[i + 1];
		}
		else if (memcmp(infos[i], "receiver", 9) == 0)
		{
			*receiver = infos[i + 1];
		}
		else if (memcmp(infos[i], "amount", 7) == 0)
		{
			*amount = (size_t)atol(infos[i + 1]);
			free(infos[i + 1]);
		}
		free(infos[i]);
	}

	return EXIT_SUCCESS;
}

void resend(int fd, const void *buf, size_t count, int flag)
{
	ssize_t send1 = send(fd, buf, count, flag);
	if (send1 == -1)
		return err(1, "Error writing");

	while (send1 < (ssize_t)count)
	{
		send1 += send(fd, buf + send1, count - send1, flag);
	}
}

void temptransaction_cmd(int client_socket_id, TL_M *tl_m)
{
	pthread_mutex_lock(&tl_m->mutex);
	char *message = tlToJson(&tl_m->tl);
	pthread_mutex_unlock(&tl_m->mutex);

	resend(client_socket_id, message, strlen(message), 0);
	free(message);
}

void server_cmd(int client_socket_id, struct server *server)
{
	size_t len = listLen(server->KnownServers->sentinel);
	char *str1 = "{\"size\":%ld}";
	char *message = malloc(sizeof(char) * (strlen(str1) + 10));
	if (message == NULL)
	{
		resend(client_socket_id, NULL, 0, 0);
	}
	else
	{
		sprintf(message, str1, len);
		resend(client_socket_id, message, strlen(message), 0);
		free(message);
	}
}

void blockchain_cmd(int client_socket_id, BLOCKCHAIN_M *bc_m)
{
	pthread_mutex_lock(&bc_m->mutex);
	char *message = blockchainToJson(&bc_m->bc);
	pthread_mutex_unlock(&bc_m->mutex);

	resend(client_socket_id, message, strlen(message), 0);
	free(message);
}

void add_transaction_cmd(int client_socket_id, shared_queue *outgoingTxs, char *ressource)
{

	char *sender = NULL;
	char *receiver = NULL;
	size_t amount = 0;
	findInfoTxs(&amount, &sender, &receiver, ressource);

	if (amount <= 0 || sender == NULL || receiver == NULL)
	{
		char *message = "{\"success\":\"error\"}";
		resend(client_socket_id, message, strlen(message), 0);
		if (sender != NULL)
		{
			free(sender);
		}
		if (receiver != NULL)
		{
			free(receiver);
		}
	}
	else
	{
		TRANSACTION *transaction = malloc(sizeof(TRANSACTION));
		if (transaction == NULL)
		{
			char *message = "{\"success\":\"error\"}";
			resend(client_socket_id, message, strlen(message), 0);
			return;
		}
		*transaction = CreateTxs(amount, sender, receiver);

		shared_queue_push(outgoingTxs, transaction);

		char *message = "{\"success\":\"ok\"}";
		resend(client_socket_id, message, strlen(message), 0);

		free(sender);
		free(receiver);
	}
}

// Define the thread function.
void *worker(void *arg)
{
	struct WORK_ARG *work_arg = arg;
	int client_socket_id = work_arg->client_socket_id;
	BLOCKCHAIN_M *bc_m = work_arg->bc_m;
	struct server *server = work_arg->server;
	TL_M *tl_m = work_arg->tl_m;
	shared_queue *outgoingTxs = work_arg->outgoingTxs;

	ssize_t request_size;
	char request[BUFFER_SIZE];

	//Get the request from the web client
	//Loop until full message is read
	char *full_request = calloc(1, sizeof(char));
	if (full_request == NULL)
	{
		close(client_socket_id);
		return NULL;
	}

	do
	{
		request_size = read(client_socket_id, request, BUFFER_SIZE - 1);
		if (request_size == -1)
		{
			perror("read");
			exit(0);
		}
		request[request_size] = '\0';
		full_request = string_append(full_request, request);
	} while (request_size > 0 &&
			 memcmp(full_request + strlen(full_request) - 4, "\r\n\r\n", 5) == 1);

	printf("request=\n %s\n\n", full_request);
	//Get resource from the request
	if (memcmp(full_request, "GET ", 4) == 0)
	{
		//Send message status
		char message200[] = "HTTP/1.1 200 OK\nAccess-Control-Allow-Origin: *\r\n\r\n";
		char message404[] = "HTTP/1.1 404 Not Found\nAccess-Control-Allow-Origin: *\r\n\r\n";

		char *resource = findPath(full_request);
		if (resource == NULL)
		{
			send(client_socket_id, message404, strlen(message404), 0);
			close(client_socket_id);
			return NULL;
		}

		//Compute and send content message depending on requested resource

		//Treat update command
		if (strcmp(resource, "transactions/get") == 0)
		{
			send(client_socket_id, message200, strlen(message200), MSG_MORE);
			temptransaction_cmd(client_socket_id, tl_m);
		}
		else if (strcmp(resource, "server/count") == 0)
		{
			send(client_socket_id, message200, strlen(message200), MSG_MORE);
			server_cmd(client_socket_id, server);
		}
		else if (strcmp(resource, "blockchain") == 0)
		{
			send(client_socket_id, message200, strlen(message200), MSG_MORE);
			blockchain_cmd(client_socket_id, bc_m);
		}
		else if (memcmp(resource, "transactions/post", 17) == 0)
		{
			send(client_socket_id, message200, strlen(message200), MSG_MORE);
			add_transaction_cmd(client_socket_id, outgoingTxs, resource);
		}
		else
		{
			send(client_socket_id, message404, strlen(message404), 0);
		}
		free(resource);

		//Close client sockets
	}
	close(client_socket_id);
	free(full_request);

	return NULL;
}

void *API(void *args)
{
	API_THREAD_ARG *api_args = (API_THREAD_ARG *)args;
	BLOCKCHAIN_M *bc_m = api_args->bc_m;
	TL_M *tl_m = api_args->tl_m;
	struct server *server = api_args->server;
	shared_queue *outgoingTxs = api_args->outgoingTxs;

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

	//If no address works, exit the program
	if (addr == NULL)
	{
		fprintf(stderr, "Could not bind\n");
		exit(0);
	}

	//Specify that the socket can be used to accept incoming connections
	if (listen(socket_id, 10) == -1)
	{
		fprintf(stderr, "Cannot wait\n");
		exit(0);
	}

	//Allow multiple connections
	while (1)
	{
		//Accept connection from a client and exit the program in case of error
		client_socket_id = accept(socket_id, addr->ai_addr, &(addr->ai_addrlen));
		if (client_socket_id == -1)
		{
			fprintf(stderr, "Cannot connect\n");
			exit(0);
		}

		int thread;
		pthread_t thread_id;
		struct WORK_ARG work_arg;
		work_arg.client_socket_id = client_socket_id;
		work_arg.bc_m = bc_m;
		work_arg.server = server;
		work_arg.tl_m = tl_m;
		work_arg.outgoingTxs = outgoingTxs;

		// - Create and execute the thread.
		thread = pthread_create(&thread_id, NULL, worker, (void *)&work_arg);
		if (thread != 0)
		{
			fprintf(stderr, "hello: Can't create thread.\n");
			exit(0);
		}
		pthread_detach(thread_id);
	}

	//Close server sockets
	close(socket_id);

	//addr_list freed
	freeaddrinfo(addr_list);
	sleep(2);
	return NULL;
}
