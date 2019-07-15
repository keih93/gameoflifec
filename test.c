#include <omp.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
  int i = 10;
  int sum;
  #pragma omp parallel private(i) reduction( +:sum)
  {
    i = 1000 + omp_get_thread_num();
    sum = omp_get_thread_num();
    printf("thread %d: i = %d\n", omp_get_thread_num(), i);
  }
  printf("i = %d, sum = %d\n", i, sum);

  return 0;
}
