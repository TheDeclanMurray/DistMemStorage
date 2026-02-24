#include <stdlib.h>
#include <stdio.h>

int main(int argc, char** argv) {
    if (argc != 2) {
        perror("Wrong number of args\n");
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
    char* request;

    int rc = send_message(fd, request);
        if (rc == -1){
            perror("Failed To Return Response To Client.");
            exit(EXIT_FAILURE);
        }
    
    char* response = receive_message(fd);
        if (response == NULL){
            perror("Node Forward Met No Response.");
            exit(EXIT_FAILURE);
        }

    printf("%s\n", response);

    


    return 0;
}