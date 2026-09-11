#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <openssl/sha.h>
#include <openssl/evp.h>
#include <openssl/hmac.h>

#define HASH_SIZE 32

unsigned char *Read_File(char fileName[], int *fileLen);
void Write_File(char fileName[], char input[]);
void Convert_to_Hex(char output[], unsigned char input[], int inputlength);
void Convert_To_Uchar(char *input_hex, unsigned char output[], int output_len);
void Concatenation32(unsigned char a[32], unsigned char b[32], unsigned char c[64]);
unsigned char *HMAC_SHA256(const unsigned char *key, int key_n,
                           const unsigned char *data, size_t data_n,
                           unsigned char *result, unsigned int *result_n);
unsigned char *Read_File(char fileName[], int *fileLen)
{
    FILE *pFile;
    pFile = fopen(fileName, "r");
    if (pFile == NULL)
    {
        printf("Error opening file.\n");
        exit(0);
    }
    fseek(pFile, 0L, SEEK_END);
    int temp_size = ftell(pFile) + 1;
    fseek(pFile, 0L, SEEK_SET);
    unsigned char *output = (unsigned char *)malloc(temp_size);
    fgets(output, temp_size, pFile);
    fclose(pFile);

    *fileLen = temp_size - 1;
    return output;
}

void Write_File(char fileName[], char input[])
{
    FILE *pFile;
    pFile = fopen(fileName, "w");
    if (pFile == NULL)
    {
        printf("Error opening file. \n");
        exit(0);
    }
    fputs(input, pFile);
    fclose(pFile);
}

void Convert_to_Hex(char output[], unsigned char input[], int inputlength)
{
    for (int i = 0; i < inputlength; i++)
    {
        sprintf(&output[2 * i], "%02x", input[i]);
    }
}

void Convert_To_Uchar(char *input_hex, unsigned char output[], int output_len)
{
    for (int i = 0; i < output_len; i++)
    {
        unsigned char tmp[2];
        tmp[0] = input_hex[2 * i];
        tmp[1] = input_hex[2 * i + 1];
        output[i] = (unsigned char)strtol(tmp, NULL, 16);
    }
}

void Concatenation32(unsigned char a[32], unsigned char b[32], unsigned char c[64])
{
    memcpy(c, a, 32);
    memcpy(c + 32, b, 32);
}

unsigned char *HMAC_SHA256(const unsigned char *key, int key_n,
                           const unsigned char *data, size_t data_n,
                           unsigned char *result, unsigned int *result_n)
{
    return HMAC(EVP_sha256(), key, key_n, data, data_n, result, result_n);
}

int main(int argc, char *argv[])
{

    int ciphertextLen, signatureLen;
    int keyLen, counterLen, nonceLen;

    unsigned char *ciphertextHex = Read_File(argv[1], &ciphertextLen);
    unsigned char *signatureHex = Read_File(argv[2], &signatureLen);
    unsigned char *key = Read_File(argv[3], &keyLen);
    unsigned char *counter = Read_File(argv[4], &counterLen);
    unsigned char *nonce = Read_File(argv[5], &nonceLen);

    // Alice wrote these in hex, change them back to bytes
    unsigned char ciphertext[HASH_SIZE];
    unsigned char aliceSignature[HASH_SIZE];

    Convert_To_Uchar((char *)ciphertextHex, ciphertext, HASH_SIZE);
    Convert_To_Uchar((char *)signatureHex, aliceSignature, HASH_SIZE);

    // check Alice's signature
    // HMAC(key, ciphertext || nonce)
    int sigInputLen = HASH_SIZE + nonceLen;
    unsigned char *sigInput = malloc(sigInputLen);

    memcpy(sigInput, ciphertext, HASH_SIZE);
    memcpy(sigInput + HASH_SIZE, nonce, nonceLen);

    unsigned char expectedSignature[HASH_SIZE];
    unsigned int expectedSignatureLen;

    HMAC_SHA256(key, keyLen, sigInput, sigInputLen, expectedSignature, &expectedSignatureLen);

    if (memcmp(aliceSignature, expectedSignature, HASH_SIZE) != 0)
    {
        printf("Signature verification failed.\n");

        free(ciphertextHex);
        free(signatureHex);
        free(key);
        free(counter);
        free(nonce);
        free(sigInput);

        return 1;
    }

    // message = ciphertext XOR SHA256(key || counter)
    int keyCounterLen = keyLen + counterLen;
    unsigned char *keyCounter = malloc(keyCounterLen);

    memcpy(keyCounter, key, keyLen);
    memcpy(keyCounter + keyLen, counter, counterLen);

    unsigned char keyCounterHash[HASH_SIZE];

    SHA256(keyCounter, keyCounterLen, keyCounterHash);

    unsigned char message[HASH_SIZE];

    for (int i = 0; i < HASH_SIZE; i++)
    {
        message[i] = ciphertext[i] ^ keyCounterHash[i];
    }

    // increase counter and nonce by 1
    unsigned long long counterNumber = strtoull((char *)counter, NULL, 10);

    unsigned long long nonceNumber = strtoull((char *)nonce, NULL, 10);

    char nextCounter[32];
    char nextNonce[32];

    // https://en.cppreference.com/c/io/printf
    //https://man.openbsd.org/snprintf
    sprintf(nextCounter, "%llu", counterNumber + 1);
    sprintf(nextNonce, "%llu", nonceNumber + 1);

    int nextCounterLen = strlen(nextCounter);
    int nextNonceLen = strlen(nextNonce);

    // response = SHA256(message || new counter || new nonce)
    int responseInputLen = HASH_SIZE + nextCounterLen + nextNonceLen;

    unsigned char *responseInput = malloc(responseInputLen);

    memcpy(responseInput, message, HASH_SIZE);

    memcpy(responseInput + HASH_SIZE, nextCounter, nextCounterLen);

    memcpy(responseInput + HASH_SIZE + nextCounterLen, nextNonce,nextNonceLen);

    unsigned char response[HASH_SIZE];

    SHA256(responseInput, responseInputLen, response);

    // write response in hex
    char responseHex[(HASH_SIZE * 2) + 1];

    Convert_to_Hex(responseHex, response, HASH_SIZE);
    responseHex[HASH_SIZE * 2] = '\0';

    Write_File("Response.txt", responseHex);

    // update Bob's counter and nonce
    Write_File(argv[4], nextCounter);
    Write_File(argv[5], nextNonce);

    free(ciphertextHex);
    free(signatureHex);
    free(key);
    free(counter);
    free(nonce);
    free(sigInput);
    free(keyCounter);
    free(responseInput);

    return 0;
}