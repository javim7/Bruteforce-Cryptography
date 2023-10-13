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
        printf("Usage: %s <encryption_key>\n", argv[0]);
        return 1;
    }

    long encryption_key = strtol(argv[1], NULL, 10);

    // Read the text from the "text.txt" file
    FILE *input_file = fopen("text.txt", "r");
    if (!input_file) {
        perror("Unable to open input file");
        return 1;
    }

    fseek(input_file, 0, SEEK_END);
    long file_size = ftell(input_file);
    fseek(input_file, 0, SEEK_SET);

    char *text = (char *)malloc(file_size);
    fread(text, 1, file_size, input_file);
    fclose(input_file);

    // Encrypt the text
    encrypt(encryption_key, text, file_size);

    // Write the encrypted text to "encrypted.txt"
    FILE *encrypted_file = fopen("encrypted.txt", "wb");
    fwrite(text, 1, file_size, encrypted_file);
    fclose(encrypted_file);

    // Decrypt the text
    decrypt(encryption_key, text, file_size);

    // Write the decrypted text to "decrypted.txt"
    FILE *decrypted_file = fopen("decrypted.txt", "w");
    fwrite(text, 1, file_size, decrypted_file);
    fclose(decrypted_file);

    printf("Original Text:\n%s\n", text);
    printf("Encrypted Text:\n%s\n", text);  // Print encrypted text
    printf("Decrypted Text:\n%s\n", text);  // Print decrypted text

    if (tryKey(encryption_key, text, file_size)) {
        printf("Keyword \"%s\" found in the decrypted text.\n", search);
    } else {
        printf("Keyword \"%s\" not found in the decrypted text.\n", search);
    }

    free(text);

    return 0;
}