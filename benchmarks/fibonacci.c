#include <stdio.h>
#include <time.h>

int fib(int n) {
  if (n<2)
    return n;
  return fib(n-2) + fib(n-1);
}

int main() {
  double start = (double)clock() / CLOCKS_PER_SEC;
  printf("%d\n", fib(35));
  double end = (double)clock() / CLOCKS_PER_SEC;
  printf("%f\n", end - start);
}