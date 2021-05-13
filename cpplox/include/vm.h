#ifndef CPPLOX_VM_H
#define CPPLOX_VM_H

#include "chunk.h"
#include "stack.h"

#include <memory>
#include <vector>
#include <iterator>

enum InterpretResult : uint8_t {
    INTERPRET_OK,
    INTERPRET_COMPILE_ERROR,
    INTERPRET_RUNTIME_ERROR
};

class VM {
public:

    InterpretResult interpret(const char* source);
    InterpretResult run();

    static constexpr int stackMax = 256;
    std::shared_ptr<Chunk> chunk;
    uint8_t* ip;
    Stack<Value> stack;
};

#endif // CPPLOX_VM_H
