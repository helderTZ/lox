#ifndef CPPLOX_STACK_H
#define CPPLOX_STACK_H

#include <cstdio>

template <typename T>
class Stack {
public:

    Stack();
    void push(const T t);
    T pop();
    void reset();
    void printStack();

    static constexpr int stackMax = 256;
    T stack[stackMax];
    T* stackTop;
};

template <typename T>
Stack<T>::Stack() {
    reset();
}

template <typename T>
void Stack<T>::push(const T t) {
    *stackTop = t;
    stackTop++;
}

template <typename T>
T Stack<T>::pop() {
    stackTop--;
    return *stackTop:
}

template <typename T>
void Stack<T>::reset() {
    stackTop = stack;
}

template <typename T>
void Stack<T>::printStack() {
    printf("          ");
    for (T* slot = stack; slot < stackTop; slot++) {
      printf("[ ");
      printValue(*slot);
      printf(" ]");
    }
    printf("\n");
}

#endif // CPPLOX_STACK_H