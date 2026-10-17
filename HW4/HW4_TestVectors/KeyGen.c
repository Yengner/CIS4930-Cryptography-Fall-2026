#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <openssl/ec.h>
#include <openssl/bn.h>
#include <openssl/sha.h>
#include <openssl/obj_mac.h>

char* Read_File (char fileName[], int *fileLen)
{
    FILE *pFile;
	pFile = fopen(fileName, "r");
	if (pFile == NULL)
	{
		printf("Error opening file.\n");
		exit(0);
	}
    fseek(pFile, 0L, SEEK_END);
    int temp_size = ftell(pFile)+1;
    fseek(pFile, 0L, SEEK_SET);
    char *output = (char*) malloc(temp_size);
	fgets(output, temp_size, pFile);
	fclose(pFile);

    *fileLen = temp_size-1;
	return output;
}

void Write_File(char fileName[], char input[]){
  FILE *pFile;
  pFile = fopen(fileName,"w");
  if (pFile == NULL){
    printf("Error opening file. \n");
    exit(0);
  }
  fputs(input, pFile);
  fclose(pFile);
}

void Convert_to_Hex(char output[], unsigned char input[], int inputlength)
{
    for (int i=0; i<inputlength; i++){
        sprintf(&output[2*i], "%02x", input[i]);
    }
    //printf("Hex format: %s\n", output);  //remove later
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("Usage: ./KeyGen seedfile\n");
        return 1;
    }

    int seedLength;
    unsigned char *seed = (unsigned char *)Read_File(argv[1], &seedLength);
    unsigned char hash[SHA256_DIGEST_LENGTH];
    char secretHex[SHA256_DIGEST_LENGTH * 2 + 1];

    if (seedLength < 32)
    {
        printf("Seed must contain at least 32 bytes.\n");
        free(seed);
        return 1;
    }

    SHA256(seed, 32, hash);
    Convert_to_Hex(secretHex, hash, SHA256_DIGEST_LENGTH);
    secretHex[SHA256_DIGEST_LENGTH * 2] = '\0';

    BIGNUM *secretKey = NULL;
    EC_KEY *key = EC_KEY_new_by_curve_name(NID_secp192k1);
    BN_CTX *context = BN_CTX_new();
    if (key == NULL || context == NULL || BN_hex2bn(&secretKey, secretHex) == 0)
    {
        printf("Error creating key.\n");
        return 1;
    }

    const EC_GROUP *group = EC_KEY_get0_group(key);
    EC_POINT *publicKey = EC_POINT_new(group);
    if (publicKey == NULL ||
        EC_POINT_mul(group, publicKey, secretKey, NULL, NULL, context) != 1)
    {
        printf("Error calculating public key.\n");
        return 1;
    }

    char *publicHex = EC_POINT_point2hex(group, publicKey,
                                        EC_KEY_get_conv_form(key), context);
    if (publicHex == NULL)
    {
        printf("Error converting public key.\n");
        return 1;
    }

    Write_File("SK_Hex.txt", secretHex);
    Write_File("PK_Hex.txt", publicHex);

    OPENSSL_free(publicHex);
    EC_POINT_free(publicKey);
    BN_free(secretKey);
    BN_CTX_free(context);
    EC_KEY_free(key);
    free(seed);
    return 0;
}
