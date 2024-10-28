#include <stdio.h>
#include "dummy_main.h"

int main(int argc, char **argv)
{
    int n;
    long int factorial = 1;
    n = 20;
    for (int i = 1; i <= n; i++)
    {
        factorial *= i;
    }
    printf("Factorial of %d = %ld\n", n, factorial);
    return 0;
}