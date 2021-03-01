#ifndef CPPLOX_CHUNK_H
#define CPPLOX_CHUNK_H

#include "common.h"
#include "value.h"

#include <vector>

enum OpCode : uint8_t {
    OP_CONSTANT,
    OP_ADD,
    OP_SUBTRACT,
    OP_MULTIPLY,
    OP_DIVIDE,
    OP_NEGATE,
    OP_RETURN,
};


class Chunk {
public:

    Chunk();
    void writeChunk(uint8_t byte, int line);
    int addConstant(Value value);

    void disassembleChunk(const char* name) const;
    int disassembleInstruction(int offset) const;
    int simpleInstruction(const char* name, int offset) const;
    int constantInstruction(const char* name, int offset) const;

    static constexpr int initialVectorSize = 8;
    std::vector<uint8_t> code;
    std::vector<Value> constants;
    std::vector<int> lines;
};

#endif // CPPLOX_CHUNK_H