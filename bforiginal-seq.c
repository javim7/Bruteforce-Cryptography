/**
 * Programa que utiliza fuerza bruta para desencriptar un texto cifrado con DES.
 * Código modificado para hacer secuencual.
*/

// Importamos las librerías necesarias
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <openssl/des.h>
#include <time.h> 

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

// Definimos la cadena a buscar
char search[] = " the ";

// Definimos la función que prueba la llave
int tryKey(long key, char *ciph, int len) {
    char temp[len + 1];
    memcpy(temp, ciph, len);
    temp[len] = 0;
    decrypt(key, temp, len);
    return strstr((char *)temp, search) != NULL;
}

unsigned char cipher[] = {108, 245, 65, 63, 125, 200, 150, 66, 17, 170, 207, 170, 34, 31, 70, 215, 0};

int main(int argc, char *argv[]) {
    long upper = (1L << 56); // Upper bound DES keys 2^56
    long found = 0;

    clock_t start_time = clock(); // Iniciar el temporizador

    for (long i = 0; i < upper; ++i) {
        if (tryKey(i, (char *)cipher, sizeof(cipher) - 1)) {
            found = i;
            break;
        }
    }

    clock_t end_time = clock(); // Detener el temporizador

    decrypt(found, (char *)cipher, sizeof(cipher) - 1);

    printf("Key found: %li %s\n", found, cipher);

    double execution_time = (double)(end_time - start_time) / CLOCKS_PER_SEC;
    printf("Total execution time: %f seconds\n", execution_time);

    return 0;
}
