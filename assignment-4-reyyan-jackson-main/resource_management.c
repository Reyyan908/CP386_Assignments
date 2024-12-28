/**
CP386 Assignment 4
2024-11-14
Jackson Blellock
Question 1 - Resource Management
*/


 
#include <unistd.h>                                                     // Header for Unix standard functions
#include <stdio.h>                                                      // Input Output Header
#include <stdlib.h>                                                     // Library Header for memory and process functions
#include <string.h>                                                     // Header for string manipulation functions
#include <pthread.h>                                                    // Allows Multithreading
#include <sys/stat.h>                                                   // The <sys/stat.h> header defines the structure of the data returned by the functions fstat(), lstat(), and stat()


#define MAX_RESOURCES 5                                                 // Max Resources available 
int available_resources = MAX_RESOURCES;                                // Initilize available with max
#define NUM_THREADS 5                                                   // number of threads 
int THREAD_NUMBER[NUM_THREADS];                                         // Array to store thread IDs
pthread_mutex_t mutex;                                                  // Taken from lecture slides

int descrease_count(int thread_number, int count){                      // Function to decrease resources 
    pthread_mutex_lock(& mutex);                                        // Lock Mutex before modifying the available resources
    if(available_resources < count){                                    // Check if there arent enough resources
        pthread_mutex_unlock(& mutex);                                  // unLock Mutex
        return -1;                                                      // Not enough available, code fails
    }else{
        available_resources -=count;                                    // if available, decrement available by count
        printf("The thread %d has aquired %d resources and %d more resources are available. \n", thread_number, count, available_resources);
        pthread_mutex_unlock(& mutex);                                  // unLock Mutex
        return 0;                                                       // end function 
    }
}   
int increase_count(int thread_number, int count){                       // Function to increase resources
    pthread_mutex_lock(& mutex);                                        // Lock Mutex
    available_resources += count;                                       // Add count to resources, like topping up the amount 
    printf("The thread %d has released %d resources and %d resources are now available. \n", thread_number, count, available_resources);
    pthread_mutex_unlock(& mutex);                                      // unLock Mutex
    return 0;                                                           // end function            
}

void* thread_function(THREAD_NUMBER){                                   // Thread function to show resource use -- Taken from assignment guidlines
    int thread_number = THREAD_NUMBER;                                  // Changed the array from earlier but converted to int to fix an issue i had previously
    if (descrease_count(thread_number,1)==0){                           // try to get 1 resource
        sleep(1);                                                       // If it works sleep for 1 second as per the assignment guidelines
        increase_count(thread_number, 1);                               // release the resource after its done sleeping
    }else{                                                              // Else print error if unsuccessful -- again from assignment guidlines
        printf("Thread %d could not aquire enough resources. \n", thread_number);  
    }
    return 0;                                                           // end function 

}

int main(){
    pthread_mutex_init (& mutex,NULL);                                  // Initializing the mutex lock - Also from slides
    pthread_t thread[NUM_THREADS];                                      // Array to hold thread ids
    for (int a = 0; a<NUM_THREADS; a++){                                // Loop to create threads
        THREAD_NUMBER[a] = a;                                           // Assign thread numbers
        pthread_create(&thread[a], NULL, thread_function, THREAD_NUMBER[a]);
    } 
    for (int a = 0; a<NUM_THREADS; a++){                                // Loop to go through all the threads to join 
        pthread_join(thread[a], NULL);                                  // Join the threads
    } 
    printf("All threads have finished excecution. Available resources: %d\n", available_resources);
    return 0;                                                           // End of Program
}

