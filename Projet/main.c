#include "./Network/network.h"

int main(int argc, char **argv)
{
	if(argc > 2)
		return -1;

	int fd[2];
	
	network(&fd[0], &fd[1], "192.168.1.22", argv[1]);
	return 0;
}
