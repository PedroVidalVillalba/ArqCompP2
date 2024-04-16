#!/bin/bash
# Solicitamos un nodo con 64 cores y 256 GB de memoria durante 2 horas
#SBATCH -n 1 -c 64 -t 02:00:00 --mem=256G
# Ponemos nombre a nuestro trabajo para poder identificarlo.
# ATENCIÓN - Debes sustituir el NN por el número de equipo.
#SBATCH --job-name p2acgNN

# Compilación de los códigos
gcc p2_apartado1.c  -o p21o0 -O0 -D DEBUG
gcc p2_apartado1.c  -o p21o2 -O2 -D DEBUG
gcc p2_apartado1.c  -o p21o3 -O3 -D DEBUG

gcc p2_apartado2.c  -o p22 -O0 -D DEBUG

# gcc p2_apartado3.c  -o p23 -O0 -march=native -D DEBUG

# gcc p2_apartado4.c  -o p24o0 -O0 -fopenmp -D DEBUG
# gcc p2_apartado4.c  -o p24o2 -O2 -fopenmp -D DEBUG


# Ejecución
echo "Id,N,C,Clocks"   # Creamos una cabecera para el CSV resultante
for i in {1..20}; do
    for N in 500 750 1000 1500 2000 3000 3500; do
        # Ejercicio 1 o0
        F1=`./p21o0 $N`
        echo "1o0,${N},1,${F1}"

        # Ejercicio 1 o2
        F2=`./p21o2 $N`
        echo "1o2,${N},1,${F2}"

        # Ejercicio 1 o3
        F3=`./p21o3 $N`
        echo "1o3,${N},1,${F3}"

        # Ejercicio 2
        F4=`./p22 $N`
        echo "2,${N},1,${F4}"

        # # Ejercicio 3
        # F5=`./p23 $N`
        # echo "3,${N},1,${F5}"

        # for C in 2, 4, 8, 16, 32, 64; do
        #     # Ejercicio 4 o0
        #     F6=`./p24o0 $N $C`
        #     echo "4o0,${N},${C},${F6}"

        #     # Ejercicio 4 O2
        #     F7=`./p24o2 $N $C`
        #     echo "4o2,${N},${C},${F7}"
        # done
    done
done

