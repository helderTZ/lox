#include <stdio.h>

#include "debug.h"
#include "value.h"

// forward declarations
static int simpleInstruction(const char* name, int offset);
static int constantInstruction(const char* name, Chunk* chunk, int offset);
static int longConstantInstruction(const char* name, Chunk* chunk, int offset);
static int byteInstruction(const char* name, Chunk* chunk, int offset);
static int jumpInstruction(const char* name, int sign, Chunk* chunk, int offset);

void inspectTable(Table* table) {
  printf("  count: %d, capacity: %d\n", table->count, table->capacity);
  for (int i = 0; i < table->capacity; ++i) {
    Entry* e = &table->entries[i];
    if (e->key != NULL) {
      printf("  [ %04d : %.*s ==> ", i, e->key->length, e->key->chars);
      printValue(e->value);
      printf(" ]\n");
    }
  }
}

void inspectStack(VM* vm) {
  printf("          ");
    for (Value* slot = vm->stack; slot < vm->stackTop; slot++) {
      printf("[ ");
      printValue(*slot);
      printf(" ]");
    }
    printf("\n");
}

void inspectVm(VM* vm) {
  printf("Table strings:\n");
  inspectTable(&vm->strings);
  printf("Table globals:\n");
  inspectTable(&vm->globals);
}

void disassembleChunk(Chunk* chunk, const char* name) {
  int chars_printed = printf("==== %s ====\n", name);

  for (int offset = 0; offset < chunk->count;) {
    offset = disassembleInstruction(chunk, offset);
  }
  for(int i = chars_printed; i > 0; i--) printf("=");
  printf("\n");
}

int disassembleInstruction(Chunk* chunk, int offset) {
  printf("%04d ", offset);

  int line = getLine(chunk, offset);
  if (offset > 0 && line == getLine(chunk, offset - 1)) {
    printf("   | ");
  } else {
    printf("%4d ", line);
  }

  uint8_t instruction = chunk->code[offset];
  switch (instruction) {
    case OP_CONSTANT:      return constantInstruction("OP_CONSTANT", chunk, offset);
    case OP_CONSTANT_LONG: return longConstantInstruction("OP_CONSTANT_LONG", chunk, offset);
    case OP_NIL:           return simpleInstruction("OP_NIL", offset);
    case OP_TRUE:          return simpleInstruction("OP_TRUE", offset);
    case OP_FALSE:         return simpleInstruction("OP_FALSE", offset);
    case OP_ZERO:          return simpleInstruction("OP_ZERO", offset);
    case OP_ONE:           return simpleInstruction("OP_ONE", offset);
    case OP_POP:           return simpleInstruction("OP_POP", offset);
    case OP_POPN:          return byteInstruction("OP_POPN", chunk, offset);
    case OP_GET_LOCAL:     return byteInstruction("OP_GET_LOCAL", chunk, offset);
    case OP_SET_LOCAL:     return byteInstruction("OP_SET_LOCAL", chunk, offset);
    case OP_GET_GLOBAL:    return constantInstruction("OP_GET_GLOBAL", chunk, offset);
    case OP_DEFINE_GLOBAL: return constantInstruction("OP_DEFINE_GLOBAL", chunk, offset);
    case OP_SET_GLOBAL:    return constantInstruction("OP_SET_GLOBAL", chunk, offset);
    case OP_EQUAL:         return simpleInstruction("OP_EQUAL", offset);
    case OP_NEQUAL:        return simpleInstruction("OP_NEQUAL", offset);
    case OP_GREATER:       return simpleInstruction("OP_GREATER", offset);
    case OP_LESS:          return simpleInstruction("OP_LESS", offset);
    case OP_ADD:           return simpleInstruction("OP_ADD", offset);
    case OP_SUBTRACT:      return simpleInstruction("OP_SUBTRACT", offset);
    case OP_MULTIPLY:      return simpleInstruction("OP_MULTIPLY", offset);
    case OP_DIVIDE:        return simpleInstruction("OP_DIVIDE", offset);
    case OP_NOT:           return simpleInstruction("OP_NOT", offset);
    case OP_NEGATE:        return simpleInstruction("OP_NEGATE", offset);
    case OP_JUMP:          return jumpInstruction("OP_JUMP", 1, chunk, offset);
    case OP_JUMP_IF_FALSE: return jumpInstruction("OP_JUMP_IF_FALSE", 1, chunk, offset);
    case OP_LOOP:          return jumpInstruction("OP_LOOP", -1, chunk, offset);
    case OP_RETURN:        return simpleInstruction("OP_RETURN", offset);
    case OP_PRINT:         return simpleInstruction("OP_PRINT", offset);
    default:
      printf("Unknown opcode %d\n", instruction);
      return offset + 1;
  }
}

static int simpleInstruction(const char* name, int offset) {
  printf("%s\n", name);
  return offset + 1;
}

static int byteInstruction(const char* name, Chunk* chunk, int offset) {
  uint8_t slot = chunk->code[offset + 1];
  printf("%-16s %4d\n", name, slot);
  return offset + 2; 
}

static int jumpInstruction(const char* name, int sign, Chunk* chunk, int offset) {
  uint16_t jump = (uint16_t)(chunk->code[offset + 1] << 8);
  jump |= chunk->code[offset + 2];
  printf("%-16s %4d -> %d\n", name, offset,
         offset + 3 + sign * jump);
  return offset + 3;
}

static int constantInstruction(const char* name, Chunk* chunk, int offset) {
  uint8_t index = chunk->code[offset + 1];
  printf("%-16s %4d ", name, index);
  printValue(chunk->constants.values[index]);
  printf("\n");
  return offset + 2;
}

static int longConstantInstruction(const char* name, Chunk* chunk, int offset) {
  uint32_t index = (uint32_t) (((uint32_t)(chunk->code[offset + 1] << 16)
                            |   (uint32_t)(chunk->code[offset + 2] << 8))
                            |   (uint32_t)(chunk->code[offset + 3]));
  printf("%-16s %4d ", name, index);
  printValue(chunk->constants.values[index]);
  printf("\n");
  return offset + 4;
}