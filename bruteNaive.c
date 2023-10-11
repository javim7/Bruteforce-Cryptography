#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <unistd.h>
#include <openssl/des.h>

void encrypt(DES_cblock key, char *ciph, int len) {
    DES_key_schedule schedule;
    DES_set_odd_parity(&key);
    DES_set_key_checked(&key, &schedule);

    // Process in 8-byte blocks
    for (int i = 0; i < len; i += 8) {
        DES_ecb_encrypt((DES_cblock *)(ciph + i), (DES_cblock *)(ciph + i), &schedule, DES_ENCRYPT);
    }
}

void decrypt(DES_cblock key, char *ciph, int len) {
    DES_key_schedule schedule;
    DES_set_odd_parity(&key);
    DES_set_key_checked(&key, &schedule);

    // Process in 8-byte blocks
    for (int i = 0; i < len; i += 8) {
        DES_ecb_encrypt((DES_cblock *)(ciph + i), (DES_cblock *)(ciph + i), &schedule, DES_DECRYPT);
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <key>\n", argv[0]);
        return 1;
    }

    char *input_filename = "text.txt";
    char *encryption_filename = "encrypted.txt";
    char *decryption_filename = "decrypted.txt";

    DES_cblock key;

    // Convert the key from string to DES_cblock
    for(int i = 0; i < 8; i++) {
        sscanf(&(argv[1][i*2]), "%02x", (unsigned int*)&key[i]);
    }

    // Open the input file for reading
    FILE *input_file = fopen(input_filename, "r");
    if (!input_file) {
        perror("Failed to open input file");
        return 1;
    }

    // Open the encryption file for writing
    FILE *encryption_file = fopen(encryption_filename, "w");
    if (!encryption_file) {
        perror("Failed to open encryption file");
        return 1;
    }

    // Open the decryption file for writing
    FILE *decryption_file = fopen(decryption_filename, "w");
    if (!decryption_file) {
        perror("Failed to open decryption file");
        return 1;
    }

    // Get the input file size
    fseek(input_file, 0, SEEK_END);
    long filesize = ftell(input_file);
    fseek(input_file, 0, SEEK_SET);

    // Read the input file contents into a buffer
    char *buffer = malloc(filesize);
    if (fread(buffer, 1, filesize, input_file) != filesize) {
        perror("Failed to read input file");
        return 1;
    }

    // Encrypt the buffer
    encrypt(key, buffer, filesize);

    // Write the encrypted data to the encryption file
    if (fwrite(buffer, 1, filesize, encryption_file) != filesize) {
        perror("Failed to write encryption file");
        return 1;
    }

    // Print the encrypted data
    printf("Encrypted data: ");
    for (int i = 0; i < filesize; i++) {
        printf("%c", buffer[i]);
    }
    printf("\n");

    // Decrypt the buffer
    decrypt(key, buffer, filesize);

    // Write the decrypted data to the decryption file
    if (fwrite(buffer, 1, filesize, decryption_file) != filesize) {
        perror("Failed to write decryption file");
        return 1;
    }

    // Print the decrypted data
    printf("Decrypted data: ");
    for (int i = 0; i < filesize; i++) {
        printf("%c", buffer[i]);
    }
    printf("\n");

    // Clean up
    free(buffer);
    fclose(input_file);
    fclose(encryption_file);
    fclose(decryption_file);

    return 0;
}
