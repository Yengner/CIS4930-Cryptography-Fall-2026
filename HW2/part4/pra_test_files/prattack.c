#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>

typedef unsigned __int128 uint128_t;
typedef __int128 int128_t;

/* String and I/O helpers for 128-bit integers */
uint128_t parse_uint128(const char *str) {
    uint128_t res = 0;
    while (*str >= '0' && *str <= '9') {
        res = res * 10 + (*str - '0');
        str++;
    }
    return res;
}

void print_uint128_to_file(FILE *fp, uint128_t val) {
    if (val == 0) {
        fputc('0', fp);
        return;
    }
    char buf[64];
    int idx = 0;
    while (val > 0) {
        buf[idx++] = (char)('0' + (val % 10));
        val /= 10;
    }
    for (int i = idx - 1; i >= 0; i--) {
        fputc(buf[i], fp);
    }
}

char* read_file_string(const char *filename) {
    FILE *fp = fopen(filename, "r");
    if (fp == NULL) {
        fprintf(stderr, "Error opening file %s\n", filename);
        exit(EXIT_FAILURE);
    }
    fseek(fp, 0, SEEK_END);
    long length = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    char *buf = (char *)malloc(length + 1);
    if (!buf) {
        fclose(fp);
        exit(EXIT_FAILURE);
    }
    size_t read_bytes = fread(buf, 1, length, fp);
    buf[read_bytes] = '\0';
    fclose(fp);

    while (read_bytes > 0 && (buf[read_bytes - 1] == '\n' || buf[read_bytes - 1] == '\r' || buf[read_bytes - 1] == ' ')) {
        buf[read_bytes - 1] = '\0';
        read_bytes--;
    }
    return buf;
}

/* Square-and-multiply modular exponentiation: (base^exp) % mod */
uint128_t mod_exp(uint128_t base, uint128_t exp, uint128_t mod) {
    uint128_t res = 1;
    base %= mod;
    while (exp > 0) {
        if (exp & 1) {
            res = (res * base) % mod;
        }
        base = (base * base) % mod;
        exp >>= 1;
    }
    return res;
}

/* Extended Euclidean Algorithm */
int128_t ext_gcd(int128_t a, int128_t b, int128_t *x, int128_t *y) {
    if (b == 0) {
        *x = 1;
        *y = 0;
        return a;
    }
    int128_t x1, y1;
    int128_t gcd = ext_gcd(b, a % b, &x1, &y1);
    *x = y1;
    *y = x1 - (a / b) * y1;
    return gcd;
}

/* Modular inverse: a^-1 mod m */
int128_t mod_inv(int128_t a, int128_t m) {
    int128_t x, y;
    int128_t g = ext_gcd(a, m, &x, &y);
    if (g != 1) return -1;
    return (x % m + m) % m;
}

/* Pseudo-random 64-bit generator for randomized starting values */
uint128_t get_random_128(uint128_t max_val) {
    if (max_val <= 1) return 0;
    uint64_t r1 = (uint64_t)rand();
    uint64_t r2 = (uint64_t)rand();
    uint128_t r = ((uint128_t)r1 << 32) | r2;
    return r % max_val;
}

/* Step transition function for Pollard's Rho */
void step(uint128_t *x, uint128_t *a, uint128_t *b, uint128_t alpha, uint128_t Y, uint128_t p, uint128_t n) {
    switch (*x % 3) {
        case 0:
            *x = (*x * *x) % p;
            *a = (*a * 2) % n;
            *b = (*b * 2) % n;
            break;
        case 1:
            *x = (*x * alpha) % p;
            *a = (*a + 1) % n;
            break;
        case 2:
            *x = (*x * Y) % p;
            *b = (*b + 1) % n;
            break;
    }
}

int main(int argc, char *argv[]) {
    srand((unsigned int)time(NULL));

    const char *a_file = (argc > 1) ? argv[1] : "PRA.txt";
    const char *y_file = (argc > 2) ? argv[2] : "PRY.txt";
    const char *p_file = (argc > 3) ? argv[3] : "PRP.txt";

    char *a_str = read_file_string(a_file);
    char *y_str = read_file_string(y_file);
    char *p_str = read_file_string(p_file);

    uint128_t alpha = parse_uint128(a_str);
    uint128_t Y     = parse_uint128(y_str);
    uint128_t p     = parse_uint128(p_str);

    free(a_str);
    free(y_str);
    free(p_str);

    uint128_t n = p - 1; // Order of Z_p*

    uint128_t x_sol = 0;
    int solved = 0;

    while (!solved) {
        // Step 1: Initialize random starting point
        uint128_t a0 = get_random_128(n);
        uint128_t b0 = get_random_128(n);

        // x0 = (alpha^a0 * Y^b0) % p
        uint128_t x0 = (mod_exp(alpha, a0, p) * mod_exp(Y, b0, p)) % p;

        uint128_t x_t = x0, a_t = a0, b_t = b0;
        uint128_t x_h = x0, a_h = a0, b_h = b0;

        // Step 2: Floyd's Cycle Finding (Tortoise & Hare)
        while (1) {
            // Tortoise takes 1 step
            step(&x_t, &a_t, &b_t, alpha, Y, p, n);

            // Hare takes 2 steps
            step(&x_h, &a_h, &b_h, alpha, Y, p, n);
            step(&x_h, &a_h, &b_h, alpha, Y, p, n);

            if (x_t == x_h) {
                break; // Collision found
            }
        }

        // Equation: (b_h - b_t) * x = (a_t - a_h) mod n
        int128_t u = (int128_t)b_h - (int128_t)b_t;
        int128_t v = (int128_t)a_t - (int128_t)a_h;

        u = (u % (int128_t)n + (int128_t)n) % (int128_t)n;
        v = (v % (int128_t)n + (int128_t)n) % (int128_t)n;

        int128_t inv_x, inv_y;
        int128_t d = ext_gcd(u, (int128_t)n, &inv_x, &inv_y);

        // If v is not divisible by d, no solution exists for this collision
        if (v % d != 0) {
            continue; // Random restart
        }

        int128_t u_prime = u / d;
        int128_t v_prime = v / d;
        int128_t n_prime = (int128_t)n / d;

        int128_t u_inv = mod_inv(u_prime, n_prime);
        if (u_inv == -1) {
            continue;
        }

        int128_t x0_cand = (v_prime * u_inv) % n_prime;
        if (x0_cand < 0) x0_cand += n_prime;

        // Test all d possible solutions: x0_cand + k * n_prime
        for (int128_t k = 0; k < d; k++) {
            uint128_t candidate = (uint128_t)(x0_cand + k * n_prime);
            if (mod_exp(alpha, candidate, p) == Y) {
                x_sol = candidate;
                solved = 1;
                break;
            }
        }
    }

    // Step 3: Write result to Exponent.txt
    FILE *out = fopen("Exponent.txt", "w");
    if (!out) {
        fprintf(stderr, "Error opening Exponent.txt\n");
        exit(EXIT_FAILURE);
    }
    print_uint128_to_file(out, x_sol);
    fclose(out);

    return 0;
}