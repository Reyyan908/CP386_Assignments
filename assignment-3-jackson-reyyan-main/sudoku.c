/*
CP386 Assignment 3
Question 1 - Sudoku
Jackson
Reyyan
2024-11-01
*/
#include <stdio.h>                                                              // Input/Output Header
#include <stdlib.h>                                                             // Library header for memory and process functions
#include <pthread.h>                                                            // Allows multithreading
#define sudokugridsize 9                                                        // Size of full grid, not sub grid 9 rows by 9 columns
#define amountofthreads 27                                                      // Based on hints in assignment 3 instructions, 11 threads can be used, will change if needed **Changed to 27 for checking 9 rows, 9 columns, 9 subgrids so 9*3 = 27
typedef struct {                                                                // From assignment help
    int row;                                                                    // Row index
    int column;                                                                 // Column index
    int thread;
} parameters; 
int solvedgrid[sudokugridsize][sudokugridsize];                                 // Initialize sudoku grid, 9x9 
int results[amountofthreads];                                                   // Array to hold result of threads
void sudoku(char *file){                                                        // Function to read Sudoku from file
    FILE *input = fopen(file, "r");                                             // Will open a file and store as input, r for read mode
    if (!input){                                                                // Checks to see if file can open
        perror("Input file failed to open");                                    // If not let the user know
        exit(EXIT_FAILURE);
    }
    for (int rows = 0; rows<sudokugridsize; rows++){                            // Looks through each row
        for (int col = 0; col<sudokugridsize; col++ ){                          // Looks through each column 
            fscanf(input, "%d", &solvedgrid[rows][col]);                        // reads the numbers and puts in grid
        }
    }
    fclose(input);                                                              // Closes the file 
}


void *validaterow(void *arg){                                                   // Check for validating each row
    parameters *params = (parameters *)arg;                                     // Cast arh to struct created earlier
    int rownumber = params->row;                                                // Get rownumber to check   
    int thread = params->thread;                                                // get thread id from struct 
    int check[sudokugridsize] = {0};                                            // set an array to track
    for (int a = 0; a<sudokugridsize; a++){                                     // loop through all numbers 1-9
        int currentcell = solvedgrid[rownumber][a];
        if (currentcell<1 || currentcell>9){                                    // Checks if it is a valid sudoku entry 1-9
            results[thread]=0;                                                  // Mark row as invalid 
            pthread_exit(NULL);
        }else if(check[currentcell-1]){                                         // Checks if the number hasnt already appeared
            results[thread]=0;                                                  // Mark row as invalid if not
            pthread_exit(NULL);
        }
        check[currentcell-1]=1;                                                 // mark as found 
    }
    results[thread]=1;                                                          // Mark row as valid
    pthread_exit(NULL);
}

void *validatecolumn(void *arg){                                                // Check for validating each column
    parameters *params = (parameters *)arg;                                     // Cast arg to struct created earlier
    int columnnumber = params->column;                                          // get columnnumber to check
    int thread = params->thread;                                                // get thread id from struct 
    int check[sudokugridsize] = {0};                                            // set array to track
    for (int a = 0; a<sudokugridsize; a++){                                     // loop through all numbers
        int currentcell = solvedgrid[a][columnnumber];                          
        if (currentcell<1 || currentcell>9){                                    // Checks if it is a valid sudoku entry 1-9
            results[thread]=0;                                                  // Mark column as invalid if not
            pthread_exit(NULL);
        }else if(check[currentcell-1]){                                         // Checks if the number hasnt already appeared
            results[thread]=0;                                                  // Mark column as invalid if not
            pthread_exit(NULL);
        }
        check[currentcell-1]=1;                                                 // mark as found 
    }
    results[thread]=1;                                                          // Mark column as valid
    pthread_exit(NULL);
}

void *validatesubgrid(void *arg){                                               // Check for validating each subgrid
    parameters *params = (parameters *)arg;                                     // Cast arg to struct created earlier
    int rownumber = params->row;                                                // get rownumber to check
    int columnnumber = params->column;                                          // get columnnumber to check
    int thread = params->thread;                                                // get thread id from struct 
    int check[sudokugridsize] = {0};                                            // set array to check
    for (int a =0; a<3; a++){                                                   // loop through rows
        for (int b = 0; b<3; b++){                                              // loop through columns
            int currentcell = solvedgrid[rownumber+a][columnnumber+b];          // get the current number in cell
            if (currentcell<1||currentcell>9){                                  // validate its between 1-9
                results[thread]=0;                                              // Mark sub as invalid
                pthread_exit(NULL);                                             // exit if not
            }else if (check[currentcell-1]){                                    // Check to make sure number hasnt appeared
                results[thread]=0;                                              // Mark sub as invalid
                pthread_exit(NULL);                                             // exit if not
            }
            check[currentcell-1]=1;                                             // mark as found 
        }
    }
    results[thread]=1;                                                          // Mark sub as valid
    pthread_exit(NULL); 
}

int main(){
    sudoku("sample_in_sudoku.txt");                                             // Read from file
    pthread_t threads[amountofthreads];                                         // Array to store threads
    parameters params[amountofthreads];                                         // array for struct earlier
    
    for (int c = 0; c<sudokugridsize; c++){                                     // Row Validation
        params[c].row = c;                                                      // Set row for validation
        params[c].thread = c;                                                   // put result in thread
        pthread_create(&threads[c], NULL, validaterow, &params[c]);             // Create thread for row
    }

    for (int d =0; d<sudokugridsize; d++){                                      // Column Validation
        params[d+9].column = d;                                                 // Set column for validation ** add 9 for more storage cause first 9 were held by row
        params[d+9].thread = d+9;                                               // put result in thread
        pthread_create(&threads[d+9], NULL, validatecolumn, &params[d+9]);      // creat thread for column
    }

    for (int e=0; e<9; e++){                                                    // Create thread for subgrid (9 subgrids not 3, idk why i had 3)
        int row = (e / 3)*3;                                                    // Starting row
        int col = (e / 3)*3;                                                    // Starting column
        params[e+18].row = row;                                                 // Set row for validation ** +18 for more space in threads
        params[e+18].column = col;                                              // Set column for validation ** ^^^^^^^^^^^^^^^^^^^^^^^^^
        params[e+18].thread = e+18;                                             // thread for subgruds **       ^^^^^^^^^^^^^^^^^^^^^^^^^
        pthread_create(&threads[e+18], NULL, validatesubgrid, &params[e+18]);   // Create thread for subgrid validation
    }

    printf("Sudoku Puzzle Solution is:\n");                                     // Output for user to match assignment guide
    for (int a = 0; a < sudokugridsize; a++) {                                  // Loop through each row
        for (int b = 0; b < sudokugridsize; b++) {                              // Loop through each column
            printf("%d ", solvedgrid[a][b]);                                    // Print each cell in row
        }
        printf("\n");                                                           // Newline after each row
    }

    int valid = 1;                                                              // Check if results are valid
    for (int h =0; h<amountofthreads; h++){                                     // loop throuhg
        if(results[h]==0){                                                      // if any result is 0; invalid puzzle
            valid = 0;
            break;
        }
    }
    if (valid){                                                                 // Print final line
        printf("Sudoku puzzle is valid. \n");                                   // If valid, let user know
    }else{
        printf("Sudoku puzzle is invalid. \n");                                 // if not state its invalid
    }
    return 0;
}