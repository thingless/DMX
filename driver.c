#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

// NUM CHANNELS + 1
#define BUF_LEN 4

#define NUM_VECTORS 500
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
    Command* temp_command;
    int fd;
    int ip, iterations;
    LightVector current;

    SharedState* state;
    state = (SharedState*)inp;

    // Open the DMX fd
    fd = open("/dev/dmx0", O_WRONLY);
    if (fd < 0) {
        perror("open");
        exit(-1);
    }

    current = state->command->vectors[0];

    // driver!
    while(1) {
        // Write the current buffer value
        write(fd, current.buffer, BUF_LEN);

        // Increment the iter count
        iterations++;

        if (iterations > current.dwell_time) {
            iterations = 0;

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

            current = state->command->vectors[ip++];
        }
    }

    close(fd);

    pthread_exit(NULL);
}

int main() {
    char line[256];

    Command command_a, command_b;

    SharedState state;
    state.command = &command_a;
    state.new_command = &command_b;
    state.new_command_ready = false;

    // default command for testing
    state.command->vectors[0] = (LightVector) {200, {0x00, 0xFF, 0xFF, 0xFF}};
    state.command->vectors[1] = (LightVector) {100, {0x00, 0xFF, 0x00, 0x00}};
    state.command->vectors[2] = (LightVector) {100, {0x00, 0x00, 0xFF, 0x00}};
    state.command->vectors[3] = (LightVector) {100, {0x00, 0x00, 0x00, 0xFF}};
    state.command->vectors[4] = (LightVector) {100, {0x00, 0x00, 0xFF, 0x00}};
    state.command->vectors[5] = (LightVector) {100, {0x00, 0xFF, 0x00, 0x00}};
    state.command->vectors[6] = (LightVector) {200, {0x00, 0x00, 0x00, 0x00}};
    state.command->length = 7;

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

            if(!strcmp(line, "BEGIN\n")) {
                vec_num = 0;
                continue;
            }

            if(!strcmp(line, "END\n")) {
                state.new_command_ready = true;
                state.new_command->length = vec_num;
                continue;
            }

            state.new_command->vectors[vec_num] = (LightVector) {0, {}};

            vec_pos = 0;

            // Find the dwell time from the first item in the string
            char *p = strtok(line, " ");
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
        }
    }

    /* Last thing that main() should do */
    pthread_exit(NULL);

    return 0;
}

