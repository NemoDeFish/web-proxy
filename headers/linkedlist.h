/*
    Written by Si Yong Lim and Ella McKercher.
*/

#ifndef LINKED_LIST_H
#define LINKED_LIST_H

typedef struct Node {
    char* req;
    struct Node* next;
    struct Node* prev;
} Node;

// Creates and returns a new node with given data.
Node* create_node(char* req);

// Adds a node with page number to the head of the doubly linked list.
void add_head(Node** head, Node** tail, char* req);

// Adds a node with page number to the tail of the doubly linked list.
void add_tail(Node** head, Node** tail, char* req);

// Finds a node by page number and moves it to the head.
unsigned int find_move_head(Node** head, Node** tail, char* req);

// Free the entire list.
void freeList(Node* head);

#endif