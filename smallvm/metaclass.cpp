#include "value.h"
#include "frame.h"
#include "allocator.h"
#include "metaclass.h"
#include "vmerror.h"

MetaClass::MetaClass(QString name, Allocator *allocator)
    :EasyForeignClass(name)
{
    this->allocator = allocator;

    methodIds[QString::fromStdWString(L"انشيء.كائن")] =
            0;
    methodArities[QString::fromStdWString(L"انشيء.كائن")] =
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

Value *MetaClass::dispatch(int id, QVector<Value *> args)
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
