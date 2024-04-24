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
#include <pmmintrin.h>
#include <omp.h>

#define M 8

#define RAND_SEED 888
/* drand48 devuelve un double aleatorio en [0, 1); le sumamos 1 para ponerlo en [1, 2)
 * lrand48 devuelve un long aleatorio; si el número es par multiplicamos por 1 y si es impar por -1 */
#define get_rand() ((2 * (lrand48() & 1) - 1) * (1 + drand48()))

/* /1* Directivas del preprocesador para escoger los diferentes tipos de scheduling durante la compilación *1/ */
/* #ifdef USE_DYNAMIC */
/* #define SCHED_TYPE dynamic */
/* #elif defined(USE_AUTO) */
/* #define SCHED_TYPE auto */
/* #elif defined(USE_GUIDED) */
/* #define SCHED_TYPE guided */
/* #elif defined(USE_RUNTIME) */
/* #define SCHED_TYPE runtime */
/* #else // Por defecto static */
/* #define SCHED_TYPE static */
/* #endif */

#define SCHED_TYPE auto   /* A la vista de los experimentos, es el que mejores resultados aporta */

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
    long line_size = sysconf(_SC_LEVEL1_DCACHE_LINESIZE);

    *index = (int *) _mm_malloc(size * sizeof(int), line_size);
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
    register int i, j;
    register int a_index, b_index;
    register int ii, jj;
    register int N;
    register int C;
    register double *a, *b, *d;
    double *transpose;
    register double d_value;
    int *ind;
    register double *c, *e;
    double f;   /* No se puede poner como register porque tiene que ser compartida entre los hilos */
    double ck;
    long line_size = sysconf(_SC_LEVEL1_DCACHE_LINESIZE);
    register uint block_size;

    /* Procesamos los argumentos */
    if (argc != 3) {
        fprintf(stderr, "Formato de ejecución: %s N C", argv[0]);
        exit(EXIT_FAILURE);
    }
    N = atoi(argv[1]);
    C = atoi(argv[2]);

    a = (double *) _mm_malloc(N * M * sizeof(double), line_size);
    b = (double *) _mm_malloc(M * N * sizeof(double), line_size);
    d = (double *) _mm_malloc(N * N * sizeof(double), line_size);
    c = (double *) _mm_malloc(M * sizeof(double), line_size);
    e = (double *) _mm_malloc(N * sizeof(double), line_size);
    transpose = (double *) _mm_malloc(N * M * sizeof(double), line_size);

    srand48(RAND_SEED);
    random_matrix(a, N, M);
    random_matrix(b, M, N);
    random_array(c, M);
    random_index(&ind, N);

    // Comenzamos el contador de ciclos
    start_counter();

    // Realizar las operaciones especificadas
    block_size = line_size / sizeof(double);

    f = 0;
#pragma omp parallel private(i, j, ii, jj, d_value, a_index, b_index) num_threads (C)
    {
        double private_f = 0;

        /* Trasponer la matriz b */
#pragma omp for schedule(SCHED_TYPE)
        for (j = 0; j < N; j++) {
            for (i = 0; i < M; i++) {
                transpose[j * M + i] = b[i * N + j];
            }
        }
#pragma omp barrier
#pragma omp single
        _mm_free(b);

        b = transpose;

#pragma omp for collapse(2) schedule(SCHED_TYPE)
        for (i = 0; i < N - N % block_size; i += block_size) {
            for (j = 0; j < N - N % block_size; j += block_size) {
                for (ii = i; ii < i + block_size; ii++) {
                    for (jj = j; jj < j + block_size; jj++) {
                        d_value = 0;
                        a_index = ii * M;
                        b_index = jj * M;

                        d_value += a[a_index++] * (b[b_index++] - c[0]);
                        d_value += a[a_index++] * (b[b_index++] - c[1]);
                        d_value += a[a_index++] * (b[b_index++] - c[2]);
                        d_value += a[a_index++] * (b[b_index++] - c[3]);
                        d_value += a[a_index++] * (b[b_index++] - c[4]);
                        d_value += a[a_index++] * (b[b_index++] - c[5]);
                        d_value += a[a_index++] * (b[b_index++] - c[6]);
                        d_value += a[a_index  ] * (b[b_index  ] - c[7]);

                        d[ii * N + jj] = 2 * d_value;
                    }
                }
            }
        }

        // Hacer operaciones restantes
#pragma omp for
        for (i = N - N % block_size; i < N; i++) {
            for (j = N - N % block_size; j < N; j++) {
                d_value = 0;
                a_index = i * M;
                b_index = j * M;

                d_value += a[a_index++] * (b[b_index++] - c[0]);
                d_value += a[a_index++] * (b[b_index++] - c[1]);
                d_value += a[a_index++] * (b[b_index++] - c[2]);
                d_value += a[a_index++] * (b[b_index++] - c[3]);
                d_value += a[a_index++] * (b[b_index++] - c[4]);
                d_value += a[a_index++] * (b[b_index++] - c[5]);
                d_value += a[a_index++] * (b[b_index++] - c[6]);
                d_value += a[a_index  ] * (b[b_index  ] - c[7]);

                d[i * N + j] = 2 * d_value;
            }
        }
#pragma omp barrier

#pragma omp for
        for (i = 0; i < N; i++) {
            e[i] = d[ind[i] * (N + 1)] / 2;
            private_f += e[i];
        }
#pragma omp atomic
        f += private_f;
    }

    ck = get_counter();

    // Imprimir el valor de f
#ifndef DEBUG
    printf("%lf\n", f);
#else   //DEBUG
    printf("%12.2lf\n", ck);
#endif  //DEBUG

    _mm_free(a);
    _mm_free(b);
    _mm_free(d);
    _mm_free(c);
    _mm_free(e);
    _mm_free(ind);

    exit(EXIT_SUCCESS);
}
