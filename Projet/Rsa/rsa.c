#include "rsa.h"
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

size_t getPemFromBio(BIO *bio, char **out){
	
	char *pem;
	long size = BIO_get_mem_data(bio, &pem);
	pem[size-1] = 0;
	*out = malloc(size);
	memcpy(*out, pem, size);
	return size - 1;
}

size_t getPemFromRSA(RSA *rsa, char **out){
	BIO *bio = BIO_new(BIO_s_mem());
	if (!PEM_write_bio_RSAPublicKey(bio, rsa)) {
		printf("err bio !\n");
		exit(1);
	}
	char *pem;
	size_t size = getPemFromBio(bio, &pem);
	*out = pem;
	BIO_free_all(bio);
	return size;
}

BIO *getBioFromPem(char *pem, size_t size){
	BIO* bio = BIO_new( BIO_s_mem() );
	BIO_write(bio, pem, size + 1);
	return bio;
}



RSA *getRsaFromBio(BIO *bio){
	char *pem;
	size_t size = getPemFromBio(bio, &pem);
	BIO *bio_cpy = getBioFromPem(pem, size);
	RSA *rsa = NULL;
	PEM_read_bio_RSAPublicKey(bio_cpy, &rsa, 0, 0);
	free(pem);
	BIO_free_all(bio_cpy);
	return rsa;
}

RSA *getRsaFromPem(char *pem, size_t size){
	BIO *bio = getBioFromPem(pem, size);
	RSA *rsa = getRsaFromBio(bio);
	BIO_free_all(bio);
	return rsa;
}


RSA *generateRsaKeys(int bits){
	BIGNUM *bn;
	bn = BN_new();
	BN_set_word(bn, RSA_F4);

	RSA *rsa;
	rsa = RSA_new();
	RSA_generate_key_ex(rsa, bits, bn, NULL);
	BN_free(bn);
	return rsa;
}

