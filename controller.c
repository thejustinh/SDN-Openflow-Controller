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
#include <signal.h>

#define DEBUG_FLAG 1

int sd = 0; // Server socket
uint32_t xid = 1;
struct ListOfLists * ll;

/**
 * Method to test if we are receiving OpenFlow packets correctly.
 * Prints out the OpenFlow packet header.
 *
 * @param OpenFlow Header struct
 **/
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

/**
 * This method prints the link state changes of ports on the switch
 *
 * @param OpenFlow Header struct
 * @param payload from OpenFlow packet
 **/
void reportPort(struct ofp_header * ofp_hdr, uint8_t * payload)
{
    uint8_t reason = payload[0];
    uint16_t portNum;

    memcpy(&portNum, payload + 1 + 7, 2);

    printf("Link state change: ");
    if (reason == 0x00)
        printf("added ");
    else if (reason == 0x01)
        printf("deleted ");
    else
        printf("modified ");
    printf("port number %d\n", ntohs(portNum));
}

/**
 * Method to print out OpenFlow ID, version, number of ports, and each
 * ports' identifier.
 *
 * @param OpenFlow header struct
 * @param payload of OpenFlow packet
 **/
void reportFeatures(struct ofp_header * ofp_hdr, uint8_t * payload)
{
    uint64_t identifier;
    uint64_t datapathID;
    uint16_t portNum;
    uint16_t packetLen = ntohs(ofp_hdr->length);
    uint8_t version = ofp_hdr->version;
    int numPorts = (packetLen - OFP_HDR_LEN - FEATURE_LEN)/PORT_DATA_LEN; 
    memcpy(&datapathID, payload, 8);
    identifier = be64toh(datapathID);

    printf("Openflow Identifier: %llu\n", (long unsigned long) identifier);
    printf("Openflow Version: %.1f\n", (float)version);
    printf("Number of Ports: %d\n", numPorts);

    for (int i = 1; i <= numPorts; i++)
    {
        memcpy(&portNum, 
                payload + FEATURE_LEN + ((i-1)*PORT_DATA_LEN), 2);
        printf("\tPort %d Identifier #: %d\n", i, ntohs(portNum));
    }
}

/**
 * Method to establish connection with openflow switch by sending a Hello
 * packet as well as a request for features.
 *
 * @param client Socket
 * @param received OpenFlow Hello packet to read transaction ID
 **/
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
    sendHello.xid = htonl(xid);
    memcpy(hPacket, &sendHello, 8);

    if (send(clientSocket, hPacket, sizeof(hPacket), 0) < 0) 
    {
        perror("send call");
        exit(-1);
    }

    xid++;
    sendFeatures.version = (uint8_t)1;
    sendFeatures.type = (uint8_t)5;
    sendFeatures.length = htons((uint16_t)8);
    sendFeatures.xid = htonl(xid);
    memcpy(frequestPacket, &sendFeatures, 8);

    if (send(clientSocket, frequestPacket, OFP_HDR_LEN, 0) < 0)
    {
        perror("send call");
        exit(-1);
    }
    xid++;
}

/** 
 * Method to send out a Set Config OpenFlow Packet, Flow Mod, and Barrier Req
 *
 * @param clientSocket
 **/
void setConfig(int clientSocket)
{
    uint8_t cPacket[12];
    uint8_t bPacket[OFP_HDR_LEN];
    uint8_t fPacket[72];
    struct ofp_header sendConfig;
    struct ofp_header sendBarrier;
//    struct ofp_header sendFlowMod;
    uint16_t configFlag = htons(0x0000);
    uint16_t maxBytes = htons(0x0080);
/*    uint32_t wildcard = htonl(1048607); // 0x0010001f mask
    uint16_t command = htons(3); // Delete all matching flows
    uint16_t priority = htons(32768);
    uint32_t bufferID = htonl(0xffffffff);
    uint16_t outPort = htons(0xffff);
*/

    memset(cPacket, 0, 12);
    memset(bPacket, 0, OFP_HDR_LEN);
    memset(fPacket, 0, 72);

    sendConfig.version = (uint8_t)1;
    sendConfig.type = (uint8_t)9;
    sendConfig.length = htons((uint16_t)12);
    sendConfig.xid = htonl(xid);
    memcpy(cPacket, &sendConfig, 8);
    memcpy(cPacket + 8, &configFlag, 2);
    memcpy(cPacket + 10, &maxBytes, 2);


    //printf("\n---->>Sending setup config..\n");
    if (send(clientSocket, cPacket, sizeof(cPacket), 0) < 0)
    {
        perror("send call");
        exit(-1);
    }
    xid++;
    //printf("---->>Sent config setup!!!!\n");

/*
    sendFlowMod.version = (uint8_t)1;
    sendFlowMod.type = (uint8_t)14;
    sendFlowMod.length = htons((uint16_t) 72);
    sendFlowMod.length = htonl((uint32_t) 4);
    memcpy(fPacket, &sendFlowMod, OFP_HDR_LEN);
    memcpy(fPacket + 8, &wildcard, 4);
    memcpy(fPacket + 56, &command, 2);
    memcpy(fPacket + 62, &priority, 2);
    memcpy(fPacket + 64, &bufferID, 4);
    memcpy(fPacket + 68, &outPort, 2);


    if (send(clientSocket, fPacket, sizeof(fPacket), 0) < 0)
    {
        perror("send call");
        exit(-1);
    }
*/
    sendBarrier.version = (uint8_t)1;
    sendBarrier.type = (uint8_t)18;
    sendBarrier.length = htons((uint16_t)8);
    sendBarrier.xid = htonl(xid);
    memcpy(bPacket, &sendBarrier, 8);

    //printf("\n---->>Sending Barrier request packet..\n");
    if (send(clientSocket, bPacket, sizeof(bPacket), 0) < 0)
    {
        perror("send call");
        exit(-1);
    }
    //printf("---->>Sent Barrier Requst!\n");
    xid++;
}

/** 
 * Method to send an OpenFlow echo reply for every echo request.
 *
 * @param clientSocket
 * @param OpenFlow Header struct
 **/
void sendEchoReply(int clientSocket, struct ofp_header * ofp_hdr)
{
    uint8_t rPacket[OFP_HDR_LEN];
    struct ofp_header sendReply;

    sendReply.version = (uint8_t)1;
    sendReply.type = (uint8_t)3;
    sendReply.length = htons((uint16_t)8);
    sendReply.xid = ofp_hdr->xid;
    memcpy(rPacket, &sendReply, 8);

    if (send(clientSocket, rPacket, sizeof(rPacket), 0) < 0) 
    {
        perror("send call");
        exit(-1);
    }
}




/**
 * Method to determine if payload is an ARP Reply 
 *
 * @param payload of openflow packet in which is the ethernet frame
 **/
int isArpReply(uint8_t * payload)
{
    uint16_t arp_code = 0x0806;
    uint16_t arp_opcode = 0x0002;
    uint16_t received_arp_code;
    uint16_t received_arp_opcode;

    memcpy(&received_arp_code, payload + 12, 2);
    
    if (ntohs(received_arp_code) != arp_code)
        return 0; // ETHERTYPE is not an ARP

    memcpy(&received_arp_opcode, payload + 20, 2);

    if (ntohs(received_arp_opcode) != arp_opcode)
        return 0; // ARP is not a reply
    
    return 1; // payload is arp reply
}

/**
 * Method to find the port that a host resides on given a mac address
 **/
uint16_t findPort(int clientSocket, uint8_t * mac)
{
    struct ListOfLists * cur = ll;
    struct PathNode * node;

    /* Loop thorugh outer linked list filtering for client socket */
    while (cur != NULL) {

        /* If switch (client socket) is found, loop inner LL-> find port. */
        if (cur->socket == clientSocket) {
            node = cur->head;
            while (node != NULL) {
                if (memcmp(mac, node->to_mac, MAC_ADDR_LEN) == 0)
                    return node->port;
                node = node->next;
            }
            return 0;
        }
        cur = cur->next;
    }

    // Did not find port
    printf("Could not find port in findPort()\n");
    return 0;
}


/**
 * Method to send bidirectional flow of an arp request.
 **/
void establishFlows(int clientSocket, struct ofp_packet_in * pkt_in)
{
    struct ofp_header flowmod_hdr;
    uint8_t raw_flowmod[OFP_FLOWMOD_LEN];
    
    uint16_t length = (uint16_t)OFP_FLOWMOD_LEN;
    uint32_t new_xid;
    uint32_t wildcard = htonl(0x00100013);
    uint16_t priority = htons(0x8000);
    uint32_t buffer_id = htonl(0xffffffff);
    uint16_t out_port = htons(0xffff);
    uint16_t action_type = htons(0x0000);
    uint16_t action_length = htons(0x0008);
    uint16_t port = pkt_in->in_port;
    uint8_t to_mac[MAC_ADDR_LEN];

    memset(to_mac, 0, MAC_ADDR_LEN);
    memset(raw_flowmod, 0, OFP_FLOWMOD_LEN);

    /* Build and Send Flow 1 */
    printf("Building flow mod 1: A..\n");
    flowmod_hdr.version = (uint8_t)1;
    flowmod_hdr.type = (uint8_t)14; // flowmod type
    flowmod_hdr.length = htons((uint16_t) length);
    flowmod_hdr.xid = htonl(xid);
    memcpy(raw_flowmod, &flowmod_hdr, OFP_HDR_LEN);
    memcpy(raw_flowmod + 8, &wildcard, 4);
    memcpy(raw_flowmod + 14, pkt_in->data, MAC_ADDR_LEN);
    memcpy(raw_flowmod + 20, pkt_in->data + 6, MAC_ADDR_LEN);
    memcpy(raw_flowmod + 62, &priority, 2);
    memcpy(raw_flowmod + 64, &buffer_id, 4);
    memcpy(raw_flowmod + 68, &out_port, 2);
    memcpy(raw_flowmod + 72, &action_type, 2);
    memcpy(raw_flowmod + 74, &action_length, 2);
    memcpy(raw_flowmod + 76, &port, 2);
    
    printf("flow mod 1: A built\n");
    if (send(clientSocket, raw_flowmod, sizeof(raw_flowmod), 0) < 0) 
    {
        perror("send call");
        exit(-1);
    }
    printf("flow mod 1: a SENT!!\n");


    printf("Building flow mod 2: B..\n");
    /* Build and Send Opposite of Flow 1 */
    xid++;
    new_xid = htonl(xid);
    memcpy(to_mac, pkt_in->data, MAC_ADDR_LEN);
    port = htons(findPort(to_mac));
    memcpy(raw_flowmod + 4, &new_xid, 4);
    memcpy(raw_flowmod + 14, pkt_in->data + 6, MAC_ADDR_LEN);
    memcpy(raw_flowmod + 20, pkt_in->data, MAC_ADDR_LEN);
    memcpy(raw_flowmod + 76, &port, 2);

    printf("flow mod 2: B built\n");
    if (send(clientSocket, raw_flowmod, sizeof(raw_flowmod), 0) < 0) 
    {
        perror("send call");
        exit(-1);
    }
    printf("flow mod 2: B SENT!!\n");
    xid++;
}

/**
 * Method to create packet out
 **/
void makeSendPacketOut(int clientSocket, struct ofp_packet_in * pkt_in,
        uint16_t output_port)
{
    uint16_t pkt_out_len = PKT_OUT_HDR_LEN + ntohs(pkt_in->total_len);
    uint32_t buffer_id = htonl(0xffffffff);
    uint16_t in_port = pkt_in->in_port;
    uint16_t actions_len = htons((uint16_t)8);
    uint16_t actions_type = htons((uint16_t) 0x0000);
    struct ofp_header pkt_out_hdr;

    uint8_t raw_pkt_out[pkt_out_len];
    memset(raw_pkt_out, 0, pkt_out_len);

    printf("\tBuilding Packet Out of length: %d\n", pkt_out_len);

    /* PACKET OUT HDR */
    pkt_out_hdr.version = (uint8_t)1;
    pkt_out_hdr.type = (uint8_t)13;
    pkt_out_hdr.length = htons((uint16_t) pkt_out_len);
    pkt_out_hdr.xid = htonl(xid);
    memcpy(raw_pkt_out, &pkt_out_hdr, OFP_HDR_LEN);
    memcpy(raw_pkt_out + 8, &buffer_id, 4);
    memcpy(raw_pkt_out + 12, &in_port, 2);
    memcpy(raw_pkt_out + 14, &actions_len, 2);
    memcpy(raw_pkt_out + 16, &actions_type, 2);
    memcpy(raw_pkt_out + 18, &actions_len, 2);
    memcpy(raw_pkt_out + 20, &output_port, 2);

    /* PACKET OUT DATA */
    memcpy(raw_pkt_out + 24, pkt_in->data, ntohs(pkt_in->total_len));

    printf("\tBuilt\n");

    // send packet out to switch socket
    if (send(clientSocket, raw_pkt_out, sizeof(raw_pkt_out), 0) < 0) 
    {
        perror("send call");
        exit(-1);
    }
  
    xid++; 
    printf("Packet out sent!!!\n"); 
}

/**
 * Method to handle all OFP_Packet_In received
 *
 * @param socket - the socket of the switch to send the packet out to
 * @param openflow header 
 * @param openflow data (post header)
 **/
void handlePacketIn(int clientSocket, uint8_t *packet, 
        uint8_t * payload, int packetLen)
{
    uint8_t raw_pkt_in[packetLen]; // total length of packet in hdr + payload 
    uint8_t to_mac[MAC_ADDR_LEN];
    struct ofp_packet_in * pkt_in;

    // Repack raw packet into Packet In struct
    memcpy(raw_pkt_in, packet, OFP_HDR_LEN);
    memcpy(raw_pkt_in + OFP_HDR_LEN, payload, packetLen - OFP_HDR_LEN);
    
    // Get data from Packet In
    pkt_in = (struct ofp_packet_in *) raw_pkt_in; 

    printf("\tTotal Length: %d\n", ntohs(pkt_in->total_len));
    printf("\tIn Port: %d\n", ntohs(pkt_in->in_port));
    if (pkt_in->reason == 0x00) {
        printf("\tNO MATCHING FLOW\n");
        return;
    } else {
        printf("\tACTION\n");
    }

    // Add a Host/Port to the list. (Host resides at port x)
    memset(to_mac, 0, MAC_ADDR_LEN);
    memcpy(to_mac, pkt_in->data + 6, MAC_ADDR_LEN);
    addPathNode(nodes, to_mac,ntohs(pkt_in->in_port)); 

    // if arp reply is received, connection now flows two ways.
    if (isArpReply(pkt_in->data) == 1) {
        printf("\tARP REPLY\n");
        printf("\tEstablishing two way flows...\n");
        establishFlows(clientSocket, pkt_in);
        makeSendPacketOut(clientSocket, pkt_in, htons((uint16_t)0xfff9));
    } else {
        // make pkt out with output port of flood (0xfffb) ONLY FOR BROADCAST   
        makeSendPacketOut(clientSocket, pkt_in, htons((uint16_t)0xfffb));
    }
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
    uint8_t payload[1000];

    memset(packet, 0, OFP_HDR_LEN);

    /* Read in only the header */
    if ((numBytes = recv(clientSocket,packet,OFP_HDR_LEN,MSG_WAITALL))<=0)
    {
        if (numBytes != 0)
            exit(-1);
        return 0;
    }

    /* Read in OF Packet Contents */
    ofp_hdr = (struct ofp_header *) packet;
    type = ofp_hdr->type;
    packetLen = ntohs(ofp_hdr->length);

    /* If theres anything in the TCP buffer, read the rest of the bytes */ 
    if (packetLen - numBytes > 0) 
    {
        memset(payload, 0, 1000);
        /* Reading additional bytes */
        if ((numBytes = recv(clientSocket, payload, packetLen-numBytes, MSG_WAITALL)) < 0)
        {
            perror("recv call");
            exit(-1);
        }
        //printf("Additional bytes read: %d\n", numBytes);
    }

    /* Execute Action based on OF Packet Type */
    if (type == OF_HELLO) {
        estOFConnection(clientSocket, ofp_hdr);
    } else if (type == OF_FEATURE_REPLY) {
        reportFeatures(ofp_hdr, payload);
        setConfig(clientSocket);
    } else if (type == OF_ECHO_REQUEST) {
        sendEchoReply(clientSocket, ofp_hdr);
    } else if (type == OF_PACKET_IN) {
        printf("RECEIVED PACKET_IN\n");
        // Determine if payload is arp reply (reply -> send flow mod)
        //if (isArpReply(payload) == 1) {
        //    printf("Send Flow mod\n");
        //    printf("send packet out with 0xfff9 as output port\n");
        //} else {
            // packet outs will be flooded
        handlePacketIn(clientSocket, packet, payload, packetLen);
        //}
    } else if (type == OF_PORT_STATUS) {
        reportPort(ofp_hdr, payload);
    } else {
        // printf("not yet implemented\n"); 
        return 1;
    }
    return 1;
}

/** 
 * PTHREAD_CREATE
 **/
void * connection_handler(void * socket_desc)
{
    int *clientSock = socket_desc;
    while(1)
        recvData(*clientSock);

    return 0;
}

/**
 * Method to handle as many switch connections to the controller.
 *
 * @param sd (server socket)
 **/
void handleConnections(int sd)
{
    static int switchID = 0;
    int clientSocket = 0;
    int nfds = 0, i = 0;
    //pthread_t thread_id;

    fd_set set;   // set
    fd_set t_set; // temp set

    FD_ZERO(&set);
    FD_ZERO(&t_set);

    FD_SET(sd, &set);
    nfds = sd;        // larget file descriptor
   
    ll = (struct ListOfLists *) smartalloc(sizeof(struct ListOfLists), 
            "controller.c", 515, '\0'); 

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

                    switchID++;

                    FD_SET(clientSocket, &set);

                    if (nfds < clientSocket) nfds = clientSocket;

                    /* PTHREAD IMPLEMENTATION */
                    /*if (pthread_create(&thread_id, NULL, connection_handler, 
                                (void*)&clientSocket) <0)
                    {
                        perror("could not create thread");
                        exit(-1);
                    }*/
                }

                else 
                {
                    // Turned off for pthread_implementation
                    recvData(i);
                }
            }
        }
    }

    //close(clientSocket);
}

/**
 * A method to catch SIGINT ctr-c and free socket descriptor
 **/
void terminate()
{
    close(sd);
    exit(EXIT_SUCCESS);
}

/**
 * MAIN
 **/
int main(int argc, char * argv[]) 
{
    int port;

    if (argc != 2) 
    {
        printf("usage: ./controller <port number>\n");
        exit(-1);
    }

    if ((port = atoi(argv[1])) == 0)
    {
        printf("Please enter a valid port number\n");
        exit(-1);
    }

    /* Set up the TCP connection on port 6633 */
    sd = tcpSetup(port);    

    signal(SIGINT, terminate);

    handleConnections(sd);

    return 0;
}
