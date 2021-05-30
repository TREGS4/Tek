#ifndef MINAGE_H
#define MINAGE_H

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <err.h>
#include <pthread.h>
#include <sys/wait.h>
#include "../Hash/sha256.h"


size_t len_of_proof(size_t proof);
unsigned int mine_from_string(char *sum, int nbthread, int diff);

#endif
