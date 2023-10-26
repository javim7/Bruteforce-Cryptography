# Uso

##### Compilar y ejectutar programa original

Compilar:
```bash
mpicc -o naive bruteaive.c -lcrypto
```

Correr:
```bash
mpirun -np 4 ./naive
```

##### Compilar y ejectutar programa con parametros - paralelo

Compilar:
```bash
mpicc -o brute bruteforce0.c -lcrypto
```

Correr:
```bash
mpirun -np 4 ./brute <key>
```

##### Compilar y ejectutar programa con parametros - secuencial

Compilar:
```bash
gcc -o naive brutenaive.c -lcrypto
```

Correr:
```bash
./brute <key>
```