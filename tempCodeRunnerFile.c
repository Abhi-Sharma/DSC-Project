#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <omp.h>

typedef struct Node
{
    int vertex;
    int weight;
    struct Node *next;
} Node;

typedef struct Graph
{
    int numVertices;
    Node **adjLists;
} Graph;

Node *createNode(int vertex, int weight);
Graph *createGraph(int vertices);
void addEdge(Graph *graph, int src, int dest, int weight);
void dijkstra(Graph *graph, int src);
void freeGraph(Graph *graph);

Node *createNode(int vertex, int weight)
{
    Node *newNode = (Node *)malloc(sizeof(Node));
    if (!newNode)
    {
        printf("Memory allocation failed!\n");
        exit(EXIT_FAILURE);
    }
    newNode->vertex = vertex;
    newNode->weight = weight;
    newNode->next = NULL;
    return newNode;
}

Graph *createGraph(int vertices)
{
    Graph *graph = (Graph *)malloc(sizeof(Graph));
    if (!graph)
    {
        printf("Memory allocation failed!\n");
        exit(EXIT_FAILURE);
    }
    graph->numVertices = vertices;
    graph->adjLists = (Node **)malloc(vertices * sizeof(Node *));
    if (!graph->adjLists)
    {
        printf("Memory allocation failed!\n");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < vertices; i++)
    {
        graph->adjLists[i] = NULL;
    }
    return graph;
}

void addEdge(Graph *graph, int src, int dest, int weight)
{
    Node *newNode = createNode(dest, weight);
    newNode->next = graph->adjLists[src];
    graph->adjLists[src] = newNode;

    newNode = createNode(src, weight);
    newNode->next = graph->adjLists[dest];
    graph->adjLists[dest] = newNode;
}

void dijkstra(Graph *graph, int src)
{
    int numVertices = graph->numVertices;
    int *dist = (int *)malloc(numVertices * sizeof(int));
    int *visited = (int *)malloc(numVertices * sizeof(int));

    if (!dist || !visited)
    {
        printf("Memory allocation failed!\n");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < numVertices; i++)
    {
        dist[i] = INT_MAX;
        visited[i] = 0;
    }

    dist[src] = 0;

    for (int count = 0; count < numVertices - 1; count++)
    {
        int u = -1;

        // Select the unvisited vertex with the smallest distance (Serial)
        for (int i = 0; i < numVertices; i++)
        {
            if (!visited[i] && (u == -1 || dist[i] < dist[u]))
            {
                u = i;
            }
        }

        if (u == -1)
            break;
        visited[u] = 1;

        Node *temp = graph->adjLists[u];

// Relax edges in parallel
#pragma omp parallel
        {
            while (temp != NULL)
            {
                int v = temp->vertex;
                int weight = temp->weight;

#pragma omp critical
                {
                    if (!visited[v] && dist[u] != INT_MAX && dist[u] + weight < dist[v])
                    {
                        dist[v] = dist[u] + weight;
                    }
                }
                temp = temp->next;
            }
        }
    }

    printf("Vertex\tDistance from Source\n");
    for (int i = 0; i < numVertices; i++)
    {
        printf("%d\t%d\n", i, dist[i]);
    }

    free(dist);
    free(visited);
}

void freeGraph(Graph *graph)
{
    for (int i = 0; i < graph->numVertices; i++)
    {
        Node *temp = graph->adjLists[i];
        while (temp)
        {
            Node *toFree = temp;
            temp = temp->next;
            free(toFree);
        }
    }
    free(graph->adjLists);
    free(graph);
}

int main()
{
    int vertices, edges, src;
    printf("Enter the number of vertices: ");
    scanf("%d", &vertices);

    Graph *graph = createGraph(vertices);

    printf("Enter the number of edges: ");
    scanf("%d", &edges);

    printf("Enter edges in the format (src dest weight):\n");
    for (int i = 0; i < edges; i++)
    {
        int src, dest, weight;
        scanf("%d %d %d", &src, &dest, &weight);
        addEdge(graph, src, dest, weight);
    }

    printf("Enter the source vertex: ");
    scanf("%d", &src);

    if (src < 0 || src >= vertices)
    {
        printf("Invalid source vertex!\n");
        freeGraph(graph);
        return 1;
    }

    printf("Dijkstra's Algorithm:\n");
    dijkstra(graph, src);

    freeGraph(graph);
    return 0;
}
