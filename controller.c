/**
 * Justin Herrera
 * Winter 2018
 * CPE 465 - Program 2
 *
 * OpenFlow Controller
 **/

#include "controller.h"
#include "networks.h"

#define DEBUG_FLAG 1

void handleConnections(int sd)
{
    int clientSocket = 0;
    int nfds = 0, i = 0;
   
    fd_set set;   // set
    fd_set t_set; // temp set

    FD_ZERO(&set);
    FD_ZERO(&t_set);

    FD_SET(sd, &set);
    nfds = sd;        // larget file descriptor
    
    while(1)
    {
        t_set = set; // repopulate temp set with sockets of interest

        if (select(nfds + 1, &t_set, NULL, NULL, NULL) < 0)
        {
            perror("error on select()\n");
            exit(-1);
        }

        for (i = 0; i <= nfds; i++)
        {
            if (FD_ISSET(i, &t_set))
            {
                if (i == sd)
                {
                    clientSocket = tcpAccept(sd, DEBUG_FLAG);

                    FD_SET(clientSocket, &set);

                    if (nfds < clientSocket) nfds = clientSocket;
                }

                else 
                {
                    printf("received some data\n");
                }
            }
        }
    }

    close(clientSocket);
}

int main(int argc, char * argv[]) 
{
    int sd = 0;

    /* Set up the TCP connection on port 6633 */
    sd = tcpSetup(6633);    

    handleConnections(sd);
    return 0;
}


