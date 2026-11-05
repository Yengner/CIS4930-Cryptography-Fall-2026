#include <stdlib.h>
#include <stdio.h>
#include <gmp.h>

unsigned char* Read_File (char fileName[], int *fileLen);
void Write_File(char fileName[], char input[]);

unsigned char* Read_File (char fileName[], int *fileLen)
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
    unsigned char *output = (unsigned char*) malloc(temp_size);
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

void mpz_init(mpz_t x);

// Initialize multiple GMP integers (list must be ended with a NULL element)
void mpz_inits(mpz_t x, ...);

// Clear GMP integer memory
void mpz_clear(mpz_t x);

// Clear multiple GMP integers (list must be ended with a NULL element)
void mpz_clears(mpz_t x, ...);

/*============================
    GMP Value Assignment
==============================*/

// Set value from string (returns 0 on success)
int mpz_set_str(mpz_t rop, const char *str, int base);

// Set value to unsigned long int
void mpz_set_ui(mpz_t rop, unsigned long int op);

// Set one mpz_t variable (out) to the value of another (src)
void mpz_set (mpz_t out, const mpz_t src);

/*============================
    GMP Arithmetic
==============================*/

// Multiplication: rop = op1 * op2
void mpz_mul(mpz_t rop, const mpz_t op1, const mpz_t op2);

// Addition: rop = op1 + op2
void mpz_add(mpz_t rop, const mpz_t op1, const mpz_t op2);

// Modulo: rop = op1 mod op2
void mpz_mod(mpz_t rop, const mpz_t op1, const mpz_t op2);

// Division: rop (quotient) = op1 / op2
void mpz_fdiv_q(mpz_t rop, const mpz_t op1, const mpz_t op2);

/*============================
    GMP Comparison Macro
==============================*/

// Evaluates the number, returns +1 if op > 0, 0 if op == 0, -1 if op < 0
//int mpz_sgn(const mpz_t op);

/*============================
    GMP Output Functions
==============================*/

// Convert number to given base in string form.  If 'str' is NULL, allocates enough memory and returns pointer to that memory.
// If 'str' is not NULL, you must ensure that str points to a buffer large enough to hold everything + null terminator.
char * mpz_get_str (char *str, int base, const mpz_t op);

// gmp version of printf, used in exactly the same way.  %Zd is the string format to use for gmp integers.
int gmp_printf (const char * outstr, ...);


typedef __int128 int128_t;


#define SHIFT_LEFT(x, n)   ((x) << (n))
#define SHIFT_RIGHT(x, n)  ((x) >> (n))

// Extract upper/lower 64 bits
#define UPPER_64(x) ((unsigned long)((x) >> 64))
#define LOWER_64(x) ((unsigned long)(x))

// Convert decimal string to int128_t
int128_t str_to_int128(const unsigned char *str, int str_len) {
    int128_t result = 0;
    int negative = 0;
    if (*str == '-') {
        negative = 1;
        str++;
    }
    for (int i=0; i < str_len && *str >= '0' && *str <= '9'; i++) {
        result = result * 10 + (*str - '0');
        str++;
    }
    return negative ? -result : result;
}

// Convert int128_t to decimal string
void int128_to_str(int128_t value, unsigned char *output, int output_len) {
    int negative = value < 0;
    if (negative) 
        value = -value;

    // Not going to need more than 40 digits, make it 50 just to be safe
    char temp[50];
    int i = 0;

    // In reverse order, reversed at the end
    do {
        temp[i++] = '0' + (value % 10);
        value /= 10;
    } while (value > 0 && i < (int)sizeof(temp) - 1);

    if (negative) 
        temp[i++] = '-';

    if (i >= output_len) {
        output[0] = '\0';  // Not enough space
        return;
    }

    // Reverse the string into the output
    for (int j = 0; j < i; j++) {
        output[j] = temp[i - j - 1];
    }
    // Null-terminate
    output[i] = '\0';
}

//__________________________________________________________________________________________________________________________

void square_and_multiply(mpz_t result, mpz_t base, mpz_t exponent, mpz_t modulo)
{
    mpz_t power, factor, two, remainder;
    mpz_inits(power, factor, two, remainder, NULL);
    mpz_set(power, exponent);
    mpz_mod(factor, base, modulo);
    mpz_set_ui(result, 1);
    mpz_mod(result, result, modulo);
    mpz_set_ui(two, 2);

    while (mpz_sgn(power) > 0)
    {
        mpz_mod(remainder, power, two);
        if (mpz_sgn(remainder) != 0)
        {
            mpz_mul(result, result, factor);
            mpz_mod(result, result, modulo);
        }
        mpz_mul(factor, factor, factor);
        mpz_mod(factor, factor, modulo);
        mpz_fdiv_q(power, power, two);
    }

    mpz_clears(power, factor, two, remainder, NULL);
}

int main(int argc, char *argv[])
{
    if (argc != 4)
    {
        printf("Usage: ./snm_gmp basefile exponentfile modulofile\n");
        return 1;
    }

    int baseLen, exponentLen, moduloLen;
    unsigned char *baseText = Read_File(argv[1], &baseLen);
    unsigned char *exponentText = Read_File(argv[2], &exponentLen);
    unsigned char *moduloText = Read_File(argv[3], &moduloLen);

    mpz_t base, exponent, modulo, result;
    mpz_inits(base, exponent, modulo, result, NULL);
    if (mpz_set_str(base, (char *)baseText, 10) != 0 ||
        mpz_set_str(exponent, (char *)exponentText, 10) != 0 ||
        mpz_set_str(modulo, (char *)moduloText, 10) != 0 ||
        mpz_sgn(modulo) <= 0 || mpz_sgn(exponent) < 0)
    {
        printf("Invalid input.\n");
        return 1;
    }

    square_and_multiply(result, base, exponent, modulo);
    char resultText[100];
    mpz_get_str(resultText, 10, result);
    Write_File("Result.txt", resultText);

    mpz_clears(base, exponent, modulo, result, NULL);
    free(baseText);
    free(exponentText);
    free(moduloText);
    return 0;
}
