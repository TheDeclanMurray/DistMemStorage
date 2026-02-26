#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdio.h>
#include "message.h"
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include "string.h"
#include <unistd.h> 
#include "socket.h"


#define MAX_MESSAGE_SIZE 256

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


int main(int argc, char** argv) {
    if (argc != 2) {
        perror("Usage: <storage port>\n");
        exit(EXIT_FAILURE);
    }

    unsigned short connection_port = atoi(argv[1]);

    /* Attempt to connect to a storage node*/
    int fd = socket_connect("localhost", connection_port);

    if(fd == -1) {
        perror("Failed to connect to storage.\n");
        exit(EXIT_FAILURE);
    }

    /* Placeholder: take in user input*/
    int rc;
    char request[MAX_MESSAGE_SIZE];
    bool waitForReply;
    char command[MAX_MESSAGE_SIZE];
    while (1){
        

        printf("Storage Command: ");
        if (fgets(command, sizeof(command), stdin) == NULL) {
            
            printf("Error Reading Command Input.\n");
            continue;
        } 
        // remove trailing newline if present
        size_t len = strlen(command);
        if (len > 0 && command[len - 1] == '\n') {
            command[len - 1] = '\0';
        }

        char args[3][256];
        char* copy = strdup(command);
        parseRequest(copy, args);
        free(copy);

        if(strcmp(args[0], "set") == 0){
            strcpy(request, "set:");
            strcat(request, args[1]);
            strcat(request, ":");
            strcat(request, args[2]);
            waitForReply = false;
        } else if (strcmp(args[0], "get") == 0){
            strcpy(request, "get:");
            strcat(request, args[1]);
            waitForReply = true;
        } else {
            printf("Invalid Command Format, Use: set:key:value   or   get:key\n");
            continue;
        }

        rc = send_message(fd, request);
        if (rc == -1){
            perror("Failed To Send Command To Storage Node\n");
            exit(EXIT_FAILURE);
        }

        if(!waitForReply){
            continue;
        }

        char* response = receive_message(fd);
        if (response == NULL){
            perror("Failed to Recieve Response\n");
            exit(EXIT_FAILURE);
        }

        printf("%s\n", response);

    }

    
    

    


    return 0;
}