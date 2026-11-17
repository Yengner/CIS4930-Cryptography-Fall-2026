
gcc alicexor.c -I"${OQS_PREFIX:-$HOME/.local/opt/liboqs-0.8.0}/include" -L"${OQS_PREFIX:-$HOME/.local/opt/liboqs-0.8.0}/lib" -I"$(brew --prefix openssl@3)/include" -L"$(brew --prefix openssl@3)/lib" -lssl -lcrypto -loqs -o alice
gcc bobxor.c -I"${OQS_PREFIX:-$HOME/.local/opt/liboqs-0.8.0}/include" -L"${OQS_PREFIX:-$HOME/.local/opt/liboqs-0.8.0}/lib" -I"$(brew --prefix openssl@3)/include" -L"$(brew --prefix openssl@3)/lib" -lssl -lcrypto -loqs -o bob

./bob bob_dilithium_private.txt alice_kyber_public.txt k1_ab.txt >> Bob1.log
./alice bob_dilithium_public.txt alice_kyber_private.txt bob_dilithium_signature.txt bob_kyber_ciphertext.txt k1_ab.txt >> Alice1.log

echo "[Testing] Alice and Bob K2"

if cmp -s "bob_k2.txt" "alice_k2.txt"
then
   echo "K2 is valid."
else
   echo "K2 does not match!"
fi 

if cmp -s "bob_final_key.txt" "alice_final_key.txt"
then
    echo "K1 XOR K2 is valid"
else
    echo "K1 XOR K2 does not match!"
fi

echo "[Passed] Alice and Bob final key (K1 XOR K2) match"

echo "[Testing] Invalid signature"

./alice bob_dilithium_public.txt alice_kyber_private.txt bob_invalid_signature.txt bob_kyber_ciphertext.txt k1_ab.txt >> Alice2.log

if [ $? -ne 0 ]; then
    echo "Alice correctly exited with invalid signature."
else
    echo "Alice did not exit with an invalid signature."
fi

echo "[Passed] Alice exited with invalid signature"

echo "Done"