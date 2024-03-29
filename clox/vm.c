#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "common.h"
#include "compiler.h"
#include "debug.h"
#include "object.h"
#include "memory.h"
#include "vm.h"

VM vm;

static Value clockNative(int argCount, Value* args) {
  return NUMBER_VAL((double)clock() / CLOCKS_PER_SEC);
}

static void resetStack() {
  vm.stackTop = vm.stack;
  vm.frameCount = 0;
  vm.openUpvalues = NULL;
  vm.objects = NULL;
}

static void runtimeError(const char* format, ...) {
  va_list args;
  va_start(args, format);
  vfprintf(stderr, format, args);
  va_end(args);
  fputs("\n", stderr);

  // print stack trace
  for (int i = vm.frameCount - 1; i >= 0; i--) {
    CallFrame* frame = &vm.frames[i];
    ObjFunction* function = frame->closure->function;
    // -1 because the IP is sitting on the next instruction to be executed.
    size_t instruction = frame->ip - function->chunk.code - 1;
    int line = getLine(&function->chunk, instruction);
    fprintf(stderr, "[line %d] in ", line);
    if (function->name == NULL) {
      fprintf(stderr, "script\n");
    } else {
      fprintf(stderr, "%s()\n", function->name->chars);
    }
  }

  resetStack();
}

static void defineNative(const char* name, NativeFn function) {
  push(OBJ_VAL(copyString(name, (int)strlen(name))));
  push(OBJ_VAL(newNative(function)));
  tableSet(&vm.globals, AS_STRING(vm.stack[0]), vm.stack[1]);
  pop();
  pop();
}

void initVM() {
  resetStack();
  initTable(&vm.globals);
  initTable(&vm.strings);

  defineNative("clock", clockNative);
}

void freeVM() {
  freeTable(&vm.globals);
  freeTable(&vm.strings);
  freeObjects();
}

void push(Value value) {
  *vm.stackTop = value;
  vm.stackTop++;
}

Value pop() {
  vm.stackTop--;
  return *vm.stackTop;
}

static Value peek(int distance) {
  return vm.stackTop[-1 - distance];
}

static bool call(ObjClosure* closure, int argCount) {
  if (argCount != closure->function->arity) {
    runtimeError("Expected %d arguments but got %d.",
        closure->function->arity, argCount);
    return false;
  }

  if (vm.frameCount == FRAMES_MAX) {
    runtimeError("Stack overflow.");
    return false;
  }

  CallFrame* frame = &vm.frames[vm.frameCount++];
  frame->closure = closure;
  frame->ip = closure->function->chunk.code;
  frame->slots = vm.stackTop - argCount - 1;
  return true;
}


static bool callValue(Value callee, int argCount) {
  if (IS_OBJ(callee)) {
    switch (OBJ_TYPE(callee)) {
      case OBJ_CLOSURE: 
        return call(AS_CLOSURE(callee), argCount);
      case OBJ_NATIVE: {
        NativeFn native = AS_NATIVE(callee);
        Value result = native(argCount, vm.stackTop - argCount);
        vm.stackTop -= argCount + 1;
        push(result);
        return true;
      }
      default:
        // Non-callable object type.
        break;
    }
  }

  runtimeError("Can only call functions and classes.");
  return false;
}

static ObjUpvalue* captureUpvalue(Value* local) {
  ObjUpvalue* prevUpvalue = NULL;
  ObjUpvalue* upvalue = vm.openUpvalues;

  while (upvalue != NULL && upvalue->location > local) {
    prevUpvalue = upvalue;
    upvalue = upvalue->next;
  }

  if (upvalue != NULL && upvalue->location == local) {
    return upvalue;
  }

  ObjUpvalue* createdUpvalue = newUpvalue(local);
  createdUpvalue->next = upvalue;

  if (prevUpvalue == NULL) {
    vm.openUpvalues = createdUpvalue;
  } else {
    prevUpvalue->next = createdUpvalue;
  }

  return createdUpvalue;
}

static void closeUpvalues(Value* last) {
  while (vm.openUpvalues != NULL &&
         vm.openUpvalues->location >= last) {
    ObjUpvalue* upvalue = vm.openUpvalues;
    upvalue->closed = *upvalue->location;
    upvalue->location = &upvalue->closed;
    vm.openUpvalues = upvalue->next;
  }
}

static bool isFalsey(Value value) {
  return IS_NIL(value) || (IS_BOOL(value) && !AS_BOOL(value));
}

static void concatenate() {
  ObjString* b = AS_STRING(pop());
  ObjString* a = AS_STRING(pop());

  int length = a->length + b->length;
  ObjString* result = makeString(length);
  memcpy(result->chars, a->chars, a->length);
  memcpy(result->chars + a->length, b->chars, b->length);
  result->chars[length] = '\0';

  uint32_t hash = hashString(result->chars, length);
  ObjString* interned = tableFindString(&vm.strings, result->chars, length, hash);
  if (interned == NULL) {
    tableSet(&vm.strings, result, NIL_VAL);
  }

  push(OBJ_VAL(result));
}

static void convertNumStr(double number) {
#define MAX_DIGITS_DOUBLE 24
  char string[24];
  snprintf(string, MAX_DIGITS_DOUBLE, "%g", number);

  int length = strlen(string);
  ObjString* result = makeString(length);
  memcpy(result->chars, string, length);
  result->chars[length] = '\0';

  uint32_t hash = hashString(string, length);
  ObjString* interned = tableFindString(&vm.strings, string, length, hash);
  if (interned == NULL) {
    tableSet(&vm.strings, result, NIL_VAL);
  }

  push(OBJ_VAL(result));
#undef MAX_DIGITS_DOUBLE
}

static InterpretResult run() {
  CallFrame* frame = &vm.frames[vm.frameCount - 1];
  register uint8_t* ip = frame->ip;

#define READ_BYTE()     (*ip++)
#define READ_SHORT()    (ip += 2, (uint16_t)((ip[-2] << 8) | ip[-1]))
#define READ_CONSTANT() (frame->closure->function->chunk.constants.values[READ_BYTE()])
#define READ_STRING()   AS_STRING(READ_CONSTANT())
#define BINARY_OP(valueType, op)                        \
    do {                                                \
      if (!IS_NUMBER(peek(0)) || !IS_NUMBER(peek(1))) { \
        frame->ip = ip;                                 \
        runtimeError("Operands must be numbers.");      \
        return INTERPRET_RUNTIME_ERROR;                 \
      }                                                 \
      double b = AS_NUMBER(pop());                      \
      double a = AS_NUMBER(pop());                      \
      push(valueType(a op b));                          \
    } while (false)

  for (;;) {
#ifdef DEBUG_VM_TABLES
    // inspectVm(&vm);
    // printf("Table globals: "); inspectTable(&vm.globals);
#endif
#ifdef DEBUG_TRACE_EXECUTION
    inspectStack(&vm);
    // inspectChunk(&frame->closure->function->chunk);
    disassembleInstruction(
        &frame->closure->function->chunk,
        (int)(frame->ip - frame->closure->function->chunk.code));
#endif
    uint8_t instruction;
    switch (instruction = READ_BYTE()) {
      case OP_CONSTANT: {
        Value constant = READ_CONSTANT();
        push(constant);
        break;
      }

      case OP_CONSTANT_LONG: {
        uint8_t index_byte1 = READ_BYTE();
        uint8_t index_byte2 = READ_BYTE();
        uint8_t index_byte3 = READ_BYTE();
        uint32_t full_index = index_byte1 << 16 | index_byte2 << 8 | index_byte3;
        Value constant = frame->closure->function->chunk.constants.values[full_index];
        push(constant);
        break;
      }

      case OP_NIL: push(NIL_VAL); break;
      case OP_TRUE: push(BOOL_VAL(true)); break;
      case OP_FALSE: push(BOOL_VAL(false)); break;
      case OP_ZERO: push(NUMBER_VAL(0)); break;
      case OP_ONE: push(NUMBER_VAL(1)); break;
      case OP_POP: pop(); break;

      case OP_POPN: {
        uint8_t pops = READ_BYTE();
        while(pops-- > 0) pop();
        break;
      }

      case OP_DUP: push(peek(0)); break;

      case OP_GET_LOCAL: {
        uint8_t slot = READ_BYTE();
        push(frame->slots[slot]);
        break;
      }

      case OP_SET_LOCAL: {
        uint8_t slot = READ_BYTE();
        frame->slots[slot] = peek(0);
        break;
      }

      case OP_GET_GLOBAL: {
        ObjString* name = READ_STRING();
        Value value;
        if (!tableGet(&vm.globals, name, &value)) {
          frame->ip = ip;
          runtimeError("Undefined variable '%s'.", name->chars);
          return INTERPRET_RUNTIME_ERROR;
        }
        push(value);
        break;
      }

      case OP_DEFINE_GLOBAL: {
        ObjString* name = READ_STRING();
        tableSet(&vm.globals, name, peek(0));
        pop();
        break;
      }

      case OP_SET_GLOBAL: {
        ObjString* name = READ_STRING();
        if (tableSet(&vm.globals, name, peek(0))) {
          tableDelete(&vm.globals, name);
          frame->ip = ip;
          runtimeError("Undefined variable '%s'.", name->chars);
          return INTERPRET_RUNTIME_ERROR;
        }
        break;
      }

      case OP_GET_UPVALUE: {
        uint8_t slot = READ_BYTE();
        push(*frame->closure->upvalues[slot]->location);
        break;
      }

      case OP_SET_UPVALUE: {
        uint8_t slot = READ_BYTE();
        *frame->closure->upvalues[slot]->location = peek(0);
        break;
      }

      case OP_EQUAL: {
        Value b = pop();
        Value a = pop();
        push(BOOL_VAL(valuesEqual(a, b)));
        break;
      }

      case OP_NEQUAL: {
        Value b = pop();
        Value a = pop();
        push(BOOL_VAL(!valuesEqual(a, b)));
        break;
      }

      case OP_GREATER:  BINARY_OP(BOOL_VAL, >); break;
      case OP_LESS:     BINARY_OP(BOOL_VAL, <); break;

      case OP_ADD: {
        if (IS_STRING(peek(0)) && IS_STRING(peek(1))) {
          concatenate();
        } else if (IS_NUMBER(peek(0)) && IS_NUMBER(peek(1))) {
          double b = AS_NUMBER(pop());
          double a = AS_NUMBER(pop());
          push(NUMBER_VAL(a + b));
        } else if (IS_STRING(peek(0)) && IS_NUMBER(peek(1))) {
          Value str = pop();
          double num = AS_NUMBER(pop());
          push(str);
          convertNumStr(num);
          concatenate();
        } else if (IS_NUMBER(peek(0)) && IS_STRING(peek(1))) {
          double num = AS_NUMBER(pop());
          convertNumStr(num);
          concatenate();
        } else {
          frame->ip = ip;
          runtimeError("Operands must be two numbers or two strings.");
          return INTERPRET_RUNTIME_ERROR;
        }
        break;
      }

      case OP_SUBTRACT: BINARY_OP(NUMBER_VAL, -); break;
      case OP_MULTIPLY: BINARY_OP(NUMBER_VAL, *); break;
      case OP_DIVIDE:   BINARY_OP(NUMBER_VAL, /); break;

      case OP_NOT:
        push(BOOL_VAL(isFalsey(pop())));
        break;

      case OP_NEGATE:
        if (!IS_NUMBER(peek(0))) {
          frame->ip = ip;
          runtimeError("Operand must be a number.");
          return INTERPRET_RUNTIME_ERROR;
        }
        // push(NUMBER_VAL(-AS_NUMBER(pop())));
        *(vm.stackTop-1) = NUMBER_VAL(-AS_NUMBER(*(vm.stackTop-1)));
        break;

      case OP_PRINT: {
        printValue(pop());
        printf("\n");
        break;
      }

      case OP_JUMP: {
        uint16_t offset = READ_SHORT();
        ip += offset;
        break;
      }

      case OP_JUMP_IF_FALSE: {
        uint16_t offset = READ_SHORT();
        if (isFalsey(peek(0))) ip += offset;
        break;
      }

      case OP_LOOP: {
        uint16_t offset = READ_SHORT();
        ip -= offset;
        break;
      }

      case OP_CALL: {
        int argCount = READ_BYTE();
        frame->ip = ip;
        if (!callValue(peek(argCount), argCount)) {
          return INTERPRET_RUNTIME_ERROR;
        }
        frame = &vm.frames[vm.frameCount - 1];
        ip = frame->ip;
        break;
      }

      case OP_CLOSURE: {
        ObjFunction* function = AS_FUNCTION(READ_CONSTANT());
        ObjClosure* closure = newClosure(function);
        push(OBJ_VAL(closure));
        for (int i = 0; i < closure->upvalueCount; i++) {
          uint8_t isLocal = READ_BYTE();
          uint8_t index = READ_BYTE();
          if (isLocal) {
            closure->upvalues[i] = captureUpvalue(frame->slots + index);
          } else {
            closure->upvalues[i] = frame->closure->upvalues[index];
          }
        }
        break;
      }

      case OP_CLOSE_UPVALUE:
        closeUpvalues(vm.stackTop - 1);
        pop();
        break;

      case OP_RETURN: {
        Value result = pop();

        closeUpvalues(frame->slots);

        vm.frameCount--;
        if (vm.frameCount == 0) {
          pop();
          return INTERPRET_OK;
        }

        vm.stackTop = frame->slots;
        push(result);

        frame = &vm.frames[vm.frameCount - 1];
        ip = frame->ip;
        break;
      }
    }
  }

#undef READ_BYTE
#undef READ_CONSTANT
#undef READ_SHORT
#undef READ_STRING
#undef BINARY_OP
}

InterpretResult interpret(const char* source) {
  ObjFunction* function = compile(source);
  if (function == NULL) return INTERPRET_COMPILE_ERROR;

  push(OBJ_VAL(function));
  ObjClosure* closure = newClosure(function);
  pop();
  push(OBJ_VAL(closure));
  callValue(OBJ_VAL(closure), 0);

  return run();
}