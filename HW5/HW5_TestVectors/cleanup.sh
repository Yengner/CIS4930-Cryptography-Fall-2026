#!/bin/bash

# Cleanup script for ECC-BAF
# Removes all generated files and outputs

echo "Cleaning up ECC-BAF generated files..."

# Remove executables
rm -f ecc_baf_sign_and_update
rm -f ecc_baf_verify

# Remove signature files
rm -f signature.txt 

# Remove verification result files
rm -f verify_result.txt

# Remove private state files
rm -f private_state.txt

# Remove reconstructed files
rm -f reconstructed_k.txt
rm -f reconstructed_r.txt


# Remove any other temporary files
rm -f *.o
rm -f core
rm -f *~
echo "Files preserved:"
echo "  ===================================== "
echo "  - ecc_baf_sign_and_update (executable)"
echo "  - ecc_baf_verify (executable)"
echo "  ===================================== "
echo "  - CorrectPublicVector.txt"
echo "  - CorrectPrivateVector.txt"
echo "  - CorrectReconstructedK.txt"
echo "  - CorrectReconstructedR.txt"
echo "  - CorrectSignature.txt"
echo "  - CorrectVerifyResult.txt"
echo "  ===================================== "
echo "  - EccParams.txt"
echo "  - Message.txt"
echo "  - PublicVector.txt"
echo "  ===================================== "

echo "Cleanup completed!"