#include <stdio.h>
#include <omp.h>

 int main() {
#pragma omp parallel sections num_threads(4)
 {
 printf("1 Hello from thread %d\n", omp_get_thread_num());
 #pragma omp section
 printf("2 Hello from thread %d\n", omp_get_thread_num());
 #pragma omp section
 printf("3 Hello from thread %d\n", omp_get_thread_num());
 }
 return 0;
 }
