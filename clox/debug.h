#ifndef clox_debug_h
#define clox_debug_h

#include "chunk.h"
#include "vm.h"
#include "table.h"
#include "object.h"

void disassembleChunk(Chunk* chunk, const char* name);
int disassembleInstruction(Chunk* chunk, int offset);
void inspectVm(VM* vm);
void inspectStack(VM* vm);
void inspectTable(Table* table);
void inspectChunk(Chunk* chunk);

#endif