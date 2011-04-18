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
    InitObjectLayoutHelper(this, object, allocator);
}

void ValueClass::InitObjectLayoutHelper(ValueClass *_class, Object *object, Allocator *allocator)
{
    //todo:
    for(QVector<ValueClass *>::iterator i=_class->BaseClasses.begin(); i!=_class->BaseClasses.end(); ++i)
    {
        (*i)->InitObjectLayout(object, allocator);
    }
    for(QSet<QString>::iterator i = _class->fields.begin(); i!=_class->fields.end();  ++i)
    {
        object->setSlotValue(*i, allocator->null());
    }
    for(QVector<QString>::iterator i = _class->fieldNames.begin(); i!=_class->fieldNames.end(); ++i)
    {
        object->slotNames.append(*i);
    }

}
