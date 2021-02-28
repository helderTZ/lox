#ifndef CPPLOX_CHUNK_H
#define CPPLOX_CHUNK_H

#include "common.h"

#include <vector>

enum OpCode : uint8_t {
    OP_RETURN,
};


class Chunk {
public:

    Chunk();
    void writeChunk(uint8_t byte);

    void dissasembleChunk(const char* name) const;
    int dissasembleInstruction(int offset) const;
    static int simpleInstruction(const char* name, int offset);

    static constexpr int initialVectorSize = 8;
    std::vector<uint8_t> code;
};

#endif // CPPLOX_CHUNK_H