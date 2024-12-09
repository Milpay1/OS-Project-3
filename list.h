#ifndef LIST_H
#define LIST_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

// Node structure for managing users
struct node {
    char username[30];         // Username of the user
    int socket;                // Socket descriptor for the user
    struct node *next;         // Pointer to the next node
};

// Room structure for managing chat rooms
struct room {
    char roomName[50];         // Name of the chat room
    struct node *users;        // Linked list of users in this room
    struct room *next;         // Pointer to the next room
};

// Connection structure for managing direct user connections
struct connection {
    char user1[30];            // Username of the first user
    char user2[30];            // Username of the second user
    struct connection *next;   // Pointer to the next connection
};

//////////////////////// USER LIST OPERATIONS ////////////////////////

// Insert a user at the beginning of the list
struct node* insertFirstU(struct node *head, int socket, char *username);

// Find a user by username
struct node* findU(struct node *head, char *username);

//////////////////////// ROOM LIST OPERATIONS ////////////////////////

// Insert a room at the beginning of the list
struct room* insertFirstR(struct room *head, char *roomName);

// Find a room by name
struct room* findR(struct room *head, char *roomName);

// Add a user to a room
void addUserToRoom(struct room *room, struct node *user);

// Remove a user from a room
void removeUserFromRoom(struct room *room, char *username);

///////////////////// CONNECTION LIST OPERATIONS /////////////////////

// Add a direct connection between two users
struct connection* addConnection(struct connection *head, char *user1, char *user2);

// Find a direct connection between two users
struct connection* findConnection(struct connection *head, char *user1, char *user2);

// Remove a direct connection between two users
struct connection* removeConnection(struct connection **head, char *user1, char *user2);

//////////////////////// THREAD SAFETY ///////////////////////////////

#include <pthread.h>

// Mutexes for thread safety
extern pthread_mutex_t userLock;       // Protect user list
extern pthread_mutex_t roomLock;       // Protect room list
extern pthread_mutex_t connectionLock; // Protect connection list

#endif // LIST_H
