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
#include "references.h"
#include "vmerror.h"
#include "vmutils.h"
#include "runtime/spriteclass.h"

#define QSTR(x) QString::fromStdWString(x)

ValueClass *BuiltInTypes::ObjectType = new ValueClass(QSTR(L"شيء"), NULL);
ValueClass *BuiltInTypes::NumericType = new ValueClass(QSTR(L"عددي"), BuiltInTypes::ObjectType);
ValueClass *BuiltInTypes::IntType = new ValueClass(QSTR(L"عدد.صحيح"), BuiltInTypes::NumericType);
ValueClass *BuiltInTypes::LongType = new ValueClass(QSTR(L"عدد.صحيح.واسع"), BuiltInTypes::NumericType);
ValueClass *BuiltInTypes::DoubleType = new ValueClass(QSTR(L"عدد.حقيقي"), BuiltInTypes::NumericType);
ValueClass *BuiltInTypes::BoolType = new ValueClass(QSTR(L"قيمة.منطقية"), BuiltInTypes::ObjectType);
ValueClass *BuiltInTypes::MethodType = new ValueClass("Method", BuiltInTypes::ObjectType);
ValueClass *BuiltInTypes::ExternalMethodType = new ValueClass("ExternalMethod", BuiltInTypes::ObjectType);
ValueClass *BuiltInTypes::ExternalLibrary = new ValueClass("ExternalLibrary", BuiltInTypes::ObjectType);
MetaClass  *BuiltInTypes::ClassType = new MetaClass("Class", NULL);
ValueClass *BuiltInTypes::IndexableType = new ValueClass(QSTR(L"مفهرس"), BuiltInTypes::ObjectType);
ValueClass *BuiltInTypes::ArrayType = new ValueClass(QSTR(L"مصفوفة.قيم"), BuiltInTypes::IndexableType);
ValueClass *BuiltInTypes::MapType = new ValueClass(QSTR(L"قاموس.قيم"), BuiltInTypes::IndexableType);
ValueClass *BuiltInTypes::StringType = new ValueClass(QSTR(L"نص"), BuiltInTypes::IndexableType);
IClass *BuiltInTypes::SpriteType = new SpriteClass(QSTR(L"طيف"));
ValueClass *BuiltInTypes::FileType = NULL;
ValueClass *BuiltInTypes::RawFileType = new ValueClass("RawFile", BuiltInTypes::ObjectType);
ValueClass *BuiltInTypes::WindowType = new ValueClass("Window", BuiltInTypes::ObjectType);
ValueClass *BuiltInTypes::RefType = new ValueClass("Reference", BuiltInTypes::ObjectType);
ValueClass *BuiltInTypes::FieldRefType = new ValueClass("FieldReference", BuiltInTypes::ObjectType);
ValueClass *BuiltInTypes::ArrayRefType = new ValueClass("ArrayReference", BuiltInTypes::ObjectType);
ValueClass *BuiltInTypes::NullType = new ValueClass(QSTR(L"لاشيء"), BuiltInTypes::ObjectType);
ValueClass *BuiltInTypes::ChannelType = new ValueClass(QSTR(L"قناة"), BuiltInTypes::ObjectType);
ValueClass *BuiltInTypes::QObjectType = new ValueClass(QSTR(L"QObject"), BuiltInTypes::ObjectType);
IClass *BuiltInTypes::ActivationFrameType = new FrameClass(QSTR(L"إطار.تفعيل"));

ValueClass *BuiltInTypes::c_int = new ValueClass(QSTR(L"صحيح32.سي"), BuiltInTypes::ObjectType);
ValueClass *BuiltInTypes::c_long = new ValueClass(QSTR(L"طويل.سي"), BuiltInTypes::ObjectType);
ValueClass *BuiltInTypes::c_float = new ValueClass(QSTR(L"طفوي.سي"), BuiltInTypes::ObjectType);
ValueClass *BuiltInTypes::c_double = new ValueClass(QSTR(L"مزدوج.سي"), BuiltInTypes::ObjectType);
ValueClass *BuiltInTypes::c_char = new ValueClass(QSTR(L"حرفي.سي"), BuiltInTypes::ObjectType);
ValueClass *BuiltInTypes::c_asciiz = new ValueClass(QSTR(L"نص.آسكي.سي"), BuiltInTypes::ObjectType);
ValueClass *BuiltInTypes::c_wstr = new ValueClass(QSTR(L"نص.سي"), BuiltInTypes::ObjectType);
ValueClass *BuiltInTypes::c_ptr= new ValueClass(QSTR(L"مشير.سي"), BuiltInTypes::ObjectType);

Value *Value::NullValue = NULL;

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
    case Long:
    case Double:
    case Boolean:
    case RawVal:
        break;
    case ObjectVal:
        delete v.objVal;
        break;
    case StringVal:
        // not needed if we don't use a pointer to QString
        //delete v.strVal;
        break;
    case ArrayVal:
        delete[] v.arrayVal->Elements;
        delete v.arrayVal;
        break;
    case MapVal:
        //todo: leak here?
        break;
    case MultiDimensionalArrayVal:
        delete v.multiDimensionalArrayVal;
        break;
    case RefVal:
        delete v.refVal;
        break;
    case ChannelVal:
        delete v.channelVal;
        break;
    case NullVal:
        break;
    case QObjectVal:
        // todo: should we delete QObject values?
        // delete v.objVal;
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

QList<QString> Object::getSlotNames()
{
    return _slots.keys();
}

Value *Object::getSlotValue(QString name)
{
    return _slots[name];
}
void Object::setSlotValue(QString name, Value *val)
{
    _slots[name] = val;
}

int Value::unboxInt() const
{
    return v.intVal;
}

long Value::unboxLong() const
{
    return v.longVal;
}

double Value::unboxDouble() const
{
    return v.doubleVal;
}

double Value::unboxNumeric()
{
    if(tag == Int)
        return unboxInt();
    if(tag == Double)
        return unboxDouble();
    if(tag == Long)
        return unboxLong();
    // This should not be called
    return 0.0;
}

VIndexable *Value::unboxIndexable() const
{
    if(tag == ArrayVal)
        return unboxArray();
    if(tag == MapVal)
        return unboxMap();
    // This should not be called
    return NULL;
}

bool Value::unboxBool() const
{
    return v.boolVal;
}

IObject *Value::unboxObj() const
{
    return v.objVal;
}

VArray *Value::unboxArray() const
{
    return v.arrayVal;
}

VMap *Value::unboxMap() const
{
    return v.mapVal;
}

MultiDimensionalArray<Value *> *Value::unboxMultiDimensionalArray() const
{
    return v.multiDimensionalArrayVal;
}

QString Value::unboxStr() const
{
    return vstrVal;
}

void *Value::unboxRaw() const
{
    return v.rawVal;
}

Reference *Value::unboxRef() const
{
    return v.refVal;
}

Channel *Value::unboxChan() const
{
    return v.channelVal;
}

QObject *Value::unboxQObj() const
{
    return v.qobjVal;
}

QString ArrayToString(VArray *arr)
{
    QStringList lst;
    for(int i=0; i<arr->count(); i++)
        lst.append(arr->Elements[i]->toString());
    return QString("[%1]").arg(lst.join(", "));
}

QString MapToString(VMap *map)
{
    QStringList lst;
    for(QMap<Value, Value *>::const_iterator i=map->Elements.begin(); i!=map->Elements.end(); ++i)
    {
        const Value &key = i.key();
        const Value *v = i.value();
        lst.append(QString("%1=%2").arg(key.toString()).arg(v->toString()));
    }

    return QString("{%1}").arg(lst.join(", "));
}

QString Value::toString() const
{
    QString sv;
    QString ret = "<unprintable value>";
    QStringList elems;
    void *val ;
    const Value *v = this;
    //QLocale loc(QLocale::Arabic, QLocale::Egypt);
    QLocale loc(QLocale::English, QLocale::UnitedStates);
    loc.setNumberOptions(QLocale::OmitGroupSeparator);
    QVector<int> dims;
    switch(v->tag)
    {
    case Int:
        ret = loc.toString(v->unboxInt());
        break;
    case Long:
        ret = loc.toString((qlonglong)v->unboxLong());
        break;
    case Double:
        ret = QString("%1").arg(v->unboxDouble());
        break;
    case ObjectVal:
        ret = QString("<%1>").arg(v->type->getName());
        break;
    case StringVal:
        ret = v->unboxStr();
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
    case MultiDimensionalArrayVal:
        dims = v->unboxMultiDimensionalArray()->dimensions;

        for(QVector<int>::const_iterator i=dims.begin(); i != dims.end(); ++i)
            elems.append(QString("%1").arg(*i));
        ret = QString::fromWCharArray(L"]مصفوفة بأبعاد [%1>").arg(elems.join(", "));
        break;
    case MapVal:
        ret = MapToString(v->unboxMap());
        break;
    case Boolean:
        ret = v->unboxBool()? QString::fromStdWString(L"صحيح") : QString::fromStdWString(L"خطأ");
        break;
    case ChannelVal:
        ret = QSTR(L"<قناة %1>").arg((long) v->unboxChan());
        break;
    case QObjectVal:
        ret = QString("<%1>").arg(v->unboxQObj()->metaObject()->className());
    }
    QString str = ret;
    return str;
}

bool VArray::keyCheck(Value *key, VMError &err)
{
    if(key->tag != Int)
    {
        err = VMError(SubscribtMustBeInteger);
        return false;
    }
    int i = key->unboxInt();
    if(!(i>=1 && i<=this->count()))
    {
        err = VMError(SubscriptOutOfRange2).arg(str(i)).arg(str(this->count()));
        return false;
    }
    return true;
}

Value *VArray::get(Value *key)
{
    int i = key->unboxInt();
    return this->Elements[i-1];
}

void VArray::set(Value *key, Value *v)
{
    int i = key->unboxInt();
    this->Elements[i-1] = v;
}

bool VMap::keyCheck(Value *key, VMError &err)
{
    if(key->tag == Int || key->tag == StringVal || key->tag == Long)
        return true;
    if(key->tag == ArrayVal)
    {
        VArray *arr = key->unboxArray();
        for(int i=0; i<arr->count(); i++)
        {
            if(!keyCheck(arr->Elements[i], err))
                return false;
        }
        return true;
    }
    err = VMError(UnacceptableKeyFormMap1).arg(key->type->toString());
    return false;
}

Value *VMap::get(Value *key)
{
    if(Elements.contains(*key))
        return this->Elements[*key];
    return NULL;
}

void VMap::set(Value *key, Value *v)
{
    allKeys.append(key);
    Elements[*key] = v;
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

QList<QString> ValueClass::getSlotNames()
{
    return QList<QString>();
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

bool ValueClass::subclassOf(IClass *c)
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

IMethod *ValueClass::lookupMethod(QString name)
{
    if(methods.contains(name))
        return (IMethod *) methods[name]->v.objVal;
    for(int i=0; i<BaseClasses.count(); i++)
    {
        IMethod *ret = BaseClasses[i]->lookupMethod(name);
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
    return "class " + name;
}

ForeignClass::ForeignClass(QString name)
{
   this->name = name;
}

bool ForeignClass::hasSlot(QString name)
{
    return false;
}

QList<QString> ForeignClass::getSlotNames()
{
    return QList<QString>();
}

Value *ForeignClass::getSlotValue(QString name)
{
    return NULL;
}

void ForeignClass::setSlotValue(QString name, Value *val)
{

}

QString ForeignClass::getName()
{
    return this->name;
}

IClass *ForeignClass::baseClass()
{
    return BuiltInTypes::ObjectType;
}

bool ForeignClass::subclassOf(IClass *c)
{
    return c == this || this->baseClass()->subclassOf(c);
}

QString ForeignClass::toString()
{
    return this->name;
}

bool LexicographicLessThan(VArray *arr1, VArray *arr2)
{
    for(int i=0; i<arr2->count(); i++)
    {
        if(i== arr1->count())
            return true; // arr1 is a prefix of arr2
        Value &v1 = *arr1->Elements[i];
        Value &v2 = *arr2->Elements[i];
        if(v1 < v2)
            return true;
        else if(v1 == v2)
            continue;
        else
            return false;

    }
    return false;
}

inline bool operator<(const Value &v1, const Value &v2)
{
    // Only int, long, string, and arrays of [comparable value] are comparable values
    // Since we need a partial order, we will assume ints <long < string < array
    if(v1.tag == Int && v2.tag == Int)
        return v1.unboxInt() < v2.unboxInt();

    if(v1.tag == Long && v2.tag == Long)
        return v1.unboxLong() < v2.unboxLong();

    if(v1.tag == StringVal && v2.tag == StringVal)
        return v1.unboxStr() < v2.unboxStr();

    if(v1.tag == ArrayVal && v2.tag == ArrayVal)
        return LexicographicLessThan(v1.unboxArray(), v2.unboxArray());

    if(v1.tag == Int &&
            (v2.tag == Long || v2.tag == StringVal || v2.tag == ArrayVal))
    {
        return true;
    }

    if(v1.tag == Long && (v2.tag == StringVal || v2.tag == ArrayVal))
    {
        return true;
    }

    if(v1.tag == StringVal && v2.tag == ArrayVal)
    {
        return true;
    }

    return false;
}

bool ElementWiseCompare(VArray *arr1, VArray *arr2)
{
    if(arr1->count() != arr2->count())
        return false;
    for(int i=0; i<arr1->count(); i++)
    {
        Value &v1 = *arr1->Elements[i];
        Value &v2 = *arr2->Elements[i];
        if(!(v1 == v2))
            return false;
    }
    return true;
}

inline bool operator==(const Value &v1, const Value &v2)
{
    // The == function is only used as a helper for LexicographicLessThan, defined above
    // Only int, string, and arrays of [comparable value] are comparable values

    if(v1.tag != v2.tag)
        return false;

    if(v1.tag == Int)
        return v1.unboxInt() == v2.unboxInt();

    if(v1.tag == StringVal)
        return v1.unboxStr() == v2.unboxStr();

    if(v1.tag == ArrayVal && v2.tag == ArrayVal)
        return ElementWiseCompare(v1.unboxArray(), v2.unboxArray());

    return false;
}
