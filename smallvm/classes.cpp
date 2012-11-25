#include "classes.h"
#include "allocator.h"
#include "vmerror.h"

IObject *ValueClass::newValue(Allocator *allocator)
{
    Object *newObj = new Object();
    InitObjectLayout(newObj, allocator);
    return newObj;
}

bool ValueClass::getFieldAttribute(QString attr, Value *&ret, Allocator *allocator)
{
    if(!fieldAttributes.contains(attr))
        return false;
    ret = fieldAttributes[attr];
    return true;
}

QVector<PropertyDesc> ValueClass::getProperties()
{
    return properties;
}

void ValueClass::InitObjectLayout(Object *object, Allocator *allocator)
{
    InitObjectLayoutHelper(this, object, allocator);
}

void ValueClass::InitObjectLayoutHelper(ValueClass *_class, Object *object, Allocator *allocator)
{
    //todo:
    for(QVector<IClass *>::iterator i=_class->BaseClasses.begin(); i!=_class->BaseClasses.end(); ++i)
    {
        IClass *aBase = *i;
        ValueClass *vc = dynamic_cast<ValueClass *>(aBase);
        if(vc)
            vc->InitObjectLayout(object, allocator);
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

PointerClass::PointerClass(IClass *pointee)
{
    this->pointee = pointee;
}

bool PointerClass::hasSlot(QString name)
{
    return false;
}

QList<QString> PointerClass::getSlotNames()
{
    return QList<QString>();
}

Value *PointerClass::getSlotValue(QString name)
{
    return NULL;
}

void PointerClass::setSlotValue(QString name, Value *val)
{

}

QString PointerClass::getName()
{
    return QString::fromStdWString(L"مشير(%1)").arg(pointee->getName());
}

bool PointerClass::hasField(QString name)
{
    return false;
}

IClass *PointerClass::baseClass()
{
    return BuiltInTypes::ObjectType;
}

bool PointerClass::subclassOf(IClass *c)
{
    // We assume "Pointer" is covariant
    // todo: revise this decision
    if(c == BuiltInTypes::ObjectType)
        return true;
    PointerClass *c2 = dynamic_cast<PointerClass *>(c);
    if(!c2)
        return false;
    return pointee->subclassOf(c2->pointee);
}

IMethod *PointerClass::lookupMethod(QString name)
{
    return baseClass()->lookupMethod(name);
}

IObject *PointerClass::newValue(Allocator *allocator)
{
    throw VMError(InternalError);
}

bool PointerClass::getFieldAttribute(QString attr, Value *&ret, Allocator *allocator)
{
    return false;
}

QVector<PropertyDesc> PointerClass::getProperties()
{
    return QVector<PropertyDesc>();
}

QString PointerClass::toString()
{
    return getName();
}
