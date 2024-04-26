/**
 * PRÁCTICA 2 ARQUITECTURA DE COMPUTADORES
 * Programación Multinúcleo y extensiones SIMD.
 * Programa secuencial con optimización por reducción de operaciones.
 *
 * @date 12/03/2024
 * @authors Cao López, Carlos
 * @authors Vidal Villalba, Pedro
 */


#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>

#define M 8             /* Dimensión fija */
#define RAND_SEED 888   /* Semilla aleatoria fija para poder comprobar que los cálculos dan los mismos resultados */
/* Añadir el flag DEBUG (bien manualmente, con #define DEBUG en este archivo, bien al compilar, con -D DEBUG) para imprimir los ciclos
 * de reloj que tarda en ejecutarse en lugar de el valor de f */


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

/**
 * Rellena una matriz (ya creada, y codificada como array unidimensional) con valores
 * aleatorios de valor absoluto en el intervalo [1, 2), y con signo aleatorio.
 *
 * @param matrix    Matriz a rellenar
 * @param rows      Número de filas de la matriz
 * @param columns   Número de columnas de la matriz
 */
void random_matrix(double *matrix, int rows, int columns) {
    int i, j;

    for (i = 0; i < rows; i++) {
        for (j = 0; j < columns; j++) {
            matrix[i * columns + j] = get_rand();
        }
    }
}

/**
 * Rellena un array unidimensional (ya creado) con valores aleatorios
 * de valor absoluto en el intervalo [1, 2), y con signo aleatorio.
 *
 * @param array Array a rellenar
 * @param size  Tamaño del array
 */
void random_array(double* array, int size) {
    int i;

    for (i = 0; i < size; i++) {
        array[i] = get_rand();
    }
}

/**
 * Crea un índice con una permutación aleatoria de 
 * size elementos utilizando el algoritmo de Fisher-Yates shuffle.
 *
 * @param index Puntero al vector de enteros en la que guardar la permutación
 * @param size  Número de elementos en la permutación
 */
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
    int i, j, k; 
    int N; 
    double *a, *b, *d;
    int *ind; 
    double *c, *e;
    double f;
    double ck;

    /* Procesamos los argumentos */
    if (argc != 2) {
        fprintf(stderr, "Formato de ejecución: %s N ", argv[0]);
        exit(EXIT_FAILURE);
    }
    N = atoi(argv[1]);

    /* Reservamos dinámicamente las matrices y vectores */
    a = (double *) malloc(N * M * sizeof(double));
    b = (double *) malloc(M * N * sizeof(double));
    c = (double *) malloc(M * sizeof(double));
    e = (double *) malloc(N * sizeof(double));

    /* Rellenamos con valores aleatorios */
    srand48(RAND_SEED);
    random_matrix(a, N, M);
    random_matrix(b, M, N);
    random_array(c, M);
    random_index(&ind, N);

    /* Creación de la matriz d e inicialización de todas sus componentes a cero.
     * No lo incluimos en el tiempo computable ya que depende de la disponibilidad 
     * del kernel para ejecutar la reserva de memoria */
    d = (double *) calloc(N * N, sizeof(double));

    /*** Comenzamos el contador de ciclos ***/
    start_counter();

    /* Realizar las operaciones especificadas.
     * Aplicamos reducción del número de operaciones realizadas,
     * pero sin cambiar el estado final de ninguna de las variables
     * respecto a la versión base. Reescribimos el programa para usar
     * arrays unidimensionales. */
    for (i = 0; i < N; i++) {
        for (j = 0; j < N; j++) {
            for (k = 0; k < M; k++) {
                d[i * N + j] += a[i * M + k] * (b[k * N + j] - c[k]);
            }
            d[i * N + j] *= 2;
        }
    }

    f = 0;

    for (i = 0; i < N; i++) {
        e[i] = d[ind[i] * (N + 1)] / 2;
        f += e[i];
    }

    ck = get_counter();
    /*** Paramos el contador de ciclos ***/

    /* Imprimir el valor de f o el tiempo de ejecución, según el flag DEBUG */
#ifndef DEBUG
    printf("%lf\n", f);
#else   //DEBUG
    printf("%14.2lf\n", ck);
#endif  //DEBUG

    /* Liberar las variables reservadas dinámicamente */
    free(a);
    free(b);
    free(d);
    free(c);
    free(e);
    free(ind);

    exit(EXIT_SUCCESS);
}
