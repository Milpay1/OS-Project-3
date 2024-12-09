#include "server.h"

#define DEFAULT_ROOM "Lobby"

// USE THESE LOCKS AND COUNTER TO SYNCHRONIZE
extern int numReaders;
extern pthread_mutex_t rw_lock;
extern pthread_mutex_t mutex;

extern struct node *head;
extern struct room *rooms;
extern struct connection *connections;

extern char *server_MOTD;

char *trimwhitespace(char *str)
{
    char *end;

    while (isspace((unsigned char)*str)) str++;

    if (*str == 0) return str;

    end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--;

    end[1] = '\0';
    return str;
}

void *client_receive(void *ptr) {
    int client = *(int *) ptr;  // socket
    int received, i;
    char buffer[MAXBUFF], sbuffer[MAXBUFF];  // data buffer of 2K  
    char tmpbuf[MAXBUFF];  // data temp buffer of 1K  
    char cmd[MAXBUFF], username[20];
    char *arguments[80];

    struct node *currentUser;

    send(client, server_MOTD, strlen(server_MOTD), 0); // Send Welcome Message of the Day.

    // Creating the guest username
    sprintf(username, "guest%d", client);
    head = insertFirstU(head, client, username);

    // Add the guest to the default room (Lobby)
    rooms = insertFirstR(rooms, DEFAULT_ROOM);
    struct room *lobby = findR(rooms, DEFAULT_ROOM);
    addUserToRoom(lobby, findU(head, username));

    while (1) {
        if ((received = read(client, buffer, MAXBUFF)) != 0) {
            buffer[received] = '\0'; 
            strcpy(cmd, buffer);  
            strcpy(sbuffer, buffer);

            arguments[0] = strtok(cmd, " ");
            i = 0;
            while (arguments[i] != NULL) {
                arguments[++i] = strtok(NULL, " ");
                if (arguments[i - 1]) {
                    arguments[i - 1] = trimwhitespace(arguments[i - 1]);
                }
            }

            if (strcmp(arguments[0], "create") == 0) {
                printf("create room: %s\n", arguments[1]);
                rooms = insertFirstR(rooms, arguments[1]);
                sprintf(buffer, "Room '%s' created.\nchat>", arguments[1]);
                send(client, buffer, strlen(buffer), 0);
            } else if (strcmp(arguments[0], "join") == 0) {
                printf("join room: %s\n", arguments[1]);
                struct room *room = findR(rooms, arguments[1]);
                if (room) {
                    addUserToRoom(room, findU(head, username));
                    sprintf(buffer, "Joined room '%s'.\nchat>", arguments[1]);
                } else {
                    sprintf(buffer, "Room '%s' not found.\nchat>", arguments[1]);
                }
                send(client, buffer, strlen(buffer), 0);
            } else if (strcmp(arguments[0], "leave") == 0) {
                printf("leave room: %s\n", arguments[1]);
                struct room *room = findR(rooms, arguments[1]);
                if (room) {
                    removeUserFromRoom(room, username);
                    sprintf(buffer, "Left room '%s'.\nchat>", arguments[1]);
                } else {
                    sprintf(buffer, "Room '%s' not found.\nchat>", arguments[1]);
                }
                send(client, buffer, strlen(buffer), 0);
            } else if (strcmp(arguments[0], "connect") == 0) {
                printf("connect to user: %s\n", arguments[1]);
                struct node *targetUser = findU(head, arguments[1]);
                if (targetUser) {
                    connections = addConnection(connections, username, arguments[1]);
                    sprintf(buffer, "Connected to '%s'.\nchat>", arguments[1]);
                } else {
                    sprintf(buffer, "User '%s' not found.\nchat>", arguments[1]);
                }
                send(client, buffer, strlen(buffer), 0);
            } else if (strcmp(arguments[0], "disconnect") == 0) {
                printf("disconnect from user: %s\n", arguments[1]);
                connections = removeConnection(&connections, username, arguments[1]);
                sprintf(buffer, "Disconnected from '%s'.\nchat>", arguments[1]);
                send(client, buffer, strlen(buffer), 0);
            } else if (strcmp(arguments[0], "rooms") == 0) {
                struct room *currentRoom = rooms;
                sprintf(buffer, "Rooms:\n");
                while (currentRoom) {
                    strcat(buffer, currentRoom->roomName);
                    strcat(buffer, "\n");
                    currentRoom = currentRoom->next;
                }
                strcat(buffer, "chat>");
                send(client, buffer, strlen(buffer), 0);
            } else if (strcmp(arguments[0], "users") == 0) {
                struct node *currentUser = head;
                sprintf(buffer, "Users:\n");
                while (currentUser) {
                    strcat(buffer, currentUser->username);
                    strcat(buffer, "\n");
                    currentUser = currentUser->next;
                }
                strcat(buffer, "chat>");
                send(client, buffer, strlen(buffer), 0);
            } else if (strcmp(arguments[0], "login") == 0) {
                struct node *currentUser = findU(head, username);
                strcpy(currentUser->username, arguments[1]);
                strcpy(username, arguments[1]);
                sprintf(buffer, "Logged in as '%s'.\nchat>", arguments[1]);
                send(client, buffer, strlen(buffer), 0);
            } else if (strcmp(arguments[0], "exit") == 0 || strcmp(arguments[0], "logout") == 0) {
                struct room *currentRoom = rooms;
                while (currentRoom) {
                    removeUserFromRoom(currentRoom, username);
                    currentRoom = currentRoom->next;
                }
                close(client);
                break;
            } else {
                sprintf(tmpbuf, "::%s> %s\nchat>", username, sbuffer);
                strcpy(sbuffer, tmpbuf);
                currentUser = head;
                while (currentUser != NULL) {
                    if (client != currentUser->socket) {
                        send(currentUser->socket, sbuffer, strlen(sbuffer), 0);
                    }
                    currentUser = currentUser->next;
                }
            }
            memset(buffer, 0, sizeof(buffer));
        }
    }
    return NULL;
}
