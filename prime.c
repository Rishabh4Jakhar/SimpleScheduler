#include <stdio.h>
#include <math.h>

int main()
{
    int n = 63;
    int f = 1;
    if (n == 1 || n == 0)
    {
        printf("%d is neither prime nor composite", n);
        f = 0;
    }
    for (int i = 2; i < n; i++)
    {
        if (n % i == 0)
        {
            printf("%d is not prime\n", n);
            f = 0;
            break;
        }
    }
    if (f == 1)
    {
        printf("%d is prime\n", n);
    }
    return 0;
}