#!/bin/bash

gcc alice.c -lssl -lcrypto -o alice
gcc bob.c -lssl -lcrypto -o bob

for i in 1 2 3 4 5
do
    echo "Testing case $i..."
    
	printf 1 > A_ctr.txt
	printf 55 > A_nonce.txt
	printf 1 > B_ctr.txt
	printf 55 > B_nonce.txt
	rm -f Ciphertext.txt
	rm -f Signature.txt
	rm -f Response.txt
	rm -f Acknowledgment.txt


    if [ $i == 4 ]; then
	    printf 90 > B_nonce.txt
    fi
    if [ $i == 5 ]; then
	    printf 2 > B_ctr.txt
    fi

    # Run Alice first
    ./alice Message$i.txt SharedKey$i.txt A_ctr.txt A_nonce.txt > alice$i.log
    
    # Wait a bit to ensure files are written
    sleep 2
    
    # Run Bob
    ./bob Ciphertext.txt Signature.txt SharedKey$i.txt B_ctr.txt B_nonce.txt > bob$i.log
    
    # Wait for Bob to complete
    sleep 2
    
    # Run Alice again for verification
    ./alice Message$i.txt SharedKey$i.txt A_ctr.txt A_nonce.txt > alice$i.log
    
    # Wait for completion
    sleep 2

    # Verify outputs
    echo "Verifying outputs for test case $i..."
    
    for file in Key Ciphertext Signature Response Acknowledgment A_ctr A_nonce B_ctr B_nonce
    do
        if ! test -f "${file}.txt"; then
            if ! test -f "Correct${file}${i}.txt"; then
                echo "${file}${i} is correctly missing."
            fi
        elif cmp -s "${file}.txt" "Correct${file}${i}.txt"; then
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
