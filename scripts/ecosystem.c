#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <omp.h>

#define MAX_LINE 1024
// Códigos de celda
#define EMPTY 0
#define PLANT 1
#define HERB  2
#define CARN  3

// Probabilidad de reproducción de plantas (0–100)
#define PLANT_SPREAD_PCT 30

// Atributos y reglas de H/C
#define H_ENERGY_GAIN   1
#define C_ENERGY_GAIN   2
#define H_REPRO_THRESH  2
#define C_REPRO_THRESH  2
#define H_STARVE_TICKS  3
#define C_STARVE_TICKS  4

// Vecindad 4-direcciones (más fácil de paralelizar que 8)
static const int DX[4] = {-1, 1, 0, 0};
static const int DY[4] = { 0, 0,-1, 1};

static inline int in_bounds(int i, int j, int h, int w){
    return (i>=0 && i<h && j>=0 && j<w);
}

// Fisher–Yates sobre 4 direcciones para evitar sesgo direccional
static inline void shuffle_dirs(int ord[4]){
    for(int k=0;k<4;k++) ord[k]=k;
    for(int k=3;k>0;k--){
        int r = rand() % (k+1);
        int t = ord[k]; ord[k]=ord[r]; ord[r]=t;
    }
}

static inline int** alloc2d(int h,int w){
    int **a=(int**)malloc(h*sizeof(int*));
    for(int i=0;i<h;i++) a[i]=(int*)calloc(w,sizeof(int));
    return a;
}
static inline void free2d(int **a,int h){
    for(int i=0;i<h;i++) free(a[i]);
    free(a);
}
static inline void zero2d(int **a,int h,int w){
    for(int i=0;i<h;i++) memset(a[i],0,w*sizeof(int));
}

void cleanup(FILE *file, int **matrix, int height){
    fclose(file);
    for (int i = 0; i < height; i++) {
        free(matrix[i]);
    }
    free(matrix);
}

void print_matrix_and_counts(int **matrix, int height, int width, int tick) {
    int cntP=0,cntH=0,cntC=0;

    printf("\n--- Tick %d ---\n", tick);
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            int v = matrix[i][j];
            if(v == EMPTY) printf(" - ");
            else if(v == PLANT){ printf("\033[32m P \033[0m"); cntP++; }
            else if(v == HERB ){ printf("\033[34m H \033[0m"); cntH++; }
            else if(v == CARN ){ printf("\033[31m C \033[0m"); cntC++; }
        }
        printf("\n");
    }
    printf("Plantas:%d  Herbivoros:%d  Carnivoros:%d\n", cntP, cntH, cntC);
    printf("Fin del tick %d\n", tick);
}

int main(int argc, char *argv[]){
    // Argumentos
        if(argc!=4){
        printf("Wrong usage: %s <input path> <MAX_TICKS> <Verbose>\n", argv[0]);
        return 1;
    }

    char *file_path = argv[1];
    int max_ticks = atoi(argv[2]);
    if(max_ticks<1){
        printf("Ticks must be bigger or equal than 1\n");
        return 1;
    }
    int verbose = atoi(argv[3]);
    if(verbose<0 || verbose>2){
        printf("Verbose must be either 0, 1 or 2\n");
        return 1;
    }

    srand((unsigned)time(NULL));

    // lectura del archivo de entrada
    FILE *file = fopen(file_path, "r");
    if(!file){
        perror("Opening input failed\n");
        return 1;
    }

    int width = 0;
    int height = 0;
    char line[MAX_LINE];
    // Medir dimensiones
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
    if(verbose>1){
        printf("Height: %d\nWidth: %d\n", height, width);
    }
    // Matriz actual (estado)
    int **current = (int**)malloc(height*sizeof(int*));
    for(int i = 0; i < height; i++){
        current[i] = (int*)malloc(width*sizeof(int));
    }

    // Parseo
    int compliance = 0;
    char line2[MAX_LINE];
    int row = 0;
    while(fgets(line2, sizeof(line2), file) && row < height){
        int col = 0;
        char *token = strtok(line2, " \n");
        while(token != NULL && col < width){
            if(strcmp(token, "_") == 0 || strcmp(token, "-") == 0){
                current[row][col] = EMPTY;
            } else if(strcmp(token, "p") == 0 || strcmp(token, "P") == 0){
                current[row][col] = PLANT;
            } else if(strcmp(token, "h") == 0 || strcmp(token, "H") == 0){
                current[row][col] = HERB;
            } else if(strcmp(token, "c") == 0 || strcmp(token, "C") == 0){
                current[row][col] = CARN;
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
        cleanup(file, current, height);
        return 1;
    }

    // Atributos de H y C (energía, hambre), comienzan en 0.
    int **eH   = alloc2d(height,width), **eC   = alloc2d(height,width);
    int **hunH = alloc2d(height,width), **hunC = alloc2d(height,width);

    // Simulación por ticks
    double t0 = omp_get_wtime();
    for(int tick = 0; tick <= max_ticks; tick++){
        // Mostrar el estado actual y conteos
        if(verbose==2){
            print_matrix_and_counts(current, height, width, tick);
        }
        if(tick == max_ticks) break; // no generamos siguiente estado después del último print

        // next arranca vacío
        int **next = alloc2d(height,width);
        int **eHn   = alloc2d(height,width), **eCn   = alloc2d(height,width);
        int **hunHn = alloc2d(height,width), **hunCn = alloc2d(height,width);

        // PLANTAS: persistencia y reproducción
        // plantas en 'next' Si luego H/C pisan esa celda, la sobreescriben.
        for(int i = 0; i < height; i++){
            for(int j = 0; j < width; j++){
                if(current[i][j] == PLANT){
                    // Planta persiste
                    next[i][j] = PLANT;

                    // Reproducción 4-dir a celdas vacías del estado ACTUAL
                    int ord[4]; shuffle_dirs(ord);
                    for(int k = 0; k < 4; k++){
                        int d = ord[k];
                        int ni = i + DX[d];
                        int nj = j + DY[d];
                        if(!in_bounds(ni,nj,height,width)) continue;
                        if(current[ni][nj] == EMPTY && next[ni][nj] == EMPTY){
                            if((rand() % 100) < PLANT_SPREAD_PCT){
                                next[ni][nj] = PLANT;
                            }
                        }
                    }
                }
            }
        }

        // CARNÍVOROS: cazan H; si no, mueven a vacío; inanición y reproducción
        for(int i = 0; i < height; i++){
            for(int j = 0; j < width; j++){
                if(current[i][j] != CARN) continue;

                int ord[4]; shuffle_dirs(ord);
                int ti=i, tj=j; int moved=0, ate=0;

                // a) Buscar H adyacente (cazar)
                for(int k=0;k<4 && !moved;k++){
                    int d=ord[k], ni=i+DX[d], nj=j+DY[d];
                    if(!in_bounds(ni,nj,height,width)) continue;
                    if(current[ni][nj]==HERB){
                        ti=ni; tj=nj; moved=1; ate=1;
                    }
                }
                // b) Si no cazó, mover a vacío (en next)
                if(!moved){
                    for(int k=0;k<4 && !moved;k++){
                        int d=ord[k], ni=i+DX[d], nj=j+DY[d];
                        if(!in_bounds(ni,nj,height,width)) continue;
                        if(next[ni][nj]==EMPTY || next[ni][nj]==PLANT){ ti=ni; tj=nj; moved=1; } //mover a vacío (en next)
                    }
                }
                // c) Si no pudo, quedarse (si libre o había planta)
                if(!moved && (next[i][j]==EMPTY || next[i][j]==PLANT)){ ti=i; tj=j; }

                // Actualizar energía o hambre
                int energy = eC[i][j] + (ate ? C_ENERGY_GAIN : 0);
                int hungry = ate ? 0 : (hunC[i][j] + 1);

                // Muerte por inanición
                if(hungry >= C_STARVE_TICKS) continue;

                // Escribir al destino
                next[ti][tj] = CARN;
                eCn[ti][tj] = energy;
                hunCn[ti][tj] = hungry;

                // Reproducción si alcanzó umbral y hay espacio
                if(energy >= C_REPRO_THRESH){
                    int babyPlaced=0; int ord2[4]; shuffle_dirs(ord2);
                    for(int kk=0;kk<4;kk++){
                        int d=ord2[kk], bi=ti+DX[d], bj=tj+DY[d];
                        if(!in_bounds(bi,bj,height,width)) continue;
                        if(next[bi][bj]==EMPTY){
                            next[bi][bj]=CARN; // el bebesito 
                            // bebesito con atributos 0 por default
                            babyPlaced=1; break;
                        }
                    }
                    if(babyPlaced){
                        energy -= C_REPRO_THRESH;
                        if(energy<0) energy=0;
                        eCn[ti][tj] = energy; // actualizar de los papis
                    }
                }
            }
        }

        // HERBÍVOROS: escapan si hay C, si no, comen P; si no, mueven a vacío, inanición y reproducción
        for(int i = 0; i < height; i++){
            for(int j = 0; j < width; j++){
                if(current[i][j] != HERB) continue;

                int ord[4]; shuffle_dirs(ord);
                int ti=i, tj=j; int moved=0, ate=0;

                // a) ¿Peligro? (C adyacente) -> escapar a vacío
                int danger=0;
                for(int k=0;k<4;k++){
                    int ni=i+DX[k], nj=j+DY[k];
                    if(in_bounds(ni,nj,height,width) && current[ni][nj]==CARN){ danger=1; break; }
                }
                if(danger){
                    for(int k=0;k<4 && !moved;k++){
                        int d=ord[k], ni=i+DX[d], nj=j+DY[d];
                        if(!in_bounds(ni,nj,height,width)) continue;
                        if(next[ni][nj]==EMPTY || next[ni][nj]==PLANT){ ti=ni; tj=nj; moved=1; } // huir si hay C
                    }
                }

                // b) Si no escapó, buscar planta para comer
                if(!moved){
                    for(int k=0;k<4 && !moved;k++){
                        int d=ord[k], ni=i+DX[d], nj=j+DY[d];
                        if(!in_bounds(ni,nj,height,width)) continue;
                        if(current[ni][nj]==PLANT && next[ni][nj]!=CARN){
                            ti=ni; tj=nj; moved=1; ate=1;
                        }
                    }
                }

                // c) Si no comió, moverse a vacío
                if(!moved){
                    for(int k=0;k<4 && !moved;k++){
                        int d=ord[k], ni=i+DX[d], nj=j+DY[d];
                        if(!in_bounds(ni,nj,height,width)) continue;
                        if(next[ni][nj]==EMPTY || next[ni][nj]==PLANT){ ti=ni; tj=nj; moved=1; } //moverse a vacío si no comió
                    }
                }

                // d) Si nada funcionó, quedarse si no hay C ya puesto
                if(!moved && (next[i][j]==EMPTY || next[i][j]==PLANT)){ ti=i; tj=j; }

                // Actualizar energía/hambre
                int energy = eH[i][j] + (ate ? H_ENERGY_GAIN : 0);
                int hungry = ate ? 0 : (hunH[i][j] + 1);

                // Muerte por inanición
                if(hungry >= H_STARVE_TICKS) continue;

                // Escribir al destino
                if(next[ti][tj]!=CARN){ // no pisar C
                    next[ti][tj] = HERB;
                    eHn[ti][tj] = energy;
                    hunHn[ti][tj] = hungry;

                    // Reproducción si alcanzó umbral y hay espacio
                    if(energy >= H_REPRO_THRESH){
                        int babyPlaced=0; int ord2[4]; shuffle_dirs(ord2);
                        for(int kk=0;kk<4;kk++){
                            int d=ord2[kk], bi=ti+DX[d], bj=tj+DY[d];
                            if(!in_bounds(bi,bj,height,width)) continue;
                            if(next[bi][bj]==EMPTY){
                                next[bi][bj]=HERB; // cría
                                // cría con atributos 0
                                babyPlaced=1; break;
                            }
                        }
                        if(babyPlaced){
                            energy -= H_REPRO_THRESH;
                            if(energy<0) energy=0;
                            eHn[ti][tj] = energy; // actualizar del progenitor
                        }
                    }
                }
            }
        }

        // Intercambio de buffers: current <- next
        for(int i=0;i<height;i++){
            memcpy(current[i], next[i], width*sizeof(int));
        }

        // Swap atributos: eH<-eHn, eC<-eCn, hunH<-hunHn, hunC<-hunCn
        for(int i=0;i<height;i++){
            memcpy(eH[i],   eHn[i],   width*sizeof(int));
            memcpy(eC[i],   eCn[i],   width*sizeof(int));
            memcpy(hunH[i], hunHn[i], width*sizeof(int));
            memcpy(hunC[i], hunCn[i], width*sizeof(int));
        }

        // Liberar buffers next
        free2d(next,  height);
        free2d(eHn,   height);
        free2d(eCn,   height);
        free2d(hunHn, height);
        free2d(hunCn, height);
    }
    double t1 = omp_get_wtime();
    if(verbose>0){
        printf("\nTiempo total de simulacion: %.6f segundos\n", t1 - t0);
    }
    // Liberar todo
    free2d(eH,   height);
    free2d(eC,   height);
    free2d(hunH, height);
    free2d(hunC, height);
    cleanup(file, current, height);
    return 0;
}
