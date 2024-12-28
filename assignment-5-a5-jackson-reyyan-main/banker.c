
/**
CP386 Assignment 5
2024-11-23
Jackson Blellock
Question 1 - Banker
*/ 
#include <unistd.h>                                                     // Header for Unix standard functions - Using same from assignment 5, will change when needed
#include <stdio.h>                                                      // Input Output Header
#include <stdlib.h>                                                     // Library Header for memory and process functions
#include <string.h>                                                     // Header for string manipulation functions
#include <pthread.h>                                                    // Allows Multithreading
#include <sys/stat.h>                                                   // The <sys/stat.h> header defines the structure of the data returned by the functions fstat(), lstat(), and stat()
#include <stdbool.h>                                                    // For boolean data type

#define customers 5                                                     // 5 Customers -- As defined in outline
#define resourcetype 4                                                  // 4 types of resources -- As defined in outline

int available[resourcetype];                                            // The available amount of each resource
int maximum[customers][resourcetype];                                   // the maximum demand of each customer
int allocated[customers][resourcetype];                                 // the amount currently allocated to each customer
int need[customers][resourcetype];                                      // the remaining need of each customer
pthread_mutex_t mutex;                                                  // Lock for safety

void Need(){                                                            // Function to calculate need matrix
    for (int a = 0; a<customers; a++){                                  // Loop through the customera
        for (int b = 0; b<resourcetype; b++){                           // loop through the resource type
            need[a][b] = maximum[a][b]- allocated[a][b];                // need is the result of max - allocated
        }
    }
}

bool safe(int *safeseq){                                                // Safe state check  
    int safety[resourcetype];                                           // array for resource allocation
    bool completed[customers] = {false};                                // check if customers have finished
    memcpy(safety, available, sizeof(available));                       // copy the available resources into the resource allocation array defined above
    int count = 0;                                                      // count for safeseq
    while (count<customers){                                            // loop until all customers have gone through
        bool found = false;                                             // indicates if customer was completed
        for (int a = 0; a<customers; a++){                              // loop through customers to get through their excecution
            if(!completed[a]){                                          // if customer hasn't completed
                bool safetycheck = true;                                // check if resource satisfy customers request                   
                for (int b =0; b<resourcetype; b++){                    // check if the resources can match customers request
                    if (need[a][b]> safety[b]){                         // if the need is greater than available 
                        safetycheck = false;                            // Customer cant proceed
                        break;                                          // break in code
                    }
                }
                if (safetycheck){                                       // if customer can proceed mark as complete a
                    for (int c =0; c<resourcetype; c++){                // loop through each type
                        safety[c] += allocated[a][c];                   // add allocated resources back to work
                    }
                    safeseq[count++] = a;                               // add customer to safe seq
                    completed[a] = true;                                // mark customer as done
                    found = true;                                       // set as done
                    }                               
                }       
            }
            if (!found){                                                // if no customer processed 
                return false;                                           // unsafe state
            } 
        }
    return true;                                                        // else, state is safe
}

void request(int custid, int *req){                                     // RQ command
    pthread_mutex_lock(&mutex);                                         // lock mutex to insure thread safety
    for (int a = 0; a<resourcetype; a++){                               // Check if request exceeds the customers needs
        if (req[a]> need[custid][a]){                                   // check if request is valid -- not greater than what is needed
            printf("Error: Request exceeds need for resource %d.\n", a);// if not:
            pthread_mutex_unlock(&mutex);                               // unlock 
            return;                                                     // exit loop
        }
        if (req[a] > available[a]) {                                    // check if request is valid -- not greater than what is availab;e
            printf("Error: Not enough resources available for resource %d.\n", a); //if not:
            pthread_mutex_unlock(&mutex);                               // unlock 
            return;                                                     // exit loop
        }
    }                                                     
    for (int a = 0; a<resourcetype; a++){                               // allocate resources before safety check
        available[a] -= req[a];                                         // Decrease available
        allocated[custid][a] += req[a];                                 // increase allocated
        need[custid][a] -= req[a];                                      // decrease needed
    }
    int safeseq[customers];
    if (safe(safeseq)){                                                 // check if it is safe after allocating the materials
        printf("State is Safe, and request is satisfied\n");            // if so let user know
    }else{                                                              // if not:
        for (int a = 0; a<resourcetype; a++){                           // give resources back if it isnt safe
            available[a] += req[a];                                     // increase available
            allocated[custid][a] -= req[a];                             // decrease allocated
            need[custid][a] += req[a];                                  // increase need
        }
        printf("State is Unsafe, Request denied");                      // let the user know
        
    }
    pthread_mutex_unlock(&mutex);                                       // unlock mutex
}

void release(int custid, int *rel){                                     // RL command
    pthread_mutex_lock(&mutex);                                         // lock to insure thread safety
    for (int a = 0; a<resourcetype; a++){                               // check if requested release is greater than allocated
        if (rel[a]> allocated[custid][a]){                              // if so:
            printf("Error: Cant release more than allocated");          // print error message letting user know 
            pthread_mutex_unlock(&mutex);                               // unlock before returning
            return;                                                     // exit
        }
    }
    for (int a = 0; a<resourcetype; a++){                               // update arrays with release resources
        available[a] += rel[a];                                         // increase available
        allocated[custid][a] -= rel[a];                                 // decrease allocated
        need[custid][a]+= rel[a];                                       // Need increased
    }
    printf("The resources have been released");                         // let user know it was a success
    pthread_mutex_unlock(&mutex);                                       // unlock the mutex after release
}

void status(){                                                          // Staus Command
    printf("\nAvailable Resources:\n");                                 // As per guildines
    for (int a =0; a<resourcetype; a++){                                // loop through the types
        printf("%d ", available[a]);                                    // print whats available
    }
    printf("\nMaximum Resources:\n");                                   // As per guildines
    for (int a =0; a<customers; a++){                                   // loop through customers
        for (int b =0; b<resourcetype; b++){                            // loop through type
            printf("%d ", maximum[a][b]);                               // print max
        }
        printf("\n");                                                   // so it outputs like a matrix
    }
    printf("\nAllocated Resources:\n");                                 // As per guildines
    for (int a =0; a<customers; a++){                                   // lopp through customers
        for (int b =0; b<resourcetype; b++){                            // loop through type
            printf("%d ", allocated[a][b]);                             // print allocated
        }
        printf("\n");                                                   // so it outputs like a matrix
    }
    printf("\nNeed Resources:\n");                                      // As per guildines
    for (int a =0; a<customers; a++){                                   // loop through customers
        for (int b =0; b<resourcetype; b++){                            // loop through type
            printf("%d ", need[a][b]);                                  // print need
        }
        printf("\n");                                                   // so it outputs like a matrix
    }                                         
}

void run(){                                                             // Run Command
    pthread_mutex_lock(&mutex);                                         // lock mutex
    int safseq[customers];                                              // array to store safe sequence
    if(!safe(safseq)){                                                  // check if state is safe
        printf("System not in safe state. \n");                         // If not
        pthread_mutex_unlock(&mutex);                                   // Unlock the mutex after fail
        return;                                                         // exit when failed
    }
    printf("Safe sequence is: ");                                       // print safe sequcence as per guildines
    for (int a =0; a<customers; a++){                                   // go 1 by 1
        printf("%d ", safseq[a]);                                       // output
    }
    printf("\n");                                                       // format as per guildines
    for (int a =0; a<customers; a++){                                   // now loop through but everything below will be done for each customer
        int cust = safseq[a];                                           // current cust to display as per guildines
        printf("\n--> Customer/Thread %d", cust);                         // as per guildines
        printf("\n    Allocated resources: ");                          // as per guildines
        for (int b = 0; b<resourcetype; b++){                           // loop through the type
            printf("%d ", allocated[cust][b]);                          // print allocared
        }
        printf("\n    Needed : ");                                      // as per guildines
        for (int b = 0; b<resourcetype; b++){                           // loop through type
            printf("%d ", need[cust][b]);                               // print needed
        }
        printf("\n    Available resources: ");                            //as per guildines
        for (int b = 0; b<resourcetype; b++){                           // loop through type
            printf("%d ", available[b]);                                // print available
        }
        printf("\n    Thread has started");                             // as per guildines
        printf("\n    Thread has finished");                            //as per guildines
        printf("\n    Thread is releasing resources. \n");              // as per guildines
        for (int b= 0; b<resourcetype; b++){                            // loop through type
            available[b]+=allocated[cust][b];                           // update and release
            allocated[cust][b]=0;                                       // update and release
            need[cust][b] = maximum[cust][b];                           // update and release
        }
        printf("    New Available resources: ");                        // as per guidlines 
        for (int b = 0; b<resourcetype; b++){                           // loop through type
            printf("%d ", available[b]);                                // output updated available
        }
    }
    printf("\n");
    pthread_mutex_unlock(&mutex);                                       // Unlock the mutex after simulation
}

int main(int argc, char *argv[]) {                                      // Main function
    if (argc != resourcetype + 1) {                                     // Checks for accurate arguments
        printf("Use ./banker followed by amount per resource\n");       // Error instructing what to do if not,
        return -1;                                                      // exit if incorrect
    }
    for (int a = 0; a < resourcetype; a++) {                            // Loops through all resources  
        available[a] = atoi(argv[a + 1]);                               // converts the string input to integer
    }
    printf("Number of Customers %d\n", customers);                      // Display customers as per guidlines
    printf("Currently available resources: ");                          // Display available resources as per guidlines
    for (int a = 0; a<resourcetype; a++){                               // loop through each type of resources
        printf("%d ", available[a]);                                    // print out the resource amount
    }
    FILE *file = fopen("sample_in_banker.txt", "r");
    if (!file) {                                                        // Check if the file could not be opened
        perror("Error opening file");                                   // Print the error message
        return -1;                                                      // Exit the program with an error code
    }
    printf("\nMaximum resources from file:\n");
    char line[100];                                                     // Buffer to hold a line from the file
    for (int i = 0; i < customers; i++) {                               // Loop through each customer
        if (fgets(line, sizeof(line), file)) {                          // Read a line from the file
            char *token = strtok(line, ",");                            // Tokenize the line using commas as the delimiter
            for (int j = 0; j < resourcetype; j++) {                    // Loop through each resource type
                if (token) {                                            // Ensure the token is not NULL
                    maximum[i][j] = atoi(token);                        // Convert the token to an integer and store it in the matrix
                    token = strtok(NULL, ",");                          // Get the next token
                } else {
                    maximum[i][j] = 0;                                  // Handle missing values in the file
                }
                allocated[i][j] = 0;                                    // Initialize the allocated matrix to 0
                printf("%d ", maximum[i][j]);                           // Print the value from the maximum matrix
            }
            printf("\n");                                               // Print a newline after each customer row
        }
    }
    fclose(file);                                                       // close file when done reading
    Need();                                                             // Calculate need matrix
    pthread_mutex_init(&mutex, NULL);                                   // initialize the mutex
    char commandsinput[1000];                                           // Stores input commands
    printf("Enter command: \n");                                        // ask user for input;
    fgets(commandsinput, sizeof(commandsinput), stdin);                 // scan that input
    commandsinput[strcspn(commandsinput, "\n")] = 0;                    // Remove newline character from input
    while(strcmp(commandsinput, "Exit")!=0){                            // infinite loop until user exits
        if (strcmp(commandsinput, "Status")==0|| strcmp(commandsinput, "status")==0){// If its status inputted
            status();                                                   // run status function
            printf("Enter command: \n");                                // ask user for input;
            fgets(commandsinput, sizeof(commandsinput), stdin);         // scan that input
            commandsinput[strcspn(commandsinput, "\n")] = 0;            // Remove newline character from input                   
        }else if (strcmp(commandsinput, "Run")==0||strcmp(commandsinput, "run")==0){// if run is inputted
            run();                                                      // run the run function
            printf("Enter command: \n");                                // ask user for input;
            fgets(commandsinput, sizeof(commandsinput), stdin);         // scan that input
            commandsinput[strcspn(commandsinput, "\n")] = 0;            // Remove newline character from input
        }else if ((strncmp(commandsinput, "RQ", 2)==0)){                // if RQ is entered
            int custid;                                                 // what cust is it
            int req[resourcetype];                                      // what is being requested per type
            int scan = sscanf(commandsinput + 2, "%d %d %d %d %d %d", &custid, &req[0], &req[1], &req[2], &req[3], &req[4]);  //scan and save inputted data from line
            if (scan !=5){                                              // If 5 numbers not entered
                printf("Invalid input, use one of RQ, RL, Status, Run, Exit \n"); // Error message
            }else{                                                      // if 5 numbers
                request(custid, req);                                   // run request function with inputted data
            }
            printf("Enter command: \n");                                // ask user for input;
            fgets(commandsinput, sizeof(commandsinput), stdin);         // scan that input
            commandsinput[strcspn(commandsinput, "\n")] = 0;            // Remove newline character from input
        }else if ((strncmp(commandsinput, "RL", 2)==0)){                // if RL is entered
            int custid;                                                 // what cust is it
            int rel[resourcetype];                                      // what is being released per type
            int scan = sscanf(commandsinput + 2, "%d %d %d %d %d %d", &custid, &rel[0], &rel[1], &rel[2], &rel[3], &rel[4]);   //scan and save the inputted data from line
            if (scan !=5){
                printf("Invalid input, use one of RQ, RL, Status, Run, Exit \n");
            }else{
                release(custid, rel);                                    // run request function with inputted data
            }                                                                                    
            printf("Enter command: \n");                                // ask user for input;
            fgets(commandsinput, sizeof(commandsinput), stdin);         // scan that input
            commandsinput[strcspn(commandsinput, "\n")] = 0;            // Remove newline character from input
        }else{                                                          // Error in input, resubmit command
            printf("Invalid input, use one of RQ, RL, Status, Run, Exit \n");
            printf("Enter command: \n");                                // ask user for input;
            fgets(commandsinput, sizeof(commandsinput), stdin);         // scan that input
            commandsinput[strcspn(commandsinput, "\n")] = 0;            // Remove newline character from input
        }
    }
    return 0;                                                           // exit program
}