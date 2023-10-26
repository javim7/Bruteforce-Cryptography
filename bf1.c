/**
 * Programa encriptacion en bloques paralelo.
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

    int chunk_size = filesize / size;
    int start_idx = rank * chunk_size;
    int end_idx = (rank == size - 1) ? filesize : start_idx + chunk_size;

    char *local_buffer = buffer + start_idx;
    int local_size = end_idx - start_idx;

    // Divide el archivo en bloques más pequeños
    int block_size = 8;  // Tamaño de bloque en bytes (DES)
    int num_blocks = local_size / block_size;
    for (int i = 0; i < num_blocks; i++) {
        int block_start = i * block_size;
        int block_end = block_start + block_size;
        encrypt(key, local_buffer + block_start, block_size);
    }

    MPI_Barrier(MPI_COMM_WORLD);

    char *gathered_buffer = NULL;
    if (rank == 0) {
        gathered_buffer = (char *)malloc(filesize);
    }

    MPI_Gather(local_buffer, local_size, MPI_CHAR, gathered_buffer, local_size, MPI_CHAR, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        // Desencripta el texto completo
        for (int i = 0; i < num_blocks; i++) {
            int block_start = i * block_size;
            int block_end = block_start + block_size;
            decrypt(key, gathered_buffer + block_start, block_size);
        }

        double end_time = MPI_Wtime();
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
    fclose(input_file);
    MPI_Finalize();

    return 0;
}