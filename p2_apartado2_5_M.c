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
#include <unistd.h>
#include <time.h>

#define M 8
#define RAND_SEED 888

/* drand48 devuelve un double aleatorio en [0, 1); le sumamos 1 para ponerlo en [1, 2)
 * lrand48 devuelve un long aleatorio; si el número es par multiplicamos por 1 y si es impar por -1 */
#define get_rand() ((2 * (lrand48() & 1) - 1) * (1 + drand48()))


/*** CÓDIGO ASOCIADO A LA MEDIDA DE CICLOS ***/

void start_counter();
double get_counter();

/* Initialize the cycle counter */

static unsigned cyc_hi = 0;
static unsigned cyc_lo = 0;


/* Set *hi and *lo to the high and low order bits of the cycle counter.
Implementation requires assembly code to use the rdtsc instruction. */
void access_counter(unsigned *hi, unsigned *lo)
{
    asm("rdtsc; movl %%edx,%0; movl %%eax,%1" /* Read cycle counter */
            : "=r" (*hi), "=r" (*lo) /* and move results to */
            : /* No input */ /* the two outputs */
            : "%edx", "%eax");
}

/* Record the current value of the cycle counter. */
void start_counter()
{
    access_counter(&cyc_hi, &cyc_lo);
}

/* Return the number of cycles since the last call to start_counter. */
double get_counter()
{
    unsigned ncyc_hi, ncyc_lo;
    unsigned hi, lo, borrow;
    double result;

    /* Get cycle counter */
    access_counter(&ncyc_hi, &ncyc_lo);

    /* Do double precision subtraction */
    lo = ncyc_lo - cyc_lo;
    borrow = lo > ncyc_lo;
    hi = ncyc_hi - cyc_hi - borrow;
    result = (double) hi * (1 << 30) * 4 + lo;
    if (result < 0) {
        fprintf(stderr, "Error: counter returns neg value: %.0f\n", result);
    }
    return result;
}

/*** FIN CÓDIGO ASOCIADO A LA MEDIDA DE CICLOS ***/

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

/** Fisher-Yates shuffle */
void random_index(int** index, int size) {
    int i, j;
    int temp;

    *index = (int *) malloc(size * sizeof(int));
    for (i = 0; i < size; i++) {
        (*index)[i] = i;
    }

    srandom(time(NULL));
    for (i = size - 1; i > 0; i--) {
        j = random() % i;

        temp = (*index)[i];
        (*index)[i] = (*index)[j];
        (*index)[j] = temp;
    }
}

int main(int argc, char** argv) {
    double **a, **b, **d;
    double *c, *e;
    double f;
    double ck;
    int *ind;
    int i, j, k;
    int ii, jj;
    int N;
    long line_size = sysconf(_SC_LEVEL1_DCACHE_LINESIZE);


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
    
    srand48(RAND_SEED);
    random_matrix(a, N, M);
    random_matrix(b, M, N);
    random_array(c, M);
    random_index(&ind, N);

    // Inicialización de todas las componentes de d a cero
    alloc_matrix(&d, N, N);

    // Comenzamos el contador de ciclos
    start_counter();

    // Realizar las operaciones por bloques especificadas
    uint block_size = line_size / sizeof(double);

    for (i=0; i < N - block_size ; i+=block_size)
        for (j=0; j < N - block_size; j+=block_size)
            for (ii=i; ii<i+block_size; ii++) {
                for (jj=j; jj<j+block_size; jj++){
                    for (k=0; k < M; k++){
                        d[ii][jj] += 2 * a[ii][k] * (b[k][jj] - c[k]);
                    }
                }

            }


    // Hacer operaciones restantes
    for(i = ii; i < N; i++) {
        for(j = jj; j < N; j++){
            for(k=0; k < M; k++){
                d[i][j] += 2 * a[i][k] * (b[k][j] - c[k]);
            }
        }
    }

    f = 0;

    for (i = 0; i < N; i++) {
        e[i] = d[ind[i]][ind[i]] / 2;
        f += e[i];
    }

    ck=get_counter();

    // Imprimir el valor de f
    printf("%lf\n", f);

    printf("\n Clocks=%1.10lf \n",ck);

    free_matrix(a, N, M);
    free_matrix(b, M, N);
    free_matrix(d, N, N);
    free(c);
    free(e);
    free(ind);

    exit(EXIT_SUCCESS);
}
