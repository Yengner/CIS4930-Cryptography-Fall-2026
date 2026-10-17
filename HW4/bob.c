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
        printf("ElGamal usage: ./Bob secretkeyfile cfile dfile\n");
        return 1;
    }

    int secretLength, cLength, dLength;
    char *secretHex = Read_File(argv[1], &secretLength);
    char *cHex = Read_File(argv[2], &cLength);
    char *dHex = Read_File(argv[3], &dLength);
    secretHex[strcspn(secretHex, "\r\n")] = '\0';
    cHex[strcspn(cHex, "\r\n")] = '\0';
    dHex[strcspn(dHex, "\r\n")] = '\0';

    BIGNUM *secretKey = NULL;
    EC_KEY *key = EC_KEY_new_by_curve_name(NID_secp192k1);
    BN_CTX *context = BN_CTX_new();
    if (key == NULL || context == NULL || BN_hex2bn(&secretKey, secretHex) == 0)
    {
        printf("Error reading secret key.\n");
        return 1;
    }

    const EC_GROUP *group = EC_KEY_get0_group(key);
    EC_POINT *c = EC_POINT_hex2point(group, cHex, NULL, context);
    EC_POINT *d = EC_POINT_hex2point(group, dHex, NULL, context);
    EC_POINT *sharedPoint = EC_POINT_new(group);
    EC_POINT *message = EC_POINT_new(group);
    if (c == NULL || d == NULL || sharedPoint == NULL || message == NULL)
    {
        printf("Error reading curve point.\n");
        return 1;
    }

    if (EC_POINT_mul(group, sharedPoint, NULL, c, secretKey, context) != 1 ||
        EC_POINT_invert(group, sharedPoint, context) != 1 ||
        EC_POINT_add(group, message, d, sharedPoint, context) != 1)
    {
        printf("Error decrypting message.\n");
        return 1;
    }

    char *messageHex = EC_POINT_point2hex(group, message, EC_KEY_get_conv_form(key), context);
    if (messageHex == NULL)
    {
        printf("Error converting plaintext.\n");
        return 1;
    }

    Write_File("Plaintext.txt", messageHex);

    OPENSSL_free(messageHex);
    EC_POINT_free(c);
    EC_POINT_free(d);
    EC_POINT_free(sharedPoint);
    EC_POINT_free(message);
    BN_free(secretKey);
    BN_CTX_free(context);
    EC_KEY_free(key);
    free(secretHex);
    free(cHex);
    free(dHex);
    return 0;
}
