#ifndef CPPLOX_VM_H
#define CPPLOX_VM_H

#include "chunk.h"
#include "stack.h"

#include <stack>

enum InterpretResult : uint8_t {
    INTERPRET_OK,
    INTERPRET_COMPILE_ERROR,
    INTERPRET_RUNTIME_ERROR
};

class VM {
public:

    InterpretResult interpret(Chunk* chunk);
    InterpretResult run();

    static constexpr int stackMax = 256;
    Chunk* chunk;
    uint8_t* ip;
    Stack<Value> stack;
};

#endif // CPPLOX_VM_H
