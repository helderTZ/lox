#include <time.h>
#include <stdio.h>

#define STRINGIGY(f) #f

clock_t G_start, G_end;

void start();
void end();
float get_duration_s();
float get_duration_ms();
float get_duration_us();
void report();
float median(float* durations, int size);