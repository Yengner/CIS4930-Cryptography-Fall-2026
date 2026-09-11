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

    int messageLen, keyLen, counterLen, nonceLen;

    unsigned char *message = Read_File(argv[1], &messageLen);
    unsigned char *key = Read_File(argv[2], &keyLen);
    unsigned char *counter = Read_File(argv[3], &counterLen);
    unsigned char *nonce = Read_File(argv[4], &nonceLen);

    // write key in hex
    char *keyHex = malloc((keyLen * 2) + 1);

    Convert_to_Hex(keyHex, key, keyLen);
    keyHex[keyLen * 2] = '\0';

    Write_File("Key.txt", keyHex);

    // ciphertext = message XOR SHA256(key || counter)
    int keyCounterLen = keyLen + counterLen;
    unsigned char *keyCounter = malloc(keyCounterLen);

    memcpy(keyCounter, key, keyLen);
    memcpy(keyCounter + keyLen, counter, counterLen);

    unsigned char keyCounterHash[HASH_SIZE];

    SHA256(keyCounter, keyCounterLen, keyCounterHash);

    unsigned char ciphertext[HASH_SIZE];

    for (int i = 0; i < HASH_SIZE; i++)
    {
        ciphertext[i] = message[i] ^ keyCounterHash[i];
    }

    char ciphertextHex[(HASH_SIZE * 2) + 1];

    Convert_to_Hex(ciphertextHex, ciphertext, HASH_SIZE);
    ciphertextHex[HASH_SIZE * 2] = '\0';

    Write_File("Ciphertext.txt", ciphertextHex);

    // signature = HMAC(key, ciphertext || nonce)
    int sigInputLen = HASH_SIZE + nonceLen;
    unsigned char *sigInput = malloc(sigInputLen);

    memcpy(sigInput, ciphertext, HASH_SIZE);
    memcpy(sigInput + HASH_SIZE, nonce, nonceLen);

    unsigned char signature[HASH_SIZE];
    unsigned int signatureLen;

    HMAC_SHA256(key, keyLen, sigInput, sigInputLen, signature, &signatureLen);

    char signatureHex[(HASH_SIZE * 2) + 1];

    Convert_to_Hex(signatureHex, signature, HASH_SIZE);
    signatureHex[HASH_SIZE * 2] = '\0';

    Write_File("Signature.txt", signatureHex);

    // Bob has not responded yet on the first run
    FILE *responseFile = fopen("Response.txt", "r");

    if (responseFile == NULL)
    {
        printf("Waiting for Bob's response.\n");

        free(message);
        free(key);
        free(counter);
        free(nonce);
        free(keyHex);
        free(keyCounter);
        free(sigInput);

        return 0;
    }

    fclose(responseFile);

    // read Bob's response
    int responseLen;
    unsigned char *responseHex = Read_File("Response.txt", &responseLen);

    // increase counter and nonce by 1
    int counterNumber = atoi((char *)counter);
    int nonceNumber = atoi((char *)nonce);

    char nextCounter[50];
    char nextNonce[50];

    sprintf(nextCounter, "%d", counterNumber + 1);
    sprintf(nextNonce, "%d", nonceNumber + 1);

    // message || new counter || new nonce
    int nextCounterLen = strlen(nextCounter);
    int nextNonceLen = strlen(nextNonce);

    int responseInputLen =
        messageLen + nextCounterLen + nextNonceLen;

    unsigned char *responseInput = malloc(responseInputLen);

    memcpy(responseInput, message, messageLen);

    memcpy(responseInput + messageLen, nextCounter, nextCounterLen);

    memcpy(responseInput + messageLen + nextCounterLen, nextNonce, nextNonceLen);

    unsigned char expectedResponse[HASH_SIZE];

    SHA256(responseInput, responseInputLen, expectedResponse);

    // Bob wrote his response in hex, change it back
    unsigned char bobResponse[HASH_SIZE];

    Convert_To_Uchar(
        (char *)responseHex,
        bobResponse,
        HASH_SIZE);

    // check Bob's response
    if (memcmp(expectedResponse, bobResponse, HASH_SIZE) == 0)
    {
        Write_File("Acknowledgment.txt", "Acknowledgment Successful");
    }
    else
    {
        Write_File("Acknowledgment.txt", "Acknowledgment Failed");
    }

    // update Alice's counter and nonce
    Write_File(argv[3], nextCounter);
    Write_File(argv[4], nextNonce);

    free(message);
    free(key);
    free(counter);
    free(nonce);
    free(keyHex);
    free(keyCounter);
    free(sigInput);
    free(responseHex);
    free(responseInput);

    return 0;
}