#include "minage.h"

//read stdin "difficulty-index-prevhashmerklehash"
//read : string(hash infos)
//write stdout "difficulty-index-prevhashmerklehash-proof"
//write : string(either : "occupied" or "proof")
//
//struct for threads
struct mining_arg{
	char * sum;
	int diff;
	unsigned long nbthread;
	int start;
	int *ismining;
};

//len of the proof if it was a string
size_t len_of_proof(size_t proof){
	size_t len = 0;
	size_t tmp = proof;
	while(tmp != 0){
		tmp /= 10;
		len++;
	}
	return len;
}


//mine with the string from string_splitting
void *thread_mining(void *arg){
	struct mining_arg *m_a = arg;
	int diff = m_a->diff;
	char * sum = m_a->sum;
	unsigned long nbthread = m_a->nbthread;
	unsigned long *proof = malloc(sizeof(unsigned long));
	*proof = m_a->start;
	int *ismining = m_a->ismining;
	free(m_a);
	//printf("start : %d\n nbthread : %ld\n", m_a->start, nbthread);
	//printf("sum : %s\n",sum);
	unsigned int res = 1;
	char str[8*SHA256_BLOCK_SIZE];
	while(res == 1 && *ismining == 0){
		//str = sum + proof
		sprintf((char *)str, "%ld%s", *proof, sum);
		//printf("proof : %ld\nstring : %s\n",*proof, str);

		//hash with sha256
		BYTE buf[SHA256_BLOCK_SIZE];
		sha256((BYTE *)str, buf);
		//uncomment if you want to print the hash
		/*printf("buf :");
		  for(int i = 0; i < 32; i++)
		  printf("%02x", buf[i]);
		  printf("\n");*/
		res = 0;
		for(int i = SHA256_BLOCK_SIZE - diff; i < SHA256_BLOCK_SIZE; i++)
		{
			if(buf[i] != 0)
				res = 1;
		}
		if(res == 1)
			*proof += nbthread;
		else
			*ismining = 1;
	}
	return (void *)proof;
}

//return 0 if proof is correct, 1 if not
unsigned int testproof(int diff, char * sum, unsigned long proof){
	unsigned int res = 0;
	BYTE str[4 * SHA256_BLOCK_SIZE + len_of_proof(proof) + 1];
	//str = sum + proof
	sprintf((char *)str, "%ld%s", proof, sum);

	//hash with sha256
	BYTE buf[SHA256_BLOCK_SIZE];
	sha256(str, buf);
	for(int i = SHA256_BLOCK_SIZE - diff; i < SHA256_BLOCK_SIZE; i++)
	{
		if(buf[i] != 0)
			res = 1;
	}
	return res;
}


//find the proof from the string
unsigned int mine_from_string(char *sum, int nbthread, int diff){


	//uncomment to debug
	//printf("mine from string : \nsum : %s\n", sum);

	//threads for the mining
	pthread_t thr[nbthread];
	unsigned int proof = 0;
	unsigned int *output = NULL;
	int ismining = 0;
	for(int n = 0; n < nbthread; n++){
		//struct for arg
		struct mining_arg *m_a = malloc(sizeof(struct mining_arg));
		m_a->diff = diff;
		m_a->nbthread = nbthread;
		m_a->start = n;
		m_a->sum = sum;
		m_a->ismining = &ismining;

		if(pthread_create(thr + n, NULL, thread_mining, (void *)m_a) != 0)
			err(3,"mining.c: mine_from_string: error while creating the threads");
	}
	for(int n = 0; n < nbthread; n++){
		pthread_join(thr[n], (void **)&output);
		//printf("output thread %d :%d\n", n, *output);
		if(testproof(diff, sum, *output) == 0){
			proof = *output;
		}
	}
	free(output);
	return proof;
}

