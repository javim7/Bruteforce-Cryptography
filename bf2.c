/**
 * Programa cifrado de flujo paralelo.
 * @author Pablo Gonzalez
 * @author Jose Hernandez
 * @author Javier Mombiela
*/


// importar librerias
#include <mpi.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <openssl/des.h>

//funcion que desencripta el texto cifrado con la llave key
void decrypt(long key, char *ciph, int len) {
    DES_cblock des_key;
    DES_key_schedule key_schedule;

    // Seteamos la paridad de llave
    for (int i = 0; i < 8; ++i) {
        des_key[i] = (key >> (i * 8)) & 0xFF;
    }

    DES_set_odd_parity(&des_key);
    DES_set_key_checked(&des_key, &key_schedule);

    for (int i = 0; i < len; i += 8) {
        DES_ecb_encrypt((DES_cblock *)(ciph + i), (DES_cblock *)(ciph + i), &key_schedule, DES_DECRYPT);
    }
}

// funcion que encripta el texto con la llave key
void encrypt(long key, char *ciph, int len) {
    DES_cblock des_key;
    DES_key_schedule key_schedule;

    // Seteamos la paridad de llave
    for (int i = 0; i < 8; ++i) {
        des_key[i] = (key >> (i * 8)) & 0xFF;
    }

    DES_set_odd_parity(&des_key);
    DES_set_key_checked(&des_key, &key_schedule);

    for (int i = 0; i < len; i += 8) {
        DES_ecb_encrypt((DES_cblock *)(ciph + i), (DES_cblock *)(ciph + i), &key_schedule, DES_ENCRYPT);
    }
}

char search[] = "es una prueba de"; // cadena a buscar

int main(int argc, char *argv[]) {
    MPI_Init(&argc, &argv);
    int rank, size;

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (argc != 2) {
        if (rank == 0) {
            printf("Usage: %s <key>\n", argv[0]);
        }
        MPI_Finalize();
        return 1;
    }

    char *input_filename = "text.txt";
    long key = strtol(argv[1], NULL, 10);

    FILE *input_file = fopen(input_filename, "r");
    if (!input_file) {
        perror("Failed to open input file");
        MPI_Finalize();
        return 1;
    }

    fseek(input_file, 0, SEEK_END);
    long filesize = ftell(input_file);
    fseek(input_file, 0, SEEK_SET);

    char *buffer = malloc(filesize + 1);
    if (fread(buffer, 1, filesize, input_file) != filesize) {
        perror("Failed to read input file");
        MPI_Finalize();
        return 1;
    }

    buffer[filesize] = '\0';

    if (rank == 0) {
        printf("\nOriginal data : %s\n", buffer);
    }

    double start_time = MPI_Wtime();

    // Calcular el tamaño del bloque para cada proceso
    int local_size = filesize / size;
    char *local_buffer = malloc(local_size + 1);

    // Leer y cifrar el archivo en tiempo real
    fseek(input_file, rank * local_size, SEEK_SET);
    if (rank == size - 1) {
        local_size = filesize - (rank * local_size);
    }
    fread(local_buffer, 1, local_size, input_file);

    // Cifrar los datos a medida que se leen
    for (int i = 0; i < local_size; i += 8) {
        int block_size = (local_size - i >= 8) ? 8 : local_size - i;
        encrypt(key, local_buffer + i, block_size);
    }

    MPI_Barrier(MPI_COMM_WORLD);

    char *gathered_buffer = NULL;
    if (rank == 0) {
        gathered_buffer = (char *)malloc(filesize);
    }

    MPI_Gather(local_buffer, local_size, MPI_CHAR, gathered_buffer, local_size, MPI_CHAR, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        double end_time = MPI_Wtime();

        // Desencriptar el texto completo
        for (int i = 0; i < filesize; i += 8) {
            int block_size = (filesize - i >= 8) ? 8 : filesize - i;
            decrypt(key, gathered_buffer + i, block_size);
        }

        printf("Decrypted data: %s\n", gathered_buffer);

        char *found = strstr(gathered_buffer, search);

        if (found != NULL) {
            printf("Keyword \"%s\" found in the decrypted text.\n", search);
        } else {
            printf("Keyword \"%s\" not found in the decrypted text.\n", search);
        }

        printf("Total execution time: %f seconds\n", end_time - start_time);

        free(gathered_buffer);
    }

    free(buffer);
    free(local_buffer);
    fclose(input_file);
    MPI_Finalize();

    return 0;
}