#ifndef OPERANDSTACK_H
#define OPERANDSTACK_H

#include "stack.h"
#include <QStack>

const int fourK = 4096;
const int ValueSize = 16;

// OperandStackChunkSize _must_ be a power of two!
// or module(...) won't work!!
const int OperandStackChunkSize = fourK / ValueSize;

typedef Stack<Value *, OperandStackChunkSize> VOperandStack;
//typedef QStack<Value *> VOperandStack;

#endif // OPERANDSTACK_H
