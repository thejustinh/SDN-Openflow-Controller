#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "smartalloc.h"

/* A structure to represent an adjacency list node */
struct AdjListNode
{
    int dest; // Switch ID
    int destPort; // Port Number leading to Switch ID
    struct AdjListNode * next; 
};

/* A structure to represent the port used to get to a MAC address */
struct PathNode
{
    uint8_t to_mac[6];
    uint16_t port;
    struct PathNode * next;
};

/* A structure to represent the flows */
struct FlowNode
{
    uint8_t dst_mac[6];
    uint8_t src_mac[6];
    uint16_t port;
    struct FlowNode * next;
};

/* A structure to represent a link list of linked lists */
struct ListOfLists
{
    int socket;
    struct PathNode * head;
    struct ListOfLists * next;
};

/* A Link list of flows for each switch connected */
struct ListOfFlows
{
    int socket;
    struct FlowNode * head;
    struct ListOfFlows * next;
};

/* A structure to represent an adjacency list */
struct AdjList
{
    struct AdjListNode *head; // pointer to head node of list
};

/* A structure to represent a graph. A graph is an array of adjacency lists.
   Size of array will be V (number of vertices in the graph */
struct Graph
{
    int V;
    struct AdjList * array;
};

/* A utility function to create a new adjacency list node */
struct AdjListNode * newAdjListNode (int dest, int destPort);

/* A utility function that creates a graph of V vertices */
struct Graph * createGraph(int V); 

/* Adds an edge to a directed graph */
void addEdge(struct Graph * graph, int src, int dest, int destPort); 

struct FlowNode * findRemoveNode(int socket, struct ListOfFlows * flows, uint16_t port);

int findNode(int socket, struct ListOfLists * ll, uint8_t * to_mac);

/* Method to print graph */
void printGraph(struct Graph * graph);

int addPathNode(int socket, struct ListOfLists * ll, uint8_t * to_mac, uint16_t port);

int addFlowNode(int socket, struct ListOfFlows * flows, uint8_t * dst_mac, uint8_t * src_mac, uint16_t prt);
