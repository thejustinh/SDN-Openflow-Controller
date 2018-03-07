/**
 * A C program to create an adjacency list representation of graphs
 **/

#include <stdio.h>
#include <stdlib.h>
#include "myGraph.h"

/* A utility function to create a new adjacency list node */
struct AdjListNode * newAdjListNode (int dest, int destPort)
{
    struct AdjListNode * newNode = (struct AdjListNode *) smartalloc(
        sizeof(struct AdjListNode), "myGraph.c", 33, '\0');
    newNode->dest = dest;
    newNode->destPort = destPort;
    newNode->next = NULL;
    return newNode;
}

/* A utility function that creates a graph of V vertices */
struct Graph * createGraph(int V)
{
    int i;

    struct Graph* graph = (struct Graph *) smartalloc(sizeof(struct Graph), 
        "myGraph.c", 25, '\0');
    graph->V = V;

    graph->array = (struct AdjList *) smartalloc(V * sizeof(struct AdjList), 
        "myGraph.c", 29, '\0');

    for (i = 0; i < V; ++i) 
        graph->array[i].head = NULL;

    return graph;
}

/* Adds an edge to a directed graph */
void addEdge(struct Graph * graph, int src, int dest, int destPort)
{
    // Add edge from src to dest.
    struct AdjListNode * newNode = newAdjListNode(dest, destPort);
    newNode->next = graph->array[src].head;
    graph->array[src].head = newNode;
}

void printGraph(struct Graph * graph)
{
    for (int v = 0; v < graph->V; ++v)
    {
        struct AdjListNode * p = graph->array[v].head;
        printf("\nAdjacency list of vertex %d\n head ", v);
        while (p)
        {
            printf("-> take %d to %d", p->destPort, p->dest);
            p = p->next;
        }
        printf("\n");
    }
}


/* Method to return 1 if a mac address exists*/
int findNode(int socket, struct ListOfLists * ll, uint8_t * to_mac)
{
    struct ListOfLists * cur = ll;
    struct PathNode * cur_node;

    while (cur != NULL) {
        if (cur->socket == socket) {
            cur_node = cur->head;
            while(cur_node != NULL) {
                if (memcmp(cur_node->to_mac, to_mac, 6) == 0)
                    return 1;
                cur_node = cur_node->next;
            }
            return 0;
        }
        cur = cur->next;
    }

    return 0; // No existing node found
}

struct FlowNode * findFlow(int socket, struct ListOfFlows * flows, uint8_t * dst_mac, uint8_t * src_mac)
{
    struct ListOfFlows * cur = flows;
    struct FlowNode * cur_node;

    while (cur != NULL) {
        if (cur->socket == socket) {
            cur_node = cur->head;
            while (cur_node != NULL)
            {
                if (memcmp(cur_node->dst_mac, dst_mac, 6) == 0 &&
                        memcmp(cur_node->src_mac, src_mac, 6) == 0) 
                    return cur_node;
                cur_node = cur_node ->next;
            }
            return NULL;
        }
        cur = cur->next;
    }

    return NULL;
}

struct FlowNode * findRemoveNode(int socket, struct ListOfFlows * flows, uint16_t port)
{
    struct FlowNode * tmp;
    struct FlowNode * prev;
    struct ListOfFlows * cur = flows;

    while (cur != NULL)
    {
        if (cur->socket == socket) 
        {
            tmp = cur->head;

            if (tmp == NULL)
                return NULL;

            if (tmp->port == port) {
                cur->head = tmp->next;
                return tmp;
            }

            while (tmp != NULL && tmp->port != port)
            {
                prev = tmp;
                tmp = tmp->next;
            }

            if (tmp == NULL)
                return NULL; // did not find node with port specified

            prev->next = tmp->next;
            return tmp; // Return deleted node, dont forget to free!
        }
        cur = cur->next;
    }

    return NULL; // Did not find node with port specified
}

int addPathNode(int socket, struct ListOfLists * ll, uint8_t * to_mac, uint16_t port)
{
    struct PathNode * newNode;
    struct ListOfLists * listHead;
    struct ListOfLists * cur;

    /* PathNode already exists, no need to add */
    if (findNode(socket, ll, to_mac) == 1)
        return 0;

    /* Nothing in outer loop (socket list) */
    if (ll == NULL) {
       listHead = (struct ListOfLists *) smartalloc(sizeof(struct ListOfLists),"mygraph.c", 84, '\0');    
       listHead->socket = socket;
       listHead->head = NULL;
       listHead->next = NULL;
       ll = listHead;
    } 

    /* Find the inner linked list given a socket # */
    cur = ll;
    while (cur != NULL) {
        if (cur->socket == socket) {
            break;
        }
        if (cur->next == NULL) {
            listHead = (struct ListOfLists *) smartalloc(sizeof(struct ListOfLists), "mygraph.c", 98, '\0');    
            listHead->socket = socket;
            listHead->head = NULL;
            listHead->next = NULL;
            cur->next = listHead;
            cur = cur->next;
            break;
        }
        cur = cur->next;
    }   

    newNode = (struct PathNode *) smartalloc(sizeof(struct PathNode), 
            "mygraph.c", 110, '\0');
    
    memcpy(newNode->to_mac, to_mac, 6);
    newNode->port = port;

    newNode->next = cur->head;
    cur->head = newNode;

    return 1; // Success!
}

int addFlowNode(int socket, struct ListOfFlows * flows, uint8_t * dst_mac, uint8_t * src_mac, uint16_t port) 
{
    struct FlowNode * newNode;
    struct ListOfFlows * listHead;
    struct ListOfFlows * cur;
    
    if (findFlow(socket, flows, dst_mac, src_mac) != NULL)
        return 0; // Node already present (Did not add)

    if (flows == NULL) {
        listHead = (struct ListOfFlows *) smartalloc(sizeof(struct ListOfFlows), "myGraph.c", 167, '\0');
        listHead->socket = socket;
        listHead->head = NULL;
        listHead->next = NULL;
        flows = listHead;
    }
   
    cur = flows;
    while (cur != NULL) {
        if (cur->socket == socket) {
            break;
        }
        if (cur->next == NULL) {
            listHead = (struct ListOfFlows * ) smartalloc(sizeof(struct ListOfFlows), "myGraph.c", 225, '\0');
            listHead->socket = socket;
            listHead->head = NULL;
            listHead->next = NULL;
            cur->next = listHead;
            cur = cur->next;
            break;
        }
        cur = cur->next;
    }

    newNode = (struct FlowNode * ) smartalloc(sizeof(struct FlowNode), "myGraph.c", 190, '\0');

    memcpy(newNode->dst_mac, dst_mac, 6);
    memcpy(newNode->src_mac, src_mac, 6);
    newNode->port = port;

    newNode->next = cur->head;
    cur->head = newNode;

    return 1; // successfully added!
}
