/**
 * Programa naive que lee el texto des un archivo.
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
    MPI_Init(&argc, &argv); // Inicializar MPI
    int rank, size;

    MPI_Comm_rank(MPI_COMM_WORLD, &rank); // Obtener el rank del proceso
    MPI_Comm_size(MPI_COMM_WORLD, &size); // Obtener el número de procesos

    if (argc != 2) {
        if (rank == 0) {
            printf("Usage: %s <key>\n", argv[0]);
        }
        MPI_Finalize(); // Finalizar MPI
        return 1;
    }

    char *input_filename = "text.txt";
    long key = strtol(argv[1], NULL, 10);

    // abrir el archivo de entrada
    FILE *input_file = fopen(input_filename, "r");
    if (!input_file) {
        perror("Failed to open input file");
        MPI_Finalize(); // Finalize MPI
        return 1;
    }

    // Obtener el tamaño del archivo
    fseek(input_file, 0, SEEK_END);
    long filesize = ftell(input_file);
    fseek(input_file, 0, SEEK_SET);

    // leer el archivo completo
    char *buffer = malloc(filesize + 1);
    if (fread(buffer, 1, filesize, input_file) != filesize) {
        perror("Failed to read input file");
        MPI_Finalize(); // Finalize MPI
        return 1;
    }

    // Asegurar que el buffer sea una cadena de caracteres
    buffer[filesize] = '\0';

    // imprimir el contenido del archivo original
    if (rank == 0) {
        printf("\nOriginal data : %s\n", buffer);
    }

    // Medir el tiempo de ejecución
    double start_time = MPI_Wtime();

    // Cada proceso encripta su parte
    int chunk_size = filesize / size;
    int start_idx = rank * chunk_size;
    int end_idx = (rank == size - 1) ? filesize : start_idx + chunk_size;

    char *local_buffer = buffer + start_idx;
    int local_size = end_idx - start_idx;

    // Cada proceso encripta su parte
    encrypt(key, local_buffer, local_size);

    // imprimir el contenido encriptado
    // printf("Encrypted data: %s\n", local_buffer);

    // sincronizar todos los procesos
    MPI_Barrier(MPI_COMM_WORLD);

    // juntar todas las partes en el proceso 0
    char *gathered_buffer = NULL;
    if (rank == 0) {
        gathered_buffer = (char *)malloc(filesize);
    }

    MPI_Gather(local_buffer, local_size, MPI_CHAR, gathered_buffer, local_size, MPI_CHAR, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        decrypt(key, gathered_buffer, filesize); // desencriptar el texto completo
        double end_time = MPI_Wtime(); // medir el tiempo de ejecución

        // Imprimir el contenido desencriptado
        printf("Decrypted data: %s\n", gathered_buffer);

        // revisar si la palabra clave se encuentra en el texto desencriptado
        char *found = strstr(gathered_buffer, search);

        if (found != NULL) {
            printf("Keyword \"%s\" found in the decrypted text.\n", search);
        } else {
            printf("Keyword \"%s\" not found in the decrypted text.\n", search);
        }

        // imprimir el tiempo de ejecución
        printf("Total execution time: %f seconds\n", end_time - start_time);

        // Liberar la memoria
        free(gathered_buffer);
    }

    // Liberar la memoria
    free(buffer);
    fclose(input_file);
    MPI_Finalize();

    return 0;
}