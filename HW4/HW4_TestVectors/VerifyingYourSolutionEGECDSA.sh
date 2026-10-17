gcc KeyGen.c -I$(brew --prefix openssl@3)/include -L$(brew --prefix openssl@3)/lib -lssl -lcrypto -o KeyGen
gcc alice.c -I$(brew --prefix openssl@3)/include -L$(brew --prefix openssl@3)/lib -lssl -lcrypto -o Alice
gcc bob.c -I$(brew --prefix openssl@3)/include -L$(brew --prefix openssl@3)/lib -lssl -lcrypto -o Bob

for i in 1 2
do
./KeyGen Seed$i.txt >> CA$i.log
./Alice  PK_Hex.txt alice_sk$i.txt Random_k_Hex$i.txt Message$i.txt  >> Alice$i.log
./Bob SK_Hex.txt alice_pk$i.txt alice_c.txt alice_d.txt alice_signature.txt >> Bob$i.log

if cmp -s "SK_Hex.txt" "correct_SK_Hex$i.txt"
then
   echo "SK_Hex$i is valid."
else
   echo "SK_Hex$i does not match!"
fi 

if cmp -s "PK_Hex.txt" "correct_PK_Hex$i.txt"
then
   echo "PK_Hex$i is valid."
else
   echo "PK_Hex$i does not match!"
fi

if cmp -s "alice_c.txt" "correct_alice_c$i.txt"
then
   echo "alice_c$i is valid."
else
   echo "alice_c$i does not match!"
fi

if cmp -s "alice_d.txt" "correct_alice_d$i.txt"
then
   echo "alice_d$i is valid."
else
   echo "alice_d$i does not match!"
fi

if cmp -s "Plaintext.txt" "correct_Plaintext$i.txt"
then
   echo "Plaintex$i is valid."
else
   echo "Plaintex$i does not match!"
fi

done

./Bob SK_Hex.txt alice_pk1.txt alice_c.txt alice_d.txt invalid_signature.txt >> Bob3.log

if [ $? -ne 0 ]; then
    echo "Bob correctly exited with invalid signature."
else
    echo "Bob did not exit with an invalid signature."
fi