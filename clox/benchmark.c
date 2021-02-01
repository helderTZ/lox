#include "benchmark.h"

void start() {
  G_start = clock();
}

void end() {
  G_end = clock();
}

float get_duration_s() {
  return ((double) (G_end - G_start)) / CLOCKS_PER_SEC;
}

float get_duration_ms() {
  return get_duration_s() * 1000;
}

float get_duration_us() {
  return get_duration_ms() * 1000;
}

void report() {
  printf("Took %.f us.\n", get_duration_us());
}

float median(float* durations, int size) {
  if (!size % 2) return durations[size/2];
  return (durations[size/2] + durations[size/2+1]) / 2;
}