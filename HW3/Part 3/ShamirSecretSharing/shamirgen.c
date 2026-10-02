#include <stdio.h>
#include <stdlib.h>
#include <stdint.h> // for uint64_t
#include <string.h> // for memcpy()
#include <unistd.h> // for ssize_t
#if defined(__has_include)
#  if __has_include(<openssl/rand.h>)
#    include <openssl/rand.h> // for RAND_bytes()
#    define HAVE_OPENSSL_RAND 1
#  endif
#endif

#define LENGTH_OF_EACH_MESSAGE 64
#define N_SHARES 5
#define THRESHOLD 3

// Reads a file into an allocated unsigned char buffer
unsigned char* Read_File(char fileName[], int *fileLen) {
    FILE *pFile = fopen(fileName, "r");
    if (pFile == NULL) {
        printf("Error opening file %s\n", fileName);
        exit(1);
    }
    fseek(pFile, 0L, SEEK_END);
    int temp_size = ftell(pFile) + 1;
    fseek(pFile, 0L, SEEK_SET);
    unsigned char *output = (unsigned char*) malloc(temp_size);
    if (!fgets((char*)output, temp_size, pFile)) {
        output[0] = '\0';
    }
    fclose(pFile);
    *fileLen = temp_size - 1;
    return output;
}

// Converts a decimal ASCII string into uint64_t
uint64_t str_to_uint64(const unsigned char *str, int str_len) {
    uint64_t result = 0;
    for (int i = 0; i < str_len && str[i] >= '0' && str[i] <= '9'; i++) {
        result = result * 10 + (str[i] - '0');
    }
    return result;
}

// Writes two lines (X and Y coordinates) to a share file
void Write_Share_File(const char *fileName, uint64_t x, uint64_t y) {
    FILE *pFile = fopen(fileName, "w");
    if (pFile == NULL) {
        printf("Error opening file %s\n", fileName);
        exit(1);
    }
    fprintf(pFile, "%lu\n%lu\n", (unsigned long)x, (unsigned long)y);
    fclose(pFile);
}

int main(int argc, char *argv[]) {
    char secretFile[64] = "Secret.txt";
    char modulusFile[64] = "Modulus.txt";

    // Handle command-line arguments if provided
    if (argc >= 3) {
        strncpy(secretFile, argv[1], sizeof(secretFile) - 1);
        strncpy(modulusFile, argv[2], sizeof(modulusFile) - 1);
    }

    int secretLen = 0, modLen = 0;
    unsigned char *secretStr = Read_File(secretFile, &secretLen);
    unsigned char *modStr = Read_File(modulusFile, &modLen);

    uint64_t secret = str_to_uint64(secretStr, secretLen);
    uint64_t p = str_to_uint64(modStr, modLen);

    free(secretStr);
    free(modStr);

    // Degree is THRESHOLD - 1 (for t = 3, polynomial has 3 coefficients: a0, a1, a2)
    uint64_t coeff[THRESHOLD];
    coeff[0] = secret % p; // a0 is the secret

    // Generate random coefficients for a1 ... a_(t-1)
    for (int i = 1; i < THRESHOLD; i++) {
        uint32_t rand_val = 0;
    #ifdef HAVE_OPENSSL_RAND
        if (RAND_bytes((unsigned char*)&rand_val, sizeof(rand_val)) != 1) {
            rand_val = (uint32_t)rand();
        }
    #else
        rand_val = (uint32_t)rand();
    #endif
        coeff[i] = ((uint64_t)rand_val) % p;
    }

    // Generate N_SHARES points: (x, f(x) mod p)
    for (uint64_t x = 1; x <= N_SHARES; x++) {
        // Horner's method for polynomial evaluation: f(x) = a0 + x*(a1 + x*a2)
        uint64_t y = 0;
        for (int deg = THRESHOLD - 1; deg >= 0; deg--) {
            y = ((y * x) % p + coeff[deg]) % p;
        }

        char shareFileName[32];
        snprintf(shareFileName, sizeof(shareFileName), "Share%lu.txt", (unsigned long)x);
        Write_Share_File(shareFileName, x, y);
    }

    return 0;
}