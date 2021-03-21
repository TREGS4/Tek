#include <unistd.h>
#include <err.h>
#include <stdlib.h>
#include <sys/wait.h>

#include "client.h"
#include "server.h"


int network(char **argv)
{
    int r = 0;
        if((r = fork()))
        {
            if(r < 0)
            {
                err(EXIT_FAILURE, "Fail lauching server side in network.c");
            } 
                
	        server(argv);            
	    } 
        else if((r = fork()))
        {
            if(r < 0)
                err(EXIT_FAILURE, "Fail lauching client side in network.c");
            
            argv[2] = "2048";
            argv[1] = "localhost"; 
            sleep(1);
            client(argv);
        } 
    

    wait(NULL);
    wait(NULL);
    
    return EXIT_SUCCESS;
}
