/*
 -------------------------------------
 File: allocation.c
 Project: Assignment 5
 -------------------------------------
 Author:  Reyyan Qureshi
 ID:      169033850
 Email:   qure3850@mylaurier.ca
 Version  2024-12-2
 -------------------------------------
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h> 

#define MAX_PROCESS_NAME 50 //max length for process names

typedef struct Block { //mem block
    unsigned int start; //starting address
    unsigned int size; //size of block in bytes
    char process[MAX_PROCESS_NAME]; //filler occupying block
    struct Block* next; //ext mem block pointer
} Block;

Block* head = NULL; //head pointer of linked list
unsigned int MAX_MEMORY = 0; //total mem size

//create new mem block 
Block* create_block(unsigned int start, unsigned int size, const char* process) {
    Block* new_block = (Block*)malloc(sizeof(Block)); 
    if(new_block == NULL) { //error handle
        printf("Memory allocation failed.\n");
        exit(EXIT_FAILURE);
    }
    new_block->start = start;
    new_block->size = size;

    if (process != NULL) { //copy process name, if no name mark as free
        strncpy(new_block->process, process, MAX_PROCESS_NAME - 1);
        new_block->process[MAX_PROCESS_NAME - 1] = '\0';
    } else {
        strcpy(new_block->process, "Free");
    }
    new_block->next = NULL;
    return new_block;
}

void initialize_mem(unsigned int size) { //single free block covering entire mem
    head = create_block(0, size, NULL); //address 0 to size-1
}

Block* find_best_fit(unsigned int size, Block** prev) {
    Block* current = head; //starts at head
    Block* previous = NULL; //stores previous block
    int num_free_blocks = 0;

    while(current != NULL) { //count number of free blocks
        if(strcmp(current->process, "Free") == 0) {
            num_free_blocks++;
        }
        current = current->next;
    }

    int debug = (num_free_blocks > 1) ? 1:0;
    
    current = head;
    previous = NULL;
    Block* best = NULL; //stores best fit block
    Block* best_prev = NULL; //stores previous block of best fit 
    int index = 0; //keeps track of position of current block
    unsigned int best_delta = MAX_MEMORY + 1;

    while (current != NULL) { //find best fit
        if(strcmp(current->process, "Free") == 0) { //check if block is free 
            if(current->size >= size) {
                unsigned int delta = current->size - size;
                if(debug) {
                    printf("index = %d delta = %u best delta = %u\n", index, delta, best_delta);
                }
                if (delta < best_delta) { //if block is smaller than current best, update
                    best = current;
                    best_prev = previous;
                    best_delta = delta;
                }
            }
            index++;
        }
        previous = current;
        current = current->next;
    }
    if(prev != NULL) {
        *prev = best_prev;
    }
    return best;
}

//allocates mem to process using correct allocation algo (best fit)
int allocate_memory(const char* process, unsigned int size, char algorithm) { 
    if (algorithm != 'B' && algorithm != 'F' && algorithm != 'W') { //validate allocation algo flag
        printf("Invalid allocation algorithm.\n");
        return -1;
    }

    Block* prev = NULL; //previous block
    Block* selected_block = NULL; //free block for allocation

    if (algorithm == 'B') { //best fit algo
        selected_block = find_best_fit(size, &prev); //find best fit free block
    }
    else if (algorithm == 'F') { //first fit algo
        printf("First Fit algorithm not implemented.\n");
        return -1;
    }
    else if (algorithm == 'W') { //worst fit algo
        printf("Worst Fit algorithm not implemented.\n");
        return -1;
    }

    if (selected_block == NULL) { //if memory insufficient
        printf("No hole of sufficient size\n");
        return -1;
    }

    if (selected_block->size == size) { //if block selected
        strcpy(selected_block->process, process); //assign process to block
    } else { //split free block into allocated block and smaller block
        Block* allocated = create_block(selected_block->start, size, process);
        selected_block->start += size; //update starting address for remaining free block
        selected_block->size -= size; //update size of remaining free block

        //insert allocated block into list
        if (prev == NULL) { //if inserted at beginning
            allocated->next = head; 
            head = allocated;
        } else { //inserted after previous block
            allocated->next = prev->next;
            prev->next = allocated;
        }
    }
    printf("Successfully allocated %u to process %s\n", size, process);
    return 0;
 }

 int release_memory(const char* process) {
    printf("releasing memory for process %s\n", process); //to match expected output layout

    Block* current = head; //starting from head of list
    Block* prev = NULL; //keep track of previous block
    int found = 0; //cheeck if process found

    while (current != NULL) { //go through to find the allocated block to process
        if(strcmp(current->process, process) == 0) {
            strcpy(current->process, "Free");
            found = 1;
            break;
        }
        prev = current;
        current = current->next;
    }

    if(!found) { //if not found, output error
        printf("Process %s not found.\n", process);
        return -1;
    }

    //merge adjacent blocks
    current = head; 
    while (current != NULL && current->next != NULL) {
        if(strcmp(current->process, "Free") == 0 && strcmp(current->next->process, "Free") == 0)    {
            Block* temp = current -> next; //temp pointer to next block
            current -> size += temp->size; //increase size of current free block
            current->next = temp->next; //remove next block 
            free(temp);
        } else {
            current = current->next;
        }
    }
    printf("Successfully released memory for process %s\n", process);
    return 0;

 }

 void report_status() { //report memory block status
    Block* current = head; //starting from head of list
    unsigned int allocated = 0; //keep track of allocated memory
    unsigned int free_mem = 0; //keep track of free memory

    Block* temp = current;
    while (temp != NULL) {
        if (strcmp(temp->process, "Free") != 0) { //calculated total allocated mem
            allocated += temp->size;
        }
        temp = temp->next;
    }
    printf("Partitions [Allocated memory = %u]: \n", allocated);

    while (current != NULL) { //go through list to print allocated partitions
        if(strcmp(current->process, "Free") != 0) {
            printf("Address [%u:%u] Process %s\n", current->start, current->start + current->size - 1, current->process);
        }
        current = current->next;
    }

    printf("\n"); //add newline to match expected output formatting

    current = head; //reset pointer to report the free memory
    temp = current;
    while (temp != NULL) {
        if(strcmp(temp->process, "Free") == 0) {
            free_mem += temp->size;
        }
        temp = temp->next;
    }
    printf("Holes [Free memory = %u]:\n", free_mem);

    while(current != NULL) { //go through list and print the holes
        if(strcmp(current->process, "Free") == 0) {
            printf("Address [%u:%u] len = %u\n", current->start, current->start + current->size - 1, current->size);
        }
        current = current->next;
    }
 }

//moving all allocated blocks to the beginning and merging all free memory into one block 
 void compact_memory() { 
    Block* current = head; //start of list
    unsigned int next_free_address = 0; //address of next allocated block
    Block* new_head = NULL; //head of new list
    Block* last = NULL; //keeping track of last block

    while(current != NULL) { //collect allocated block 
        if(strcmp(current->process, "Free") != 0) { 
            //create new allocated block at next free address
            Block* allocated = create_block(next_free_address, current->size, current->process);
            next_free_address += current->size;

            if (new_head == NULL) { //add allocated block
                new_head = allocated;
                last = allocated;
            } else {
                last->next=allocated;
                last = allocated;
            }
        }
        current = current->next;
    }

    unsigned int total_free = MAX_MEMORY - next_free_address; //total free memory after compacting

    if(total_free > 0) { //create single free block if any free memory
        Block* free_block = create_block(next_free_address, total_free, NULL);
        if(new_head == NULL) {
            new_head = free_block; 
        } else {
            last->next = free_block;
        }
    }

    current = head; //free memory of old list
    while(current != NULL) {
        Block* temp = current;
        current = current->next;
        free(temp);
    }
    head = new_head;
    printf("Compaction process is successful\n");
 }


int main(int argc, char* argv[]) {
    if(argc != 2) { //check if mem size is provided
        printf("Usage %s <memory_size>\n", argv[0]);
        return -1;
    }

    MAX_MEMORY = atoi(argv[1]); //convert string to int
    initialize_mem(MAX_MEMORY);

    printf("Here, the Best Fit approach has been implemented and the allocated %u bytes of memory.\n", MAX_MEMORY);
    printf("allocator> ");

    char command[100];

    while(fgets(command, sizeof(command), stdin)) {
        command[strcspn(command, "\n")] = 0; //removes new line from input
        char* tokens[5]; //stores upto 5 tokens
        int token_count = 0; 
        char* token = strtok(command, " "); //first token

        while(token != NULL && token_count < 5) { //extract and store
            tokens[token_count++] = token;
            token = strtok(NULL, " ");
        }

        if(token_count == 0) { //if no tokens, try again
            printf("allocator> ");
            continue;
        }

        if(strcmp(tokens[0], "RQ") == 0) { //request command
            if(token_count != 4) {
                printf("Invalid RQ command format.\n");
            } else {
                char* process = tokens[1]; //process name
                unsigned int size = atoi(tokens[2]); 
                char algorithm = tokens[3][0]; //allocation algorithm flag
                allocate_memory(process, size, algorithm); 
            }
        }
        else if (strcmp(tokens[0], "RL") == 0) { //release command
            if(token_count != 2) {
                printf("Invalid RL command format.\n");
            } else {
                char* process = tokens[1];
                release_memory(process);
            }
        }
        else if(strcmp(tokens[0], "C") == 0) { //compact command
            compact_memory();
        }
        else if(strcmp(tokens[0], "Status") == 0) { //status command
            report_status();
        }
        else if(strcmp(tokens[0], "Exit") == 0) {
            break;
        }
        else {
            printf("Unknown command.\n");
        }
        printf("allocator> ");
    }

    //free all memory before exiting
    Block* current = head;
    while(current != NULL) {
        Block* temp = current;
        current = current->next;
        free(temp);
    }
    return 0;

}