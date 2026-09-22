#!/bin/bash

gcc prattack.c -o pra

for i in 1 2 3 4 5 6
do
    # echo "Testing case $i..."
    
    rm -f Exponent.txt

    # time the run
    start=$(date +%s.%N)

    # Run
    ./pra PRA$i.txt PRY$i.txt PRP$i.txt > pra$i.log

    # end timing the run
    end=$(date +%s.%N)
    runtime=$(echo "$end - $start" | bc)
    echo -n "Elapsed Time: $runtime seconds"

    # append the bit length of p/n
    num=$(<"PRP${i}.txt")
    bits=$(echo "obase=2; $num" | bc | awk '{print length}')
    echo " for modulo with bit length = $bits"

    # Verify outputs
    echo -n "Verifying outputs for test case $i..."
    
    for file in Exponent
    do
        if cmp -s "${file}.txt" "Correct${file}${i}.txt"; then
            echo "${file}${i} is correct."
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
