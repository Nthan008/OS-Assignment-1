#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#define LOWER_NUM 1
#define UPPER_NUM 10000
#define BUFFER_SIZE 100
#define MAX_COUNT 10000

int buffer[BUFFER_SIZE];
int buffer_count = 0;
pthread_mutex_t lock;
pthread_cond_t can_produce;
pthread_cond_t can_consume;

FILE *all_file, *even_file, *odd_file;

// Producer function
void* producer(void* args) {
    srand(time(NULL)); // Seed for random number generation
    for (int i = 0; i < MAX_COUNT; i++) {
        pthread_mutex_lock(&lock);
        while (buffer_count == BUFFER_SIZE) {
            pthread_cond_wait(&can_produce, &lock);
        }

        int num = (rand() % (UPPER_NUM - LOWER_NUM + 1)) + LOWER_NUM;
        fprintf(all_file, "%d\n", num);
        buffer[buffer_count++] = num;
        
        pthread_cond_signal(&can_consume);
        pthread_mutex_unlock(&lock);
    }
    return NULL;
}

// Consumer function
void* consumer(void* args) {
    int is_even = *(int*)args;
    FILE *file = is_even ? even_file : odd_file;
    int condition_met = 0;

    while (condition_met < MAX_COUNT / 2) {
        pthread_mutex_lock(&lock);
        while (buffer_count == 0 || (is_even ^ (buffer[buffer_count-1] % 2)) ) {
            pthread_cond_wait(&can_consume, &lock);
        }

        int num = buffer[--buffer_count];
        fprintf(file, "%d\n", num);
        condition_met++;

        pthread_cond_signal(&can_produce);
        pthread_mutex_unlock(&lock);
    }
    return NULL;
}

int main() {
    pthread_t prod, cons1, cons2;
    int even = 1, odd = 0;

    // Initialize mutex and condition variables
    pthread_mutex_init(&lock, NULL);
    pthread_cond_init(&can_produce, NULL);
    pthread_cond_init(&can_consume, NULL);

    // Open files
    all_file = fopen("all.txt", "w");
    even_file = fopen("even.txt", "w");
    odd_file = fopen("odd.txt", "w");

    // Create threads
    pthread_create(&prod, NULL, producer, NULL);
    pthread_create(&cons1, NULL, consumer, &even);
    pthread_create(&cons2, NULL, consumer, &odd);

    // Wait for threads to finish
    pthread_join(prod, NULL);
    pthread_join(cons1, NULL);
    pthread_join(cons2, NULL);

    // Clean up
    fclose(all_file);
    fclose(even_file);
    fclose(odd_file);
    pthread_mutex_destroy(&lock);
    pthread_cond_destroy(&can_produce);
    pthread_cond_destroy(&can_consume);

    return 0;
}
