/**************************************************************************
**   The Kalimat programming language
**   Copyright (C) 2010 Mohamed Samy Ali - samy2004@gmail.com
**   This project is released under the Apache License version 2.0
**   as described in the included license.txt file
**************************************************************************/

#include <QObject>
#include <QString>
#include <QStringList>
#include <QSet>
#include <QMap>
#include <QVector>
#include <QLocale>
#include "value.h"


ValueClass *BuiltInTypes::ObjectType = new ValueClass("Object", NULL);
ValueClass *BuiltInTypes::IntType = new ValueClass("Integer", BuiltInTypes::ObjectType);
ValueClass *BuiltInTypes::DoubleType = new ValueClass("Double", BuiltInTypes::ObjectType);
ValueClass *BuiltInTypes::BoolType = new ValueClass("Bool", BuiltInTypes::ObjectType);
ValueClass *BuiltInTypes::MethodType = new ValueClass("Method", BuiltInTypes::ObjectType);
ValueClass *BuiltInTypes::ExternalMethodType = new ValueClass("ExternalMethod", BuiltInTypes::ObjectType);
ValueClass *BuiltInTypes::ClassType = new ValueClass("Class", BuiltInTypes::ObjectType);
ValueClass *BuiltInTypes::ArrayType = new ValueClass("Array", BuiltInTypes::ObjectType);
ValueClass *BuiltInTypes::StringType = new ValueClass("String", BuiltInTypes::ObjectType);
ValueClass *BuiltInTypes::SpriteType = new ValueClass("Sprite", BuiltInTypes::ObjectType);
ValueClass *BuiltInTypes::FileType = NULL;
ValueClass *BuiltInTypes::RawFileType = new ValueClass("RawFile", BuiltInTypes::ObjectType);
ValueClass *BuiltInTypes::RefType = new ValueClass("Reference", BuiltInTypes::ObjectType);
ValueClass *BuiltInTypes::FieldRefType = new ValueClass("FieldReference", BuiltInTypes::ObjectType);
ValueClass *BuiltInTypes::ArrayRefType = new ValueClass("ArrayReference", BuiltInTypes::ObjectType);
ValueClass *BuiltInTypes::NullType = new ValueClass("Null", BuiltInTypes::ObjectType);

Value *Value::NullValue;
Value::Value()
{
    mark = 0;
}
Value::~Value()
{
    // In the cases of RefVal, ArrayVal and StringVal
    // we _always_ assume the Value owns the stored pointer.

    // In the case of RawVal, we assume the external code that passed the RawVal
    // to the 'Value' object owns the passed RawVal.
    switch(this->tag)
    {
    case Int:
    case Double:
    case Boolean:
    case RawVal:
        break;
    case ObjectVal:
        delete v.objVal;
        break;
    case StringVal:
        delete v.strVal;
        break;
    case ArrayVal:
        delete[] v.arrayVal->Elements;
        delete v.arrayVal;
        break;
    case MultiDimensionalArrayVal:
        delete v.multiDimensionalArrayVal;
        break;
    case RefVal:
        delete v.refVal;
        break;
    case NullVal:
        break;
    }
}

QString Object::toString()
{
    return QString("%1").arg((long)this);
}
bool Object::hasSlot(QString name)
{
    return _slots.contains(name);
}
Value *Object::getSlotValue(QString name)
{
    return _slots[name];
}
void Object::setSlotValue(QString name, Value *val)
{
    _slots[name] = val;
}

void FieldReference::Set(Value *v)
{
    object->setSlotValue(SymRef, v);
}
Value *FieldReference::Get()
{
    return object->getSlotValue(SymRef);
}

void ArrayReference::Set(Value *val)
{
    array->Elements[index] = val;
}
Value *ArrayReference::Get()
{
    return array->Elements[index];
}

void MultiDimensionalArrayReference::Set(Value *val)
{
    array->set(index, val);
}
Value *MultiDimensionalArrayReference::Get()
{
    return array->get(index);
}

int Value::unboxInt()
{
    return v.intVal;
}

double Value::unboxDouble()
{
    return v.doubleVal;
}

bool Value::unboxBool()
{
    return v.boolVal;
}

Object *Value::unboxObj()
{
    return v.objVal;
}
VArray *Value::unboxArray()
{
    return v.arrayVal;
}
MultiDimensionalArray<Value *> *Value::unboxMultiDimensionalArray()
{
    return v.multiDimensionalArrayVal;
}

QString *Value::unboxStr()
{
    return v.strVal;
}

void *Value::unboxRaw()
{
    return v.rawVal;
}

Reference *Value::unboxRef()
{
    return v.refVal;
}

QString ArrayToString(VArray *arr)
{
    QStringList lst;
    for(int i=0; i<arr->count; i++)
        lst.append(arr->Elements[i]->toString());
    return QString("[%1]").arg(lst.join(", "));
}

QString Value::toString()
{
    QString *sv = NULL;
    QString ret = "<unprintable value>";
    void *val ;
    Value *v = this;
    //QLocale loc(QLocale::Arabic, QLocale::Egypt);
    QLocale loc(QLocale::English, QLocale::UnitedStates);
    loc.setNumberOptions(QLocale::OmitGroupSeparator);
    switch(v->tag)
    {
    case Int:
        ret = loc.toString(v->unboxInt());
        break;
    case Double:
        ret = QString("%1").arg(v->unboxDouble());
        break;
    case ObjectVal:
        ret = QString("<%1>").arg(v->type->getName());
        break;
    case StringVal:
        val = v->unboxStr();
        sv = (QString *) val;
        ret = *sv;
        break;
    case NullVal:
        ret = QString::fromWCharArray(L"<لاشيء>");
        break;
    case RawVal:
        ret = QString("<raw %1>").arg((long) v->unboxRaw());
        break;
    case RefVal:
        ret = "<a reference>";
    case ArrayVal:
        ret = ArrayToString(v->unboxArray());
        break;
    case Boolean:
        ret = v->unboxBool()? QString::fromStdWString(L"صحيح") : QString::fromStdWString(L"خطأ");
        break;
    }
    QString str = ret;
    return str;
}

ValueClass::ValueClass(QString name, ValueClass *baseClass)
{
    this->name = name;
    if(baseClass !=NULL)
        this->BaseClasses.append(baseClass);

}

bool ValueClass::hasSlot(QString name)
{
    return false;
}

Value *ValueClass::getSlotValue(QString name)
{
    return NULL;
}

void ValueClass::setSlotValue(QString name, Value *val)
{

}

QString ValueClass::getName()
{
    return name;
}
bool ValueClass::hasField(QString name)
{
    return fields.contains(name);
}
IClass *ValueClass::baseClass()
{
    if(BaseClasses.count() == 0)
        return NULL;
    return BaseClasses[0];
}
bool ValueClass::subclassOf(ValueClass *c)
{
    if(c == this)
        return true;

    for(int i=0; i<BaseClasses.count(); i++)
    {
        if(BaseClasses[i]->subclassOf(c))
            return true;
    }
    return  false;
}

Method *ValueClass::lookupMethod(QString name)
{
    if(methods.contains(name))
        return (Method *) methods[name]->v.objVal;
    for(int i=0; i<BaseClasses.count(); i++)
    {
        Method *ret = BaseClasses[i]->lookupMethod(name);
        if(ret != NULL)
            return ret;
    }
    return NULL;
}

ValueClass::~ValueClass()
{

}
QString ValueClass::toString()
{
    return "class "+ name;
}

