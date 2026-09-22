#!/bin/bash

gcc eea_gmp.c -I$(brew --prefix gmp)/include -L$(brew --prefix gmp)/lib -lgmp -o eea_gmp

for i in 1 2 3
do
    echo "Testing case $i..."
    
    rm -f Result.txt

    # Run
    ./eea_gmp Number$i.txt Modulo$i.txt > eea$i.log

    # Verify outputs
    echo "Verifying outputs for test case $i..."
    
    for file in Result
    do
        if cmp -s "${file}.txt" "Correct${file}${i}.txt"; then
            echo "${file}${i} is valid."
        else
            echo "${file}${i} does not match!"
            echo "Differences between Correct${file}${i}.txt and ${file}.txt:"
            # Using hexdump to show differences in hex format
            echo "Expected:"
            hexdump -C "Correct${file}${i}.txt"
            echo "Got:"
            hexdump -C "${file}.txt"
            echo "---"
        fi
        grep "mpz_powm" eea_gmp.c 
    done

done