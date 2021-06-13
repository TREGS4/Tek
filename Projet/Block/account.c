#include "account.h"
#include "../Rsa/rsa.h"
#include <stdlib.h>
#include <string.h>




ACCOUNT generate_account(){
    RSA *rsa;
	rsa = generateRsaKeys(2048);
	
	char *pem;
	ull_t size = (ull_t)getPemFromRSA(rsa, &pem);
	ACCOUNT acc;
	acc.publicKey_pem = malloc(sizeof(char) * size + 1);
	memcpy(acc.publicKey_pem, pem, size);
	acc.publicKey_pem[size] = 0;
	acc.size = size;
	return acc;
}

void free_account(ACCOUNT *acc){
    free(acc->publicKey_pem);
}