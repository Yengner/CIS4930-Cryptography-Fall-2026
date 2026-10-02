#include <stdio.h>
#include <stdlib.h>
#include <stdint.h> // for uint64_t
#include <string.h> // for memcpy()
#include <unistd.h> // for ssize_t

#define LENGTH_OF_EACH_MESSAGE 64
#define THRESHOLD 3

// Modular exponentiation: computes (base^exp) % mod using square-and-multiply
uint64_t power_mod(uint64_t base, uint64_t exp, uint64_t mod) {
    uint64_t result = 1;
    base = base % mod;
    while (exp > 0) {
        if (exp & 1) {
            result = (uint64_t)((__uint128_t)result * base % mod);
        }
        base = (uint64_t)((__uint128_t)base * base % mod);
        exp >>= 1;
    }
    return result;
}

// Modular inverse via Fermat's Little Theorem: a^(p - 2) mod p
uint64_t mod_inverse(uint64_t a, uint64_t p) {
    return power_mod(a, p - 2, p);
}

// Reads a single line / file as unsigned char*
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

// Parses decimal string into uint64_t
uint64_t str_to_uint64(const unsigned char *str, int str_len) {
    uint64_t result = 0;
    for (int i = 0; i < str_len && str[i] >= '0' && str[i] <= '9'; i++) {
        result = result * 10 + (str[i] - '0');
    }
    return result;
}

// Reads X and Y lines from a Share file
void Read_Share(const char *fileName, uint64_t *x, uint64_t *y) {
    FILE *fp = fopen(fileName, "r");
    if (!fp) {
        printf("Error opening share file %s\n", fileName);
        exit(1);
    }
    char line1[LENGTH_OF_EACH_MESSAGE];
    char line2[LENGTH_OF_EACH_MESSAGE];
    if (fgets(line1, sizeof(line1), fp) == NULL || fgets(line2, sizeof(line2), fp) == NULL) {
        printf("Error reading share points from %s\n", fileName);
        fclose(fp);
        exit(1);
    }
    fclose(fp);
    *x = str_to_uint64((unsigned char*)line1, strlen(line1));
    *y = str_to_uint64((unsigned char*)line2, strlen(line2));
}

int main(int argc, char *argv[]) {
    char modulusFile[64] = "Modulus.txt";
    if (argc >= 2) {
        strncpy(modulusFile, argv[1], sizeof(modulusFile) - 1);
    }

    int modLen = 0;
    unsigned char *modStr = Read_File(modulusFile, &modLen);
    uint64_t p = str_to_uint64(modStr, modLen);
    free(modStr);

    uint64_t xs[THRESHOLD];
    uint64_t ys[THRESHOLD];

    // Read THRESHOLD shares (Share1.txt, Share2.txt, Share3.txt)
    for (int i = 0; i < THRESHOLD; i++) {
        char shareName[32];
        snprintf(shareName, sizeof(shareName), "Share%d.txt", i + 1);
        Read_Share(shareName, &xs[i], &ys[i]);
    }

    // Lagrange Interpolation to evaluate f(0) mod p
    uint64_t secret = 0;

    for (int j = 0; j < THRESHOLD; j++) {
        uint64_t num = 1;
        uint64_t den = 1;

        for (int m = 0; m < THRESHOLD; m++) {
            if (m != j) {
                // Compute numerator term: (0 - x_m) mod p = (p - (x_m % p)) mod p
                uint64_t neg_xm = (p - (xs[m] % p)) % p;
                num = (uint64_t)((__uint128_t)num * neg_xm % p);

                // Compute denominator term: (x_j - x_m) mod p with underflow protection
                uint64_t diff;
                if (xs[j] >= xs[m]) {
                    diff = (xs[j] - xs[m]) % p;
                } else {
                    diff = (p - ((xs[m] - xs[j]) % p)) % p;
                }
                den = (uint64_t)((__uint128_t)den * diff % p);
            }
        }

        // Modular division: num * den^(-1) mod p
        uint64_t den_inv = mod_inverse(den, p);
        uint64_t basis = (uint64_t)((__uint128_t)num * den_inv % p);
        uint64_t term = (uint64_t)((__uint128_t)ys[j] * basis % p);

        secret = (secret + term) % p;
    }

    // Write recovered secret to Recovered.txt
    FILE *out = fopen("Recovered.txt", "w");
    if (!out) {
        printf("Error creating Recovered.txt\n");
        exit(1);
    }
    fprintf(out, "%lu", (unsigned long)secret);
    fclose(out);

    return 0;
}