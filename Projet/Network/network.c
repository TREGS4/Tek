#include <unistd.h>
#include <err.h>
#include <stdlib.h>
#include <sys/wait.h>

#include "client.h"
#include "server.h"


int network(int argc, char **argv)
{
    int r;
        if((r = fork()))
        {
            if(r < 0)
                err(EXIT_FAILURE, "Fail lauching server side in network.c");
        
            if(execvp("./server.c", argv) < 0)
                err(EXIT_FAILURE, "Fail lauching server side in network.c");
        } 
        else if((r = fork()))
        {
            if(r < 0)
                err(EXIT_FAILURE, "Fail lauching client side in network.c");
            
            argv[2] = "2048";
            argv[1] = "127.0.0.1";   
            if(execvp("./client.c", argv) < 0)
                err(EXIT_FAILURE, "Fail lauching client side in network.c");
        } 
    

    wait(NULL);
    wait(NULL);
    
    return EXIT_SUCCESS;
}
