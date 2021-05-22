#ifndef INFORMATIONS_H
#define INFORMATIONS_H

#include "queue/shared_queue.h"

#define PORT "6969"

//Boolean part
#ifndef TRUE 
#define TRUE 1
#endif

#ifndef FALSE 
#define FALSE 0
#endif

// Message part
#define BUFFER_SIZE_SOCKET 512
#define SIZE_DATA_LEN_HEADER 8
#define SIZE_TYPE_MSG sizeof(char)
#define HEADER_SIZE SIZE_DATA_LEN_HEADER + SIZE_TYPE_MSG

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
#define NOTUSED 255
#define DEAD 254

#endif