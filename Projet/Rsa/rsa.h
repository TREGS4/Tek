#ifndef RSA_H
#define RSA_H

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include "../Hash/sha256.h"
#include <openssl/rsa.h>
#include <openssl/pem.h>

size_t getPemFromRSA(RSA *rsa, char **out);
RSA *getRsaFromPem(char *pem, size_t size);
RSA *generateRsaKeys(int bits);

#endif
