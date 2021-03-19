#include "../Hash/sha256.c"
#include <string.h>
#include <stdio.h>

//read stdin "prevhash\ntimestamp\nindex\ntransaction\0"
//read : string(hash infos)
//write stdout "prevhash\ptimestamp\nindex\ntransaction\nproof\0"
//write : string(either : "occupied" or "proof")
//
//(infos of the current block)
//char *info[prevhash,timestamp,index,transaction,proof]


//string : 
char *char_to_info(char *str){
	int i = 0;
	char delim[] = "\n";
	char *ptr = strtok(str, delim);
	while(ptr != NULL)
	{
		printf("'%s'\n", ptr);
		ptr = strtok(NULL, delim);
	}	
	return ptr;
}

//brut force and return an x
long mine(){

}

void main(){

	return 0;
}
