#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "headers/linkedlist.h"

// Creates and returns a new node with given data.
Node* create_node(char* req) {
    Node* new_node = (Node*)malloc(sizeof(Node));
    if (!new_node) {
        fprintf(stderr, "Failed to allocate memory to new node.\n");
        exit(EXIT_FAILURE);
    }

    new_node->req = req;
    new_node->next = NULL;
    new_node->prev = NULL;
    return new_node;
}

// Adds a node with page number to the head of the doubly linked list.
void add_head(Node** head, Node** tail, char* req) {
    Node* new_node = create_node(req);
    new_node->next = *head;

    // If the head is not NULL, set the prev pointer to the new node
    if (*head != NULL) (*head)->prev = new_node;

    // Set the new head
    *head = new_node;

    // If the list was previously empty
    if (*tail == NULL) {
        // Set the tail to the new node
        *tail = new_node;
    }
}

// Adds a node with page number to the tail of the doubly linked list.
void add_tail(Node** head, Node** tail, char* req) {
    Node* newNode = create_node(req);

    // If the list is empty
    if (*tail == NULL) {
        // Head and tail are the same
        *head = *tail = newNode;
    } else {  // If list is nonempty
        // Append to the end of the list
        newNode->prev = *tail;
        (*tail)->next = newNode;
        *tail = newNode;
    }
}

// Finds a node by page number and moves it to the head.
unsigned int find_move_head(Node** head, Node** tail, char* req) {
    Node* current = *head;

    while (current != NULL) {
        if (strcmp(current->req, req) == 0) {
            // Do nothing if page already at head
            if (current == *head) {
                return 1;
            }

            // Detatch the node from its current position
            if (current->prev) {
                current->prev->next = current->next;
            }
            if (current->next) {
                current->next->prev = current->prev;
            }

            // Update the tail if necessary
            if (current == *tail) {
                *tail = current->prev;
            }

            // Move the node to head
            current->prev = NULL;
            current->next = *head;
            if (*head != NULL) (*head)->prev = current;
            *head = current;

            return 1;
        }
        current = current->next;
    }

    // If not found
    return -1;
}

// Free the entire list.
void freeList(Node* head) {
    Node* temp;
    while (head != NULL) {
        temp = head;
        head = head->next;
        free(temp->req);
        free(temp);
    }
}