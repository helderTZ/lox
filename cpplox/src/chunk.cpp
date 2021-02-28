#include "chunk.h"

#include <cstdio>

Chunk::Chunk() {
    code.reserve(initialVectorSize);
}

void Chunk::writeChunk(uint8_t byte) {
    code.push_back(byte);
}

void Chunk::dissasembleChunk(const char* name) const {
    printf("== %s ==\n", name);

    for (int offset = 0; offset < (int)code.size();) {
        offset = dissasembleInstruction(offset);
    }
}

int Chunk::dissasembleInstruction(int offset) const {
    printf("%04d ", offset);

    uint8_t instruction = code[offset];
    switch (instruction) {
        case OP_RETURN: return simpleInstruction("OP_RETURN", offset);
        default:
            printf("Unknown opcode %d\n", instruction);
            return offset + 1;
    }
}

int Chunk::simpleInstruction(const char* name, int offset) {
    printf("%s\n", name);
    return offset + 1;
}