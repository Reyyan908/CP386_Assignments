/**
 * CP386
 * Assignment 2
 * Question 1
 * Collatz_sequence.c
 * Jackson 
 * Reyyan
 */

#include <stdio.h>                                                                                              // Input/Output header
#include <stdlib.h>                                                                                             // Library header for memory and process functions
#include <sys/mman.h>                                                                                           // header for memory mapping
#include <fcntl.h>                                                                                              // Header for file control operations 
#include <unistd.h>                                                                                             // Header for Unix standard functions 
#include <sys/wait.h>                                                                                           // Header for waiting for child process termination
#include <string.h>                                                                                             // Header for string manipulation functions
#include <errno.h>                                                                                              // Error Number definitions
#include <ctype.h>                                                                                              // Character operations
#include <sys/types.h>
#include <sys/stat.h>

#define SHMNAME "collatzshm"                                                                                    // Define the name for the shared memory object
#define maxseqlen 1024                                                                                          // Define max length of sequence

void collatz_sequence(int n, char *sequence){
    char temp[32];                                                                                              // Temp to store each number in the sequence as a string 
    sequence[0] = '\0';                                                                                         // Set sequence to an empty string
    while (n!= 1){
        snprintf(temp, sizeof(temp),"%d ", n);                                                                  // Writes current number into temp as string
        strncat(sequence, temp, maxseqlen - strlen(sequence) - 1);                                              // Append temp to the sequence string
        if (n%2 == 0){
            n = n/2;                                                                                            // Divide by 2 if num is even
        }else{
            n = 3 * n + 1;                                                                                      // Apply 3n+1 if number is odd
        }
    }
    snprintf(temp, sizeof(temp), "%d", n);                                                                      // Add final number 1 to sequence once while loop ends
    strncat(sequence, temp, maxseqlen);                                                                         // Append final num to the sequence    
}


int main(){
    FILE *input = fopen("start_numbers.txt", "r");
    if (input == NULL){
        perror("Error: Failed opening file");
        exit(EXIT_FAILURE);                                                                                     // If file is empty or fails to read start numbers, exit
    }
    int nums[100];                                                                                              // Array to hold 10 numbers, can be scaled, testing 100 after 10  
    int numscount = 0;                                                                                          // counter to check read numbers
    while (fscanf(input, "%d", &nums[numscount])!= EOF){                                                        // While scanning throught the file and its not at end of file(EOF) continue looping                                                              
        if (numscount < 100){
            numscount++;                                                                                        // if less than max, count++
        }else{                                        
            break;                                                                                              // if more or equal to max, break
        }
    }
    fclose(input);
    if (numscount == 0){
        perror("Error: No numbers found in file! Check file again and add numbers");                            // No numbers in file, exit
        exit(EXIT_FAILURE);
    }
    for(int a = 0; a<numscount; a++){                                                                           // Loop through each number
        char shmnamelower[256];
        snprintf(shmnamelower, sizeof(shmnamelower), "%s_%d", SHMNAME, a);                                      // Created a unique sharedmem                                                                       
        int sharedmem = shm_open(shmnamelower, O_CREAT| O_RDWR, 0666);                                          // Creating a shared memory, 0666 allows everyone to read and write
        if (sharedmem == -1){                                                                                   // Check for error, output error message if error 
            perror("Error: Couldn't open shared memory");
            exit(EXIT_FAILURE);                                                                                 // If error occurs, exit
        }

        if (ftruncate(sharedmem, maxseqlen)==-1){                                                               // Set the size of shared memory
            perror("Error: Failed to set memory size");
            close(sharedmem);                                                                                   // Close shared mem if error occurs
            exit(EXIT_FAILURE);
        }                                                              
        char *sharedmemory_pointer = mmap(NULL, maxseqlen, PROT_WRITE | PROT_READ,  MAP_SHARED, sharedmem, 0);  
        if (sharedmemory_pointer == MAP_FAILED){                                                                // Checks if mmap failed with its mmap built in function
            perror("Error: Failed to Map Shared Memory");
            close(sharedmem);                                                                                   // Close shared mem if error occurs
            exit(EXIT_FAILURE);                                                                                 // Exit on error
        }
        printf("Parent Process: The positive integer read from file is %d\n", nums[a]);                         // Display Message for the current number, not a user friendly message this is important
        collatz_sequence(nums[a], sharedmemory_pointer);                                                        // Generate and store collatz sequence
        munmap(sharedmemory_pointer, maxseqlen);                                                                // Unmap the shared memory
        close(sharedmem);                                                                                       // Close Shared memory

        pid_t child = fork();                                                                                   // Create a child process 

        if (child == 0){ 
            int childsharedmem = shm_open(shmnamelower, O_CREAT | O_RDWR, 0666);                                // Sharedmem opened in read only for child
            if (childsharedmem == -1) {
                perror("Error");
                exit(EXIT_FAILURE);
            }
            char *childsharedmem_pointer = mmap(NULL, maxseqlen, PROT_READ, MAP_SHARED, childsharedmem, 0);     // Map shared memory in child process for reading
            if (childsharedmem_pointer == MAP_FAILED){                                                          //Checks if map failed with its mmap built in function
                perror("Error: Failed to Map Shared Memory");
                close(childsharedmem);                                                                          //Close shared mem if error occurs
                exit(EXIT_FAILURE); 
            }
            printf("Child Process: The generated collatz sequence is %s\n", childsharedmem_pointer);            // Prints Collatz sequence in child
            munmap(childsharedmem_pointer, maxseqlen);                                                          // Unmap sharedmem in child
            close(childsharedmem);                                                                              // Close sharedmem
            if (shm_unlink(shmnamelower)==-1){                                                                       
                perror("Error: failed to unlink shared memory");    
                exit(EXIT_FAILURE);                                                                             // unlink shared mem, if fail, exit
            }
            exit(EXIT_SUCCESS);
        }else if (child >0 ){                                                                                   // in parent process
            wait(NULL);                                                                                         // Waiting for child process to finish 
        }else{
            perror("Error: Fork failed");
            exit(EXIT_FAILURE);                                                                                 //if fork fails, exit
        }
    }
    return EXIT_SUCCESS;
}