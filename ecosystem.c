#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_LINE 1024
void matrix_string(char *rslt, int **matrix, int height, int width){
    int tem;
    for(int i = 0; i<height; i++){
        for(int j = 0; j<width; j++){

            if(matrix[i][j] == 1){
                strcat(rslt, "\033[32m P \033[0m");
            }
            if(matrix[i][j] == 2){
                strcat(rslt, "\033[34m H \033[0m");
            }
            if(matrix[i][j] == 3){
                strcat(rslt, "\033[31m C \033[0m");
            }
        }
        strcat(rslt, "\n");
    }
}

void cleanup(
    FILE *file,
    char *rslt,
    int **matrix,
    int height
){
    free(rslt);
    fclose(file);
    for (int i = 0; i < height; i++) {
        free(matrix[i]);
    }
    free(matrix);
    printf("Finished cleanup!\n");
}

int main(int argc, char *argv[]){
    // if(argc!=3){
    //     printf("ERROR: arguments should be \"./exe input\"");
    //     exit(1);
    // }

    // Fetch file
    FILE *file = fopen("input.txt", "r");
    if(!file){
        perror("Opening input failed\n");
        return 1;
    }

    // Character analysis

    // witdth and height
    int width = 0;
    int height = 0;
    char line[MAX_LINE];
    while(fgets(line, sizeof(line), file)){
        line[strcspn(line, "\n")] = 0;
        char *token = strtok(line, " ");
        height+=1;
        if(width == 0){
            while(token!=NULL){
                width+=1;
                token = strtok(NULL, " ");
            }
        }
    }
    rewind(file);
    printf("Height: %d\nWidth: %d\n", height, width);

    // Init Matrix
    int **matrix = malloc(height * sizeof(int *));
    for(int i = 0; i < height; i++){
        matrix[i] = malloc(width * sizeof(int));
    }

    // Fill Matrix
    int i = 0;
    char line2[MAX_LINE];
    int compliance = 0;

    while(fgets(line2, sizeof(line2), file)){
        int idx = 0;
        int j = 0;
        int last = 0; // 0 = space, 1 = character
        if(j>=height){
            printf("Too many rows!!");
            compliance=1;
            break;
        }
        while(line2[idx]!='\0'){
            char c = line2[idx];
            idx++;
            if(i>=width){
                compliance=1;
                break;
            }
            if(c=='\n'){ // Line breaks
                i++;
                last=0;
                continue;
            }
            if(isspace(c)){
                if(last==1){
                    j++;
                    last=0; // 0 = space
                }
                continue;
            }
            if(last==1){
                j++;
            }
            int recognized = 1;
            if(c=='p' || c=='P'){
                matrix[i][j] = 1;
                last=1;
                recognized=0;
            }
            if(c=='h' || c=='H'){
                matrix[i][j] = 2;
                last=1;
                recognized=0;
            }
            if(c=='c' || c=='C'){
                matrix[i][j] = 3;
                last=1;
                recognized=0;
            }
            if(recognized == 1){
                compliance=1;
                printf("Unrecognized character: \'%c\'\n",c);
                break;
            }
        }
    }

    // Compliance test

        
    // Print Test
    int strsize = height * width* 10;
    char *rslt = calloc(strsize, sizeof(char));

    if(compliance==1){
        printf("Something went wrong when parsing input file!\n");
        cleanup(file, rslt, matrix, height);
        return 1;
    }

    matrix_string(rslt, matrix, height, width);
    printf("Tick 0:\n%s",rslt);

    // Cleanup
    cleanup(file, rslt, matrix, height);
    return 0;
}