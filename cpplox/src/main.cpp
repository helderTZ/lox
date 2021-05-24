#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#include <sysexits.h>

#include "common.h"
#include "chunk.h"
#include "vm.h"

static void repl(VM* vm);
static void runFile(VM* vm, const char* path);

int main(int argc, const char* argv[]) {

    VM vm;

    if (argc == 1) {
        repl(&vm);
    } else if (argc == 2) {
        runFile(&vm, argv[1]);
    } else {
        fprintf(stderr, "Usage: clox [path]\n");
        exit(64);
    }

    Chunk chunk;

    int constant = chunk.addConstant(1.2);
    chunk.writeChunk(OpCode::OP_CONSTANT, 123);
    chunk.writeChunk(constant, 123);

    constant = chunk.addConstant(3.4);
    chunk.writeChunk(OpCode::OP_CONSTANT, 123);
    chunk.writeChunk(constant, 123);

    chunk.writeChunk(OP_ADD, 123);

    constant = chunk.addConstant(5.6);
    chunk.writeChunk(OpCode::OP_CONSTANT, 123);
    chunk.writeChunk(constant, 123);

    chunk.writeChunk(OpCode::OP_DIVIDE, 123);
    chunk.writeChunk(OpCode::OP_NEGATE, 123);

    chunk.writeChunk(OpCode::OP_RETURN, 123);

    chunk.disassembleChunk("test chunk");
    return 0;
}

static void repl(VM* vm) {
    std::string line;
    for (;;) {
        std::cout << "> ";
        if (!std::getline(std::cin, line)) {
            std::cout << '\n';
            break;
        }
        vm->interpret(line.c_str());
    }
}

static void runFile(VM* vm, const char* path) {
    std::ifstream t(path);
    std::stringstream buffer;
    buffer << t.rdbuf();

    InterpretResult result = vm->interpret(buffer.str().c_str());

    if (result == INTERPRET_COMPILE_ERROR) exit(EX_DATAERR);
    if (result == INTERPRET_RUNTIME_ERROR) exit(EX_SOFTWARE);
}
