/**
 * Justin Herrera
 * Winter 2018
 * CPE 465 - Program 2
 *
 * OpenFlow Controller
 * controller.c
 **/

#include "controller.h"
#include "networks.h"

#define DEBUG_FLAG 1

void printOFPHdr(struct ofp_header * ofp_hdr)
{
    if (ofp_hdr != NULL) 
    {
        printf("\nOF Packet Received:....\n");
        printf("\tOF Version: %d\n", ofp_hdr->version);
        printf("\tOF Type: %d\n", ofp_hdr->type);
        printf("\tOF length: %d\n", ntohs(ofp_hdr->length));
        printf("\tOF XID: %d\n", ntohl(ofp_hdr->xid));
    }
}

void estOFConnection(int clientSocket, struct ofp_header * recvHello)
{
    uint8_t hPacket[OFP_HDR_LEN];
    uint8_t frequestPacket[OFP_HDR_LEN];
    struct ofp_header sendHello;
    struct ofp_header sendFeatures;

    memset(hPacket, 0, OFP_HDR_LEN);
    memset(frequestPacket, 0, OFP_HDR_LEN);

    sendHello.version = (uint8_t)1;
    sendHello.type = (uint8_t)0;
    sendHello.length = htons((uint16_t)8);
    sendHello.xid = recvHello->xid;
    memcpy(hPacket, &sendHello, 8);

    if (send(clientSocket, hPacket, sizeof(hPacket), 0) < 0) 
    {
        perror("send call");
        exit(-1);
    }

    sendFeatures.version = (uint8_t)1;
    sendFeatures.type = (uint8_t)5;
    sendFeatures.length = htons((uint16_t)8);
    sendFeatures.xid = htonl((uint32_t)2323);
    memcpy(frequestPacket, &sendFeatures, 8);

    if (send(clientSocket, frequestPacket, OFP_HDR_LEN, 0) < 0)
    {
        perror("send call");
        exit(-1);
    }
}

void setConfig(int clientSocket)
{
    uint8_t cPacket[12];
    uint8_t bPacket[OFP_HDR_LEN];
    struct ofp_header sendConfig;
    struct ofp_header sendBarrier;
    uint16_t configFlag = htons(0x0000);
    uint16_t maxBytes = htons(0x0080);

    memset(cPacket, 0, 12);
    memset(bPacket, 0, OFP_HDR_LEN);

    sendConfig.version = (uint8_t)1;
    sendConfig.type = (uint8_t)9;
    sendConfig.length = htons((uint16_t)12);
    sendConfig.xid = htonl((uint32_t)3);
    memcpy(cPacket, &sendConfig, 8);
    memcpy(cPacket + 8, &configFlag, 2);
    memcpy(cPacket + 10, &maxBytes, 2);

    printf("\n---->>Sending setup config..\n");
    if (send(clientSocket, cPacket, sizeof(cPacket), 0) < 0)
    {
        perror("send call");
        exit(-1);
    }
    printf("---->>Sent config setup!!!!\n");

    sendBarrier.version = (uint8_t)1;
    sendBarrier.type = (uint8_t)18;
    sendBarrier.length = htons((uint16_t)8);
    sendBarrier.xid = htonl((uint32_t)5);
    memcpy(bPacket, &sendBarrier, 8);

    printf("\n---->>Sending Barrier request packet..\n");
    if (send(clientSocket, bPacket, sizeof(bPacket), 0) < 0)
    {
        perror("send call");
        exit(-1);
    }
    printf("---->>Sent Barrier Requst!\n");

}

/**
 * Method to receive data from socket.
 *
 * @param socket descriptor that we are reading from
 * @return 1 on success
 **/
int recvData(int clientSocket)
{
    uint8_t packet[OFP_HDR_LEN];
    uint16_t packetLen = 0;
    uint8_t type;
    int numBytes = 0;
    struct ofp_header *ofp_hdr = NULL;

    // Dump buffer
    uint8_t payload[1500];

    memset(packet, 0, OFP_HDR_LEN);

    /* Read in only the header */
    if ((numBytes = recv(clientSocket, packet, OFP_HDR_LEN, MSG_WAITALL)) <= 0)
    {
        if (numBytes != 0)
            exit(-1);
        return 0;
    }

    /* Read in OF Packet Contents */
    ofp_hdr = (struct ofp_header *) packet;
    type = ofp_hdr->type;
    packetLen = ntohs(ofp_hdr->length);

    /* Printing Packet for Debugging purposes */
    printf("\n**********\n  New Packet\n**********\n");
    printf("\nPacket Length: %d\n", packetLen);
    printf("Bytes Read: %d\n", numBytes);
    printf("left to read: %d\n", packetLen - numBytes);
    
    if (packetLen - numBytes > 0) 
    {
        printf("Reading additional bytes...\n");
        /* TEMP IMPLEMENTATION */
        /* If theres anything in the TCP buffer, read the rest of the bytes */
        if ((numBytes = recv(clientSocket, payload, packetLen-numBytes, MSG_WAITALL)) < 0)
        {
            perror("recv call");
            exit(-1);
        }
        printf("Additional bytes read: %d\n", numBytes);
    }

    printOFPHdr(ofp_hdr);

    /* Execute Action based on OF Packet Type */
    if (type == OF_HELLO)
        estOFConnection(clientSocket, ofp_hdr);
    else if (type == OF_FEATURE_REPLY)
        //printf("Received features reply!\n");
        setConfig(clientSocket);
    else if (type == OF_ECHO_REQUEST)
        sendEchoReply(clientSocket);
    else
        printf("Dont know openflow type... do i care?\n"); 



    return 1;
}

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
                    recvData(clientSocket);
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
