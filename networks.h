#ifndef __NETWORKS_H__
#define __NETWORKS_H__

#define BACKLOG 10

int tcpAccept(int server_socket, int debugFlag);
int tcpSetup(int portNumber);

#endif
