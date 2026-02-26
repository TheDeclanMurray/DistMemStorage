#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <pthread.h>
#include <stdint.h>
#include <ctype.h>
#include <unistd.h>
#include "message.h"
#include "hashmap.h"
#include "socket.h"

#define NUM_S_NODES 2

HashTable_t storage_table;
int thisNodesID;
int next_node;
unsigned short storageNodes[NUM_S_NODES] = {8080, 8081};
int nextNodeFD = -1;

/**
 * @brief Parses a colon-separated request string into up to three argument tokens.
 *
 * Expected input format: "{action}:{key}:{value?}". The function splits the input string
 * using ':' as delimiter and copies each token sequentially into the provided args array.
 * 
 * Example:
 * Input:  "GET:temp:75"
 * Output: args[0] = "GET", args[1] = "temp", args[2] = "75"
 *
 * @param message Pointer to null-terminated input string to parse (MODIFIED by strtok).
 * @param args    Pre-allocated 2D char array [3][256] to store up to 3 tokens.
 *
 * @note Modifies original message buffer due to strtok usage
 * @note No bounds checking - tokens longer than 255 chars may overflow
 * @note Unused args slots retain previous values/garbage data
 */
void parseRequest(char *message, char args[3][256])
{
    char* token = strtok(message, ":");
    int i = 0;
    while (token != NULL){
        strcpy(args[i],token);
        i++;
        token = strtok(NULL, ":");
    }
}

/**
 * @brief Determines if a given key belongs to this storage node's range.
 *
 * Hashes the input key and checks if the least significant bit matches
 * this node's ID to determine ownership in a distributed hash table ring.
 *
 * @param key Null-terminated string key to check.
 * @return True if key belongs to this node, false otherwise.
 */
bool isKeyInMyRange(char* key){
    uint64_t hashedint = hash(key);

    /* If the least significant bit is a one, it belongs with the one node, etc...*/
    if ((hashedint & 1) == thisNodesID) {
        return true;
    }
    return false;
    
}

/**
 * @brief Listener thread for handling client requests in a distributed storage node.
 *
 * Receives messages from clients, parses commands (get/set), determines if the key
 * belongs to this node using isKeyInMyRange(). Forwards to next node if needed,
 * handles get/set operations on local hash table, and routes responses back.
 *
 * @param args Client socket file descriptor cast as void*.
 * @return NULL on completion.
 */
void *listenerThread(void *args)
{
    intptr_t client_socket_fd = (intptr_t)args;
    char *message;
    char* copy;
    char* key;
    int rc;
    char* response;
    char *action;
    char *value;

    while (true)
    {
        // Read a message from the client
        message = receive_message(client_socket_fd);
        if (message == NULL)
        {
            perror("Client Disconected");
            return NULL;
        }
        printf("Command Received: %s\n", message);

        // seperate the arguments
        char args[3][256];
        copy = strdup(message);
        parseRequest(copy, args);
        free(copy);


        // check if key is in our range if not send to next
        key = args[1];
        if (!isKeyInMyRange(key))
        {
            printf("Key <%s> not in Node, Forwarding To Next Node\n", key);
            // forward message to next storage node
            rc = send_message(nextNodeFD, message);
            if (rc == -1){
                perror("Node Forwarding Failed.");
                exit(EXIT_FAILURE);
            }
            free(message);

            // only wait for and forward responses if a get request
            if(strcmp(args[0], "get") != 0){
                continue;
            }

            // wait for next sorage node to respond
            response = receive_message(nextNodeFD);
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
            free(response);
        }else{
            printf("Key in Node\n");
            free(message);
            action = args[0];
            if (strcmp(action, "get") == 0)
            {
                // get
                value = ht_get(key, &storage_table);
                // handle if we dont have key
                if(value == NULL){
                    value = "Key Not Set.\n";
                }
                printf("Key <%s> = <%s>", key, value);
                // send message back to requester
                rc = send_message(client_socket_fd, value);
                if (rc == -1){
                    perror("Failed To Return Response To Client.");
                    exit(EXIT_FAILURE);
                }

            }
            else if (strcmp(action, "set") == 0)
            {
                // set
                value = args[2];
                ht_set(key, value, &storage_table);
            }
        }

        
    }
    // close client socket
    close(client_socket_fd);
    return NULL;
}

/**
 * @brief Thread function to establish connection to next storage node.
 *
 * Retries connection to next storage node every 3 seconds until successful.
 * Updates global nextNodeFD upon connection.
 *
 * @param args Unused argument parameter.
 * @return NULL on successful connection.
 */
void *connectToStorageNode(void * args){
    unsigned short next_port = storageNodes[next_node];
    // attempt to connect to other nodes
    do
    {
        sleep(3);
        printf("Attempting Connecton\n");
        nextNodeFD = socket_connect("localhost", next_port);
    } while (nextNodeFD == -1);
    printf("Connection Established!\n");
    return NULL;
}

/**
 * @brief Main entry point for distributed storage node server.
 *
 * Initializes hash table, opens server socket, starts connection thread to next node,
 * and spawns listener threads for each accepted client connection. Implements a ring
 * topology where nodes forward requests based on key ownership.
 *
 * @param argc Number of command line arguments.
 * @param argv Argument array; expects argv[1] as NodeID (0 or 1).
 * @return Never returns normally (exit on error).
 */
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

    // Start listening for connections, with a maximum of one queued connection
    if (listen(server_socket_fd, 1)) {
        perror("listen failed");
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