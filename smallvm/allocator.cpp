/**************************************************************************
**   The Kalimat programming language
**   Copyright (C) 2010 Mohamed Samy Ali - samy2004@gmail.com
**   This project is released under the Apache License version 2.0
**   as described in the included license.txt file
**************************************************************************/

#include "vm_incl.h"
#include "allocator.h"

Allocator::Allocator(QMap<QString, Value *> *constantPool, QStack<Frame> *stack)
{
    this->constantPool = constantPool;
    this->stack = stack;
    currentAllocationInBytes = 0;
    maxAllocationInBytes = NORMAL_MAX_HEAP;
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
            heap.insert(ret);
            currentAllocationInBytes += sizeof(Value);
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
    return newInt(i, true);
}

Value *Allocator::newInt(int i, bool gcMonitor=true)
{
    Value *ret = allocateNewValue(gcMonitor);
    ret->tag = Int;
    ret->type = Value::IntType;
    ret->v.intVal = i;
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
    ret->type = Value::DoubleType;
    ret->v.doubleVal = d;
    return ret;
}

Value *Allocator::newObject(ValueClass *_class)
{
    Object *newObj = new Object();
    InitObjectLayout(newObj, _class);

    return newObject(newObj, _class);
}
Value *Allocator::newObject(Object *newObj, ValueClass *_class)
{
    Value *ret = allocateNewValue();
    ret->tag = ObjectVal;
    ret->type = _class;
    ret->v.objVal = newObj;
    return ret;
}
Value *Allocator::null()
{
    if(Value::NullValue == NULL)
    {
        Value::NullValue = allocateNewValue();
        Value::NullValue->tag = NullVal;
        Value::NullValue->type = Value::NullType;
    }

    return Value::NullValue;
}

Value *Allocator::newArray(int size)
{
    Value *ret = allocateNewValue();
    ret->tag = ArrayVal;
    ret->type = Value::ArrayType;
    ret->v.arrayVal = new VArray();
    ret->v.arrayVal->Elements = new Value*[size];
    ret->v.arrayVal->count = size;
    for(int i=0; i<size; i++)
        ret->v.arrayVal->Elements[i] = null();
    return ret;
}
Value *Allocator::newMultiDimensionalArray(QVector<int>dimensions)
{
    Value *ret = allocateNewValue();
    ret->tag = MultiDimensionalArrayVal;
    ret->type = Value::ArrayType;
    ret->v.multiDimensionalArrayVal = new MultiDimensionalArray<Value *>(dimensions);
    //TODO: init all elementes with a Kalimat-compaitble null value
    // instead of null pointers
    return ret;
}

Value *Allocator::newString(QString *str)
{
    Value *ret = allocateNewValue();
    ret->tag = StringVal;
    ret->type = Value::StringType;
    ret->v.strVal = str;
    return ret;
}
Value *Allocator::newRaw(void *ptr, ValueClass *_class)
{
    Value *ret = allocateNewValue();
    ret->tag = RawVal;
    ret->type = _class;
    ret->v.rawVal = ptr;
    return ret;
}

Value *Allocator::newFieldReference(Object *obj, QString SymRef)
{
    FieldReference *ref = new FieldReference();
    ref->object = obj;
    ref->SymRef = SymRef;

    Value *ret = allocateNewValue();
    ret->tag = RefVal;
    ret->type = Value::FieldRefType;
    ret->v.refVal = ref;
    return ret;
}

Value *Allocator::newArrayReference(VArray *array, int index)
{
    ArrayReference *ref = new ArrayReference();
    ref->array = array;
    ref->index = index;

    Value *ret = allocateNewValue();
    ret->tag = RefVal;
    ret->type = Value::ArrayRefType;
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
    ret->type = Value::ArrayRefType;
    ret->v.refVal = ref;
    return ret;
}
void Allocator::InitObjectLayout(Object *object, ValueClass *_class)
{
    //todo:
    for(QVector<ValueClass *>::iterator i=_class->BaseClasses.begin(); i!=_class->BaseClasses.end(); ++i)
    {
        InitObjectLayout(object, *i);
    }
    for(QSet<QString>::iterator i = _class->fields.begin(); i!=_class->fields.end();  ++i)
    {
        object->setSlotValue(*i, this->null());
    }
}
void Allocator::gc()
{
    mark();
    sweep();
}
void Allocator::mark()
{
    currentAllocationInBytes = 0;
    QStack<Value *> reachable;
    reachable.clear();
    for(int i=0; i<constantPool->values().count(); i++)
        reachable.push(constantPool->values()[i]);
    for(int i=0; i<stack->count(); i++)
    {
        Frame f = stack->at(i);
        for(int j=0; j<f.Locals.count(); j++)
        {
            reachable.push(f.Locals.values()[j]);
        }
        for(int j=0; j<f.OperandStack.count(); j++)
        {
            reachable.push(f.OperandStack.value(j));
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
            for(int i=0; i<elements->count; i++)
            {
                Value *v2 = elements->Elements[i];
                if(!v2->mark)
                    reachable.push(v2);
            }
        }
        if(v->tag == ObjectVal)
        {
            Object *obj = v->unboxObj();
            for(int i=0; i<obj->_slots.count(); i++)
            {
                Value *v2 = obj->_slots.values()[i];
                if(!v2->mark)
                    reachable.push(v2);
            }
        }
    }
}
void Allocator::sweep()
{
    objsDeleted = 0;
    QVector<Value *> toDel;
    for(QSet<Value *>::iterator i=heap.begin(); i!=heap.end(); ++i)
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
        objsDeleted++;
        heap.remove(v);
        delete v;

    }
  //  wcout << objsDeleted <<" objects deleted in garbage collection\n";
  //  cout.flush();
}

