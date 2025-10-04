// 
// File: brace-topia.c
// Description: Generates a random grid of endline and vacant people, and moves 
//              them if their happiness (percentage of same people around them) 
//              is below a certain threshold.
//
// @author Colin Rindge cfr1524@rit.edu
//
// // // // // // // // // // // // // // // // // // // // // // // // // // // //

#define _DEFAULT_SOURCE

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <stdbool.h>
#include <string.h>

#include "display.h"

#define DEFAULT_DELAY 900000

// delay between cycles in microseconds
int delay = DEFAULT_DELAY;
// max number of cycles, only used if explicitly set
int max_cycle;
// Whether or not to use infinite mode or print mode, default infinite
bool use_infinite_mode = true;
// width and height of grid
int dim = 15;
// percent of same neighbors needed for happiness
int strength = 50;
// percent of board vacant
int percent_vacant = 20;
// percent of non-vacant cells with an e in them
int percent_endline = 60;

// the last checked row in the current cycle when moving
int last_checked_row = 0;
// the lsat checked column in the current cycle when moving
int last_checked_col = 0;

/*
 * Creates a grid in the heap with the amount of vacant, endline, and newlines based on either the 
 * default percentages or the ones given on the command line, then randomly shuffles the order.
 *
 * @returns     A pointer to the grid
 */
char *** get_initial_grid(){
    char *** grid = malloc(sizeof(char**)); 
    if(grid == 0){
        fprintf(stderr, "Error: memory allocation failed");
        exit(EXIT_FAILURE);
    }

    *grid = malloc(dim * sizeof(char*));
    if(*grid == 0){
        fprintf(stderr, "Error: memory allocation failed");
        exit(EXIT_FAILURE);
    }

    for(int i = 0; i < dim; i++){
        (*grid)[i] = malloc(dim);
        if( (*grid)[i] == 0){
            fprintf(stderr, "Error: memory allocation failed");
            exit(EXIT_FAILURE);
        }
    }

    int num_vacant = dim*dim*(percent_vacant/100.0);
    int num_endline = dim*dim*(1-(percent_vacant/100.0))*(percent_endline/100.0);
    int count = 0;

    //puts the correct amount of vacancies, endlines, and newlines (in that order, not random yet)
    for(int i = 0; i < dim; i++){
        for(int j = 0; j < dim; j++){
            if(count < num_vacant){
                (*grid)[i][j] = '.';
            } else if(count < num_vacant+num_endline){
                (*grid)[i][j] = 'e';
            } else {
                (*grid)[i][j] = 'n';
            }
            count++;
        }
    }

    //Shuffle the values in the arary
    for(int i = 0; i < dim; i++){
        for(int j = 0; j < dim; j++){
            int rand_row = random() % dim;
            int rand_col = random() % dim;
            char temp = (*grid)[i][j];
            (*grid)[i][j] = (*grid)[rand_row][rand_col];
            (*grid)[rand_row][rand_col] = temp;
        }
    }

    return grid;
}

/*
 * Frees the grid from the heap
 *
 * @param grid      The grid to free
 */
void free_grid(char *** grid){
    
    if(*grid != 0 && grid != 0){

        for(int i = 0; i < dim; i++){
            free((*grid)[i]);
        }
    }

    if(grid != 0){
        free(*grid);
    }

    free(grid);

}

/*
 * Moves the character at the passed row and col to the first free space availible
 *
 * @param grid2     The grid to actually make the changes to
 * @param grid      The grid to check for neighbors on, before any moves have occurred
 * @param old_row   The row of the character to be moved
 * @param old_col   The column of the character to be moved
 * @returns         1 if the character was successfully moved, 0 if there was no availible spot to move
 */
int move(char *** grid2, char *** grid, int old_row, int old_col){
    // move algorithm goes through the grid until it finds an empty spot, then stores that spot and starts 
    // from there when it moves on to the next one to move in the cycle
    
    for( ; last_checked_row < dim; last_checked_row++){
        for( ; last_checked_col < dim; last_checked_col++){
            if( (*grid)[last_checked_row][last_checked_col] == '.'){

                (*grid2)[last_checked_row][last_checked_col] = (*grid)[old_row][old_col];
                (*grid2)[old_row][old_col] = '.';
                last_checked_col++;
                return 1;
            }
        }
        last_checked_col = 0;
    }
    return 0; // If no spot to move, it doesn't move
}

/*
 * Finds the happiness of the character at the passed position
 *
 * @param grid      The grid of characters
 * @param row       The row of the character to check
 * @param col       The column of the character to check
 * @returns         The happiness of the character, as a decimal from 0 to 1
 */ 
float check_neighbors(char *** grid, int row, int col){
    char value = (*grid)[row][col];
    // total number of neighbors
    int total = 0;
    // number of neighbors same as passed
    int same = 0;
    if(row-1 >= 0 && col-1 >= 0 && ( (*grid)[row-1][col-1] == 'e' || (*grid)[row-1][col-1] == 'n') ){
        total++;
        if( (*grid)[row-1][col-1] == value ){
        same++;
        }
    }

    if(row-1 >= 0 && ( (*grid)[row-1][col] == 'e' || (*grid)[row-1][col] == 'n') ){
        total++;
        if( (*grid)[row-1][col] == value ){
            same++;
        }
    }

    if(row-1 >= 0 && col+1 < dim && ( (*grid)[row-1][col+1] == 'e' || (*grid)[row-1][col+1] == 'n') ){
        total++;
        if( (*grid)[row-1][col+1] == value ){
            same++;
        }
    }

    if(col-1 >= 0 && ( (*grid)[row][col-1] == 'e' || (*grid)[row][col-1] == 'n') ){
        total++;
        if( (*grid)[row][col-1] == value ){
            same++;
        }
    }

    if(col+1 < dim && ( (*grid)[row][col+1] == 'e' || (*grid)[row][col+1] == 'n') ){
        total++;
        if( (*grid)[row][col+1] == value ){
            same++;
        }
    }

    if(row+1 < dim && col-1 >= 0 && ( (*grid)[row+1][col-1] == 'e' || (*grid)[row+1][col-1] == 'n') ){
        total++;
        if( (*grid)[row+1][col-1] == value ){
            same++;
        }
    }

    if(row+1 < dim && ( (*grid)[row+1][col] == 'e' || (*grid)[row+1][col] == 'n') ){
        total++;
        if( (*grid)[row+1][col] == value ){
            same++;
        }
    }

    if(row+1 < dim && col+1 < dim && ( (*grid)[row+1][col+1] == 'e' || (*grid)[row+1][col+1] == 'n') ){
        total++;
    if( (*grid)[row+1][col+1] == value ){
        same++;
        }
    }

    if(total == 0){
        return 1;
    }
    return (float)same/total;
}

/*
 * Prints the number of cycles equal to what was passed on the command line
 * 
 * @param grid      The initial grid containing the characters
 */
void print_mode(char *** grid){
    int moves = 0;

    for(int i = 0; i <= max_cycle; i++){
        last_checked_row = 0;
        last_checked_col = 0;

        float total_happiness = 0;
        int total_people = 0;
        //print grid and calculate happiness
        for(int j = 0; j < dim; j++){
            for(int k = 0; k < dim; k++){
                printf("%c", (*grid)[j][k]);
                if( (*grid)[j][k] != '.' ){
                    total_people++;
                    total_happiness += check_neighbors(grid, j, k);
                }
            }
            printf("\n");
        }

        printf("cycle: %d\n", i);
        printf("moves this cycle: %d\n", moves);
        if(total_people != 0){
            printf("teams' \"happiness\": %f\n", total_happiness/total_people);
        } else {
            printf("teams' \"happiness\": %f\n", 0.0);
        }
        printf("dim: %d, %%strength of preference:  %d%%, %%vacancy:  %d%%, %%end:  %d%%\n", \
            dim, strength, percent_vacant, percent_endline);
        printf("Use Control-C to quit.\n");
        

        moves = 0;
        //allocate memory for temporary grid, prevents moving the same char twice
        char *** grid2 = malloc(sizeof(char**));
        if(grid2 == 0){
            fprintf(stderr, "Error: memory allocation failed");
            exit(EXIT_FAILURE);
        }
        *grid2 = malloc(dim * sizeof(char*));
        if(*grid2 == 0){
            fprintf(stderr, "Error: memory allocation failed");
            exit(EXIT_FAILURE);
        }
        for(int i = 0; i < dim; i++){
            (*grid2)[i] = malloc(dim);
            if( (*grid2)[i] == 0){
                fprintf(stderr, "Error: memory allocation failed");
                exit(EXIT_FAILURE);
            }
            memcpy( (*grid2)[i], (*grid)[i], dim);
        }

        for(int j = 0; j < dim; j++){
            for(int k = 0; k < dim; k++){
                char current = (*grid)[j][k];
                if( (current == 'n' || current == 'e') && check_neighbors(grid, j, k) < (float)strength/100){
                    moves += move(grid2, grid, j, k);
                    
                }
            }
        }

        char *** temp = grid;
        grid = grid2;
        free_grid(temp);

    }

    free_grid(grid);
}

/*
 * Prints an infinite amount of cycles, with a delay in between each cycle
 *
 * @param grid      The initial grid containing the characters
 */
void infinite_mode(char *** grid){
    int moves = 0;

    for(int i = 0; ; i++){
        set_cur_pos(0,0);
        clear();

        last_checked_row = 0;
        last_checked_col = 0;

        float total_happiness = 0;
        int total_people = 0;
        //print grid and calculate happiness
        for(int j = 0; j < dim; j++){
            for(int k = 0; k < dim; k++){
                put((*grid)[j][k]);
                if( (*grid)[j][k] != '.' ){
                    total_people++;
                    total_happiness += check_neighbors(grid, j, k);
                }
            }
            put('\n');
        }

        printf("cycle: %d\n", i);
        printf("moves this cycle: %d\n", moves);
        if(total_people != 0){
            printf("teams' \"happiness\": %f\n", total_happiness/total_people);
        } else {
            printf("teams' \"happiness\": %f\n", 0.0);
        }
        printf("dim: %d, %%strength of preference:  %d%%, %%vacancy:  %d%%, %%end:  %d%%\n", \
            dim, strength, percent_vacant, percent_endline);
        printf("Use Control-C to quit.\n");
        
        moves = 0;
        //allocate memory for temporary grid, prevents moving the same char twice
        char *** grid2 = malloc(sizeof(char**));
        if(grid2 == 0){
            fprintf(stderr, "Error: memory allocation failed");
            exit(EXIT_FAILURE);
        }
        *grid2 = malloc(dim * sizeof(char*));
        if(*grid2 == 0){
            fprintf(stderr, "Error: memory allocation failed");
            exit(EXIT_FAILURE);
        }
        for(int i = 0; i < dim; i++){
            (*grid2)[i] = malloc(dim);
            if( (*grid2)[i] == 0){
                fprintf(stderr, "Error: memory allocation failed");
                exit(EXIT_FAILURE);
            }
            memcpy( (*grid2)[i], (*grid)[i], dim);
        }

        for(int j = 0; j < dim; j++){
            for(int k = 0; k < dim; k++){
                char current = (*grid)[j][k];
                if( (current == 'n' || current == 'e') && check_neighbors(grid, j, k) < (float)strength/100){
                    moves += move(grid2, grid, j, k);
                    
                }
            }
        }

        char *** temp = grid;
        grid = grid2;
        free_grid(temp);
        
        usleep(delay);
    }

    free_grid(grid);
}

/*
 * Prints the usage message to stderr
 */
void print_usage(){
    fprintf(stderr, "usage:\n");
    fprintf(stderr, "brace-topia [-h] [-t N] [-c N] [-d dim] [-s %%str] [-v %%vac] [-e %%end]\n");
}

/*
 * Updates values based on command line arguments, and runs the correct mode
 * Also handles errors if command line arguments are incorrect in some way
 *
 * @param argc      The number of command line arguments
 * @param argv      An array containing the command line arguments
 * @returns         EXIT_SUCCESS on success, EXIT_FAILURE on failure (like incomplete values), or 
 *                      1 + EXIT_FAILURE on incorrect values
 */
int main(int argc, char * argv[]){
    srandom(41);

    int opt;
    while ( (opt = getopt( argc, argv, "ht:c:d:s:v:e:") ) != -1 ) {
        switch(opt){
            case 'h':
                fprintf(stderr, "usage:\n");
                fprintf(stderr, "brace-topia [-h] [-t N] [-c N] [-d dim] [-s %%str] [-v %%vac] [-e %%end]\n");
                fprintf(stderr, "Option      Default   Example   Description\n");
                fprintf(stderr, "'-h'        NA        -h        print this usage message.\n");
                fprintf(stderr, "'-t N'      900000    -t 5000   microseconds cycle delay.\n");
                fprintf(stderr, "'-c N'      NA        -c4       count cycle maximum value.\n");
                fprintf(stderr, "'-d dim'    15        -d 7      width and height dimension.\n");
                fprintf(stderr, "'-s %%str'   50        -s 30     strength of preference.\n");
                fprintf(stderr, "'-v %%vac'   20        -v30      percent vacancies.\n");
                fprintf(stderr, "'-e %%endl'  60        -e75      percent Endline braces. Others want Newline.\n");
                return EXIT_FAILURE;
                break;
            case 't':
                if(optarg == NULL){
                    print_usage();
                    return EXIT_FAILURE;
                }

                delay = (int)strtol(optarg, NULL, 10);
                if(delay < 0){
                    delay = DEFAULT_DELAY;
                }
                break;
            case 'c':
                if(optarg == NULL){
                    print_usage();
                    return EXIT_FAILURE;
                }

                max_cycle = (int)strtol(optarg, NULL, 10);
                if(max_cycle < 0){
                    fprintf(stderr, "count (%d) must be a non-negative integer.\n", max_cycle);
                    print_usage();
                    return 1 + EXIT_FAILURE;
                }
                
                use_infinite_mode = false;
                break;
            case 'd':
                if(optarg == NULL){
                    print_usage();
                    return EXIT_FAILURE;
                }

                dim = (int)strtol(optarg, NULL, 10);
                if(dim < 5 || dim > 39) {
                    fprintf(stderr, "dimension (%d) must be a value in [5...39]\n", dim);
                    print_usage();
                    return 1 + EXIT_FAILURE;
                }
                break;
            case 's':
                if(optarg == NULL){
                    print_usage();
                    return EXIT_FAILURE;
                }

                strength = (int)strtol(optarg, NULL, 10);
                if(strength < 1 || strength > 99) {
                    fprintf(stderr, "preference strength (%d) must be a value in [1...99]\n", strength);
                    print_usage();
                    return 1 + EXIT_FAILURE;
                }

                break;
            case 'v':
                if(optarg == NULL){
                    print_usage();
                    return EXIT_FAILURE;
                }

                percent_vacant = (int)strtol(optarg, NULL, 10);
                if(percent_vacant < 1 || percent_vacant > 99){
                    fprintf(stderr, "vacancy (%d) must be a value in [1...99]\n", percent_vacant);
                    print_usage();
                    return 1 + EXIT_FAILURE;
                }
                break;
            case 'e':
                if(optarg == NULL){
                    print_usage();
                    return EXIT_FAILURE;
                }

                percent_endline = (int)strtol(optarg, NULL, 10);
                if(percent_endline < 1 || percent_endline > 99){
                    fprintf(stderr, "endline proportion (%d) must be a value in [1...99]\n", percent_endline);
                    print_usage();
                    return 1 + EXIT_FAILURE;
                }

                break;
            case '?':
                print_usage();
                return EXIT_FAILURE;
                break;
        }
    }

    char *** grid = get_initial_grid();
    if(use_infinite_mode){
        infinite_mode(grid);
    } 
    else {
        print_mode(grid);
    }

    return EXIT_SUCCESS;
}
