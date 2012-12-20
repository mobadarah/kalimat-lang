/**************************************************************************
**   The Kalimat programming language
**   Copyright (C) 2010 Mohamed Samy Ali - samy2004@gmail.com
**   This project is released under the Apache License version 2.0
**   as described in the included license.txt file
**************************************************************************/

#include "vm_incl.h"
#include "allocator.h"

const int intCacheSize = 40;
//Value *Allocator::_true = NULL;
//Value *Allocator::_false = NULL;
//Value *Allocator::_ints[intCacheSize];

Allocator::Allocator(QHash<int, Value *> *constantPool, QSet<Scheduler *>schedulers)
{
    this->constantPool = constantPool;
    this->schedulers = schedulers;
    currentAllocationInBytes = 0;
    maxAllocationInBytes = NORMAL_MAX_HEAP;

    _true = newBool(true, false);
    _false = newBool(false, false);

    _ints = new Value*[intCacheSize];
    for(int i=0; i<intCacheSize; i++)
        _ints[i] = newInt(i, false);
}

Allocator::~Allocator()
{
    delete _true;
    delete _false;
    for(int i=0; i<intCacheSize; ++i)
    {
        delete _ints[i];
    }
    delete[] _ints;
}

Value *Allocator::allocateNewValue(bool gcMonitor)
{
    if((currentAllocationInBytes + sizeof(Value)) > (maxAllocationInBytes))
    {
        gc();
        if((currentAllocationInBytes + sizeof(Value))> (maxAllocationInBytes))
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
    Value *ret = new Value();
    if(ret!=NULL)
    {
        if(gcMonitor)
        {
            makeGcMonitored(ret);
        }
    }
    else
    {
        //TODO: Signal out of memory
    }
    return ret;
}

Value *Allocator::newInt(int i)
{
    if(i>=0 && i < intCacheSize)
        return _ints[i];

    return newInt(i, true);
}

Value *Allocator::newInt(int i, bool gcMonitor=true)
{
    Value *ret = allocateNewValue(gcMonitor);
    ret->tag = Int;
    ret->type = BuiltInTypes::IntType;
    ret->v.intVal = i;
    return ret;
}

Value *Allocator::newLong(long i)
{
    return newLong(i, true);
}

Value *Allocator::newLong(long i, bool gcMonitor=true)
{
    Value *ret = allocateNewValue(gcMonitor);
    ret->tag = Long;
    ret->type = BuiltInTypes::LongType;
    ret->v.longVal= i;
    return ret;
}

Value *Allocator::newDouble(double d)
{
    return newDouble(d, true);
}

Value *Allocator::newDouble(double d, bool gcMonitor)
{
    Value *ret = allocateNewValue(gcMonitor);
    ret->tag = Double;
    ret->type = BuiltInTypes::DoubleType;
    ret->v.doubleVal = d;
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
    Value *ret = allocateNewValue(gcMonitor);
    ret->tag = Boolean;
    ret->type = BuiltInTypes::BoolType;
    ret->v.boolVal = b;
    return ret;
}

Value *Allocator::newObject(IClass *_class)
{
    IObject *newObj = _class->newValue(this);

    return newObject(newObj, _class);
}

Value *Allocator::newObject(IObject *newObj, IClass *_class, bool gcMonitor)
{
    Value *ret = allocateNewValue(gcMonitor);
    ret->tag = ObjectVal;
    ret->type = _class;
    ret->v.objVal = newObj;
    return ret;
}

Value *Allocator::null()
{
    if(Value::NullValue == NULL)
    {
        Value::NullValue = allocateNewValue(false);
        Value::NullValue->tag = NullVal;
        Value::NullValue->type = BuiltInTypes::NullType;
    }

    return Value::NullValue;
}

Value *Allocator::newArray(int size)
{
    Value *ret = allocateNewValue();
    ret->tag = ArrayVal;
    ret->type = BuiltInTypes::ArrayType;
    ret->v.arrayVal = new VArray();
    ret->v.arrayVal->Elements = new Value*[size];
    ret->v.arrayVal->_count = size;
    for(int i=0; i<size; i++)
        ret->v.arrayVal->Elements[i] = null();
    return ret;
}

Value *Allocator::newMap()
{
    Value *ret = allocateNewValue();
    ret->tag = MapVal;
    ret->type = BuiltInTypes::MapType;
    ret->v.mapVal = new VMap();
    return ret;
}

Value *Allocator::newMultiDimensionalArray(QVector<int>dimensions)
{
    Value *ret = allocateNewValue();
    ret->tag = MultiDimensionalArrayVal;
    ret->type = BuiltInTypes::ArrayType;
    ret->v.multiDimensionalArrayVal = new MultiDimensionalArray<Value *>(dimensions);

    // Init all elementes with a Kalimat-compaitble null value
    for(int i=0; i<ret->v.multiDimensionalArrayVal->elements.count(); i++)
    {
        ret->v.multiDimensionalArrayVal->elements[i] = this->null();
    }
    return ret;
}

Value *Allocator::newString(QString str, bool gcMonitor)
{
    Value *ret = allocateNewValue(gcMonitor);
    ret->tag = StringVal;
    ret->type = BuiltInTypes::StringType;
    ret->vstrVal = str;
    return ret;
}

Value *Allocator::newString(QString str)
{
    return newString(str, true);
}

Value *Allocator::newRaw(void *ptr, IClass *_class)
{
    Value *ret = allocateNewValue();
    ret->tag = RawVal;
    ret->type = _class;
    ret->v.rawVal = ptr;
    return ret;
}

Value *Allocator::newFieldReference(IObject *obj, QString SymRef)
{
    FieldReference *ref = new FieldReference(obj, SymRef);

    Value *ret = allocateNewValue();
    ret->tag = RefVal;
    ret->type = BuiltInTypes::FieldRefType;
    ret->v.refVal = ref;
    return ret;
}

Value *Allocator::newArrayReference(VArray *array, int index)
{
    ArrayReference *ref = new ArrayReference(array, index);

    Value *ret = allocateNewValue();
    ret->tag = RefVal;
    ret->type = BuiltInTypes::ArrayRefType;
    ret->v.refVal = ref;
    return ret;
}

Value *Allocator::newMultiDimensionalArrayReference(MultiDimensionalArray<Value *> *array, QVector<int> index)
{
    MultiDimensionalArrayReference *ref = new MultiDimensionalArrayReference();
    ref->array = array;
    ref->index = index;

    Value *ret = allocateNewValue();
    ret->tag = RefVal;
    ret->type = BuiltInTypes::ArrayRefType;
    ret->v.refVal = ref;
    return ret;
}

Value *Allocator::newChannel(bool gc)
{
    Channel *chan = new Channel();

    Value *ret = allocateNewValue(gc);
    ret->tag = ChannelVal;
    ret->type = BuiltInTypes::ChannelType;
    ret->v.channelVal = chan;
    return ret;
}

Value *Allocator::newQObject(QObject *qobj)
{
    Value *ret = allocateNewValue();
    ret->tag = QObjectVal;
    ret->type = BuiltInTypes::QObjectType;
    ret->v.qobjVal = qobj;
    return ret;
}

void Allocator::makeGcMonitored(Value *v)
{
//    if(!heap.contains(v))
//        currentAllocationInBytes += sizeof(Value);
    protectedValues.remove(v);
}

void Allocator::stopGcMonitoring(Value *v)
{
//    if(heap.contains(v))
//        currentAllocationInBytes -= sizeof(Value);
    protectedValues.insert(v);
}

void Allocator::gc()
{
    return;
    mark();
    sweep();
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
        QSet<QQueue<Process *> *> qs = sched->getProcesses();
        for(QSet<QQueue<Process *> *>::const_iterator queueIter= qs.begin(); queueIter != qs.end(); ++queueIter)
        {
            const QQueue<Process *> &q = *(*queueIter);
            for(QQueue<Process *>::const_iterator iter=q.begin(); iter!=q.end(); ++iter)
            {
                const Frame *stack = (*iter)->stack;
                while(stack != NULL)
                {
                    const Frame &f = *stack;
                    for(int j=0; j<f.fastLocalCount; j++)
                    {
                        reachable.push(f.fastLocals[j]);
                    }
                    for(Stack<Value *>::const_iterator j=f.OperandStack.begin(); j !=f.OperandStack.end(); ++j)
                    {
                        reachable.push(*j);
                    }
                    stack = stack->next;
                }
            }
        }

    }


    while(!reachable.empty())
    {
        Value *v = reachable.pop();

        if(v->mark)
            continue;
        currentAllocationInBytes += sizeof(Value);
        v->mark = 1;
        if(v->tag == ArrayVal)
        {
            VArray *elements = v->unboxArray();
            for(int i=0; i<elements->count(); i++)
            {
                Value *v2 = elements->Elements[i];
                if(!v2->mark)
                    reachable.push(v2);
            }
        }
        if(v->tag == MapVal)
        {
            VMap *map = v->unboxMap();
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
        if(v->tag == RefVal)
        {
            Value *v2 = v->unboxRef()->Get();
            if(!v2->mark)
                reachable.push(v2);
        }
        if(v->tag == ObjectVal)
        {
            IObject *obj = v->unboxObj();
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

void Allocator::sweep()
{
    QVector<Value *> toDel;
    QSet<Value *>::const_iterator i;
    for (i = heap.begin(); i != heap.end(); ++i)
    {
        Value *v = *i;
        if(v->mark)
            v->mark = 0;
        else
        {
            toDel.append(v);
        }
    }
    for(int i=0; i<toDel.count(); i++)
    {
        Value *v = toDel[i];
        heap.remove(v);
        delete v;
    }
}
