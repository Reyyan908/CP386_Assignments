/*
 -------------------------------------
 File: filecopy.c
 Project: Assignment 1
 -------------------------------------
 Author:  Reyyan Qureshi
 ID:      169033850
 Email:   qure3850@mylaurier.ca
 Version  2024-09-26
 -------------------------------------
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

int main(int argCount, char *argv[])
{
    if (argCount != 3)
    {
        printf("Insufficient parameters passed.\n");
        exit(1);
    }

    int inputFd = open(argv[1], O_RDONLY);
    if (inputFd == -1)
    {
        perror("Error: Unable to open input file");
        exit(1);
    }

    // Create output file with write permissions and truncate 
    int outputFd = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (outputFd == -1)
    {
        perror("Error: Unable to open/create output file");
        close(inputFd); 
        exit(1);
    }

    // Copy from input to output
    char buffer[1024];
    ssize_t bytesRead, bytesWritten;
    while ((bytesRead = read(inputFd, buffer, sizeof(buffer))) > 0)
    {
        bytesWritten = write(outputFd, buffer, bytesRead);
        if (bytesWritten != bytesRead) {
            perror("Error writing to output file");
            close(inputFd);
            close(outputFd);
            exit(1);
        }
    }

    // Check for error when read
    if (bytesRead == -1)
    {
        perror("Error reading from input file");
    }

    close(inputFd);
    close(outputFd);

    printf("The contents of file %s have been successfully copied into the %s file\n", argv[1], argv[2]);
    return 0;
}