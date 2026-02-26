#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include "message.h"
#include "socket.h"


#define MAX_MESSAGE_SIZE 256

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
 * @brief Client program for interacting with distributed storage nodes.
 *
 * Connects to a storage node on specified port and provides interactive command-line
 * interface for set/get operations. Parses user input, constructs request strings,
 * sends to storage node, and displays responses for get commands.
 *
 * @param argc Number of command line arguments.
 * @param argv Argument array; expects argv[1] as storage port number.
 * @return Never returns normally (exit on error).
 */

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
    char* copy;
    char* response;
    char request[MAX_MESSAGE_SIZE];
    bool waitForReply;
    char command[MAX_MESSAGE_SIZE];
    while (1){
        // get commands
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

        // separate args
        char args[3][256] = {(""), (""), ("")};
        copy = strdup(command);
        parseRequest(copy, args);
        free(copy);

        // create valid command
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

        response = receive_message(fd);
        if (response == NULL){
            perror("Failed to Recieve Response\n");
            exit(EXIT_FAILURE);
        }

        printf("%s\n", response);
        free(response);
    }
    return 0;
}