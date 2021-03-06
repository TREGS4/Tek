#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

int reseau()
{
	
	if(fork())
	{
		printf("test\n");
	}
	else
	{
		printf("salut\n");
	}


	WAIT()
	return 0;
}

