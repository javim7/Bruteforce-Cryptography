#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <openssl/des.h>

void decrypt(long key, char *ciph, int len) {
    DES_cblock des_key;
    DES_key_schedule key_schedule;

    for (int i = 0; i < 8; ++i) {
        des_key[i] = (key >> (i * 8)) & 0xFF;
    }

    DES_set_odd_parity(&des_key);
    DES_set_key_checked(&des_key, &key_schedule);

    for (int i = 0; i < len; i += 8) {
        DES_ecb_encrypt((DES_cblock *)(ciph + i), (DES_cblock *)(ciph + i), &key_schedule, DES_DECRYPT);
    }
}

void encrypt(long key, char *ciph, int len) {
    DES_cblock des_key;
    DES_key_schedule key_schedule;

    for (int i = 0; i < 8; ++i) {
        des_key[i] = (key >> (i * 8)) & 0xFF;
    }

    DES_set_odd_parity(&des_key);
    DES_set_key_checked(&des_key, &key_schedule);

    for (int i = 0; i < len; i += 8) {
        DES_ecb_encrypt((DES_cblock *)(ciph + i), (DES_cblock *)(ciph + i), &key_schedule, DES_ENCRYPT);
    }
}

char search[] = "Mundo";
int tryKey(long key, char *ciph, int len) {
    char temp[len + 1];
    memcpy(temp, ciph, len);
    temp[len] = 0;
    decrypt(key, temp, len);
    return strstr((char *)temp, search) != NULL;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <key>\n", argv[0]);
        return 1;
    }

    char *input_filename = "text.txt";
    char *encryption_filename = "encrypted.txt";
    char *decryption_filename = "decrypted.txt";

    long key = strtol(argv[1], NULL, 10);

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

    // Print the original data
    printf("\nOriginal data : ");
    for (int i = 0; i < filesize; i++) {
        printf("%c", buffer[i]);
    }
    printf("\n");

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

    // // Check for the keyword in the decrypted text
    // if (tryKey(key, buffer, filesize)) {
    //     printf("Keyword \"%s\" found in the decrypted text.\n", search);
    // } else {
    //     printf("Keyword \"%s\" not found in the decrypted text.\n", search);
    // }

    // Clean up
    free(buffer);
    fclose(input_file);
    fclose(encryption_file);
    fclose(decryption_file);

    return 0;
}
