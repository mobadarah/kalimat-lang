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
    ret->type = BuiltInTypes::IntType;
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
    ret->type = BuiltInTypes::DoubleType;
    ret->v.doubleVal = d;
    return ret;
}

Value *Allocator::newBool(bool b)
{
    return newBool(b, true);
}

Value *Allocator::newBool(bool b, bool gcMonitor)
{
    Value *ret = allocateNewValue(gcMonitor);
    ret->tag = Boolean;
    ret->type = BuiltInTypes::BoolType;
    ret->v.boolVal = b;
    return ret;
}

Value *Allocator::newObject(ValueClass *_class)
{
    Object *newObj = new Object();
    InitObjectLayout(newObj, _class);

    return newObject(newObj, _class);
}

Value *Allocator::newObject(IObject *newObj, ValueClass *_class)
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
    ret->v.arrayVal->count = size;
    for(int i=0; i<size; i++)
        ret->v.arrayVal->Elements[i] = null();
    return ret;
}
Value *Allocator::newMultiDimensionalArray(QVector<int>dimensions)
{
    Value *ret = allocateNewValue();
    ret->tag = MultiDimensionalArrayVal;
    ret->type = BuiltInTypes::ArrayType;
    ret->v.multiDimensionalArrayVal = new MultiDimensionalArray<Value *>(dimensions);
    //TODO: init all elementes with a Kalimat-compaitble null value
    // instead of null pointers
    return ret;
}

Value *Allocator::newString(QString *str)
{
    Value *ret = allocateNewValue();
    ret->tag = StringVal;
    ret->type = BuiltInTypes::StringType;
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
    for(QVector<QString>::iterator i = _class->fieldNames.begin(); i!=_class->fieldNames.end(); ++i)
    {
        object->slotNames.append(*i);
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

    for(QMap<QString, Value *>::const_iterator i=constantPool->begin(); i != constantPool->end(); ++i)
    {
        Value * v = i.value();
        reachable.push(v);
    }
    for(int i=0; i<stack->count(); i++)
    {
        const Frame &f = stack->at(i);
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
            IObject *_obj = v->unboxObj();
            Object *obj = dynamic_cast<Object *>(_obj);
            if(obj != NULL)
            {
                for(int i=0; i<obj->_slots.count(); i++)
                {
                    Value *v2 = obj->_slots.values()[i];
                    if(!v2->mark)
                        reachable.push(v2);
                }
                ValueClass *c = dynamic_cast<ValueClass *>(v->v.objVal);
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
