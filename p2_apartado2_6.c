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


/* CÓDIGO ASOCIADO A LA MEDIDA DE CICLOS */

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

/* FIN CÓDIGO ASOCIADO A LA MEDIDA DE CICLOS */

void random_matrix(double *matrix, int rows, int columns) {
    int i, j;

    for (i = 0; i < rows; i++) {
        for (j = 0; j < columns; j++) {
            matrix[i * columns + j] = get_rand();
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

    srandom(time(NULL));
    for (i = size - 1; i > 0; i--) {
        j = random() % i;

        temp = (*index)[i];
        (*index)[i] = (*index)[j];
        (*index)[j] = temp;
    }
}

int main(int argc, char** argv) {
    register int i, j, k; // Reordenamos datos 1
    register int d_index, a_index;
    int ii, jj;
    int N; // Reordenamos datos 2
    double *a, *b, *d;
    register double d_value;
    int *ind; // Reordenamos datos 3 (así esta cerca de d)
    double *c, *e;
    double f;
    double ck;
    long line_size = sysconf(_SC_LEVEL1_DCACHE_LINESIZE);
    uint block_size;

    /* Procesamos los argumentos */
    if (argc != 2) {
        fprintf(stderr, "Formato de ejecución: %s N ", argv[0]);
        exit(EXIT_FAILURE);
    }
    N = atoi(argv[1]);

    a = (double *) malloc(N * M * sizeof(double));
    b = (double *) malloc(M * N * sizeof(double));
    c = (double *) malloc(M * sizeof(double));
    e = (double *) malloc(N * sizeof(double));

    srand48(RAND_SEED);
    random_matrix(a, N, M);
    random_matrix(b, M, N);
    random_array(c, M);
    random_index(&ind, N);

    // Inicialización de todas las componentes de d a cero
    d = (double *) calloc(N * N, sizeof(double));

    // Comenzamos el contador de ciclos
    start_counter();

    // Realizar las operaciones especificadas
    /**
     *
     * NOTA: HEMOS USADO OPERACIONES p2_apartado_2_1.c sin d_index porque es una movida con bloques. Hemos usado p2_apartado2_5.c y p2_apartado2_4.c para el desenrrollamiento con d_value en registro.
     * RESULTADO N=3500 CÓDIGO: 612729748
     * RESULTADO N=3500 O3: 58013340
     */


    block_size = line_size / sizeof(double);

    for (i=0; i < N - block_size ; i+=block_size)
        for (j=0; j < N - block_size; j+=block_size)
            for (ii=i; ii<i+block_size; ii++) {
                for (jj=j; jj<j+block_size; jj++){
                    d_value = 0;
                    a_index = ii * M;

                    d_value += a[a_index   ] * (b[        jj] - c[0]);
                    d_value += a[a_index + 1] * (b[    N + jj] - c[1]);
                    d_value += a[a_index + 2] * (b[2 * N + jj] - c[2]);
                    d_value += a[a_index + 3] * (b[3 * N + jj] - c[3]);
                    d_value += a[a_index + 4] * (b[4 * N + jj] - c[4]);
                    d_value += a[a_index + 5] * (b[5 * N + jj] - c[5]);
                    d_value += a[a_index + 6] * (b[6 * N + jj] - c[6]);
                    d_value += a[a_index + 7] * (b[7 * N + jj] - c[7]);

                    d[ii * N + jj] = 2 * d_value;
                }

            }

    // Hacer operaciones restantes
    for(i = ii; i < N; i++) {
        for(j = jj; j < N; j++){
            a_index = i * M;
            for(k=0; k < M; k++){
                d[i * N + j] += a[a_index++] * (b[k * N + j] - c[k]);
            }
            d[i * N + j] *=2;
        }
    }

    f = 0;

    /* Desenrollamos el m.c.d.(todos los posibles valores de N) PERO NO! Después, al traducir a instrucciones vectoriales, queremos que sea múltiplo de 2 -> El más cercano es 8 */
    for (i = 0; i < N - 8; i += 8) {
        e[i] = d[ind[i] * (N + 1)] / 2;
        e[i+1] = d[ind[i+1] * (N + 1)] / 2;
        e[i+2] = d[ind[i+2] * (N + 1)] / 2;
        e[i+3] = d[ind[i+3] * (N + 1)] / 2;
        e[i+4] = d[ind[i+4] * (N + 1)] / 2;
        e[i+5] = d[ind[i+5] * (N + 1)] / 2;
        e[i+6] = d[ind[i+6] * (N + 1)] / 2;
        e[i+7] = d[ind[i+7] * (N + 1)] / 2;

        f += e[i] + e[i+1] + e[i+2] + e[i+3] + e[i+4] + e[i+5] + e[i+6] + e[i+7];
    }
    for (; i < N; i++) {
        e[i] = d[ind[i] * (N + 1)] / 2;
        f += e[i];
    }

    ck=get_counter();

    // Imprimir el valor de f
    printf("%lf\n", f);

    printf("\n Clocks=%1.10lf \n",ck);

    free(a);
    free(b);
    free(d);
    free(c);
    free(e);
    free(ind);

    exit(EXIT_SUCCESS);
}