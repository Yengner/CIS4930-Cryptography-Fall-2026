#include <stdio.h>
#include <stdlib.h>
#include <gmp.h>

/**
 * Reads the entire content of a file into a dynamically allocated string.
 * Strips potential trailing newline characters.
 */
char* read_file_string(const char *filename) {
    FILE *fp = fopen(filename, "r");
    if (fp == NULL) {
        fprintf(stderr, "Error opening file %s\n", filename);
        exit(EXIT_FAILURE);
    }

    // Seek to the end to get file size
    fseek(fp, 0, SEEK_END);
    long length = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    char *buffer = (char *)malloc(length + 1);
    if (buffer == NULL) {
        fprintf(stderr, "Memory allocation failed for %s\n", filename);
        fclose(fp);
        exit(EXIT_FAILURE);
    }

    size_t read_bytes = fread(buffer, 1, length, fp);
    buffer[read_bytes] = '\0';
    fclose(fp);

    // Strip trailing newline or carriage return if present
    while (read_bytes > 0 && (buffer[read_bytes - 1] == '\n' || buffer[read_bytes - 1] == '\r')) {
        buffer[read_bytes - 1] = '\0';
        read_bytes--;
    }

    return buffer;
}

/**
 * Square-and-Multiply Modular Exponentiation:
 * Computes result = (base ^ exp) % mod
 */
void square_and_multiply(mpz_t result, const mpz_t base, const mpz_t exp, const mpz_t mod) {
    // If modulus is 1, any value mod 1 is 0
    if (mpz_cmp_ui(mod, 1) == 0) {
        mpz_set_ui(result, 0);
        return;
    }

    // If exponent is 0, base^0 mod mod is 1
    if (mpz_cmp_ui(exp, 0) == 0) {
        mpz_set_ui(result, 1);
        return;
    }

    mpz_t r, b;
    mpz_init(r);
    mpz_init(b);

    // Ensure base is within [0, mod - 1]
    mpz_mod(b, base, mod);
    mpz_set_ui(r, 1);

    // Find the number of bits in the exponent
    size_t num_bits = mpz_sizeinbase(exp, 2);

    // Scan bits from Most Significant Bit (MSB) down to 0
    for (long i = (long)num_bits - 1; i >= 0; i--) {
        // Square: r = (r * r) % mod
        mpz_mul(r, r, r);
        mpz_mod(r, r, mod);

        // Multiply if current bit is 1: r = (r * b) % mod
        if (mpz_tstbit(exp, i) == 1) {
            mpz_mul(r, r, b);
            mpz_mod(r, r, mod);
        }
    }

    mpz_set(result, r);

    // Clean up temporary variables
    mpz_clear(r);
    mpz_clear(b);
}

int main(int argc, char *argv[]) {
    // Allow filename inputs via command-line arguments or default fallbacks
    const char *base_filename = (argc > 1) ? argv[1] : "Base.txt";
    const char *exp_filename  = (argc > 2) ? argv[2] : "Exponent.txt";
    const char *mod_filename  = (argc > 3) ? argv[3] : "Modulo.txt";

    // Read input files as raw strings
    char *base_str = read_file_string(base_filename);
    char *exp_str  = read_file_string(exp_filename);
    char *mod_str  = read_file_string(mod_filename);

    // Initialize GMP variables
    mpz_t base, exp, mod, result;
    mpz_init(base);
    mpz_init(exp);
    mpz_init(mod);
    mpz_init(result);

    // Parse decimal strings (base 10) into GMP integers
    if (mpz_set_str(base, base_str, 10) != 0 ||
        mpz_set_str(exp, exp_str, 10)   != 0 ||
        mpz_set_str(mod, mod_str, 10)   != 0) {
        fprintf(stderr, "Error parsing numeric strings into GMP integers.\n");
        exit(EXIT_FAILURE);
    }

    // Execute square-and-multiply
    square_and_multiply(result, base, exp, mod);

    // Write result to Result.txt in decimal format
    // Write result to Result.txt in decimal format
    FILE *out_fp = fopen("Result.txt", "w");
    if (out_fp == NULL) {
        fprintf(stderr, "Error opening Result.txt for writing.\n");
        exit(EXIT_FAILURE);
    }

    mpz_out_str(out_fp, 10, result);
    fclose(out_fp);

    // Free buffers and clean GMP structures
    free(base_str);
    free(exp_str);
    free(mod_str);

    mpz_clear(base);
    mpz_clear(exp);
    mpz_clear(mod);
    mpz_clear(result);

    return 0;
}