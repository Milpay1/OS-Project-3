#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <pthread.h>
#include "list.h"

// Mutexes for thread safety
pthread_mutex_t userLock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t roomLock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t connectionLock = PTHREAD_MUTEX_INITIALIZER;

///////////////////////// USER LIST FUNCTIONS //////////////////////////

// Insert a user at the beginning of the list
struct node* insertFirstU(struct node *head, int socket, char *username) {
    pthread_mutex_lock(&userLock);

    if (findU(head, username) == NULL) {
        struct node *link = (struct node*) malloc(sizeof(struct node));
        link->socket = socket;
        strcpy(link->username, username);
        link->next = head;
        head = link;
    } else {
        printf("Duplicate: %s\n", username);
    }

    pthread_mutex_unlock(&userLock);
    return head;
}

// Find a user by username
struct node* findU(struct node *head, char* username) {
    struct node *current = head;

    while (current != NULL && strcmp(current->username, username) != 0) {
        current = current->next;
    }

    return current; // Returns NULL if user not found
}

///////////////////////// ROOM LIST FUNCTIONS //////////////////////////

// Insert a room at the beginning of the list
struct room* insertFirstR(struct room *head, char *roomName) {
    pthread_mutex_lock(&roomLock);

    struct room *link = (struct room*) malloc(sizeof(struct room));
    strcpy(link->roomName, roomName);
    link->users = NULL;
    link->next = head;
    head = link;

    pthread_mutex_unlock(&roomLock);
    return head;
}

// Find a room by name
struct room* findR(struct room *head, char *roomName) {
    struct room *current = head;

    while (current != NULL && strcmp(current->roomName, roomName) != 0) {
        current = current->next;
    }

    return current; // Returns NULL if room not found
}

// Add a user to a room
void addUserToRoom(struct room *room, struct node *user) {
    pthread_mutex_lock(&roomLock);

    if (room != NULL && user != NULL && findU(room->users, user->username) == NULL) {
        room->users = insertFirstU(room->users, user->socket, user->username);
    }

    pthread_mutex_unlock(&roomLock);
}

// Remove a user from a room
void removeUserFromRoom(struct room *room, char *username) {
    pthread_mutex_lock(&roomLock);

    if (room != NULL && username != NULL) {
        struct node *current = room->users;
        struct node *prev = NULL;

        while (current != NULL && strcmp(current->username, username) != 0) {
            prev = current;
            current = current->next;
        }

        if (current != NULL) { // User found
            if (prev == NULL) { // User is the first node
                room->users = current->next;
            } else {
                prev->next = current->next;
            }
            free(current);
        }
    }

    pthread_mutex_unlock(&roomLock);
}

/////////////////////// CONNECTION LIST FUNCTIONS ///////////////////////

// Add a direct connection between two users
struct connection* addConnection(struct connection *head, char *user1, char *user2) {
    pthread_mutex_lock(&connectionLock);

    struct connection *link = (struct connection*) malloc(sizeof(struct connection));
    strcpy(link->user1, user1);
    strcpy(link->user2, user2);
    link->next = head;
    head = link;

    pthread_mutex_unlock(&connectionLock);
    return head;
}

// Find a direct connection between two users
struct connection* findConnection(struct connection *head, char *user1, char *user2) {
    struct connection *current = head;

    while (current != NULL) {
        if ((strcmp(current->user1, user1) == 0 && strcmp(current->user2, user2) == 0) ||
            (strcmp(current->user1, user2) == 0 && strcmp(current->user2, user1) == 0)) {
            return current;
        }
        current = current->next;
    }

    return NULL; // Connection not found
}

// Remove a direct connection between two users
struct connection* removeConnection(struct connection **head, char *user1, char *user2) {
    pthread_mutex_lock(&connectionLock);

    struct connection *current = *head;
    struct connection *prev = NULL;

    while (current != NULL) {
        if ((strcmp(current->user1, user1) == 0 && strcmp(current->user2, user2) == 0) ||
            (strcmp(current->user1, user2) == 0 && strcmp(current->user2, user1) == 0)) {
            if (prev == NULL) { // Remove the head connection
                *head = current->next;
            } else {
                prev->next = current->next;
            }
            free(current);
            pthread_mutex_unlock(&connectionLock);
            return *head;
        }
        prev = current;
        current = current->next;
    }

    pthread_mutex_unlock(&connectionLock);
    return *head; // Connection not found
}
