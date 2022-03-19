#ifndef INFORMATIONS_H
#define INFORMATIONS_H

#include "../general_informations.h"
#include "../Hash/sha256.h"

/*
*   This file contain some informations about the network, there is no .c attach to it
*/

#define DEFAULT_PORT "1313"

// Message part
#define BUFFER_SIZE_SOCKET 512
#define PORT_SIZE 5
#define HEADER_HOSTNAME_SIZE 2
#define SIZE_DATA_LEN_HEADER 8
#define SIZE_TYPE_MSG sizeof(BYTE)
#define HEADER_SIZE SIZE_DATA_LEN_HEADER + SIZE_TYPE_MSG



#endif
