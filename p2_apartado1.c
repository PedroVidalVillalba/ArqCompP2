/**
 * PRÁCTICA 2 ARQUITECTURA DE COMPUTADORES
 * Programación Multinúcleo y extensiones SIMD
 *
 * @date 12/03/2024
 * @authors Cao López, Carlos
 * @authors Vidal Villalba, Pedro
 */


#include <stdlib.h>
#include <stdio.h>

#define M 8

/* drand48 devuelve un double aleatorio en [0, 1); le sumamos 1 para ponerlo en [1, 2)
 * lrand48 devuelve un long aleatorio; si el número es par multiplicamos por 1 y si es impar por -1 */
#define get_rand() ((2 * (lrand48() & 1) - 1) * (1 + drand48()))

void alloc_matrix(double*** matrix, int rows, int columns) {
    *matrix = (double **) malloc(rows * sizeof(double));
    for (int i = 0; i < rows; i++) {
        (*matrix)[i] = (double *) calloc(columns, sizeof(double));
    }
}

void free_matrix(double **matrix, int rows, int columns) {   
    int i;
     
    for (i = 0; i < (rows); i++) {              
        free(matrix[i]);                        
    }                                           
    free(matrix);                               
}

void random_matrix(double **matrix, int rows, int columns) {  
    int i, j;

    for (i = 0; i < rows; i++) {                
        for (j = 0; j < columns; j++) {         
            matrix[i][j] = get_rand();          
        }                                       
    }                                           
}

void random_array(double* array, int size) {         
    int i;

    for (i = 0; i < size; i++) {            
        array[i] = get_rand();                  
    }                                           
}

void random_index(int** index, int size) {
    int i, j;
    int temp;

    *index = (int *) malloc(size * sizeof(int));
    for (i = 0; i < size; i++) {
        (*index)[i] = i;
    }

    srand(42);
    for (i = size - 1; i >= 0; i--) {
        j = rand() % (size - 1);

        temp = (*index)[i];
        (*index)[i] = (*index)[j];
        (*index)[j] = (*index)[i];
    }
}

/* drand48 devuelve un double aleatorio en [0, 1); le sumamos 1 para ponerlo en [1, 2)
 * lrand48 devuelve un long aleatorio; si el número es par multiplicamos por 1 y si es impar por -1 */
#define get_rand() ((2 * (lrand48() & 1) - 1) * (1 + drand48()))


int main(int argc, char** argv) {
    double **a, **b, **d;
    double *c, *e;
    double f;
    int *ind;
    int i, j, k;
    int N;


    /* Procesamos los argumentos */
    if (argc != 2) {
        fprintf(stderr, "Formato de ejecución: %s N ", argv[0]);
        exit(EXIT_FAILURE);
    }
    N = atoi(argv[1]);

    alloc_matrix(&a, N, M);
    alloc_matrix(&b, M, N);
    c = (double *) malloc(M * sizeof(double));
    e = (double *) malloc(N * sizeof(double));
    
    srand48(888);
    random_matrix(a, N, M);
    random_matrix(b, M, N);
    random_array(c, M);
    random_index(&ind, N);

    // Inicialización de todas las componentes de d a cero
    alloc_matrix(&d, N, N);

    // Realizar las operaciones especificadas
    for (i = 0; i < N; i++) {
        for (j = 0; j < N; j++) {
            for (k = 0; k < M; k++) {
                d[i][j] += 2 * a[i][k] * (b[k][j] - c[k]);
            }
        }
    }

    f = 0;

    for (i = 0; i < N; i++) {
        e[i] = d[ind[i]][ind[i]] / 2;
        f += e[i];
    }

    // Imprimir el valor de f
    printf("%lf\n", f);

    free_matrix(a, N, M);
    free_matrix(b, M, N);
    free_matrix(d, N, N);
    free(c);
    free(e);
    free(ind);

    exit(EXIT_SUCCESS);
}
