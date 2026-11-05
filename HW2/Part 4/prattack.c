#include <stdlib.h>
#include <time.h>
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

/*============================
        Write to File
==============================*/
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


/*************************************************************
                    GMP Functions
**************************************************************/

/*============================
    GMP Integer Operations
==============================*/

// Initialize GMP integer
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


/*************************************************************
                    int128_t Functions
**************************************************************/

/*============================
    128-bit Integer Type
==============================*/

// 128-bit integer type, typedef'd for better readability
typedef __int128 int128_t;

/*============================
    128-bit Operations
==============================*/

// Bitwise operations
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

int128_t multiply_mod(int128_t a, int128_t b, int128_t modulo)
{
    if (modulo <= 9223372036854775807LL)
        return (a * b) % modulo;

    int128_t result = 0;
    while (b > 0)
    {
        if (b % 2 != 0)
            result = (result + a) % modulo;
        a = (a + a) % modulo;
        b /= 2;
    }
    return result;
}

int128_t power_mod(int128_t base, int128_t exponent, int128_t modulo)
{
    int128_t result = 1 % modulo;
    base %= modulo;
    while (exponent > 0)
    {
        if (exponent % 2 != 0)
            result = multiply_mod(result, base, modulo);
        base = multiply_mod(base, base, modulo);
        exponent /= 2;
    }
    return result;
}

int128_t gcd128(int128_t a, int128_t b)
{
    while (b != 0)
    {
        int128_t remainder = a % b;
        a = b;
        b = remainder;
    }
    return a;
}

int128_t inverse_mod(int128_t number, int128_t modulo)
{
    int128_t oldR = number, r = modulo;
    int128_t oldT = 1, t = 0;

    while (r != 0)
    {
        int128_t q = oldR / r;
        int128_t nextR = oldR - q * r;
        int128_t nextT = oldT - q * t;
        oldR = r;
        r = nextR;
        oldT = t;
        t = nextT;
    }

    if (oldR != 1)
        return -1;
    return (oldT % modulo + modulo) % modulo;
}

typedef struct
{
    int128_t x;
    int128_t a;
    int128_t b;
} Walk;

void step(Walk *walk, int128_t generator, int128_t target,
          int128_t prime, int128_t order)
{
    int part = (int)(walk->x % 3);
    if (part == 0)
    {
        walk->x = multiply_mod(walk->x, target, prime);
        walk->b = (walk->b + 1) % order;
    }
    else if (part == 1)
    {
        walk->x = multiply_mod(walk->x, walk->x, prime);
        walk->a = (walk->a * 2) % order;
        walk->b = (walk->b * 2) % order;
    }
    else
    {
        walk->x = multiply_mod(walk->x, generator, prime);
        walk->a = (walk->a + 1) % order;
    }
}

int128_t random_below(int128_t limit)
{
    int128_t value = (int128_t)rand() * ((int128_t)RAND_MAX + 1) + rand();
    return value % limit;
}

int128_t solve_collision(Walk slow, Walk fast, int128_t generator,
                         int128_t target, int128_t prime, int128_t order)
{
    int128_t coefficient = (slow.b - fast.b + order) % order;
    int128_t right = (fast.a - slow.a + order) % order;
    int128_t divisor = gcd128(coefficient, order);

    if (coefficient == 0 || right % divisor != 0 || divisor > 1000)
        return -1;

    int128_t reducedOrder = order / divisor;
    int128_t inverse = inverse_mod(coefficient / divisor, reducedOrder);
    if (inverse == -1)
        return -1;

    int128_t first = multiply_mod(right / divisor, inverse, reducedOrder);
    for (int128_t k = 0; k < divisor; k++)
    {
        int128_t candidate = first + k * reducedOrder;
        if (power_mod(generator, candidate, prime) == target)
            return candidate;
    }
    return -1;
}

int128_t pollard_rho(int128_t generator, int128_t target, int128_t prime)
{
    int128_t order = prime - 1;
    if (target == 1)
        return 0;

    while (1)
    {
        Walk slow, fast;
        slow.a = random_below(order);
        slow.b = random_below(order);
        slow.x = multiply_mod(power_mod(generator, slow.a, prime),
                              power_mod(target, slow.b, prime), prime);
        fast = slow;

        do
        {
            step(&slow, generator, target, prime, order);
            step(&fast, generator, target, prime, order);
            step(&fast, generator, target, prime, order);
        } while (slow.x != fast.x);

        int128_t answer = solve_collision(slow, fast, generator,
                                          target, prime, order);
        if (answer >= 0)
            return answer;
    }
}

int main(int argc, char *argv[])
{
    if (argc != 4)
    {
        printf("Usage: ./pra generatorfile targetfile primefile\n");
        return 1;
    }

    int generatorLen, targetLen, primeLen;
    unsigned char *generatorText = Read_File(argv[1], &generatorLen);
    unsigned char *targetText = Read_File(argv[2], &targetLen);
    unsigned char *primeText = Read_File(argv[3], &primeLen);
    int128_t generator = str_to_int128(generatorText, generatorLen);
    int128_t target = str_to_int128(targetText, targetLen);
    int128_t prime = str_to_int128(primeText, primeLen);

    if (prime < 2 || generator <= 0 || generator >= prime ||
        target <= 0 || target >= prime)
    {
        printf("Invalid input.\n");
        return 1;
    }

    srand((unsigned int)time(NULL));
    int128_t answer = pollard_rho(generator, target, prime);
    unsigned char answerText[50];
    int128_to_str(answer, answerText, sizeof(answerText));
    Write_File("Exponent.txt", (char *)answerText);
    Write_File("Result.txt", (char *)answerText);

    free(generatorText);
    free(targetText);
    free(primeText);
    return 0;
}
