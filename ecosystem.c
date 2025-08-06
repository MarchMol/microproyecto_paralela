#include <stdio.h>
#include <stdlib.h>

// Init Mesh

int main(int argc, char *argv[]){

    if(argc!=3){
        printf("ERROR: arguments should be ./exe <width> <height>");
        exit(1);
    }

    int width = atoi(argv[1]);
    int height = atoi(argv[2]);

    printf("Grid witg dimentions %dx%d", width, height);
    return 0;
}