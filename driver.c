#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

// To enable debugging
//#define DEBUG

// NUM CHANNELS + 1
#define BUF_LEN 13

#define NUM_VECTORS 32767
#define COMMAND_IDLE_MICROSECONDS 10000
#define false 0
#define true 1

typedef struct LightVector {
    int dwell_time;  // How long to show this vector?
    unsigned char buffer[BUF_LEN];  // Buffer bytes (MUST START with 0)
} LightVector;

typedef struct Command {
    int length;
    LightVector vectors[NUM_VECTORS];
} Command;

typedef struct SharedState {
    int new_command_ready;
    Command* command;
    Command* new_command;
} SharedState;


static volatile int keepRunning = 1;

void intHandler(int dummy) {
    keepRunning = 0;
}

void zeroize(unsigned char* buffer, int length) {
    int i;
    for(i=0;i<length;i++) {
        buffer[i] = 0;
    }
}

void *DriverThread(void *inp)
{
    // UNCOMMENT TO DISABLE THE HARDWARE THREAD (for testing)
    //while(1);

    Command* temp_command;
    int fd;
    int ip, iterations;
    LightVector current;

    SharedState* state;
    state = (SharedState*)inp;

    unsigned char zero_buffer[BUF_LEN];
    zeroize(zero_buffer, BUF_LEN);

    // Open the DMX fd
#ifndef DEBUG
    fd = open("/dev/dmx0", O_WRONLY);
#endif
#ifdef DEBUG
    fd = open("/dev/null", O_WRONLY);
#endif
    if (fd < 0) {
        perror("failure opening /dev/dmx0");
        exit(-1);
    }

    current = state->command->vectors[0];

    iterations = 0;
    ip = 0;

    // driver!
    while(keepRunning) {
#ifndef DEBUG
        // Write the current buffer value
        write(fd, current.buffer, BUF_LEN);
#endif

#ifdef DEBUG
        // Print what we would have written
        printf("Iteration %d of %d for vector %d of %d...\n", iterations, current.dwell_time, ip, state->command->length);
        printf("Would write: %02X %02X %02X %02X\n", current.buffer[0], current.buffer[1], current.buffer[2], current.buffer[3]);
        usleep(500000);
#endif

        // Increment the iter count
        iterations++;

        if (iterations >= current.dwell_time) {
            iterations = 0;
            ip++;

            if (ip >= state->command->length) {
                if (state->new_command_ready) {
                    // Pointer SWAP!
                    temp_command = state->command;
                    state->command = state->new_command;
                    state->new_command = temp_command;

                    state->new_command_ready = false;
                }
                ip = 0;
            }

            current = state->command->vectors[ip];
        }
    }

    // If we got here, someone hit Ctrl-C - write all 0s
    write(fd, zero_buffer, BUF_LEN);

    close(fd);

    printf("Driver thread exiting\n");
    printf("Press Ctrl-D (to send EOF) if MainThread doesn't quit promptly\n");
    pthread_exit(NULL);
}

int parse_light_vector(LightVector* vector, char* line) {
    int vec_pos = 0;
    // Find the dwell time from the first item in the string
    char *p = strtok(line, " ");
    if(!p) {
        return false;
    }
    vector->dwell_time = atoi(p);
#ifdef DEBUG
    //printf("vector %d dwell time: '%s' -> %d\n", vec_num, p, state.new_command->vectors[vec_num].dwell_time);
#endif
    p = strtok(NULL, " ");

    // The rest of the items in the string are
    for (; p != NULL; p = strtok(NULL, " ")) {
        vector->buffer[vec_pos] = (unsigned char)atoi(p);
        vec_pos++;
    }
    return true;
}

int main() {
    char line[1024];

    // Handle SIGINT to gracefully shutdown
    signal(SIGINT, intHandler);

    Command command_a = { 3, {
            {1, {0x00, 0x01, 0x00, 0x00, 0x01, 0x00, 0x00, 0x01, 0x00, 0x00, 0x01, 0x00, 0x00}},
            {1, {0x00, 0x00, 0x01, 0x00, 0x00, 0x01, 0x00, 0x00, 0x01, 0x00, 0x00, 0x01, 0x00}},
            {1, {0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x01, 0x00, 0x00, 0x01, 0x00, 0x00, 0x01}}
    }};
    Command command_b;

    SharedState state;
    state.command = &command_a;
    state.new_command = &command_b;
    state.new_command_ready = false;

    // Three vectors to process
    // END default command for testing

    // Start the driver thread!
    pthread_t driver_thread;
    int rc;
    rc = pthread_create(&driver_thread, NULL, DriverThread, (void *)(&state));
    if (rc){
        printf("ERROR; return code from pthread_create() is %d\n", rc);
        exit(-1);
    }

    int vec_num, vec_pos;
    int eof;
    vec_num = 0;

    // DO INPUT STUFF
    while(keepRunning) {
        if(!state.new_command_ready) {
            printf("waiting for input:\n");
            // We can ready a new command!
            eof = (fgets(line, sizeof(line), stdin) == NULL);
            //printf("line *%s*\n", line);

            if(eof || !strcmp(line, "END\n")) {
                if (vec_num > 0) {
                    state.new_command_ready = true;
                    state.new_command->length = vec_num;
                    vec_num=0;
                    printf("loading new command of length %d\n", state.new_command->length);
                }
                if (eof) break;
                continue;
            }

            state.new_command->vectors[vec_num] = (LightVector) {0, {0x00, 0x00, 0x00, 0x00}};
            if (parse_light_vector(&(state.new_command->vectors[vec_num]),
                                   line)) vec_num++;

        }
        else {
            printf("not ready for a new command yet...\n");
            usleep(COMMAND_IDLE_MICROSECONDS);
        }
    }
    while(eof && keepRunning) {
        usleep(COMMAND_IDLE_MICROSECONDS);
    }

    printf("Main thread exiting\n");

    /* Last thing that main() should do */
    pthread_exit(NULL);

    return 0;
}

