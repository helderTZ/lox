#include "vm.h"
#include "compiler.h"

#include <cstdio>
#include <memory>

InterpretResult VM::interpret(const char* source) {
    chunk = std::make_shared<Chunk>();
    Compiler compiler;

    auto err = compiler.compile(source, chunk);
    if(!err) {
        return InterpretResult::INTERPRET_COMPILE_ERROR;
    }

    ip = chunk->code.begin();

    return run();
}

InterpretResult VM::run() {
#define READ_BYTE() (*ip++)
#define READ_CONSTANT() (chunk->constants[READ_BYTE()])
#define BINARY_OP(op)           \
    do {                        \
      double b = stack.pop();   \
      double a = stack.pop();   \
      stack.push(a op b);       \
    } while (false)

#ifdef DEBUG_TRACE_EXECUTION
    stack.printStack();
    chunk->disassembleInstruction((int)(ip - &chunk->code[0]));
#endif
    
    for(;;) {
        OpCode instruction;
        switch (instruction = static_cast<OpCode>(READ_BYTE())) {
            case OP_CONSTANT: {
                Value constant = READ_CONSTANT();
                stack.push(constant);
                break;
            }
            case OP_ADD:      BINARY_OP(+); break;
            case OP_SUBTRACT: BINARY_OP(-); break;
            case OP_MULTIPLY: BINARY_OP(*); break;
            case OP_DIVIDE:   BINARY_OP(/); break;
            case OP_NEGATE:   stack.push(-stack.pop()); break;
            case OP_RETURN: {
                printValue(stack.pop());
                printf("\n");
                return InterpretResult::INTERPRET_OK;
            }
        }
    }

    return InterpretResult::INTERPRET_RUNTIME_ERROR;

#undef READ_BYTE
#undef READ_CONSTANT
#undef BINARY_OP
}