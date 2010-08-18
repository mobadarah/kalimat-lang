/**************************************************************************
**   The Kalimat programming language
**   Copyright (C) 2010 Mohamed Samy Ali - samy2004@gmail.com
**   This project is released under the Apache License version 2.0
**   as described in the included license.txt file
**************************************************************************/

#ifndef ALLOCATOR_H
#define ALLOCATOR_H

#include "value.h"

const int MEGA = 1024 * 1024;
const double GC_FACTOR = 0.75;
const double HEAP_GROWTH_FACTOR = 1.4;
const int NORMAL_MAX_HEAP = 1*MEGA;
class Allocator
{
    QSet<Value *> heap;
    int objsDeleted;
    unsigned int currentAllocationInBytes;
    unsigned int maxAllocationInBytes;

    // Store VM root objects for GC
    QMap<QString, Value*> *constantPool;
    QStack<Frame> *stack;
public:
    Allocator(QMap<QString, Value*> *constantPool,
              QStack<Frame> *stack);

    void gc();

    Value *newInt(int i);
    Value *newInt(int i, bool gcMonitor);
    Value *newDouble(double d);
    Value *newDouble(double d, bool gcMonitor);
    Value *newObject(ValueClass *);
    Value *newString(QString *);
    Value *newObject(Object *, ValueClass *);
    Value *newArray(int size);
    Value *newMultiDimensionalArray(QVector<int> dimensions);
    Value *newRaw(void*, ValueClass *);
    Value *newFieldReference(Object *obj, QString SymRef);
    Value *newArrayReference(VArray *array, int index);
    Value *newMultiDimensionalArrayReference(MultiDimensionalArray<Value *> *array, QVector<int> index);
    Value *null();

    void InitObjectLayout(Object *object, ValueClass *_class);

private:
    void mark();
    void sweep();

    Value *allocateNewValue(bool gcMonitor=true);
};

#endif // ALLOCATOR_H
