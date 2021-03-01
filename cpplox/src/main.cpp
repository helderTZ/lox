#include "common.h"
#include "chunk.h"

int main(int argc, const char* argv[]) {
    (void) argc; (void) argv;

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