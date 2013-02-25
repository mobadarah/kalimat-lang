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

#ifndef SCHEDULER_H
    #include "scheduler.h"
#endif

#ifndef PROCESSITERATOR_H
    #include "processiterator.h"
#endif

#include <QStack>
#include <QQueue>
#include <QMutex>

template <class T, int ChunkSize> struct AllocationNode
{
    T data[ChunkSize];
    AllocationNode<T, ChunkSize> *next;
    AllocationNode(AllocationNode<T, ChunkSize> *next)
        :next(next)
    {

    }
};

template <class T, int ChunkSize> struct StaticAllocator
{
    AllocationNode<T,ChunkSize> *list;
    int count;
    StaticAllocator()
        :count(0)
    {
        list = new AllocationNode<T, ChunkSize>(NULL);
    }

    T &add()
    {
        if(count == ChunkSize)
        {
            list = new AllocationNode<T, ChunkSize>(list);
            count = 0;
        }
        return list->data[count++];
    }
    ~StaticAllocator()
    {
        while(list != NULL)
        {
            AllocationNode<T, ChunkSize> *temp = list->next;
            delete list;
            list = temp;
        }
    }
};

class Allocator
{
    Value *heap;
    QSet<Value *> protectedValues; // if it's here, it's reachable
                                   // the GC can't do nothin'
    int objsDeleted;
    unsigned int currentAllocationInBytes;
    unsigned int maxAllocationInBytes;

    Value * _true;
    Value * _false;
    IntVal * _ints;

    // Store VM root objects for GC
    QHash<int, Value*> *constantPool;
    QSet<Scheduler *> schedulers;
    QSet<QMap<QString, Value *> *> otherFrames;

    // only for stoptheworld and starttheworld
    VM *vm;

    // We can't have more than one thread trying to GC at the same time
    QMutex gcLock;

    // The heap is shared, no more than one thread can allocate
    QMutex heapAllocationLock;

    StaticAllocator<IntVal, 120> intAllocator;
    IntVal *intFreeList;
public:
    Allocator(QHash<int, Value*> *constantPool,
              QSet<Scheduler *> schedulers, VM *vm);

    ~Allocator();

    void addOtherFrameAsRoot(QMap<QString, Value *> *f) { otherFrames.insert(f); }
    void makeGcMonitored(Value *v);
    void stopGcMonitoring(Value *v);
    void gc();

    Value *newInt(int i);
    Value *zero();
    Value *one();
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
    void markProcess(Process * proc, QStack<Value *> &reachable);
    void sweep();
    void deleteValue(Value *v);

    template<class VType, class InitialType> Value *allocateNewValue(InitialType, bool gcMonitor=true);
};

#endif // ALLOCATOR_H
