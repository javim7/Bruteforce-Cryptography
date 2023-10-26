/**
 * Programa encriptacion por bloques secuencial.
 * @author Pablo Gonzalez
 * @author Jose Hernandez
 * @author Javier Mombiela
*/

// Importar librerías
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <openssl/des.h>
#include <time.h> 

// Función que desencripta el texto cifrado con la llave key
void decrypt(long key, char *ciph, int len) {
    DES_cblock des_key;
    DES_key_schedule key_schedule;

    // Seteamos la paridad de la llave
    for (int i = 0; i < 8; ++i) {
        des_key[i] = (key >> (i * 8)) & 0xFF;
    }

    DES_set_odd_parity(&des_key);
    DES_set_key_checked(&des_key, &key_schedule);

    for (int i = 0; i < len; i += 8) {
        DES_ecb_encrypt((DES_cblock *)(ciph + i), (DES_cblock *)(ciph + i), &key_schedule, DES_DECRYPT);
    }
}

// Función que encripta el texto con la llave key
void encrypt(long key, char *ciph, int len) {
    DES_cblock des_key;
    DES_key_schedule key_schedule;

    // Seteamos la paridad de la llave
    for (int i = 0; i < 8; ++i) {
        des_key[i] = (key >> (i * 8)) & 0xFF;
    }

    DES_set_odd_parity(&des_key);
    DES_set_key_checked(&des_key, &key_schedule);

    for (int i = 0; i < len; i += 8) {
        DES_ecb_encrypt((DES_cblock *)(ciph + i), (DES_cblock *)(ciph + i), &key_schedule, DES_ENCRYPT);
    }
}


char search[] = "es una prueba de"; // Cadena a buscar

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <key>\n", argv[0]);
        return 1;
    }

    char *input_filename = "text.txt";
    long key = strtol(argv[1], NULL, 10);

    FILE *input_file = fopen(input_filename, "r");
    if (!input_file) {
        perror("Failed to open input file");
        return 1;
    }

    fseek(input_file, 0, SEEK_END);
    long filesize = ftell(input_file);
    fseek(input_file, 0, SEEK_SET);

    char *buffer = malloc(filesize + 1);
    if (fread(buffer, 1, filesize, input_file) != filesize) {
        perror("Failed to read input file");
        return 1;
    }

    buffer[filesize] = '\0';

    printf("\nOriginal data : %s\n", buffer);

    clock_t start_time = clock();

    int block_size = 8; // Tamaño de bloque en bytes (DES)
    int num_blocks = filesize / block_size;

    // Divide el archivo en bloques más pequeños y encripta
    for (int i = 0; i < num_blocks; i++) {
        int block_start = i * block_size;
        int block_end = block_start + block_size;
        decrypt(key, buffer + block_start, block_size);
    }

    clock_t end_time = clock(); // Tiempo de ejecución
    printf("Decrypted data: %s\n", buffer);

    char *found = strstr(buffer, search); // Busca la cadena en el texto desencriptado

    // if para imprimir si se encontro o no la cadena
    if (found != NULL) {
        printf("Keyword \"%s\" found in the decrypted text.\n", search);
    } else {
        printf("Keyword \"%s\" not found in the decrypted text.\n", search);
    }

    double execution_time = (double)(end_time - start_time) / CLOCKS_PER_SEC;
    printf("Total execution time: %f seconds\n", execution_time);

    free(buffer);
    fclose(input_file);

    return 0;
}
