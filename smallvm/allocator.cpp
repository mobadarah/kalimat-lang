/**************************************************************************
**   The Kalimat programming language
**   Copyright (C) 2010 Mohamed Samy Ali - samy2004@gmail.com
**   This project is released under the Apache License version 2.0
**   as described in the included license.txt file
**************************************************************************/

#include "vm_incl.h"
#include "allocator.h"
#include "vm.h"

#include <QDebug>
#include <time.h>

const int intCacheSize = 256;
const int MEGA = 1024 * 1024;
const double GC_FACTOR = 0.75;
const double HEAP_GROWTH_FACTOR = 1.4;
const int NORMAL_MAX_HEAP = 1*MEGA;
//const int NORMAL_MAX_HEAP = 1024 * 40;

Allocator::Allocator(QHash<int, Value *> *constantPool, QSet<Scheduler *>schedulers, VM *vm)
    : heap(NULL)
{
    this->constantPool = constantPool;
    this->schedulers = schedulers;
    this->vm = vm;
    currentAllocationInBytes = 0;
    maxAllocationInBytes = NORMAL_MAX_HEAP;

    _true = newBool(true, false);
    _false = newBool(false, false);

    _ints = new IntVal[intCacheSize];
    for(int i=0; i<intCacheSize; i++)
    {
       // _ints[i] = newInt(i, false);
        _ints[i].type = BuiltInTypes::IntType;
        _ints[i].v = i;
    }
    intFreeList = NULL;
}

Allocator::~Allocator()
{
    // farewell, any objects remaining
    // we didn't directly call GC since it would've tried
    // to stop the world
    mark();
    sweep();
    delete _true;
    delete _false;
    /*
    for(int i=0; i<intCacheSize; ++i)
    {
        delete _ints[i];
    }
    //*/
    delete[] _ints;


    int freedInts = 0;
    while(intFreeList != NULL)
    {
        IntVal *tmp = (IntVal *) intFreeList->type;
        delete intFreeList;
        intFreeList = tmp;
        freedInts++;
    }
    // qDebug() << "Freed " << freedInts << " integer objects at end of VM execution";
}

template<class Vtype, class InitialType> Value *Allocator::allocateNewValue(InitialType initVal, bool gcMonitor)
{
    if((currentAllocationInBytes + sizeof(Vtype)) > (maxAllocationInBytes))
    {
        gc();
        if((currentAllocationInBytes + sizeof(Vtype))> (maxAllocationInBytes))
        {
            maxAllocationInBytes *= HEAP_GROWTH_FACTOR;
        }
    }
    else
    {
        double utilization = (double) currentAllocationInBytes / (double) maxAllocationInBytes;
        if(utilization < GC_FACTOR)
        {
            int newSize = (int) (((double) maxAllocationInBytes) *GC_FACTOR);
            if(newSize>=NORMAL_MAX_HEAP)
                maxAllocationInBytes = newSize;
        }
    }
    Value *ret = new Vtype(initVal);
    if(ret!=NULL)
    {
        heapAllocationLock.lock();
        //heap.insert(ret);
        ret->heapNext = heap;
        heap = ret;
        currentAllocationInBytes += sizeof(Vtype);
        if(!gcMonitor)
        {
            protectedValues.insert(ret);
        }
        heapAllocationLock.unlock();
    }
    else
    {
        //TODO: Signal out of memory
    }
    return ret;
}

void Allocator::makeGcMonitored(Value *v)
{
    protectedValues.remove(v);
}

void Allocator::stopGcMonitoring(Value *v)
{
    protectedValues.insert(v);
}

Value *Allocator::newInt(int i)
{
    if(i>=0 && i < intCacheSize)
        return &_ints[i];

    return newInt(i, true);
}

Value *Allocator::newInt(int i, bool gcMonitor=true)
{
    if(intFreeList != NULL)
    {
        IntVal *v = intFreeList;
        intFreeList = (IntVal *) v->type;
        v->type = BuiltInTypes::IntType;
        v->v = i;
        return v;
    }

    //*
    Value *ret = allocateNewValue<IntVal, int>(i, gcMonitor);
    ret->type = BuiltInTypes::IntType;
    return ret;
    //*/

    //*

    /*
    IntVal *ret = &intAllocator.add();
    ret->type = BuiltInTypes::IntType;
    ret->v = i;

    heapAllocationLock.lock();
    //heap.insert(ret);
    ret->heapNext = heap;
    currentAllocationInBytes += sizeof(IntVal);
    if(!gcMonitor)
    {
        protectedValues.insert(ret);
    }
    heapAllocationLock.unlock();
    return ret;
    //*/
}

Value *Allocator::newLong(long i)
{
    return newLong(i, true);
}

Value *Allocator::newLong(long i, bool gcMonitor=true)
{
    Value *ret = allocateNewValue<LongVal, long>(i, gcMonitor);
    ret->type = BuiltInTypes::LongType;
    return ret;
}

Value *Allocator::newDouble(double d)
{
    return newDouble(d, true);
}

Value *Allocator::newDouble(double d, bool gcMonitor)
{
    Value *ret = allocateNewValue<DoubleVal, double>(d, gcMonitor);
    ret->type = BuiltInTypes::DoubleType;
    return ret;
}

Value *Allocator::newBool(bool b)
{
    if(b)
    {
        return _true;
    }
    else
    {
        return _false;
    }
}

Value *Allocator::newBool(bool b, bool gcMonitor)
{
    Value *ret = allocateNewValue<BoolVal, bool>(b, gcMonitor);
    ret->type = BuiltInTypes::BoolType;
    return ret;
}

Value *Allocator::newObject(IClass *_class)
{
    IObject *newObj = _class->newValue(this);

    return newObject(newObj, _class);
}

Value *Allocator::newObject(IObject *newObj, IClass *_class, bool gcMonitor)
{
    Value *ret = allocateNewValue<ObjVal, IObject *>(newObj, gcMonitor);
    ret->type = _class;
    return ret;
}

Value *Allocator::null()
{
    if(Value::NullValue == NULL)
    {
        Value::NullValue = new NullVal();
        Value::NullValue->type = BuiltInTypes::NullType;
    }

    return Value::NullValue;
}

Value *Allocator::newArray(int size)
{
    VArray *varr = new VArray();
    varr->Elements = new Value*[size];
    varr->_count = size;

    for(int i=0; i<size; i++)
        varr->Elements[i] = null();

    Value *ret = allocateNewValue<ArrayVal, VArray *>(varr);
    ret->type = BuiltInTypes::ArrayType;

    return ret;
}

Value *Allocator::newMap()
{
    Value *ret = allocateNewValue<MapVal, VMap *>(new VMap());
    ret->type = BuiltInTypes::MapType;
    return ret;
}

Value *Allocator::newMultiDimensionalArray(QVector<int>dimensions)
{
    MultiDimensionalArray<Value *> *arr = new MultiDimensionalArray<Value *>(dimensions);

    // Init all elementes with a Kalimat-compaitble null value
    for(int i=0; i<arr->elements.count(); i++)
    {
        arr->elements[i] = this->null();
    }

    Value *ret = allocateNewValue<MultiDimensionalArrayVal, MultiDimensionalArray<Value *> *>(arr);
    ret->type = BuiltInTypes::ArrayType;

    return ret;
}

Value *Allocator::newString(QString str, bool gcMonitor)
{
    Value *ret = allocateNewValue<StringVal, QString>(str, gcMonitor);
    ret->type = BuiltInTypes::StringType;
    return ret;
}

Value *Allocator::newString(QString str)
{
    return newString(str, true);
}

Value *Allocator::newRaw(void *ptr, IClass *_class)
{
    Value *ret = allocateNewValue<RawVal, void *>(ptr);
    ret->type = _class;
    return ret;
}

Value *Allocator::newFieldReference(IObject *obj, QString SymRef)
{
    FieldReference *ref = new FieldReference(obj, SymRef);

    Value *ret = allocateNewValue<RefVal, Reference *>(ref);
    ret->type = BuiltInTypes::FieldRefType;
    return ret;
}

Value *Allocator::newArrayReference(VArray *array, int index)
{
    ArrayReference *ref = new ArrayReference(array, index);

    Value *ret = allocateNewValue<RefVal, Reference *>(ref);
    ret->type = BuiltInTypes::ArrayRefType;
    return ret;
}

Value *Allocator::newMultiDimensionalArrayReference(MultiDimensionalArray<Value *> *array, QVector<int> index)
{
    MultiDimensionalArrayReference *ref = new MultiDimensionalArrayReference();
    ref->array = array;
    ref->index = index;

    Value *ret = allocateNewValue<RefVal, Reference *>(ref);
    ret->type = BuiltInTypes::ArrayRefType;
    return ret;
}

Value *Allocator::newChannel(bool gc)
{
    Channel *chan = new Channel();

    Value *ret = allocateNewValue<ChannelVal, Channel *>(chan, gc);
    ret->type = BuiltInTypes::ChannelType;
    return ret;
}

Value *Allocator::newQObject(QObject *qobj)
{
    Value *ret = allocateNewValue<QObjVal, QObject *>(qobj);
    ret->type = BuiltInTypes::QObjectType;
    return ret;
}

void Allocator::gc()
{
    /* We use gcLock because we don't want two threads to enter GC at the same time, because
      in that case vm->stoptheworld() would deadlock.

      However we cannot just use lock(); since there is a case where
      1- Both threads enter GC
      2- one takes the lock and the other waits
      3- But the waiting thread cannot release the semaphore waited for by stopTheWorld, thus deadlock

      Using tryLock means "exit gc without doing anything if the other thread is GCing", which is what
      we wanted anyway.

      */
    if(gcLock.tryLock())
    {
        vm->traceInstructions = true;
        vm->stopTheWorld();

        //qDebug("Running GC ---------------------------------------");
        //clock_t t1 = clock();
        mark();
        sweep();
        // clock_t t2 = clock();
        // qDebug() << "Finished GC --------------------------------------- took "
        //          <<  (double) (t2 - t1) / (double) CLOCKS_PER_SEC  << " seconds.";
        vm->traceInstructions = false;
        vm->startTheWorld();
        gcLock.unlock();
    }
}

void Allocator::mark()
{
    currentAllocationInBytes = 0;
    QStack<Value *> reachable;

    for(QHash<int, Value *>::const_iterator i=constantPool->begin(); i != constantPool->end(); ++i)
    {
        Value * v = i.value();
        reachable.push(v);
    }
    for(QSet<Value *>::const_iterator i=protectedValues.begin(); i != protectedValues.end(); ++i)
    {
        Value *v = *i;
        reachable.push(v);
    }

    for(QSet<QMap<QString, Value *> *>::const_iterator it= otherFrames.begin(); it != otherFrames.end(); ++it)
    {
        QMap<QString, Value *> *of = *it;
        for(QMap<QString, Value *>::const_iterator j=of->begin(); j != of->end(); ++j)
        {
            reachable.push(j.value());
        }
    }

    for(QSet<Scheduler *>::const_iterator iter1=schedulers.begin(); iter1 != schedulers.end(); ++iter1)
    {
        Scheduler *sched = (*iter1);
        if(sched->runningNow)
            markProcess(sched->runningNow, reachable);

        ProcessIterator *iter = sched->getProcesses();
        while(iter->hasMoreProcesses())
        {
            markProcess(iter->getProcess(), reachable);
        }
        delete iter;
    }

    while(!reachable.empty())
    {
        Value *v = reachable.pop();

        if(v->mark)
            continue;
        currentAllocationInBytes += sizeof(Value);
        v->mark = 1;
        if(v->type == BuiltInTypes::ArrayType)
        {
            VArray *elements = unboxArray(v);
            for(int i=0; i<elements->count(); i++)
            {
                Value *v2 = elements->Elements[i];
                if(!v2->mark)
                    reachable.push(v2);
            }
        }
        if(v->type == BuiltInTypes::ChannelType)
        {
            const Channel *chan = unboxChan(v);
            for(QMap<Process *, Value *>::const_iterator i = chan->data.begin(); i!= chan->data.end(); ++i)
            {
                reachable.push(i.value());
            }
            for(int i=0; i<chan->nullProcessQ.count(); ++i)
            {
                reachable.push(chan->nullProcessQ.at(i));
            }
        }
        if(v->type == BuiltInTypes::MapType)
        {
            VMap *map = unboxMap(v);
            for(int i=0; i<map->allKeys.count(); i++)
            {
                Value *v2 = map->allKeys[i];
                if(!v2->mark)
                {
                    reachable.push(v2);
                }
            }
            foreach(Value *v2, map->Elements)
            {
                if(!v2->mark)
                {
                    reachable.push(v2);
                }
            }
        }
        if(v->type == BuiltInTypes::RefType)
        {
            Value *v2 = unboxRef(v)->Get();
            if(!v2->mark)
                reachable.push(v2);
        }
        if(v->isObject())
        {
            IObject *obj = unboxObj(v);
            // todo: so unperformant!
            QList<QString> slotNames = obj->getSlotNames();
            for(int i=0; i<slotNames.count(); i++)
            {
                Value *v2 = obj->getSlotValue(slotNames[i]);
                if(!v2->mark)
                    reachable.push(v2);
            }
            ValueClass *c = dynamic_cast<ValueClass *>(obj);
            if(c)
            {
                QMap<QString, Value *>::const_iterator j;
                for(j= c->methods.begin(); j != c->methods.end(); ++j)
                {
                    reachable.push(*j);
                }
            }
        }
    }
}

void Allocator::markProcess(Process *proc, QStack<Value *> &reachable)
{
    const Frame *stack = proc->stack;
    while(stack != NULL)
    {
        const Frame &f = *stack;
        for(int j=0; j<f.fastLocalCount; j++)
        {
            Value *v = f.fastLocals[j];
            if(!v)
            {
                // This is a region reserved in 'fastLocals' for a variable
                // that has not yet been set
                continue;
            }
            reachable.push(f.fastLocals[j]);
        }
        stack = stack->next;
    }
    for(VOperandStack::const_iterator j=proc->OperandStack.begin(); j !=proc->OperandStack.end(); ++j)
    {
       reachable.push(*j);
    }
}

void Allocator::sweep()
{
    /*
    static clock_t lastgc = -1;

    if(lastgc==-1)
        lastgc = clock();

    clock_t now = clock();

    // qDebug() << "Last GC since " << (double) (now - lastgc) / (double) CLOCKS_PER_SEC << " ms.";
    lastgc = now;
    //*/

    int delCount = 0;
    int heapSizeNow = 0;
    Value *tmp;

    while(!heap->mark)
    {
        tmp = heap;
        heap = heap->heapNext;
        delCount++;
        deleteValue(tmp);
    }
    // Now the heap points to the first valid value...
    Value *p1 = heap;

    if(!p1)
        return;

    p1->mark = 0;
    Value *p2 = p1->heapNext;
    heapSizeNow =1;

    while(p2)
    {
        if(p2->mark)
        {
            p2->mark = 0;
            p1 = p2;
            p2 = p2->heapNext;
            heapSizeNow++;
        }
        else
        {
            Value *tmp = p2;
            p1->heapNext = p2->heapNext;
            p2 = p2->heapNext;
            deleteValue(tmp);
        }
    }
    // qDebug() << "Deleting: " <<  delCount << " elements. Heap size: " << delCount + heapSizeNow;
    // qDebug() << "Heap size now: " << heapSizeNow;
}

void Allocator::deleteValue(Value *v)
{
    if(v->type != BuiltInTypes::IntType)
    {
        delete v;
    }
    else
    {
        v->type = (IClass *) intFreeList;
        intFreeList = (IntVal *) v;
    }
}
