#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <mpi.h>
#include <openssl/des.h>

void decrypt(long key, char *ciph, int len, DES_cblock iv) {
    DES_key_schedule schedule;
    DES_set_key_unchecked(&key, &schedule);

    DES_ncbc_encrypt((const unsigned char *)ciph, (unsigned char *)ciph, len, &schedule, &iv, DES_DECRYPT);
}

void encrypt(long key, char *ciph, int len, DES_cblock iv) {
    DES_key_schedule schedule;
    DES_set_key_unchecked(&key, &schedule);

    DES_ncbc_encrypt((const unsigned char *)ciph, (unsigned char *)ciph, len, &schedule, &iv, DES_ENCRYPT);
}

char search[] = " the ";
int tryKey(long key, char *ciph, int len, DES_cblock iv) {
    char temp[len + 1];
    memcpy(temp, ciph, len);
    temp[len] = 0;
    decrypt(key, temp, len, iv);
    return strstr((char *)temp, search) != NULL;
}

unsigned char cipher[] = {108, 245, 65, 63, 125, 200, 150, 66, 17, 170, 207, 170, 34, 31, 70, 215, 0};
int main(int argc, char *argv[]) {
    int N, id;
    long upper = (1L << 56); // Upper bound DES keys 2^56
    long mylower, myupper;
    MPI_Status st;
    MPI_Request req;
    int flag;
    int ciphlen = strlen((char *)cipher);
    MPI_Comm comm = MPI_COMM_WORLD;

    MPI_Init(NULL, NULL);
    MPI_Comm_size(comm, &N);
    MPI_Comm_rank(comm, &id);

    DES_cblock iv = {0, 0, 0, 0, 0, 0, 0, 0}; // Initial vector, you may need to set this differently

    long range_per_node = upper / N;
    mylower = range_per_node * id;
    myupper = range_per_node * (id + 1) - 1;
    if (id == N - 1) {
        // Compensar residuo
        myupper = upper;
    }

    long found = 0;
    int ready = 0;

    MPI_Irecv(&found, 1, MPI_LONG, MPI_ANY_SOURCE, MPI_ANY_TAG, comm, &req);

    for (long i = mylower; i < myupper; ++i) {
        MPI_Test(&req, &ready, MPI_STATUS_IGNORE);
        if (ready)
            break;  // Ya encontraron, salir

        if (tryKey(i, (char *)cipher, ciphlen, iv)) {
            found = i;
            for (int node = 0; node < N; node++) {
                MPI_Send(&found, 1, MPI_LONG, node, 0, MPI_COMM_WORLD);
            }
            break;
        }
    }

    if (id == 0) {
        MPI_Wait(&req, &st);
        decrypt(found, (char *)cipher, ciphlen, iv);
        printf("%li %s\n", found, (char *)cipher);
    }

    MPI_Finalize();
}