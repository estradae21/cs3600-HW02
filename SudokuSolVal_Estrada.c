/*
 *Ernesto Estrada
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#define num_threads 27
#define TRUE 1
#define FALSE 0
typedef int bool;

int sudoku[9][9];
int valid[num_threads] = {0};
int columns[9];
int tid_columns[9];
int rows[9];
bool tid_rows[9];
bool subGrids[9];
bool tid_grids[9];

typedef struct {
    int topRow;
    int bottomRow;
    int leftColumn;
    int rightColumn;
} parameters;

void *isColumnValid(void* param) {
    pthread_t self;
    parameters *params = (parameters*) param;
    int row = params->topRow;
    int col = params->rightColumn;
    
    if (row != 0 || col > 8) {
        fprintf(stderr, "Invalid row or column for col subsection! row=%d, col=%d\n", row, col);
        pthread_exit(NULL);
    }
    
    self = pthread_self();
    int validityArray[9] = {0};
    int i;
    for (i = 0; i < 9; i++) {
        int num = sudoku[i][col];
        if (num < 1 || num > 9 || validityArray[num - 1] == 1) {
            tid_columns[col] = self;
            columns[col] = FALSE;
            pthread_exit(NULL);
        } else {
            validityArray[num - 1] = 1;
        }
    }
    
    tid_columns[col] = self;
    columns[col] = TRUE;
    valid[18 + col] = 1;
    pthread_exit(NULL);
}

void *isRowValid(void* param) {
    pthread_t self;
    parameters *params = (parameters*) param;
    int row = params->topRow;
    int col = params->leftColumn;
    
    if (col != 0 || row > 8) {
        fprintf(stderr, "Invalid row or column for row subsection! row=%d, col=%d\n", row, col);
        pthread_exit(NULL);
    }
    
    self = pthread_self();
    int validityArray[9] = {0};
    int i;
    for (i = 0; i < 9; i++) {
        int num = sudoku[row][i];
        if (num < 1 || num > 9 || validityArray[num - 1] == 1) {
            tid_rows[row] = self;
            rows[row] = FALSE;
            pthread_exit(NULL);
        } else {
            validityArray[num - 1] = 1;
        }
    }
    
    tid_rows[row] = self;
    rows[row] = TRUE;
    valid[9 + row] = 1;
    pthread_exit(NULL);
}

void *is3x3Valid(void* param) {
    pthread_t self;
    parameters *params = (parameters*) param;
    int row = params->topRow;
    int col = params->rightColumn;
    
    if (row > 6 || row % 3 != 0 || col > 6 || col % 3 != 0) {
        fprintf(stderr, "Invalid row or column for subsection! row=%d, col=%d\n", row, col);
        pthread_exit(NULL);
    }
    self = pthread_self();
    int validityArray[9] = {0};
    int i, j;
    for (i = row; i < row + 3; i++) {
        for (j = col; j < col + 3; j++) {
            int num = sudoku[i][j];
            if (num < 1 || num > 9 || validityArray[num - 1] == 1) {
                tid_grids[row + col / 3] = self;
                subGrids[row + col / 3] = FALSE;
                pthread_exit(NULL);
            } else {
                validityArray[num - 1] = 1;
            }
        }
    }
    
    tid_grids[row + col / 3] = self;
    subGrids[row + col / 3] = TRUE;
    valid[row + col/3] = 1;
    pthread_exit(NULL);
}

int main() {
    pthread_t threads[num_threads];
    bool x = TRUE, y = FALSE;
    int threadIndex = 0;
    int i,j;
    
     //FILE *file1 = fopen("SudokuPuzzleVal.txt", "r");
     FILE *file1 = fopen("SudokuPuzzle.txt", "r");
     for (i = 0; i < 9; i++) {
         for (j = 0; j < 9; j++) {
             int c;
             if (fscanf(file1, " %d", &c) != 1)
                 printf("idk");
             else
                 sudoku[i][j] = c;
         }
     }
    fclose(file1);
    
    printf("\n");
    for (i = 0; i < 9; i++) {
        for (j = 0; j < 9; j++) {
            printf("%d\t", sudoku[i][j]);
        }
    printf("\n");
    }
    printf("\n");
    
    for (i = 1; i < 10; i++) {
        parameters *data = (parameters*) malloc(sizeof(parameters));
        data -> topRow = 0;
        data -> bottomRow = 8;
        data -> leftColumn = (i - 1);
        data -> rightColumn = (i - 1);
        pthread_create(&threads[threadIndex++], NULL, isColumnValid, data);
    }
    
    for ( i = 1; i < 10; i++) {
        parameters *data = (parameters*) malloc(sizeof(parameters));
        data -> topRow = (i - 1);
        data -> bottomRow = (i - 1);
        data -> leftColumn = (0);
        data -> rightColumn = (8);
        pthread_create(&threads[threadIndex++], NULL, isRowValid, data);
    }

    for (i = 0; i < 9; i++) {
        for (j = 0; j < 9; j++) {
            if (i%3 == 0 && j%3 == 0) {
                parameters *data = (parameters *) malloc(sizeof(parameters));
                data->topRow = i;
                data->rightColumn = j;
                pthread_create(&threads[threadIndex++], NULL, is3x3Valid, data);
            }
        }
    }
    
    for (i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }
    
    printf("//---------------RESULT---------------//\n\n");
    int val = TRUE;
    for (i = 0; i < 9; i++) {
        if (columns[i] == TRUE) {
            printf("\tColumn: %x valid.\n",tid_columns[i]);
        }
        else {
            val = FALSE;
            printf("\tColumn: %x is invalid.\n", tid_columns[i]);
        }
        if (rows[i] == TRUE) {
            printf("\tRow: %x valid.\n", tid_rows[i]);
        }
        else {
            val = FALSE;
            printf("\tRow: %x invalid.\n", tid_rows[i]);
        }
        if (subGrids[i] == TRUE) {
            printf("\tGrid: %x valid.\n\n", tid_grids[i]);
        }
        else {
            val = FALSE;
            printf("\tGrid: %x invalid.\n\n", tid_grids[i]);
        }
    }
    
    if (val == TRUE) {
        printf("  Sudoku puzzle solution is valid!");
    }
    else {
        printf("  Sudoku puzzle is invalid!");
    }
    
    return EXIT_SUCCESS;
}
