#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#define N 100000
int main(int argc, char *argv[]) {
  int i, a[N];
  #pragma omp parallel for
  for (i = 0; i < N; i++) a[i] = rand();

  #pragma omp parallel for
  for (i = 0; i < N; i++) {
    // calc ( a , i ) ;
    printf("a[%d]=%d\n", i, a[i]);
  }

  return 0;
}
