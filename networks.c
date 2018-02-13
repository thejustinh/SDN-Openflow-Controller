#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

#include "networks.h"

char * getIPAddressString(uint8_t * ipAddress) 
{
    static char ipString[INET6_ADDRSTRLEN];

    if (ipAddress != NULL)
    {
        inet_ntop(AF_INET6, ipAddress, ipString, sizeof(ipString));
    }
    else
    {
        strcpy(ipString, "(IP not found)");
    }

    return ipString;
}

int tcpAccept(int server_socket, int debugFlag)
{
    struct sockaddr_in6 clientInfo;
    int clientInfoSize = sizeof(clientInfo);
    int client_socket = 0;

    if ((client_socket = accept(server_socket, (struct sockaddr *) &clientInfo, (socklen_t *) &clientInfoSize)) < 0)
    {
        perror("accept call");
        exit(-1);
    }
    
    if (debugFlag)
    {
        printf("Client accepted. Client IP: %s Client Port Number: %d\n",
               getIPAddressString(clientInfo.sin6_addr.s6_addr),ntohs(clientInfo.sin6_port));
    }

    return client_socket;
}

int tcpSetup(int portNumber)
{
    int server_socket = 0; 
    struct sockaddr_in6 server;     // Socket address for local side
    socklen_t len = sizeof(server); // length of local address

    /* Create the TCP socket */
    server_socket = socket(AF_INET6, SOCK_STREAM, 0);
    if (server_socket < 0)
    {
        perror("socket call");
        exit(EXIT_FAILURE);
    }

    server.sin6_family = AF_INET6;
    server.sin6_addr = in6addr_any; // Wildcard Machine Address
    server.sin6_port = htons(portNumber);

    /* Bind the name (address) to a port */
    if (bind(server_socket, (struct sockaddr *) &server, sizeof(server)) < 0)
    {
        perror("bind call");
        exit(-1);
    }

    /* Get port name and print */
    if (getsockname(server_socket, (struct sockaddr *) &server, &len) < 0)
    {
        perror("getsockname call");
        exit(-1);
    }

    if (listen(server_socket, BACKLOG) < 0)
    {
        perror("listen call");
        exit(-1);
    }

    printf("Controller is using port: %d\n", ntohs(server.sin6_port));
    printf("Controller is using IP: %s\n", 
            getIPAddressString(server.sin6_addr.s6_addr));

    return server_socket;
}

