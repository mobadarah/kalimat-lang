#include "value.h"
#include "frame.h"
#include "allocator.h"
#include "metaclass.h"
#include "vmerror.h"
#include "runtime_identifiers.h"

MetaClass::MetaClass(QString name, Allocator *allocator)
    :EasyForeignClass(name)
{
    this->allocator = allocator;

    methodIds[VMId::get(RId::NewObject)] =
            0;
    methodArities[VMId::get(RId::NewObject)] =
            1;
}

void MetaClass::setAllocator(Allocator *allocator)
{
    this->allocator = allocator;
}

IObject *MetaClass::newValue(Allocator *allocator)
{
    throw VMError(InternalError);
}

bool MetaClass::getFieldAttribute(QString attr, Value *&ret, Allocator *allocator)
{
    return false;
}

QVector<PropertyDesc> MetaClass::getProperties()
{
    return QVector<PropertyDesc>();
}

Value *MetaClass::dispatch(Process *proc, int id, QVector<Value *> args)
{
    if(id == 0)
    {
        IObject *theClassObj = args[0]->unboxObj();
        IClass *theClass = dynamic_cast<IClass *>(theClassObj);
        if(theClass)
        {
            return allocator->newObject(theClass->newValue(allocator), theClass);
        }
        else
        {
            throw VMError(InternalError);
        }
    }
    return NULL;
}
