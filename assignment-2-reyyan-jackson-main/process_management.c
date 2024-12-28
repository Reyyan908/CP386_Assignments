/**
 * Author: Reyyan Qureshi
 * Assessment: Assignment 2 - CP386
 * Details: Question 2
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> 
#include <sys/types.h>
#include <sys/wait.h> 
#include <sys/mman.h> 
#include <fcntl.h>      
#include <sys/stat.h>
#include <string.h> 

//Command Outputs
void writeOutput(char *command, char *output) {
    FILE *fp;
    fp = fopen("output.txt", "a");
    fprintf(fp, "The output of: %s : is\n", command);
    fprintf(fp, ">>>>>>>>>>>>>>>\n%s<<<<<<<<<<<<<<\n", output);
    fclose(fp);
}

//Part 1: Read file to shared mem by child
void readFileIntoSharedMem(char *input_file, char *shared_mem, size_t shm_size) {
    FILE *fp = fopen(input_file, "r");
    if (fp == NULL) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }

    //read contents into shared mem
    size_t bytes_read = fread(shared_mem, 1, shm_size - 1, fp);
    if (ferror(fp)) {
        perror("fread");
        fclose(fp);
        exit(EXIT_FAILURE);
    }
    shared_mem[bytes_read] = '\0'; //Null-terminate string in shared mem
    fclose(fp);
}
//Part 2: Parsing commands from share mem to array 
char **parseCommands(char *shared_mem, int *num_commands) {
    //count linux commands in shared mem
    int count = 0;
    char *ptr = shared_mem;
    while (*ptr) {
        if (*ptr == '\n') count++; //add for a new line
        ptr++;
    }
    //check for last line 
    if (*(ptr - 1) != '\n' && *(ptr - 1) != '\0') count++;
    *num_commands = count;

    char **commands = malloc(count * sizeof(char *));
    if (!commands) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    //tokenize shared mem by a newline
    int i = 0;
    char *line = strtok(shared_mem, "\n");
    while (line != NULL) {
        commands[i] = strdup(line); //duplicate and store
        if (!commands[i]) {
            perror("strdup");
            exit(EXIT_FAILURE);
        }
        i++;
        line = strtok(NULL, "\n");
    }
    return commands;
}

//Part 3: Commmand and collect output
char *executeCommand(char *command) {
    int pipefd[2]; //pipfd[0] for reading, pipfd[1] for writing
    pid_t pid;
    char *output = NULL;
    size_t output_size = 0;

    if (pipe(pipefd) == -1) { //create pipe
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    pid = fork(); //fork new process
    if (pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) { //child process
        close(pipefd[0]); // close unused read side of pipe
        
        if (dup2(pipefd[1], STDOUT_FILENO) == -1) {
            perror("dup2");
            exit(EXIT_FAILURE);
        }
        close (pipefd[1]); //close write after dup

        //parse command into arg for execvp
        char *args[100]; //max command and args
        int i = 0;
        char *token = strtok(command, " "); //tokenize w/ spaces
        while (token != NULL && i < 99) {
            args[i++] = token;
            token = strtok(NULL, " ");
        }
        args[i] = NULL; //Null-terminate arg array

        //Execute with execvp
        if (execvp(args[0], args) == -1) {
            perror("execvp");
            exit(EXIT_FAILURE);
        }
    } else {
        //Parent part
        close(pipefd[1]); //unused write end

        //read output
        char buffer[1024];
        ssize_t bytes_read;
        output = malloc(1); //initialize buffer
        if (!output) {
            perror("malloc");
            exit(EXIT_FAILURE);
        }
        output[0] = '\0'; //empty string
        while ((bytes_read = read(pipefd[0], buffer, sizeof(buffer) - 1)) > 0) {
            buffer[bytes_read] = '\0';
            output_size += bytes_read;
            output = realloc(output, output_size + 1);
            if (!output) {
                perror("realloc");
                exit(EXIT_FAILURE);
            }
            strcat(output, buffer); //add buffer to output
        }
        close(pipefd[0]);
        waitpid(pid, NULL, 0); //wait for child
    }
    return output;
 }

int main(int argc, char *argv[]) {
    //Part 1: Parents reads lines and creates child to read file
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <input_file>\n", argv[0]);
        exit(EXIT_FAILURE); 
    }
    char *input_file = argv[1];

    size_t shm_size = 4096; //shared mem size 
    const char *shm_name = "/shared_mem"; //shared mem name

    //open shared mem object
    int shm_fd = shm_open(shm_name, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("shm_open");
        exit(EXIT_FAILURE);
    }

    //shared mem object size
    if (ftruncate(shm_fd, shm_size) == -1) {
        perror("ftruncate");
        shm_unlink(shm_name);
        exit(EXIT_FAILURE);
    }

    //map shared mem
    char *shared_mem = mmap(NULL, shm_size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shared_mem == MAP_FAILED) {
        perror("mmap");
        shm_unlink(shm_name);
        exit(EXIT_FAILURE);
    }

    close(shm_fd);

    pid_t pid = fork(); //fork child to read file
    if (pid < 0) {
        perror("fork");
        shm_unlink(shm_name);
        exit(EXIT_FAILURE);
    }

    if (pid == 0) { //child process
        readFileIntoSharedMem(input_file, shared_mem, shm_size);
        munmap(shared_mem, shm_size); //unmap in child
        exit(EXIT_SUCCESS);
    }

    wait(NULL); //parent wait for child

    //Part 2: Parent copies shared mem to dynamic allocated array
    int num_commands;
    char **commands = parseCommands(shared_mem, &num_commands);
    munmap(shared_mem, shm_size); //unmap in parent
    shm_unlink(shm_name);

    //Part 3: Parents executes command one by one from child processes
    for (int i = 0; i < num_commands; i++) {
        char *command_output = executeCommand(commands[i]); //get output
        writeOutput(commands[i], command_output); //write to txt file
        free(command_output);
        free(commands[i]);
    }
    free(commands);

    return 0;
}