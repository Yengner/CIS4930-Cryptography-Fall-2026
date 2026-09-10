/*
 * CIS 4930 - Homework 1
 * Part 3: Challenge-Response Protocol - Bob
 *
 * Compile:
 * gcc bob.c -lssl -lcrypto -o bob
 *
 * Run:
 * ./bob Ciphertext.txt Signature.txt SharedKey1.txt B_ctr.txt B_nonce.txt
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <openssl/sha.h>
#include <openssl/evp.h>
#include <openssl/hmac.h>

#define HASH_SIZE 32

unsigned char* Read_File(char fileName[], int *fileLen);
void Write_File(char fileName[], char input[]);
void Convert_to_Hex(char output[], unsigned char input[], int inputlength);
void Convert_To_Uchar(char* input_hex, unsigned char output[], int output_len);
void Concatenation(unsigned char* in1, size_t in1len,
                   unsigned char* in2, size_t in2len,
                   unsigned char* out, size_t outlen);
unsigned char* HMAC_SHA256(const unsigned char *key, int key_n,
                           const unsigned char *data, size_t data_n,
                           unsigned char *result, unsigned int *result_n);

unsigned char* Read_File(char fileName[], int *fileLen)
{
    FILE *pFile = fopen(fileName, "rb");

    if (pFile == NULL) {
        printf("Error opening file: %s\n", fileName);
        return NULL;
    }

    fseek(pFile, 0L, SEEK_END);
    int size = (int)ftell(pFile);
    fseek(pFile, 0L, SEEK_SET);

    unsigned char *output = (unsigned char*)malloc(size + 1);

    if (output == NULL) {
        fclose(pFile);
        return NULL;
    }

    fread(output, 1, size, pFile);
    output[size] = '\0';
    fclose(pFile);

    *fileLen = size;
    return output;
}

void Write_File(char fileName[], char input[])
{
    FILE *pFile = fopen(fileName, "w");

    if (pFile == NULL) {
        printf("Error opening file: %s\n", fileName);
        exit(1);
    }

    fputs(input, pFile);
    fclose(pFile);
}

void Convert_to_Hex(char output[], unsigned char input[], int inputlength)
{
    for (int i = 0; i < inputlength; i++) {
        sprintf(&output[2 * i], "%02x", input[i]);
    }

    output[inputlength * 2] = '\0';
}

void Convert_To_Uchar(char* input_hex, unsigned char output[], int output_len)
{
    for (int i = 0; i < output_len; i++) {
        char tmp[3];
        tmp[0] = input_hex[2 * i];
        tmp[1] = input_hex[(2 * i) + 1];
        tmp[2] = '\0';

        output[i] = (unsigned char)strtol(tmp, NULL, 16);
    }
}

void Concatenation(unsigned char* in1, size_t in1len,
                   unsigned char* in2, size_t in2len,
                   unsigned char* out, size_t outlen)
{
    if (in1len + in2len > outlen) {
        printf("Concatenation error.\n");
        exit(1);
    }

    memcpy(out, in1, in1len);
    memcpy(out + in1len, in2, in2len);
}

unsigned char* HMAC_SHA256(const unsigned char *key, int key_n,
                           const unsigned char *data, size_t data_n,
                           unsigned char *result, unsigned int *result_n)
{
    return HMAC(EVP_sha256(), key, key_n, data, data_n, result, result_n);
}

int main(int argc, char *argv[])
{
    if (argc != 6) {
        printf("Usage: ./bob <ciphertext> <signature> <shared key> <counter> <nonce>\n");
        return 1;
    }

    int ciphertextHexLen, signatureHexLen;
    int keyLen, counterLen, nonceLen;

    unsigned char *ciphertextHex = Read_File(argv[1], &ciphertextHexLen);
    unsigned char *signatureHex = Read_File(argv[2], &signatureHexLen);
    unsigned char *key = Read_File(argv[3], &keyLen);
    unsigned char *counter = Read_File(argv[4], &counterLen);
    unsigned char *nonce = Read_File(argv[5], &nonceLen);

    if (ciphertextHex == NULL || signatureHex == NULL ||
        key == NULL || counter == NULL || nonce == NULL) {
        printf("Bob could not read one of the input files.\n");
        free(ciphertextHex);
        free(signatureHex);
        free(key);
        free(counter);
        free(nonce);
        return 1;
    }

    if (ciphertextHexLen < HASH_SIZE * 2 ||
        signatureHexLen < HASH_SIZE * 2) {
        printf("Ciphertext or signature is not the correct size.\n");
        free(ciphertextHex);
        free(signatureHex);
        free(key);
        free(counter);
        free(nonce);
        return 1;
    }

    /* Convert Alice's hex ciphertext and signature back to bytes. */
    unsigned char ciphertext[HASH_SIZE];
    unsigned char aliceSignature[HASH_SIZE];

    Convert_To_Uchar((char*)ciphertextHex, ciphertext, HASH_SIZE);
    Convert_To_Uchar((char*)signatureHex, aliceSignature, HASH_SIZE);

    /* Step 2: Compute sig' = HMAC_k(ciphertext || nonce). */
    int signatureInputLen = HASH_SIZE + nonceLen;
    unsigned char *signatureInput =
        (unsigned char*)malloc(signatureInputLen);

    Concatenation(ciphertext, HASH_SIZE, nonce, nonceLen,
                  signatureInput, signatureInputLen);

    unsigned char expectedSignature[EVP_MAX_MD_SIZE];
    unsigned int expectedSignatureLen;

    HMAC_SHA256(key, keyLen, signatureInput, signatureInputLen,
                expectedSignature, &expectedSignatureLen);

    /* Step 3: Stop if Alice's signature is not valid. */
    if (memcmp(aliceSignature, expectedSignature, HASH_SIZE) != 0) {
        printf("Signature verification failed.\n");

        free(ciphertextHex);
        free(signatureHex);
        free(key);
        free(counter);
        free(nonce);
        free(signatureInput);
        return 1;
    }

    /* Step 4: m = c XOR H(k || ctr) */
    int keyCounterLen = keyLen + counterLen;
    unsigned char *keyCounter = (unsigned char*)malloc(keyCounterLen);

    Concatenation(key, keyLen, counter, counterLen,
                  keyCounter, keyCounterLen);

    unsigned char keyCounterHash[HASH_SIZE];
    SHA256(keyCounter, keyCounterLen, keyCounterHash);

    unsigned char message[HASH_SIZE];

    for (int i = 0; i < HASH_SIZE; i++) {
        message[i] = ciphertext[i] ^ keyCounterHash[i];
    }

    /* Step 5: response = H(m || (ctr + 1) || (nonce + 1)) */
    unsigned long long counterNumber = strtoull((char*)counter, NULL, 10);
    unsigned long long nonceNumber = strtoull((char*)nonce, NULL, 10);

    char nextCounter[32];
    char nextNonce[32];

    sprintf(nextCounter, "%llu", counterNumber + 1);
    sprintf(nextNonce, "%llu", nonceNumber + 1);

    int nextCounterLen = strlen(nextCounter);
    int nextNonceLen = strlen(nextNonce);

    int messageCounterLen = HASH_SIZE + nextCounterLen;
    unsigned char *messageCounter =
        (unsigned char*)malloc(messageCounterLen);

    Concatenation(message, HASH_SIZE,
                  (unsigned char*)nextCounter, nextCounterLen,
                  messageCounter, messageCounterLen);

    int responseInputLen = messageCounterLen + nextNonceLen;
    unsigned char *responseInput =
        (unsigned char*)malloc(responseInputLen);

    Concatenation(messageCounter, messageCounterLen,
                  (unsigned char*)nextNonce, nextNonceLen,
                  responseInput, responseInputLen);

    unsigned char response[HASH_SIZE];
    SHA256(responseInput, responseInputLen, response);

    /* Step 6: Write Bob's response in hex. */
    char responseHex[(HASH_SIZE * 2) + 1];
    Convert_to_Hex(responseHex, response, HASH_SIZE);
    Write_File("Response.txt", responseHex);

    /* Step 7: Update Bob's counter and nonce. */
    Write_File(argv[4], nextCounter);
    Write_File(argv[5], nextNonce);

    free(ciphertextHex);
    free(signatureHex);
    free(key);
    free(counter);
    free(nonce);
    free(signatureInput);
    free(keyCounter);
    free(messageCounter);
    free(responseInput);

    return 0;
}
