#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#define MAX_LINE 1024
#define TICKS 20

void cleanup(FILE *file, int **matrix, int height){
    fclose(file);
    for (int i = 0; i < height; i++) {
        free(matrix[i]);
    }
    free(matrix);
    printf("Finished cleanup!\n");
}

void print_matrix(int **matrix, int height, int width, int tick) {
    printf("\n--- Tick %d ---\n", tick);
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            if(matrix[i][j] == 0) printf(" - ");
            if(matrix[i][j] == 1) printf("\033[32m P \033[0m");
            if(matrix[i][j] == 2) printf("\033[34m H \033[0m");
            if(matrix[i][j] == 3) printf("\033[31m C \033[0m");
        }
        printf("\n");
    }
    printf("Fin del tick %d\n", tick);
}

int main(){
    srand(time(NULL)); 

    FILE *file = fopen("input.txt", "r");
    if(!file){
        perror("Opening input failed\n");
        return 1;
    }

    int width = 0;
    int height = 0;
    char line[MAX_LINE];
    while(fgets(line, sizeof(line), file)){
        line[strcspn(line, "\n")] = 0;
        char *token = strtok(line, " ");
        height += 1;
        if(width == 0){
            while(token != NULL){
                width += 1;
                token = strtok(NULL, " ");
            }
        }
    }
    rewind(file);
    printf("Height: %d\nWidth: %d\n", height, width);

    int **matrix = malloc(height * sizeof(int *));
    for(int i = 0; i < height; i++){
        matrix[i] = malloc(width * sizeof(int));
    }

    int compliance = 0;
    char line2[MAX_LINE];
    int row = 0;
    while(fgets(line2, sizeof(line2), file) && row < height){
        int col = 0;
        char *token = strtok(line2, " \n");
        while(token != NULL && col < width){
            if(strcmp(token, "_") == 0 || strcmp(token, "-") == 0){
                matrix[row][col] = 0;
            } else if(strcmp(token, "p") == 0 || strcmp(token, "P") == 0){
                matrix[row][col] = 1;
            } else if(strcmp(token, "h") == 0 || strcmp(token, "H") == 0){
                matrix[row][col] = 2;
            } else if(strcmp(token, "c") == 0 || strcmp(token, "C") == 0){
                matrix[row][col] = 3;
            } else {
                printf("Unrecognized character: \"%s\" at row %d, col %d\n", token, row, col);
                compliance = 1;
                break;
            }
            col++;
            token = strtok(NULL, " \n");
        }
        row++;
    }

    if(compliance == 1){
        printf("Something went wrong when parsing input file!\n");
        cleanup(file, matrix, height);
        return 1;
    }

    for(int tick = 0; tick <= TICKS; tick++){
        print_matrix(matrix, height, width, tick);

        
        int **temp_matrix = malloc(height * sizeof(int *));
        for(int i = 0; i < height; i++){
            temp_matrix[i] = malloc(width * sizeof(int));
            memcpy(temp_matrix[i], matrix[i], width * sizeof(int));
        }

        
        for(int i = 0; i < height; i++){
            for(int j = 0; j < width; j++){
                if(matrix[i][j] == 1){
                    int dirs[4][2] = {{-1,0},{1,0},{0,-1},{0,1}};
                    for(int d = 0; d < 4; d++){
                        int ni = i + dirs[d][0];
                        int nj = j + dirs[d][1];
                        if(ni >= 0 && ni < height && nj >= 0 && nj < width){
                            if(matrix[ni][nj] == 0){
                                if((rand() % 100) < 30){ 
                                    temp_matrix[ni][nj] = 1;
                                    printf("Tick %d: planta en (%d,%d) se reproduce en (%d,%d)\n", tick, i, j, ni, nj);
                                }
                            }
                        }
                    }
                }
            }
        }

        for(int i = 0; i < height; i++){
            memcpy(matrix[i], temp_matrix[i], width * sizeof(int));
            free(temp_matrix[i]);
        }
        free(temp_matrix);
    }

    cleanup(file, matrix, height);
    return 0;
}
