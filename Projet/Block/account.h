#ifndef ACCOUNT_H
#define ACCOUNT_H

#include <stdlib.h>


typedef struct
{
	char *publicKey_pem;
    size_t size;
} ACCOUNT;


ACCOUNT generate_account();
void free_account(ACCOUNT *acc);

#endif
