#include <stdio.h>
#include <stdlib.h>

unsigned long long gcd(unsigned long long a, unsigned long long b)
{
    while (b != 0)
    {
        unsigned long long remainder = a % b;
        a = b;
        b = remainder;
    }
    return a;
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        printf("Usage: ./gcd firstnumber secondnumber\n");
        return 1;
    }

    unsigned long long a = strtoull(argv[1][0] == '-' ? argv[1] + 1 : argv[1], NULL, 10);
    unsigned long long b = strtoull(argv[2][0] == '-' ? argv[2] + 1 : argv[2], NULL, 10);
    printf("%llu", gcd(a, b));
    return 0;
}
