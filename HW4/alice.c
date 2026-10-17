#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <openssl/ec.h>
#include <openssl/bn.h>
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

int main(int argc, char *argv[])
{
    if (argc != 4)
    {
        printf("ElGamal usage: ./Alice publickeyfile randomkfile messagefile\n");
        return 1;
    }

    int publicLength, randomLength, messageLength;
    char *publicHex = Read_File(argv[1], &publicLength);
    char *randomHex = Read_File(argv[2], &randomLength);
    char *messageHex = Read_File(argv[3], &messageLength);
    publicHex[strcspn(publicHex, "\r\n")] = '\0';
    randomHex[strcspn(randomHex, "\r\n")] = '\0';
    messageHex[strcspn(messageHex, "\r\n")] = '\0';

    BIGNUM *k = NULL;
    EC_KEY *key = EC_KEY_new_by_curve_name(NID_secp192k1);
    BN_CTX *context = BN_CTX_new();
    if (key == NULL || context == NULL || BN_hex2bn(&k, randomHex) == 0)
    {
        printf("Error reading key.\n");
        return 1;
    }

    const EC_GROUP *group = EC_KEY_get0_group(key);
    EC_POINT *publicKey = EC_POINT_hex2point(group, publicHex, NULL, context);
    EC_POINT *message = EC_POINT_hex2point(group, messageHex, NULL, context);
    EC_POINT *c = EC_POINT_new(group);
    EC_POINT *sharedPoint = EC_POINT_new(group);
    EC_POINT *d = EC_POINT_new(group);
    if (publicKey == NULL || message == NULL || c == NULL ||
        sharedPoint == NULL || d == NULL)
    {
        printf("Error reading curve point.\n");
        return 1;
    }

    if (EC_POINT_mul(group, c, k, NULL, NULL, context) != 1 ||
        EC_POINT_mul(group, sharedPoint, NULL, publicKey, k, context) != 1 ||
        EC_POINT_add(group, d, sharedPoint, message, context) != 1)
    {
        printf("Error encrypting message.\n");
        return 1;
    }

    char *cHex = EC_POINT_point2hex(group, c, EC_KEY_get_conv_form(key), context);
    char *dHex = EC_POINT_point2hex(group, d, EC_KEY_get_conv_form(key), context);
    if (cHex == NULL || dHex == NULL)
    {
        printf("Error converting ciphertext.\n");
        return 1;
    }

    Write_File("alice_c.txt", cHex);
    Write_File("alice_d.txt", dHex);

    OPENSSL_free(cHex);
    OPENSSL_free(dHex);
    EC_POINT_free(publicKey);
    EC_POINT_free(message);
    EC_POINT_free(c);
    EC_POINT_free(sharedPoint);
    EC_POINT_free(d);
    BN_free(k);
    BN_CTX_free(context);
    EC_KEY_free(key);
    free(publicHex);
    free(randomHex);
    free(messageHex);
    return 0;
}
