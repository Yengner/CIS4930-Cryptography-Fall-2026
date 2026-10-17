#!/bin/bash
set -e

gcc KeyGen.c -I$(brew --prefix openssl@3)/include -L$(brew --prefix openssl@3)/lib -lssl -lcrypto -o KeyGen
gcc alice.c -I$(brew --prefix openssl@3)/include -L$(brew --prefix openssl@3)/lib -lssl -lcrypto -o Alice
gcc bob.c -I$(brew --prefix openssl@3)/include -L$(brew --prefix openssl@3)/lib -lssl -lcrypto -o Bob

for i in 1 2
do
    echo "Testing ElGamal case $i..."
    ./KeyGen Seed$i.txt
    ./Alice PK_Hex.txt Random_k_Hex$i.txt Message$i.txt
    ./Bob SK_Hex.txt alice_c.txt alice_d.txt

    for name in SK_Hex PK_Hex alice_c alice_d Plaintext
    do
        if cmp -s "${name}.txt" "correct_${name}${i}.txt"; then
            echo "${name}${i} is valid."
        else
            echo "${name}${i} does not match!"
            exit 1
        fi
    done
done
