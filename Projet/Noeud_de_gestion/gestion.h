#ifndef GESTION_H
#define GESTION_H

#include "../Block/block.h"
#include "../Block/transactions.h"
#include "../Block/blockchain.h"
#include "../Block/account.h"
#include "../Hash/sha256.h"
#include "../Network/network.h"

int gestion(int isAPI, int isMINING, int difficulty, int nb_mining_thread, struct server *network);

#endif