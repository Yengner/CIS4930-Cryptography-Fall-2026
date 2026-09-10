/*
 * CIS 4930 - Homework 1
 * Part 3: Challenge-Response Protocol - Alice
 *
 * Compile:
 * gcc alice.c -lssl -lcrypto -o alice
 *
 * Run:
 * ./alice Message1.txt SharedKey1.txt A_ctr.txt A_nonce.txt
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
    if (argc != 5) {
        printf("Usage: ./alice <message> <shared key> <counter> <nonce>\n");
        return 1;
    }

    int messageLen, keyLen, counterLen, nonceLen;

    unsigned char *message = Read_File(argv[1], &messageLen);
    unsigned char *key = Read_File(argv[2], &keyLen);
    unsigned char *counter = Read_File(argv[3], &counterLen);
    unsigned char *nonce = Read_File(argv[4], &nonceLen);

    if (message == NULL || key == NULL || counter == NULL || nonce == NULL) {
        printf("Alice could not read one of the input files.\n");
        free(message);
        free(key);
        free(counter);
        free(nonce);
        return 1;
    }

    if (messageLen != HASH_SIZE) {
        printf("Message must be exactly 32 bytes.\n");
        free(message);
        free(key);
        free(counter);
        free(nonce);
        return 1;
    }

    /* Step 2: Write the shared key in hex. */
    char *keyHex = (char*)malloc((keyLen * 2) + 1);
    Convert_to_Hex(keyHex, key, keyLen);
    Write_File("Key.txt", keyHex);

    /* Step 3: c = m XOR H(k || ctr) */
    int keyCounterLen = keyLen + counterLen;
    unsigned char *keyCounter = (unsigned char*)malloc(keyCounterLen);
    Concatenation(key, keyLen, counter, counterLen,
                  keyCounter, keyCounterLen);

    unsigned char keyCounterHash[HASH_SIZE];
    SHA256(keyCounter, keyCounterLen, keyCounterHash);

    unsigned char ciphertext[HASH_SIZE];

    for (int i = 0; i < HASH_SIZE; i++) {
        ciphertext[i] = message[i] ^ keyCounterHash[i];
    }

    /* Step 4: Write ciphertext in hex. */
    char ciphertextHex[(HASH_SIZE * 2) + 1];
    Convert_to_Hex(ciphertextHex, ciphertext, HASH_SIZE);
    Write_File("Ciphertext.txt", ciphertextHex);

    /* Step 5: sig = HMAC_k(ciphertext || nonce) */
    int signatureInputLen = HASH_SIZE + nonceLen;
    unsigned char *signatureInput =
        (unsigned char*)malloc(signatureInputLen);

    Concatenation(ciphertext, HASH_SIZE, nonce, nonceLen,
                  signatureInput, signatureInputLen);

    unsigned char signature[EVP_MAX_MD_SIZE];
    unsigned int signatureLen;

    HMAC_SHA256(key, keyLen, signatureInput, signatureInputLen,
                signature, &signatureLen);

    /* Step 6: Write signature in hex. */
    char signatureHex[(HASH_SIZE * 2) + 1];
    Convert_to_Hex(signatureHex, signature, HASH_SIZE);
    Write_File("Signature.txt", signatureHex);

    /*
     * Step 7: On the first Alice run, Response.txt does not exist yet.
     * Exit gracefully so Bob can run.
     */
    int responseHexLen;
    unsigned char *responseHex =
        Read_File("Response.txt", &responseHexLen);

    if (responseHex == NULL) {
        printf("Challenge created. Waiting for Bob's response.\n");

        free(message);
        free(key);
        free(counter);
        free(nonce);
        free(keyHex);
        free(keyCounter);
        free(signatureInput);
        return 0;
    }

    if (responseHexLen < HASH_SIZE * 2) {
        Write_File("Acknowledgment.txt", "Acknowledgment Failed");

        free(responseHex);
        free(message);
        free(key);
        free(counter);
        free(nonce);
        free(keyHex);
        free(keyCounter);
        free(signatureInput);
        return 0;
    }

    /* Step 8: response' = H(m || (ctr + 1) || (nonce + 1)) */
    unsigned long long counterNumber = strtoull((char*)counter, NULL, 10);
    unsigned long long nonceNumber = strtoull((char*)nonce, NULL, 10);

    char nextCounter[32];
    char nextNonce[32];

    sprintf(nextCounter, "%llu", counterNumber + 1);
    sprintf(nextNonce, "%llu", nonceNumber + 1);

    int nextCounterLen = strlen(nextCounter);
    int nextNonceLen = strlen(nextNonce);

    int messageCounterLen = messageLen + nextCounterLen;
    unsigned char *messageCounter =
        (unsigned char*)malloc(messageCounterLen);

    Concatenation(message, messageLen,
                  (unsigned char*)nextCounter, nextCounterLen,
                  messageCounter, messageCounterLen);

    int responseInputLen = messageCounterLen + nextNonceLen;
    unsigned char *responseInput =
        (unsigned char*)malloc(responseInputLen);

    Concatenation(messageCounter, messageCounterLen,
                  (unsigned char*)nextNonce, nextNonceLen,
                  responseInput, responseInputLen);

    unsigned char expectedResponse[HASH_SIZE];
    SHA256(responseInput, responseInputLen, expectedResponse);

    unsigned char bobResponse[HASH_SIZE];
    Convert_To_Uchar((char*)responseHex, bobResponse, HASH_SIZE);

    /* Step 9: Compare Bob's response with Alice's expected response. */
    if (memcmp(expectedResponse, bobResponse, HASH_SIZE) == 0) {
        Write_File("Acknowledgment.txt", "Acknowledgment Successful");

        /* Step 10: Update Alice's state only after a valid response. */
        Write_File(argv[3], nextCounter);
        Write_File(argv[4], nextNonce);
    }
    else {
        Write_File("Acknowledgment.txt", "Acknowledgment Failed");
    }

    free(responseHex);
    free(message);
    free(key);
    free(counter);
    free(nonce);
    free(keyHex);
    free(keyCounter);
    free(signatureInput);
    free(messageCounter);
    free(responseInput);

    return 0;
}
