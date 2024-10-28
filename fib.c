#include <stdio.h>
#include "dummy_main.h"
#include <stdlib.h>

int fib(int n)
{
  if (n < 2)
    return n;
  else
    return fib(n - 1) + fib(n - 2);
}

int main(int argc, char **argv)
{
  int n = 45;
  int val = fib(n);
  printf("Fib(%d) = %d\n", n, val);
}