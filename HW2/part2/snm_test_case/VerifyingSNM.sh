#!/bin/bash

gcc snm_gmp.c -lgmp -o snm_gmp

for i in 1 2 3
do
    echo "Testing case $i..."
    
    rm -f Result.txt

    # Run
    ./snm_gmp Base$i.txt Exponent$i.txt Modulo$i.txt > snm$i.log

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
    done

done