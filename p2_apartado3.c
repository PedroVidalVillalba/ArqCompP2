/**
 * PRÁCTICA 2 ARQUITECTURA DE COMPUTADORES
 * Programación Multinúcleo y extensiones SIMD
 *
 * @date 15/04/2024
 * @authors Cao López, Carlos
 * @authors Vidal Villalba, Pedro
 */


#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <pmmintrin.h>
#include <immintrin.h>

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

void inverse_index(int** inv_index, const int* index, int size) {
    int i;
    long line_size = sysconf(_SC_LEVEL1_DCACHE_LINESIZE);

    *inv_index = (int *) _mm_malloc(size * sizeof(int), line_size);
    for (i = 0; i < size; i++) {
        (*inv_index)[index[i]] = i;
    }
}

void transpose(double** matrix, int rows, int columns) {
    int i, j;
    long line_size = sysconf(_SC_LEVEL1_DCACHE_LINESIZE);
    double* transpose = (double *) _mm_malloc(rows * columns * sizeof(double), line_size);

    for (i = 0; i < rows; i++) {
        for (j = 0; j < columns; j++) {
            transpose[j * rows + i] = (*matrix)[i * columns + j];
        }
    }

    _mm_free(*matrix);
    *matrix = transpose;
}

int main(int argc, char** argv) {
    register int i, j; // Reordenamos datos 1
    register int ii, jj;
    int N; // Reordenamos datos 2
    double *a, *b, *d;
    int *ind; // Reordenamos datos 3 (así esta cerca de d)
    int *inv_ind; // Creamos un índice inverso para poder juntar los bucles
    register double *c, *e;
    register double f;
    double ck;
    long line_size = sysconf(_SC_LEVEL1_DCACHE_LINESIZE);
    uint block_size;
    register __m512d vec_a, vec_b, vec_c, vec_d, vec_e;
    /* Procesamos los argumentos */
    if (argc != 2) {
        fprintf(stderr, "Formato de ejecución: %s N ", argv[0]);
        exit(EXIT_FAILURE);
    }
    N = atoi(argv[1]);

    a = (double *) _mm_malloc(N * M * sizeof(double), line_size);
    b = (double *) _mm_malloc(M * N * sizeof(double), line_size);
    d = (double *) _mm_malloc(N * N * sizeof(double), line_size);
    c = (double *) _mm_malloc(M * sizeof(double), line_size);
    e = (double *) _mm_malloc(N * sizeof(double), line_size);

    srand48(RAND_SEED);
    random_matrix(a, N, M);
    random_matrix(b, M, N);
    transpose(&b, M , N);
    random_array(c, M);
    random_index(&ind, N);
    inverse_index(&inv_ind, ind, N);

    // Comenzamos el contador de ciclos
    start_counter();

    // Realizar las operaciones especificadas
    /**
     *
     * NOTA: HEMOS USADO OPERACIONES p2_apartado_2_1.c sin d_index porque es una movida con bloques. Hemos usado p2_apartado2_5.c y p2_apartado2_4.c para el desenrrollamiento con d_value en registro.
     * RESULTADO N=3500 CÓDIGO: 612729748
     * RESULTADO N=3500 O3:      58013340
     */

    block_size = line_size / sizeof(double);

    f = 0;
    vec_c = _mm512_load_pd(c);  // Cargamos el vector c, que ya es de tamaño 8
    for (i = 0; i < N - N % block_size ; i += block_size) {
        for (j = 0; j < N - N % block_size; j += block_size) {
            for (ii = i; ii < i + block_size; ii++) {
                vec_a = _mm512_load_pd(a + ii * M);
                for (jj = j; jj < j + block_size; jj++) {
                    vec_b = _mm512_load_pd(b + jj * M);
                    vec_d = _mm512_mul_pd(vec_a, _mm512_sub_pd(vec_b, vec_c));

                    d[ii * N + jj] = 2 * _mm512_reduce_add_pd(vec_d);
                }
            }
        }
        e[inv_ind[i    ]] = d[ i      * (N + 1)] / 2;
        e[inv_ind[i + 1]] = d[(i + 1) * (N + 1)] / 2;
        e[inv_ind[i + 2]] = d[(i + 2) * (N + 1)] / 2;
        e[inv_ind[i + 3]] = d[(i + 3) * (N + 1)] / 2;
        e[inv_ind[i + 4]] = d[(i + 4) * (N + 1)] / 2;
        e[inv_ind[i + 5]] = d[(i + 5) * (N + 1)] / 2;
        e[inv_ind[i + 6]] = d[(i + 6) * (N + 1)] / 2;
        e[inv_ind[i + 7]] = d[(i + 7) * (N + 1)] / 2;
        f += e[inv_ind[i]] + e[inv_ind[i + 1]] + e[inv_ind[i + 2]] + e[inv_ind[i + 3]] + e[inv_ind[i + 4]] + e[inv_ind[i + 5]] + e[inv_ind[i + 6]] + e[inv_ind[i + 7]];
    }
    // Hacer operaciones restantes
    for (i = ii; i < N; i++) {
        vec_a = _mm512_load_pd(a + i * M);
        for (j = jj; j < N; j++) {
            vec_b = _mm512_load_pd(b + j * M);
            vec_d = _mm512_mul_pd(vec_a, _mm512_sub_pd(vec_b, vec_c));

            d[i * N + j] = 2 * _mm512_reduce_add_pd(vec_d);
        }
        e[inv_ind[i]] = d[i * (N + 1)] / 2;
        f += e[inv_ind[i]];
    }

    ck = get_counter();

    // Imprimir el valor de f
    printf("%lf\n", f);

    printf("\n Clocks=%1.10lf \n",ck);

    _mm_free(a);
    _mm_free(b);
    _mm_free(d);
    _mm_free(c);
    _mm_free(e);
    _mm_free(ind);
    _mm_free(inv_ind);

    exit(EXIT_SUCCESS);
}
