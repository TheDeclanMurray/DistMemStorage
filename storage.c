#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <pthread.h>
#include <stdint.h>
#include <ctype.h>
#include <unistd.h>
#include <stdint.h>
#include "message.h"
#include "socket.h"
#include "hashmap.h"
#include "unistd.h"

#define NUM_S_NODES 2

HashTable_t storage_table;
int thisNodesID;
int next_node;
unsigned short storageNodes[NUM_S_NODES] = {8080, 8081};
int nextNodeFD = -1;


void parseRequest(char *message, char args[3][256])
{
    // "{action}:{key}:{value?}" 
    char* token = strtok(message, ":");
    int i = 0;
    while (token != NULL){
        strcpy(args[i],token);
        i++;
        token = strtok(NULL, ":");
    }
}

// use hashed key
bool isKeyInMyRange(char* key){
    uint64_t hashedint = hash(key);

    /* If the least significant bit is a one, it belongs with the one node, etc...*/
    if ((hashedint & 1) == thisNodesID) {
        return true;
    }
    return false;
    
}

void *listenerThread(void *args)
{
    intptr_t client_socket_fd = (intptr_t)args;
    while (true)
    {
        // Read a message from the client
        char *message = receive_message(client_socket_fd);
        if (message == NULL)
        {
            perror("Failed to read message from client");
            exit(EXIT_FAILURE);
        }
        else if (strcmp(message, "quit\n") == 0)
        {
            break;
        }

        //
        char args[3][256];
        parseRequest(message, args);

        // check if key is in our range if not send to next
        char *key = args[1];
        if (!isKeyInMyRange(key))
        {
            // forward message to next storage node
            int rc = send_message(nextNodeFD, message);
            if (rc == -1){
                perror("Node Forwarding Failed.");
                exit(EXIT_FAILURE);
            }

            // wait for next sorage node to respond
            char* response = receive_message(nextNodeFD);
            if (response == NULL){
                perror("Node Forward Met No Response.");
                exit(EXIT_FAILURE);
            }

            // return response to client
            rc = send_message(client_socket_fd, response);
            if (rc == -1){
                perror("Failed To Return Response To Client.");
                exit(EXIT_FAILURE);
            }
            continue;
        }

        char *action = args[0];

        if (strcmp(action, "get") == 0)
        {
            // get
            char *value = ht_get(key, &storage_table);
            // TODO: handle if we dont have key
            if(value == NULL){
                value = "Key Not Set.\n";
            }
            // send message back to requester
            int rc = send_message(client_socket_fd, value);
            if (rc == -1){
                perror("Failed To Return Response To Client.");
                exit(EXIT_FAILURE);
            }

        }
        else if (strcmp(action, "set") == 0)
        {
            // set
            char *value = args[2];
            ht_set(key, value, &storage_table);
        }
    }
    // close client socket
    close(client_socket_fd);
    return NULL;
}

void attachNewStorageNode(char *nodeHashId, char *nodeAddress);

void *connectToStorageNode(void * args){
    
    unsigned short next_port = storageNodes[next_node];
    do
    {
        nextNodeFD = socket_connect("localhost", next_port);
        printf("Attempted Connection.\n");
        sleep(2);

    } while (nextNodeFD == -1);
    printf("Connection Established!\n");
    return NULL;
}

int main(int argc, char* argv[])
{
    if (argc < 2){
        perror("args: <int NodeID>");
        exit(EXIT_FAILURE);
    }
    thisNodesID = atoi(argv[1]);
    next_node = (thisNodesID + 1) % NUM_S_NODES;

    // create hash table
    storage_table.items = malloc(sizeof(HT_Chain_t) * 2048);
    storage_table.size = 2048;
    storage_table.count = 0;


    // Open a server socket
    unsigned short port = storageNodes[thisNodesID];
    int server_socket_fd = server_socket_open(&port);
    if (server_socket_fd == -1)
    {
        perror("Server socket was not opened");
        exit(EXIT_FAILURE);
    }

    // create thread to connect to prev node
    pthread_t NodeConnect;
    pthread_create(&NodeConnect, NULL, connectToStorageNode, NULL);

    bool running = true;
    while (running)
    {
        // create client
        pthread_t client;

        intptr_t client_socket_fd = server_socket_accept(server_socket_fd);
        // update sockets
        if (client_socket_fd == -1)
        {
            perror("accept failed");
            exit(EXIT_FAILURE);
        }

        pthread_create(&client, NULL, listenerThread, (void *)client_socket_fd);
    }
}