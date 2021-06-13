#ifndef ACCOUNT_H
#define ACCOUNT_H

#include <stdlib.h>
#include "../general_informations.h"

typedef struct
{
	char *publicKey_pem;
    ull_t size;
} ACCOUNT;


ACCOUNT generate_account();
void free_account(ACCOUNT *acc);

#endif
