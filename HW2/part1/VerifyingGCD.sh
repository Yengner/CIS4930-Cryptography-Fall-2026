#!/bin/bash

gcc gcd.c -o gcd

num1=("922337203685477580" "982134798129437" "202745771679908081")
num2=("1383505805528216320" "1892347929834792948" "119945553277204427")
correct=("20" "1" "345108161")

for i in ${!num1[@]};
do
    echo "Testing case $i..."

    # Run
    var="$(./gcd ${num1[$i]} ${num2[$i]})"

    # Verify outputs
    if [[ $var == ${correct[$i]} ]]; then
        echo "$var is Correct"
    else
        echo "$var is Not Correct - ALERT"
    fi
    

done
