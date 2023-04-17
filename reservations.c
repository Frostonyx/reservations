#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

// These will be initialized in main() from the command line.
int seat_count;
int broker_count;
int *seat_taken;  // Array of seats
int transaction_count;

int seat_taken_count = 0;

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

int reserve_seat(int n)
{
    //Lock
    pthread_mutex_lock(&lock);

    //Check if seat is taken
    if (seat_taken[n] == 1) {
        //Unlock
        pthread_mutex_unlock(&lock);
        return -1;
    }

    //Mark seat as taken
    seat_taken[n] = 1;

    //Increment seat_taken_count
    seat_taken_count++;

    //Unlock
    pthread_mutex_unlock(&lock);

    return 0;
}


int free_seat(int n)
{
    //Lock
    pthread_mutex_lock(&lock);

    //Check if seat is free
    if (seat_taken[n] == 0) {
        //Unlokc
        pthread_mutex_unlock(&lock);
        return -1;
    }

    //Mark seat as free
    seat_taken[n] = 0;

    //Decrement seat_taken_count
    if (seat_taken_count > n) {
        seat_taken_count--;
    }
    //Unlock
    pthread_mutex_unlock(&lock);

    return 0;
}




int is_free(int n)
{
    //Check if seat is free
    if (seat_taken[n] == 0) {
        return 1; //Seat is free
    }

    return 0; //Seat is taken
}


int verify_seat_count(void) {
    // This function counts all the taken seats in the seat_taken[]
    // array.
    //
    // It then compares the count with the seat_count global variable.
    //
    // It returns true if they are the same, false otherwise
    //
    // You MAY modify this function, but the intended functionality must
    // still work properly.

    int count = 0;

    // Count all the taken seats
    for (int i = 0; i < seat_count; i++)
        if (seat_taken[i])
            count++;
    //printf("count = %d, seat_taken_count = %d\n", count, seat_taken_count);
    // Return true if it's the same as seat_taken_count
    return count == seat_taken_count;
}

// ------------------- DO NOT MODIFY PAST THIS LINE -------------------

void *seat_broker(void *arg)
{
    int *id = arg;

    for (int i = 0; i < transaction_count; i++) {
        int seat = rand() % seat_count;
        if (rand() & 1) {
            // buy a random seat
            reserve_seat(seat);

        } else {
            // sell a random seat
            free_seat(seat);
        }

        if (!verify_seat_count()) {
            printf("Broker %d: the seat count seems to be off! " \
                   "I quit!\n", *id);
            return NULL;
        }
    }

    printf("Broker %d: That all seemed to work very well.\n", *id);

    return NULL;
}

int main(int argc, char *argv[])
{
    // Parse command line
    if (argc != 4) {
        fprintf(stderr, "usage: reservations seat_count broker_count xaction_count\n");
        exit(1);
    }

    seat_count = atoi(argv[1]);
    broker_count = atoi(argv[2]);
    transaction_count = atoi(argv[3]);

    // Allocate the seat-taken array
    seat_taken = calloc(seat_count, sizeof *seat_taken);

    // Allocate thread handle array for all brokers
    pthread_t *thread = calloc(broker_count, sizeof *thread);

    // Allocate thread ID array for all brokers
    int *thread_id = calloc(broker_count, sizeof *thread_id);

    srand(time(NULL) + getpid());
    
    // Launch all brokers
    for (int i = 0; i < broker_count; i++) {
        thread_id[i] = i;
        pthread_create(thread + i, NULL, seat_broker, thread_id + i);
    }

    // Wait for all brokers to complete
    for (int i = 0; i < broker_count; i++)
        pthread_join(thread[i], NULL);
}

