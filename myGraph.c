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
