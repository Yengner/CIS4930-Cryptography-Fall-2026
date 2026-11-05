#!/bin/bash

# An expansion of VerifyingPRA.sh that also uses a small awk script to linear regression fit the runtimes to a square root curve.
# Gives you the R^2 value (where = 1.0 is perfectly matched and = 0.0 is utterly useless).  
# On my laptop, R^2 value is typically between 0.95 and 0.99, but the randomness of Pollard-Rho means that the final test can sometimes finish very quickly, ending in 0.2 seconds rather than the expected 3-9 seconds, which gave an R^2 score of 0.5031.  So it is recommended to run the Verifying Curve script again if you get a low R^2 score just to be sure. 

gcc prattack.c -I$(brew --prefix gmp)/include -o pra

tmp_data="runtime_vs_sqrt_value.txt"
> "$tmp_data"

for i in 1 2 3 4 5 6
do
    echo "Running test case $i..."

    rm -f Exponent.txt

    
    start=$(date +%s%N)
    ./pra PRA$i.txt PRY$i.txt PRP$i.txt > pra$i.log
    end=$(date +%s%N)


    # getting occasional negative runtimes in case 5, switching to awk instead of bc
    runtime=$(awk "BEGIN {print ($end - $start)/1000000000}")
    echo -n "Elapsed Time: $runtime seconds"

    num=$(<"PRP${i}.txt")
    sqrt_val=$(echo "scale=10; sqrt($num)" | bc -l)
    bits=$(echo "obase=2; $num" | bc | awk '{print length}')
    echo " for p = $num with bit length $bits"

    echo "$sqrt_val $runtime" >> "$tmp_data"

    echo -n "Verifying outputs for test case $i..."
    for file in Exponent
    do
        if cmp -s "${file}.txt" "Correct${file}${i}.txt"; then
            echo "${file}${i} is correct."
        else
            echo "${file}${i} does not match!"
            echo "Differences between Correct${file}${i}.txt and ${file}.txt:"
            echo "Expected:"
            hexdump -C "Correct${file}${i}.txt"
            echo "Got:"
            hexdump -C "${file}.txt"
            echo "---"
        fi
    done
done

# Analyze fit to sqrt(n)
awk '
{
    x = $1
    y = $2
    sx += x
    sy += y
    sxx += x*x
    sxy += x*y
    syy += y*y
    n++
}
END {
    denom = n * sxx - sx * sx
    a = (n * sxy - sx * sy) / denom
    b = (sy * sxx - sx * sxy) / denom

    ss_tot = syy - (sy * sy) / n
    ss_res = 0

    while ((getline < ARGV[1]) > 0) {
        x = $1
        y = $2
        y_fit = a * x + b
        ss_res += (y - y_fit)^2
    }

    r2 = 1 - ss_res / ss_tot
    #printf "\nFit: runtime = %.4f * function + %.4f\n", a, b
    printf "R^2 Score = %.4f\n", r2
}' "$tmp_data" "$tmp_data"

