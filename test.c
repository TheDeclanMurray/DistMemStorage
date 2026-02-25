#include <stdlib.h>
#include <stdio.h>
#include "socket.h"
#include "message.h"
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <stdbool.h>

// testing params
#define DO_PRINTS false
#define MAX_MESSAGE_LENGTH 124
#define KEY_LEN 8
#define VAL_LEN 16
#define NUM_TESTS 4000

// key value pair struct
typedef struct {
    char key[KEY_LEN];
    char val[VAL_LEN];
} kv_pair;

/**
 * @brief Generate a random alphanumeric string.
 *
 * Fills a given buffer with randomly chosen characters from the set
 * [a-zA-Z0-9]. The resulting string is null-terminated.
 *
 * @param buf Pointer to buffer to be filled.
 * @param len Length of the buffer (including space for null terminator).
 */
void random_string(char *buf, size_t len) {
    static const char charset[] =
        "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    for (size_t i = 0; i < len - 1; i++) {
        buf[i] = charset[rand() % (sizeof(charset) - 1)];
    }
    buf[len - 1] = '\0';
}

/**
 * @brief Measure elapsed time between two timestamp readings in milliseconds.
 *
 * Uses CLOCK_MONOTONIC for stable duration measurements unaffected by
 * system clock changes.
 *
 * @param start Start timestamp from clock_gettime().
 * @param end End timestamp from clock_gettime().
 * @return Elapsed time in milliseconds as a double.
 */
double diff_time_ms(struct timespec start, struct timespec end) {
    return (end.tv_sec - start.tv_sec) * 1000.0 +
           (end.tv_nsec - start.tv_nsec) / 1.0e6;
}

/**
 * @brief Entry point for the test client.
 *
 * This function establishes a network connection to a local storage server,
 * then executes NUM_TESTS randomized SET/GET operations:
 *   - 50% chance of performing a SET (writing a random key/value)
 *   - 50% chance of performing a GET (reading an existing key)
 *
 * Latency is measured for each operation, and GET validity is checked by
 * comparing responses with most recent known values. Final results include
 * average latency for SET and GET and the correctness ratio of GET results.
 *
 * Usage:
 * @code
 *   ./test_client <port_number>
 * @endcode
 *
 * @param argc Argument count (expecting 2).
 * @param argv Argument vector; argv[1] = port number of the storage server.
 * @return Exit status (0 on success, nonzero on failure).
 */
int main(int argc, char** argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <storagePort>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    srand(time(NULL));

    // connect to one of your storage nodes
    unsigned short connection_port = atoi(argv[1]);
    int fd = socket_connect("localhost", connection_port);
    if (fd == -1) {
        perror("Failed to connect to storage");
        exit(EXIT_FAILURE);
    }

    // setup loop vars
    char buf[MAX_MESSAGE_LENGTH];
    kv_pair kvs[NUM_TESTS];
    int key_count = 0;

    // track stats
    double total_set_latency = 0.0;
    double total_get_latency = 0.0;
    int set_count = 0;
    int get_count = 0;
    int correct_gets = 0;

    for (int i = 0; i < NUM_TESTS; i++) {

        char key[KEY_LEN];
        char val[VAL_LEN];

        // do either a put or set
        int do_set = (key_count == 0) ? 1 : rand() % 2; // 50/50

        struct timespec start, end;

        if (do_set) {
            // create a rand key and val
            random_string(key, KEY_LEN);
            random_string(val, VAL_LEN);

            // strucutre command
            strcpy(buf, "set:");
            strcat(buf, key);
            strcat(buf, ":");
            strcat(buf, val);

            // if printing
            if (DO_PRINTS){
                printf("[%d] Sending: %s\n", i, buf);
            }

            // time sets
            clock_gettime(CLOCK_MONOTONIC, &start);
            if (send_message(fd, buf) == -1) {
                perror("Failed to send message");
                exit(EXIT_FAILURE);
            }
            clock_gettime(CLOCK_MONOTONIC, &end);

            // include stats
            total_set_latency += diff_time_ms(start, end);
            set_count++;

            // record this key/value
            strcpy(kvs[key_count].key, key);
            strcpy(kvs[key_count].val, val);
            key_count++;
        } else {

            // pick a key to get
            int idx = rand() % key_count;
            strcpy(key, kvs[idx].key);

            // structure command
            strcpy(buf, "get:");
            strcat(buf, key);

            // if prints 
            if (DO_PRINTS){
                printf("[%d] Sending: %s = ", i, buf);
            }
            
            // time get
            clock_gettime(CLOCK_MONOTONIC, &start);
            if (send_message(fd, buf) == -1) {
                perror("Failed to send message");
                exit(EXIT_FAILURE);
            }

            char *response = receive_message(fd);
            clock_gettime(CLOCK_MONOTONIC, &end);

            // include stats
            total_get_latency += diff_time_ms(start, end);
            get_count++;

            // handle error
            if (!response) {
                perror("Received No Response");
                exit(EXIT_FAILURE);
            }

            // print response
            if (DO_PRINTS){
                printf("Response: %s\n", response);
            }

            // include stats
            if (strcmp(response, kvs[idx].val) == 0) {
                correct_gets++;
            }
            
            free(response);
        }
    }

    close(fd);

    // do stats
    double avg_set = set_count ? total_set_latency / set_count : 0;
    double avg_get = get_count ? total_get_latency / get_count : 0;
    double accuracy = get_count ? (100.0 * correct_gets / get_count) : 0;

    // show stats
    printf("\n==== TEST SUMMARY ====\n");
    printf("Total SETs: %d, avg latency: %.3f ms\n", set_count, avg_set);
    printf("Total GETs: %d, avg latency: %.3f ms\n", get_count, avg_get);
    printf("GET correctness: %.2f%% up-to-date\n", accuracy);

    return 0;
}