#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

// To enable debugging
//#define DEBUG

// NUM CHANNELS + 1
#define BUF_LEN 7

#define NUM_VECTORS 500
#define COMMAND_IDLE_MICROSECONDS 1000000
#define false 0
#define true 1

typedef struct LightVector {
    int dwell_time;  // How long to show this vector?
    unsigned char buffer[BUF_LEN];  // Buffer bytes (MUST START with 0)
} LightVector;

typedef struct Command {
    int length;
    LightVector vectors[BUF_LEN];
} Command;

typedef struct SharedState {
    int new_command_ready;
    Command* command;
    Command* new_command;
} SharedState;

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

    // Open the DMX fd
#ifndef DEBUG
    fd = open("/dev/dmx0", O_WRONLY);
#endif
#ifdef DEBUG
    fd = open("/dev/null", O_WRONLY);
#endif
    if (fd < 0) {
        perror("open");
        exit(-1);
    }

    current = state->command->vectors[0];

    iterations = 0;

    // driver!
    while(1) {
#ifndef DEBUG
        // Write the current buffer value
        write(fd, current.buffer, BUF_LEN);
#endif

#ifdef DEBUG
        // Print what we would have written
        printf("Iteration %d of %d for this vector...\n", iterations, current.dwell_time); 
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

    close(fd);

    pthread_exit(NULL);
}

void zeroize(unsigned char* buffer, int length) {
    int i;
    for(i=0;i<length;i++) {
        buffer[i] = 0;
    }
}

int main() {
    char line[1024];

    Command command_a, command_b;

    SharedState state;
    state.command = &command_a;
    state.new_command = &command_b;
    state.new_command_ready = false;

    // BEGIN default command for testing
    // RED -> 0x01 for one frame
    state.command->vectors[0].dwell_time = 1;
    zeroize(state.command->vectors[0].buffer, BUF_LEN);
    state.command->vectors[0].buffer[1] = 0x01;

    // GREEN -> 0x01 for one frame
    state.command->vectors[1].dwell_time = 1;
    zeroize(state.command->vectors[1].buffer, BUF_LEN);
    state.command->vectors[1].buffer[2] = 0x01;

    // BLUE -> 0x01 for one frame
    state.command->vectors[2].dwell_time = 1;
    zeroize(state.command->vectors[2].buffer, BUF_LEN);
    state.command->vectors[2].buffer[3] = 0x01;

    // Three vectors to process
    state.command->length = 3;
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
    vec_num = 0;
    vec_pos = 0;

    // DO INPUT STUFF
    while(true) {
        if(!state.new_command_ready) {
            printf("waiting for input:\n");
            // We can ready a new command!
            fgets(line, sizeof(line), stdin);
            //printf("line *%s*\n", line);

            if(!strcmp(line, "BEGIN\n")) {
                vec_num = 0;
                continue;
            }

            if(!strcmp(line, "END\n")) {
                state.new_command_ready = true;
                state.new_command->length = vec_num;
                printf("loading new command of length %d\n", state.new_command->length);
                continue;
            }

            // Somehow, this line results in line going from reasonable to empty
            //state.new_command->vectors[vec_num] = (LightVector) {0, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}};

            vec_pos = 0;

            // Find the dwell time from the first item in the string
            char *p = strtok(line, " ");
            if(!p) {
                continue;
            }
            state.new_command->vectors[vec_num].dwell_time = atoi(p);
            p = strtok(NULL, " ");

            // The rest of the items in the string are
            for (; p != NULL; p = strtok(NULL, " ")) {
                state.new_command->vectors[vec_num].buffer[vec_pos] = (unsigned char)atoi(p);
                vec_pos++;
            }

            vec_num++;

        }
        else {
            printf("not ready for a new command yet...\n");
            usleep(COMMAND_IDLE_MICROSECONDS);
        }
    }

    /* Last thing that main() should do */
    pthread_exit(NULL);

    return 0;
}

