#include "classes.h"
#include "allocator.h"
#include "vmerror.h"
#include "runtime_identifiers.h"

int IClass::compareTo(Value *v1, Value *v2)
{
    throw VMError(BuiltInOperationOnNonBuiltn2).arg(v1->type->toString()).arg(v2->type->toString());
}

int IClass::compareIntToMe(int v1, Value *v2)
{
    throw VMError(BuiltInOperationOnNonBuiltn2).
            arg(BuiltInTypes::IntType->getName()).
            arg(v2->type->toString());
}

int IClass::compareDoubleToMe(double v1, Value *v2)
{
    throw VMError(BuiltInOperationOnNonBuiltn2).
            arg(BuiltInTypes::DoubleType->getName()).
            arg(v2->type->toString());
}

int IClass::compareLongToMe(long v1, Value *v2)
{
    throw VMError(BuiltInOperationOnNonBuiltn2).
            arg(BuiltInTypes::LongType->getName()).
            arg(v2->type->toString());
}

int IClass::compareStringToMe(QString v1, Value *v2)
{
    throw VMError(BuiltInOperationOnNonBuiltn2).
            arg(BuiltInTypes::StringType->getName()).
            arg(v2->type->toString());
}

Value *IClass::addTo(Value *v1, Value *v2, Allocator *)
{
    throw VMError(BuiltInOperationOnNonBuiltn2).arg(v1->type->toString()).arg(v2->type->toString());
}

Value *IClass::addIntToMe(int v1, Value *v2, Allocator *)
{
    throw VMError(BuiltInOperationOnNonBuiltn2).
            arg(BuiltInTypes::IntType->getName()).
            arg(v2->type->toString());
}

Value *IClass::addDoubleToMe(double v1, Value *v2, Allocator *)
{
    throw VMError(BuiltInOperationOnNonBuiltn2).
            arg(BuiltInTypes::DoubleType->getName()).
            arg(v2->type->toString());
}

Value *IClass::addLongToMe(long v1, Value *v2, Allocator *)
{
    throw VMError(BuiltInOperationOnNonBuiltn2).
            arg(BuiltInTypes::LongType->getName()).
            arg(v2->type->toString());
}

Value *IClass::addStringToMe(QString v1, Value *v2, Allocator *)
{
    throw VMError(BuiltInOperationOnNonBuiltn2).
            arg(BuiltInTypes::StringType->getName()).
            arg(v2->type->toString());
}

Value *IClass::addArrayToMe(VArray *v1, Value *v2, Allocator *)
{
    throw VMError(BuiltInOperationOnNonBuiltn2).
            arg(BuiltInTypes::ArrayType->getName()).
            arg(v2->type->toString());
}

Value *IClass::minus(Value *v1, Value *v2, Allocator *)
{
    throw VMError(NumericOperationOnNonNumber3)
            .arg(VMId::get(RId::Subtraction))
            .arg(v1->type->toString())
            .arg(v2->type->toString());
}

Value* IClass::intMinusMe(int v1, Value *v2, Allocator *)
{
    throw VMError(NumericOperationOnNonNumber3)
            .arg(VMId::get(RId::Subtraction))
            .arg(BuiltInTypes::IntType->getName())
            .arg(v2->type->toString());
}

Value *IClass::doubleMinusMe(double v1,  Value *v2, Allocator *)
{
    throw VMError(NumericOperationOnNonNumber3)
            .arg(VMId::get(RId::Subtraction))
            .arg(BuiltInTypes::DoubleType->getName())
            .arg(v2->type->toString());
}

Value *IClass::longMinusMe(long v1,  Value *v2, Allocator *)
{
    throw VMError(NumericOperationOnNonNumber3)
            .arg(VMId::get(RId::Subtraction))
            .arg(BuiltInTypes::LongType->getName())
            .arg(v2->type->toString());
}

QString ArrayClass::getName()
{
    return VMId::get(RId::VArray);
}

IClass *ArrayClass::baseClass()
{
    return BuiltInTypes::IndexableType;
}

bool ArrayClass::subclassOf(IClass *c)
{
    if(c == this)
        return true;

    return baseClass()->subclassOf(c);
}

IMethod *ArrayClass::lookupMethod(QString name)
{
    return baseClass()->lookupMethod(name);
}

IObject *ArrayClass::newValue(Allocator *allocator)
{
    throw VMError(InternalError);
}

Value *ArrayClass::addTo(Value *v1, Value *v2, Allocator *allocator)
{
    return v1->type->addArrayToMe(unboxArray(v1), v2, allocator);
}

Value *ArrayClass::addArrayToMe(VArray *arr1, Value *v2, Allocator *allocator)
{
    VArray *arr2 = unboxArray(v2);
    Value *v3 = allocator->newArray(arr1->count() + arr2->count());
    VArray *arr3 = unboxArray(v3);
    int c = 0;
    for(int i=0; i<arr1->count(); i++)
        arr3->Elements[c++] = arr1->Elements[i];
    for(int i=0; i<arr2->count(); i++)
        arr3->Elements[c++] = arr2->Elements[i];
    return v3;
}

QString StringClass::getName()
{
    return VMId::get(RId::String);
}

IClass *StringClass::baseClass()
{
    return BuiltInTypes::IndexableType;
}

bool StringClass::subclassOf(IClass *c)
{
    if(c == this)
        return true;

    return baseClass()->subclassOf(c);
}

IMethod *StringClass::lookupMethod(QString name)
{
    return baseClass()->lookupMethod(name);
}

IObject *StringClass::newValue(Allocator *allocator)
{
    throw VMError(InternalError);
}

int StringClass::compareTo(Value *v1, Value *v2)
{
    return compareStringToMe(unboxStr(v1), v2);
}

int StringClass::compareStringToMe(QString v1, Value *v2)
{
    return v1.compare(unboxStr(v2));
}

Value *StringClass::addTo(Value *v1, Value *v2, Allocator *a)
{
    return v1->type->addStringToMe(unboxStr(v1), v2, a);
}

Value *StringClass::addStringToMe(QString v1, Value *v2, Allocator *a)
{
    return a->newString(v1 + unboxStr(v2));
}

QString NumericClass::getName()
{
    return VMId::get(RId::Numeric);
}

IClass *NumericClass::baseClass()
{
    return BuiltInTypes::ObjectType;
}

bool NumericClass::subclassOf(IClass *c)
{
    if(c == this)
        return true;

    return baseClass()->subclassOf(c);
}

IMethod *NumericClass::lookupMethod(QString name)
{
    return baseClass()->lookupMethod(name);
}

IObject *NumericClass::newValue(Allocator *allocator)
{
    throw VMError(InternalError);
}

QString IntClass::getName()
{
    return VMId::get(RId::Integer);
}

IClass *IntClass::baseClass()
{
    return BuiltInTypes::NumericType;
}

int IntClass::compareTo(Value *v1, Value *v2)
{
    return v2->type->compareIntToMe(unboxInt(v1), v2);
}

int IntClass::compareIntToMe(int v1, Value *v2)
{
    return v1 - unboxInt(v2);
}

int IntClass::compareDoubleToMe(double v1, Value *v2)
{
    int i2= unboxInt(v2);
    if(v1 > i2)
        return 1;
    else if(v1 < i2)
        return -1;
    else
        return 0;
}

int IntClass::compareLongToMe(long v1, Value *v2)
{
    int i2= unboxInt(v2);
    if(v1 > i2)
        return 1;
    else if(v1 < i2)
        return -1;
    else
        return 0;
}

Value *IntClass::addTo(Value *v1, Value *v2, Allocator *a)
{
    return v2->type->addIntToMe(unboxInt(v1), v2, a);
}

Value *IntClass::addIntToMe(int v1, Value *v2, Allocator *a)
{
    return a->newInt(v1 + unboxInt(v2));
}

Value *IntClass::addDoubleToMe(double v1, Value *v2, Allocator *a)
{
    return a->newDouble(v1 + unboxInt(v2));
}

Value *IntClass::addLongToMe(long v1, Value *v2, Allocator *a)
{
    return a->newLong(v1 + unboxInt(v2));
}

Value *IntClass::minus(Value *v1, Value *v2, Allocator *a)
{
    return v2->type->intMinusMe(unboxInt(v1), v2, a);
}

Value *IntClass::intMinusMe(int v1, Value *v2, Allocator *a)
{
    return a->newInt(v1 - unboxInt(v2));
}

Value *IntClass::doubleMinusMe(double v1, Value *v2, Allocator *a)
{
    return a->newDouble(v1 - unboxInt(v2));
}

Value *IntClass::longMinusMe(long v1, Value *v2, Allocator *a)
{
    return a->newLong(v1 - unboxInt(v2));
}

QString DoubleClass::getName()
{
    return VMId::get(RId::Double);
}

IClass *DoubleClass::baseClass()
{
    return BuiltInTypes::NumericType;
}

int DoubleClass::compareTo(Value *v1, Value *v2)
{
    return v2->type->compareDoubleToMe(unboxDouble(v1), v2);
}

int DoubleClass::compareIntToMe(int v1, Value *v2)
{
    double d2 = unboxDouble(v2);
    if(v1 > d2)
        return 1;
    else if(v1 < d2)
        return -1;
    else
        return 0;
}

int DoubleClass::compareDoubleToMe(double v1, Value *v2)
{
    return v1 - unboxDouble(v2);
}

int DoubleClass::compareLongToMe(long v1, Value *v2)
{
    double d2 = unboxDouble(v2);
    if(v1 > d2)
        return 1;
    else if(v1 < d2)
        return -1;
    else
        return 0;
}

Value *DoubleClass::addTo(Value *v1, Value *v2, Allocator *a)
{
    return v2->type->addDoubleToMe(unboxDouble(v1), v2, a);
}

Value *DoubleClass::addIntToMe(int v1, Value *v2, Allocator *a)
{
    return a->newDouble(v1 + unboxDouble(v2));
}

Value *DoubleClass::addDoubleToMe(double v1, Value *v2, Allocator *a)
{
    return a->newDouble(v1 + unboxDouble(v2));
}

Value *DoubleClass::addLongToMe(long v1, Value *v2, Allocator *a)
{
    return a->newDouble(v1 + unboxDouble(v2));
}

Value *DoubleClass::minus(Value *v1, Value *v2, Allocator *a)
{
    return v2->type->doubleMinusMe(unboxDouble(v1), v2, a);
}

Value *DoubleClass::intMinusMe(int v1, Value *v2, Allocator *a)
{
    return a->newDouble(v1 - unboxDouble(v2));
}

Value *DoubleClass::doubleMinusMe(double v1, Value *v2, Allocator *a)
{
    return a->newDouble(v1 - unboxDouble(v2));
}

Value *DoubleClass::longMinusMe(long v1, Value *v2, Allocator *a)
{
    return a->newDouble(v1 - unboxDouble(v2));
}

QString LongClass::getName()
{
    return VMId::get(RId::Long);
}

IClass *LongClass::baseClass()
{
    return BuiltInTypes::NumericType;
}

int LongClass::compareTo(Value *v1, Value *v2)
{
    return v2->type->compareLongToMe(unboxLong(v1), v2);
}

int LongClass::compareIntToMe(int v1, Value *v2)
{
    long l2 = unboxLong(v2);
    if(v1 > l2)
        return 1;
    else if(v1 < l2)
        return -1;
    else
        return 0;
}

int LongClass::compareDoubleToMe(double v1, Value *v2)
{
    long l2 = unboxLong(v2);
    if(v1 > l2)
        return 1;
    else if(v1 < l2)
        return -1;
    else
        return 0;
}

int LongClass::compareLongToMe(long v1, Value *v2)
{
    long l2 = unboxLong(v2);
    return v1 - l2;
}

Value *LongClass::addTo(Value *v1, Value *v2, Allocator *a)
{
    return v2->type->addLongToMe(unboxLong(v1), v2, a);
}

Value *LongClass::addIntToMe(int v1, Value *v2, Allocator *a)
{
    return a->newLong(v1 + unboxLong(v2));
}

Value *LongClass::addDoubleToMe(double v1, Value *v2, Allocator *a)
{
    return a->newDouble(v1 + unboxLong(v2));
}

Value *LongClass::addLongToMe(long v1, Value *v2, Allocator *a)
{
    return a->newLong(v1 + unboxLong(v2));
}

Value *LongClass::minus(Value *v1, Value *v2, Allocator *a)
{
    return v2->type->longMinusMe(unboxLong(v1), v2, a);
}

Value *LongClass::intMinusMe(int v1, Value *v2, Allocator *a)
{
    return a->newLong(v1 - unboxLong(v2));
}

Value *LongClass::doubleMinusMe(double v1, Value *v2, Allocator *a)
{
    return a->newDouble(v1 - unboxLong(v2));
}

Value *LongClass::longMinusMe(long v1, Value *v2, Allocator *a)
{
    return a->newLong(v1 - unboxLong(v2));
}

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

bool eq_raw(Value *a, Value *b);
PointerClass::PointerClass(IClass *pointee)
{
    this->pointee = pointee;
    this->equality = eq_raw;
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
    return VMId::get(RId::PointerOf1, pointee->getName());
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

FunctionClass::FunctionClass(IClass *retType, QVector<IClass *> argTypes)
    :retType(retType),
      argTypes(argTypes)
{
    this->equality = compareRef; // doesn't matter anyway
}

bool FunctionClass::hasSlot(QString name)
{
    return false;
}

QList<QString> FunctionClass::getSlotNames()
{
    return QList<QString>();
}

Value *FunctionClass::getSlotValue(QString name)
{
    return NULL;
}

void FunctionClass::setSlotValue(QString name, Value *val)
{

}

QString FunctionClass::getName()
{
    QString kind = retType? _ws(L"دالة"): _ws(L"إجراء");
    QStringList args;
    for(int i=0; i<argTypes.count(); ++i)
    {
        args.append(argTypes[i]->getName());
    }

    return QString("%1(%2) %3")
            .arg(kind)
            .arg(args.join(", "))
            .arg(retType? retType->getName(): "");
}

bool FunctionClass::hasField(QString name)
{
    return false;
}

IClass *FunctionClass::baseClass()
{
    return BuiltInTypes::ObjectType;
}

bool FunctionClass::subclassOf(IClass *c)
{
    // We assume "Function" is invariant
    // todo: revise this decision
    if(c == BuiltInTypes::ObjectType)
        return true;
    FunctionClass *f2 = dynamic_cast<FunctionClass*>(c);
    if(!f2)
        return false;

    return f2 == this;
}

IMethod *FunctionClass::lookupMethod(QString name)
{
    return baseClass()->lookupMethod(name);
}

IObject *FunctionClass::newValue(Allocator *allocator)
{
    throw VMError(InternalError);
}

bool FunctionClass::getFieldAttribute(QString attr, Value *&ret, Allocator *allocator)
{
    return false;
}

QVector<PropertyDesc> FunctionClass::getProperties()
{
    return QVector<PropertyDesc>();
}

QString FunctionClass::toString()
{
    return getName();
}
