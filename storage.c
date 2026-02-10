#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <pthread.h>
#include <stdint.h>
#include <ctype.h>
#include "message.h"
#include "socket.h"
#include "storage.h"
#include "hashmap.h"


HashTable_t storage_table;
storage_table.items = malloc(sizeof(HT_Chain_t) * 2048);
storage_table.size = 2048;
storage_table.count = 0;

char** parseRequest(char*){
    "{action}:{key}:{value?}"
    return NULL;
}

void* listenerThread(void* args){
    intptr_t client_socket_fd = (intptr_t)args;
    while(true) {
        // Read a message from the client                                           
        char* message = receive_message(client_socket_fd);
        if (message == NULL) {
            perror("Failed to read message from client");
            exit(EXIT_FAILURE);
        } else if (strcmp(message, "quit\n") == 0) {
            break;
        }

        // 
        char** args = parseRequest(message);

        // check if key is in our range if not send to next
        char* key = args[1];
        int hash = ??;
        bool inRange = isKeyInMyHash(hash);
        if (!inRange){
            // TODO: Forward message to next storage node
            // then wait for responce, return to requester
            continue;
        }

        char* action = args[0];
        

        if (strcmp(action, "get") == 0){
            // get
            char* value = get(key);
            // TODO: handle if we dont have key
            // send message back to requester
        }else if (strcmp(action, "set") == 0){
            // set
            char* value = args[2];
            bool success = set(key, value);
            // send message back to requester
        }

    }
    //close client socket                                                         
    close(client_socket_fd);
    return NULL;

}

void attachNewStorageNode(char* nodeHashId, char* nodeAddress);

// use hashed key
bool isHashInMyRange(int hash);

bool set(char* key, char* value);

bool get(char* key);


int main(){
    // Open a server socket                                                       
  unsigned short port = 0;
  int server_socket_fd = server_socket_open(&port);
  if (server_socket_fd == -1) {
    perror("Server socket was not opened");
    exit(EXIT_FAILURE);
  }

    bool running = true;
    while (running) {
        //create client
        pthread_t client;

        intptr_t client_socket_fd = server_socket_accept(server_socket_fd);
        //update sockets
        if (client_socket_fd == -1) {
            perror("accept failed");
            exit(EXIT_FAILURE);
        }

        pthread_create(&client, NULL, listenerThread, (void*)client_socket_fd);
    }
}