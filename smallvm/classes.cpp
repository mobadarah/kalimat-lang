#include "classes.h"
#include "allocator.h"

IObject *ValueClass::newValue(Allocator *allocator)
{
    Object *newObj = new Object();
    InitObjectLayout(newObj, allocator);
    return newObj;
}


void ValueClass::InitObjectLayout(Object *object, Allocator *allocator)
{
    //todo:
    for(QVector<ValueClass *>::iterator i=this->BaseClasses.begin(); i!=this->BaseClasses.end(); ++i)
    {
        (*i)->InitObjectLayout(object, allocator);
    }
    for(QSet<QString>::iterator i = this->fields.begin(); i!=this->fields.end();  ++i)
    {
        object->setSlotValue(*i, allocator->null());
    }
    for(QVector<QString>::iterator i = this->fieldNames.begin(); i!=this->fieldNames.end(); ++i)
    {
        object->slotNames.append(*i);
    }
}
