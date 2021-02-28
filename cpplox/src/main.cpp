#include "common.h"
#include "chunk.h"

int main(int argc, const char* argv[]) {
    (void) argc; (void) argv;

    Chunk chunk;
    chunk.writeChunk(OpCode::OP_RETURN);

    chunk.dissasembleChunk("test chunk");
    return 0;
}