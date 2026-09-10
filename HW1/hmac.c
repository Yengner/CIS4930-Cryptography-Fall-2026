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

unsigned char *Read_File(char fileName[], int *fileLen);
void Write_File(char fileName[], char input[]);
void Convert_to_Hex(char output[], unsigned char input[], int inputlength);
unsigned char *Hash_Blake2s(unsigned char *input, unsigned long inputlen);
void Concatenation(unsigned char *in1, size_t in1len, unsigned char *in2, size_t in2len, unsigned char *out, size_t outlen);

/* Read all bytes from a file. */
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

void Concatenation32(unsigned char a[32], unsigned char b[32], unsigned char c[64])
{
    memcpy(c, a, 32);
    memcpy(c + 32, b, 32);
}

unsigned char *Hash_Blake2s(unsigned char *input, unsigned long inputlen)
{
    unsigned char *hash_result = (unsigned char *)malloc(EVP_MAX_MD_SIZE); // malloc the EVP max size, which is 64, setting to 32 or 33 gives intermittent memory errors
    int hash_len;                                                          // ends up set to 32 during EVP_DigestFinal, unused

    const EVP_MD *md = EVP_get_digestbyname("BLAKE2s256"); // Get the BLAKE2s hashing (digest) algorithm
    EVP_MD_CTX *mdctx = EVP_MD_CTX_new();                  // Create the digest context required for next parts
    EVP_DigestInit_ex(mdctx, md, NULL);                    // Initialize the digest context for hashing with Blake2s
    EVP_DigestUpdate(mdctx, input, inputlen);              // Update the digest context with the input to be hashed
    EVP_DigestFinal_ex(mdctx, hash_result, &hash_len);     // Finalize the hash computation, putting result in hash_result

    EVP_MD_CTX_free(mdctx);

    return hash_result;
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        printf("Usage: ./hmac <message file> <shared key file>\n");
        return 1;
    }

 
    return 0;
}
