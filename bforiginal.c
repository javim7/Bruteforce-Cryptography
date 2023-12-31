/**
 * Programa que utiliza fuerza bruta para desencriptar un texto cifrado con DES.
 * Codigo proporcionado por el catedratico (paralelo).
*/


// importamos las librerias necesarias
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <unistd.h>
#include <openssl/des.h>

void decrypt(long key, char *ciph, int len){
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

void encrypt(long key, char *ciph, int len){
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

// definimos la cadena a buscar
char search[] = " the ";
// definimos la funcion que prueba la llave
int tryKey(long key, char *ciph, int len){
  char temp[len+1];
  memcpy(temp, ciph, len);
  temp[len]=0;
  decrypt(key, temp, len);
  return strstr((char *)temp, search) != NULL;
}

unsigned char cipher[] = {108, 245, 65, 63, 125, 200, 150, 66, 17, 170, 207, 170, 34, 31, 70, 215, 0};
int main(int argc, char *argv[]){ //char **argv
  int N, id;
  long upper = (1L <<56); //upper bound DES keys 2^56
  long mylower, myupper;
  MPI_Status st;
  MPI_Request req;
  int flag;
  int ciphlen = strlen(cipher);
  MPI_Comm comm = MPI_COMM_WORLD;

  MPI_Init(NULL, NULL);
  MPI_Comm_size(comm, &N);
  MPI_Comm_rank(comm, &id);

  long range_per_node = upper / N;
  mylower = range_per_node * id;
  myupper = range_per_node * (id+1) -1;
  if(id == N-1){
    //compensar residuo
    myupper = upper;
  }

  long found = 0;
  int ready = 0;

  MPI_Irecv(&found, 1, MPI_LONG, MPI_ANY_SOURCE, MPI_ANY_TAG, comm, &req);

  for(long i = mylower; i<myupper; ++i){
    MPI_Test(&req, &ready, MPI_STATUS_IGNORE);
    if(ready)
      break;  //ya encontraron, salir

    if(tryKey(i, (char *)cipher, ciphlen)){
      found = i;
      for(int node=0; node<N; node++){
        MPI_Send(&found, 1, MPI_LONG, node, 0, MPI_COMM_WORLD);
      }
      break;
    }
  }

  if(id==0){
    MPI_Wait(&req, &st);
    decrypt(found, (char *)cipher, ciphlen);
    printf("%li %s\n", found, cipher);
  }

  MPI_Finalize();
}

