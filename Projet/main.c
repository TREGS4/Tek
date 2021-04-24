#include "./Network/network.h"

int main(/*int argc, char **argv*/)
{
	int fd[2];
	pipe(fd); 
	network(fd[0], STDOUT_FILENO, "192.168.1.22", NULL, fd[1]);
	return 0;
}
