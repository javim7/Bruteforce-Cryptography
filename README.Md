# Uso

##### Compilar y ejectutar programa naive

Compilar:
```bash
mpicc -o naive bruteaive.c -lcrypto
```

Correr:
```bash
mpirun -np 4 ./naive
```

##### Compilar y ejectutar programa con parametros

Compilar:
```bash
mpicc -o brute bruteforce0.c -lcrypto
gcc -o naive brutenaive.c -lcrypto
```

Correr:
```bash
mpirun -np 4 ./brute <key>
./brute <key>
```