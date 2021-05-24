#include "chunk.h"

#include <cstdio>

Chunk::Chunk() {
    code.reserve(initialVectorSize);
    constants.reserve(initialVectorSize);
    lines.reserve(initialVectorSize);
}

void Chunk::writeChunk(uint8_t byte, int line) {
    code.push_back(byte);
    lines.push_back(line);
}

int Chunk::addConstant(Value value) {
    constants.push_back(value);
    return constants.size() - 1;
}

void Chunk::disassembleChunk(const char* name) const {
    printf("== %s ==\n", name);

    for (int offset = 0; offset < (int)code.size();) {
        offset = disassembleInstruction(offset);
    }
}

int Chunk::disassembleInstruction(int offset) const {
    printf("%04d ", offset);

    if (offset > 0 && lines[offset] == lines[offset-1]) {
        printf("   | ");
    } else {
        printf("%4d ", lines[offset]);
    }

    uint8_t instruction = code[offset];
    switch (instruction) {
        case OP_CONSTANT:   return constantInstruction("OP_CONSTANT", offset);
        case OP_ADD:        return simpleInstruction("OP_ADD", offset);
        case OP_SUBTRACT:   return simpleInstruction("OP_SUBTRACT", offset);
        case OP_MULTIPLY:   return simpleInstruction("OP_MULTIPLY", offset);
        case OP_DIVIDE:     return simpleInstruction("OP_DIVIDE", offset);
        case OP_NEGATE:     return simpleInstruction("OP_NEGATE", offset);
        case OP_RETURN:     return simpleInstruction("OP_RETURN", offset);
        default:
            printf("Unknown opcode %d\n", instruction);
            return offset + 1;
    }
}

int Chunk::simpleInstruction(const char* name, int offset) const {
    printf("%s\n", name);
    return offset + 1;
}

int Chunk::constantInstruction(const char* name, int offset) const {
    uint8_t constant = code[offset + 1];
    printf("%-16s %4d '", name, constant);
    printValue(constants[constant]);
    printf("'\n");
    return offset + 2;
}