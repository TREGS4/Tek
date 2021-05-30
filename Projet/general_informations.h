#ifndef TOOLS_H
#define TOOLS_H

//BOOLEAN PART
#define TRUE 1
#define FALSE 0

//Type of message
#define TYPE_NETWORK 1
#define TYPE_TXS 2
#define TYPE_BLOCKCHAIN 3
#define TYPE_TEST 255

//Different info that could be useful
#define ERROR -1
#define ENDED 0
#define CONNECTED 1
#define CONNECTING 3
#define NOTCONNECTED 4
#define SENTINEL 5
#define NOTUSED 255
#define DEAD 254

//Status part
#define ONLINE 6
#define OFFLINE 7
#define EXITING 8
#define STARTING 9

#endif