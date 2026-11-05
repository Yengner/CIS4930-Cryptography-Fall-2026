#!/bin/bash

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

BOLD="\e[1m"
NORMAL="\e[0m"

echo -e "${YELLOW}Starting ECC-BAF Test Vector Runner with File Comparison...${NC}"
echo "========================================================"

# Compile the code
echo -e "${YELLOW}Compiling the ECC-BAF code...${NC}"
read -p "Do you want to see the warnings? (y/n) " -n 1 -r
echo
if [[ $REPLY =~ ^[Yy]$ ]]; then
    gcc -O2 -Wall -Wextra -pedantic -o ecc_baf_sign_and_update ecc_baf_sign_and_update.c -I$(brew --prefix openssl@3)/include -L$(brew --prefix openssl@3)/lib -lcrypto
    gcc -O2 -Wall -Wextra -pedantic -o ecc_baf_verify ecc_baf_verify.c -I$(brew --prefix openssl@3)/include -L$(brew --prefix openssl@3)/lib -lcrypto
else
    echo -e "${YELLOW}Compiling without warnings...${NC}"
    gcc -O2 -Wall -Wextra -pedantic -o ecc_baf_sign_and_update ecc_baf_sign_and_update.c -I$(brew --prefix openssl@3)/include -L$(brew --prefix openssl@3)/lib -lcrypto -w
    gcc -O2 -Wall -Wextra -pedantic -o ecc_baf_verify ecc_baf_verify.c -I$(brew --prefix openssl@3)/include -L$(brew --prefix openssl@3)/lib -lcrypto -w
fi

# Check if executables were created
if [ ! -f "./ecc_baf_sign_and_update" ] || [ ! -f "./ecc_baf_verify" ]; then
    echo -e "${RED}Error: One or more ECC-BAF executables were not created.${NC}"
    exit 1
fi

# Function to normalize and compare files
compare_files() {
    local file1=$1
    local file2=$2
    local description=$3
    
    if [ ! -f "$file1" ]; then
        echo -e "${RED}Error: $file1 does not exist!${NC}"
        return 1
    fi
    
    if [ ! -f "$file2" ]; then
        echo -e "${RED}Error: $file2 does not exist!${NC}"
        return 1
    fi
    
    # Compare the files, ignoring whitespace and case
    if diff -i -b -B -w "$file1" "$file2" > /dev/null; then
        echo -e "${GREEN}$description match!${NC}"
        return 0
    else
        echo -e "${RED}$description do not match!${NC}"
        echo -e "${BLUE}Differences:${NC}"
        echo -e "${YELLOW}$file1:${NC}"
        head -n 5 "$file1"
        echo -e "${YELLOW}$file2:${NC}"
        head -n 5 "$file2"
        return 1
    fi
}

# Function to compare generated files with correct files
compare_with_correct_files() {
    local test_num=$1
    
    echo -e "${BLUE}Comparing generated files with correct files for Test Case $test_num...${NC}"
    echo "--------------------------------------------------------"
    
    # Compare private_state.txt
    if [ -f "CorrectPrivateVector${test_num}.txt" ]; then
        echo -e "${BLUE}Comparing updated private vector...${NC}"
        compare_files "private_state.txt" "CorrectPrivateVector${test_num}.txt" "Updated private vectors" || return 1
    fi

    # Compare signature.txt
    if [ -f "CorrectSignature${test_num}.txt" ]; then
        echo -e "${BLUE}Comparing signature...${NC}"
        compare_files "signature.txt" "CorrectSignature${test_num}.txt" "Signatures" || return 1
    fi

    # Compare reconstructed_k.txt
    if [ -f "CorrectReconstructedK${test_num}.txt" ]; then
        echo -e "${BLUE}Comparing reconstructed k values...${NC}"
        compare_files "reconstructed_k.txt" "CorrectReconstructedK${test_num}.txt" "Reconstructed k values" || return 1
    fi

    # Compare reconstructed_r.txt
    if [ -f "CorrectReconstructedR${test_num}.txt" ]; then
        echo -e "${BLUE}Comparing reconstructed r values...${NC}"
        compare_files "reconstructed_r.txt" "CorrectReconstructedR${test_num}.txt" "Reconstructed r values" || return 1
    fi

    # Compare verify_result.txt
    if [ -f "CorrectVerifyResult${test_num}.txt" ]; then
        echo -e "${BLUE}Comparing verification result...${NC}"
        compare_files "verify_result.txt" "CorrectVerifyResult${test_num}.txt" "Verification results" || return 1
    fi

    echo -e "${GREEN}All comparisons passed for Test Case $test_num!${NC}"
    echo "--------------------------------------------------------"
    return 0
}

# Function to run a test case
run_test_case() {
    local test_num=$1
    
    echo -e "${YELLOW}Running Test Case $test_num...${NC}"
    
    # Clean up any existing files
    rm -f PublicVector.txt PrivateVector.txt signature.txt verify_result.txt private_state.txt ECCParams.txt
    
    # Step 1: Setup existing key files
    echo -e "${BLUE}Step 1: Setting up existing key files for Test Case $test_num...${NC}"
    
    if [ ! -f "PublicVector$test_num.txt" ]; then
        echo -e "${RED}Error: PublicVector$test_num.txt does not exist!${NC}"
        return 1
    fi
    
    if [ ! -f "InitialPrivateState$test_num.txt" ]; then
        echo -e "${RED}Error: InitialPrivateState$test_num.txt does not exist!${NC}"
        return 1
    fi
    
    echo -e "${GREEN}Key files setup complete.${NC}"
    
    # Step 2: Signing
    echo -e "${BLUE}Step 2: Running Signing with Message$test_num.txt...${NC}"
    ./ecc_baf_sign_and_update "ECCParams$test_num.txt" "Message$test_num.txt" "InitialPrivateState$test_num.txt"
    sign_result=$?
    
    if [ $sign_result -ne 0 ] || [ ! -f signature.txt ]; then
        echo -e "${RED}Error: Signing failed or signature.txt was not created.${NC}"
        return 1
    fi
    
    # Step 3: Verification
    echo -e "${BLUE}Step 3: Running Verification...${NC}"
    ./ecc_baf_verify "ECCParams$test_num.txt" "Message$test_num.txt" "PublicVector$test_num.txt"
    verify_result=$?
    
    # Compare with correct files (fail immediately if any comparison fails)
    compare_with_correct_files $test_num || { 
        echo -e "${RED}Test Case $test_num FAILED due to file comparison mismatch!${NC}"
        return 1
    }

    # Check verification result
    if [ -f verify_result.txt ]; then
        verification=$(cat verify_result.txt)
        if [ "$verification" == "VALID" ]; then
            echo -e "${GREEN}Verification successful!${NC}"
            echo -e "${GREEN}Test Case $test_num PASSED${NC}"
            return 0
        else
            echo -e "${RED}Error: Verification failed - result was $verification${NC}"
            return 1
        fi
    else
        echo -e "${RED}Error: verify_result.txt was not created.${NC}"
        return 1
    fi
}

# Run test cases
run_test_case 1
echo "========================================================"

run_test_case 2
echo "========================================================"

run_test_case 3
echo "========================================================"

echo -e "${YELLOW}ECC-BAF Test Vector Runner Completed.${NC}"
