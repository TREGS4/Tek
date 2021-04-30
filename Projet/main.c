#include "./Network/network.h"

int main(int argc, char **argv)
{
	if(argc > 3)
		return -1;

	int fd[2];
	
	network(&fd[0], &fd[1], argv[1], argv[2]);
	return 0;

}
