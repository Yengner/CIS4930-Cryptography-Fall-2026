/*
 * CIS 4930 - Homework 1
 * Part 2: HMAC with BLAKE2s
 *
 * Compile:
 * gcc hmac.c -lssl -lcrypto -o hmac
 *
 * Run:
 * ./hmac Message1.txt SharedKey1.txt
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <openssl/evp.h>

#define BLOCK_SIZE 64
#define HASH_SIZE 32

unsigned char* Read_File(char fileName[], int *fileLen);
void Write_File(char fileName[], char input[]);
void Convert_to_Hex(char output[], unsigned char input[], int inputlength);
unsigned char* Hash_Blake2s(unsigned char* input, unsigned long inputlen);
void Concatenation(unsigned char* in1, size_t in1len,
                   unsigned char* in2, size_t in2len,
                   unsigned char* out, size_t outlen);

/* Read all bytes from a file. */
unsigned char* Read_File(char fileName[], int *fileLen)
{
    FILE *pFile = fopen(fileName, "rb");

    if (pFile == NULL) {
        printf("Error opening file: %s\n", fileName);
        exit(1);
    }

    fseek(pFile, 0L, SEEK_END);
    int size = (int)ftell(pFile);
    fseek(pFile, 0L, SEEK_SET);

    unsigned char *output = (unsigned char*)malloc(size + 1);

    if (output == NULL) {
        printf("Memory allocation error.\n");
        fclose(pFile);
        exit(1);
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

unsigned char* Hash_Blake2s(unsigned char* input, unsigned long inputlen)
{
    unsigned char *hash_result = (unsigned char*)malloc(EVP_MAX_MD_SIZE);
    unsigned int hash_len;

    const EVP_MD *md = EVP_get_digestbyname("BLAKE2s256");
    EVP_MD_CTX *mdctx = EVP_MD_CTX_new();

    if (hash_result == NULL || md == NULL || mdctx == NULL) {
        printf("BLAKE2s initialization error.\n");
        free(hash_result);
        EVP_MD_CTX_free(mdctx);
        exit(1);
    }

    EVP_DigestInit_ex(mdctx, md, NULL);
    EVP_DigestUpdate(mdctx, input, inputlen);
    EVP_DigestFinal_ex(mdctx, hash_result, &hash_len);

    EVP_MD_CTX_free(mdctx);
    return hash_result;
}

int main(int argc, char *argv[])
{
    if (argc != 3) {
        printf("Usage: ./hmac <message file> <shared key file>\n");
        return 1;
    }

    int messageLen;
    int keyLen;

    unsigned char *message = Read_File(argv[1], &messageLen);
    unsigned char *key = Read_File(argv[2], &keyLen);

    /* Step 2: Write the original key in hex. */
    char *keyHex = (char*)malloc((keyLen * 2) + 1);
    Convert_to_Hex(keyHex, key, keyLen);
    Write_File("Key.txt", keyHex);

    /* Step 3: Process the key so it is exactly 64 bytes. */
    unsigned char processedKey[BLOCK_SIZE];
    memset(processedKey, 0, BLOCK_SIZE);

    if (keyLen > BLOCK_SIZE) {
        unsigned char *hashedKey = Hash_Blake2s(key, keyLen);
        memcpy(processedKey, hashedKey, HASH_SIZE);
        free(hashedKey);
    }
    else {
        memcpy(processedKey, key, keyLen);
    }

    /* Step 4: Write the processed key in hex. */
    char processedKeyHex[(BLOCK_SIZE * 2) + 1];
    Convert_to_Hex(processedKeyHex, processedKey, BLOCK_SIZE);
    Write_File("ProcessedKey.txt", processedKeyHex);

    /* Step 5: Create the inner and outer padded keys. */
    unsigned char innerKey[BLOCK_SIZE];
    unsigned char outerKey[BLOCK_SIZE];

    for (int i = 0; i < BLOCK_SIZE; i++) {
        innerKey[i] = processedKey[i] ^ 0x36;
        outerKey[i] = processedKey[i] ^ 0x5c;
    }

    /* Inner hash: H((k XOR ipad) || message) */
    int innerInputLen = BLOCK_SIZE + messageLen;
    unsigned char *innerInput = (unsigned char*)malloc(innerInputLen);
    Concatenation(innerKey, BLOCK_SIZE, message, messageLen,
                  innerInput, innerInputLen);

    unsigned char *innerHash = Hash_Blake2s(innerInput, innerInputLen);

    /* Final hash: H((k XOR opad) || innerHash) */
    unsigned char outerInput[BLOCK_SIZE + HASH_SIZE];
    Concatenation(outerKey, BLOCK_SIZE, innerHash, HASH_SIZE,
                  outerInput, BLOCK_SIZE + HASH_SIZE);

    unsigned char *finalHash = Hash_Blake2s(
        outerInput, BLOCK_SIZE + HASH_SIZE
    );

    /* Step 6: Write the final HMAC in hex. */
    char finalHashHex[(HASH_SIZE * 2) + 1];
    Convert_to_Hex(finalHashHex, finalHash, HASH_SIZE);
    Write_File("FinalHash.txt", finalHashHex);

    free(message);
    free(key);
    free(keyHex);
    free(innerInput);
    free(innerHash);
    free(finalHash);

    return 0;
}
