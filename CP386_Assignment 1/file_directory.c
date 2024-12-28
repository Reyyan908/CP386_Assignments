/*
 -------------------------------------
 File: file_directory.c
 Project: Assignment 1
 -------------------------------------
 Author:  Reyyan Qureshi
 ID:      169033850
 Email:   qure3850@mylaurier.ca
 Version  2024-09-26
 -------------------------------------
 */

#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <string.h>

// Create directory 
void createDirectory(const char *dirName, mode_t mode) {
    if (mkdir(dirName, mode) == -1) {
        perror("Error creating directory");
    } else {
        printf("The directory %s is created successfully with mode %o\n", dirName, mode);
    }
}

// Create and write on file
void createWriteFile(const char *fileName, const char *content) {
    int fd = open(fileName, O_WRONLY | O_CREAT, 0644);
    if (fd == -1) {
        perror("Error creating file");
    } else {
        write(fd, content, strlen(content));
        printf("The file %s is created and written successfully\n", fileName);
        close(fd);
    }
}

// Read from /proc/meminfo and write tpo file
void readProcMeminfoToFile(const char *fileName) {
    int fd = open(fileName, O_WRONLY | O_CREAT, 0644);
    int src = open("/proc/meminfo", O_RDONLY);
    char buffer[1024];
    int bytes;

    if (fd == -1 || src == -1) {
        perror("Error opening files");
        return;
    }
    
    while ((bytes = read(src, buffer, sizeof(buffer))) > 0) {
        write(fd, buffer, bytes);
    }
    
    printf("Proc memory info is saved into %s successfully\n", fileName);
    close(src);
    close(fd);
}

// List files in a directory
void directoryListing(const char *startDir) {
    DIR *dir;
    struct dirent *entry;
    if ((dir = opendir(startDir)) == NULL) {
        perror("opendir() error");
    } else {
        printf("Listing contents of directory %s:\n", startDir);
        while ((entry = readdir(dir)) != NULL) {
            printf("%s\n", entry->d_name);
        }
        closedir(dir);
    }
}

// Remove directory after user confirmation
void removeDirectory(const char *dirName) {
    char confirm;
    printf("Warning: You are about to delete the directory %s and all its contents recursively. Proceed? (y/n): ", dirName);
    scanf(" %c", &confirm);

    if (confirm == 'y' || confirm == 'Y') {
        if (rmdir(dirName) == -1) {
            perror("Error removing directory");
        } else {
            printf("The directory %s has been removed successfully.\n", dirName);
        }
    } else {
        printf("Directory removal canceled.\n");
    }
}

int main() {
    int choice;
    char dirName[256];
    char fileName[256];
    char content[1024];
    mode_t mode;

    while (1) {
        printf("----- Menu -----");
        printf("\n1. Create directory\n2. Create and write to a file\n3. Write contents of /proc/meminfo to a file\n4. List directory contents\n5. Remove a directory\n99. Quit\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1:
                printf("Enter the directory name you want to create: ");
                scanf("%s", dirName);
                printf("Enter the mode of the created directory (in octal, e.g., 0755): ");
                scanf("%o", &mode);
                createDirectory(dirName, mode);
                break;
            case 2:
                printf("Enter the file name you want to change to: ");
                scanf("%s", fileName);
                printf("Enter the content that you want to write to the file: ");
                scanf(" %[^\n]", content);  
                createWriteFile(fileName, content);
                break;
            case 3:
                printf("Enter the file name you want to store memory information to: ");
                scanf("%s", fileName);
                readProcMeminfoToFile(fileName);
                break;
            case 4:
                printf("Enter the directory name you want to list the contents for (if you want to list the contents of the current directory, then pass '.'): ");
                scanf("%s", dirName);
                directoryListing(dirName);
                break;
            case 5:
                printf("Enter the directory you want to remove: ");
                scanf("%s", dirName);
                removeDirectory(dirName);
                break;
            case 99:
                printf("Exiting...\n");
                return 0;
            default:
                printf("Invalid choice. Please try again.\n");
        }
    }

    return 0;
}