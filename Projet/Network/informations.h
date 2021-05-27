#ifndef INFORMATIONS_H
#define INFORMATIONS_H

/*
*   This file contain some informations about the network, there is no .c attach to it
*/


#define DEFAULT_PORT "6969"

//Boolean part
#ifndef TRUE 
#define TRUE 1
#endif

#ifndef FALSE 
#define FALSE 0
#endif

// Message part
#define BUFFER_SIZE_SOCKET 512
#define PORT_SIZE 5
#define HEADER_HOSTNAME_SIZE 2
#define SIZE_DATA_LEN_HEADER 8
#define SIZE_TYPE_MSG sizeof(char)
#define HEADER_SIZE SIZE_DATA_LEN_HEADER + SIZE_TYPE_MSG

//Type of message
#define TYPE_NETWORK 1
#define TYPE_TEST 255

//Status part
#define ERROR -1
#define ENDED 0
#define CONNECTED 1
#define CONNECTING 3
#define NOTCONNECTED 4
#define SENTINEL 5
#define ONLINE 6
#define OFFLINE 7
#define EXITING 8
#define STARTING 9
#define NOTUSED 255
#define DEAD 254

#endif