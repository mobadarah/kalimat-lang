/**************************************************************************
**   The Kalimat programming language
**   Copyright (C) 2010 Mohamed Samy Ali - samy2004@gmail.com
**   This project is released under the Apache License version 2.0
**   as described in the included license.txt file
**************************************************************************/

#ifndef ALLOCATOR_H
#define ALLOCATOR_H

#ifndef VALUE_H
    #include "value.h"
#endif

#ifndef REFERENCES_H
    #include "references.h"
#endif

#ifndef FRAME_H
    #include "frame.h"
#endif

#ifndef PROCESS_H
    #include "process.h"
#endif

#include <QStack>
#include <QQueue>

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

    static Value * _true;
    static Value * _false;
    static Value * _ints[];

    // Store VM root objects for GC
    QHash<int, Value*> *constantPool;
    QQueue<Process *> *processes;
    QSet<Frame *> otherFrames;
public:
    Allocator(QHash<int, Value*> *constantPool,
              QQueue<Process *> *processes);

    void addOtherFrameAsRoot(Frame *f) { otherFrames.insert(f); }
    void makeGcMonitored(Value *v);
    void gc();

    Value *newInt(int i);
    Value *newInt(int i, bool gcMonitor);
    Value *newLong(long i);
    Value *newLong(long i, bool gcMonitor);
    Value *newDouble(double d);
    Value *newDouble(double d, bool gcMonitor);
    Value *newBool(bool b);
    Value *newBool(bool b, bool gcMonitor);
    Value *newObject(IClass *);
    Value *newString(QString, bool gcMonitor);
    Value *newString(QString);
    Value *newObject(IObject *, IClass *, bool gcMonitor = true);
    Value *newArray(int size);
    Value *newMap();
    Value *newMultiDimensionalArray(QVector<int> dimensions);
    Value *newRaw(void*, IClass *);
    Value *newFieldReference(IObject *obj, QString SymRef);
    Value *newArrayReference(VArray *array, int index);
    Value *newMultiDimensionalArrayReference(MultiDimensionalArray<Value *> *array, QVector<int> index);
    Value *newChannel(bool gc = true);
    Value *newQObject(QObject *qobj);
    Value *null();

private:
    void mark();
    void sweep();

    Value *allocateNewValue(bool gcMonitor=true);
};

#endif // ALLOCATOR_H
