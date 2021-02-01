#ifndef clox_memory_h
#define clox_memory_h

#include "common.h"

/**
 * GROW_STACK() is used for the VM stack
 * and is basically a GROW_CAPACITY() that
 * increases faster (starts bigger).
 */

#define GROW_CAPACITY(capacity) \
    ((capacity) < 8 ? 8 : (capacity) * 2)

#define GROW_STACK(capacity) \
    ((capacity) < 256 ? 256 : (capacity) * 2)

#define GROW_ARRAY(type, pointer, oldCount, newCount) \
    (type*)reallocate(pointer, sizeof(type) * (oldCount), \
        sizeof(type) * (newCount))

#define FREE_ARRAY(type, pointer, oldCount) \
    reallocate(pointer, sizeof(type) * (oldCount), 0)

void* reallocate(void* pointer, size_t oldSize, size_t newSize);

#endif