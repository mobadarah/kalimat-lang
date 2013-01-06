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
#include "runtime/spriteclass.h"
#include "runtime_identifiers.h"

#define QSTR(x) QString::fromStdWString(x)

ValueClass *BuiltInTypes::ObjectType = NULL;
IClass *BuiltInTypes::NumericType = NULL;
IClass *BuiltInTypes::IntType = NULL;
IClass *BuiltInTypes::LongType = NULL;
IClass *BuiltInTypes::DoubleType = NULL;
ValueClass *BuiltInTypes::BoolType = NULL;
ValueClass *BuiltInTypes::RawType = NULL;
IClass *BuiltInTypes::IMethodType = NULL;
IClass *BuiltInTypes::MethodType = NULL;
ValueClass *BuiltInTypes::ExternalMethodType = NULL;
ValueClass *BuiltInTypes::ExternalLibrary = NULL;
MetaClass  *BuiltInTypes::ClassType = NULL;
ValueClass *BuiltInTypes::IndexableType = NULL;
ValueClass *BuiltInTypes::ArrayType = NULL;
ValueClass *BuiltInTypes::MD_ArrayType= NULL;
ValueClass *BuiltInTypes::MapType = NULL;
IClass *BuiltInTypes::StringType = NULL;
IClass *BuiltInTypes::SpriteType = NULL;
ValueClass *BuiltInTypes::FileType = NULL;
ValueClass *BuiltInTypes::RawFileType = NULL;
ValueClass *BuiltInTypes::WindowType = NULL;
ValueClass *BuiltInTypes::RefType = NULL;
ValueClass *BuiltInTypes::FieldRefType = NULL;
ValueClass *BuiltInTypes::ArrayRefType = NULL;
ValueClass *BuiltInTypes::NullType = NULL;
ValueClass *BuiltInTypes::ChannelType = NULL;
ValueClass *BuiltInTypes::QObjectType = NULL;
ValueClass *BuiltInTypes::LambdaType = NULL;
IClass *BuiltInTypes::ActivationFrameType = NULL;

ValueClass *BuiltInTypes::c_int = NULL;
ValueClass *BuiltInTypes::c_long = NULL;
ValueClass *BuiltInTypes::c_float = NULL;
ValueClass *BuiltInTypes::c_double = NULL;
ValueClass *BuiltInTypes::c_char = NULL;
ValueClass *BuiltInTypes::c_asciiz = NULL;
ValueClass *BuiltInTypes::c_wstr = NULL;
ValueClass *BuiltInTypes::c_void = NULL;
ValueClass *BuiltInTypes::c_ptr= NULL;

bool eq_int(Value *a, Value *b) { return unboxInt(a) == unboxInt(b);}
bool eq_long(Value *a, Value *b) { return unboxLong(a) == unboxLong(b);}
bool eq_double(Value *a, Value *b) { return unboxDouble(a) == unboxDouble(b);}
bool eq_bool(Value *a, Value *b) { return unboxBool(a) == unboxBool(b);}
bool eq_raw(Value *a, Value *b) { return unboxRaw(a) == unboxRaw(b);}
bool eq_qobject(Value *a, Value *b) { return unboxQObj(a) == unboxQObj(b);}
bool  eq_str(Value *a, Value *b)
{
    return QString::compare(unboxStr(a), unboxStr(b), Qt::CaseSensitive) == 0;
}

bool eq_null(Value *a, Value *b) { return true;}

void BuiltInTypes::init()
{
    if(ObjectType != NULL)
        return;

    ObjectType = new ValueClass(VMId::get(RId::Object), NULL);
    NumericType = new NumericClass();
    IntType = new IntClass();
    LongType = new LongClass();
    DoubleType = new DoubleClass();
    BoolType = new ValueClass(VMId::get(RId::Boolean), BuiltInTypes::ObjectType);
    RawType = new ValueClass(VMId::get(RId::Raw), BuiltInTypes::ObjectType);
    RawType->equality = eq_raw;

    IMethodType = new MethodClass(VMId::get(RId::IMethod), BuiltInTypes::ObjectType);
    MethodType = new MethodClass(VMId::get(RId::Method), BuiltInTypes::IMethodType);
    ExternalMethodType = new ValueClass(VMId::get(RId::ExternalMethod), BuiltInTypes::IMethodType);
    // Actually, ExternalMethodType is-a method, but has equality using eq_raw
    // the correct way to define it's behavior is using multiple inheritance or
    // something similar if we had it..
    ExternalMethodType->equality = eq_raw;
    ExternalLibrary = new ValueClass(VMId::get(RId::ExternalLibrary), BuiltInTypes::RawType);
    ClassType = new MetaClass(VMId::get(RId::Class), NULL);
    IndexableType = new ValueClass(VMId::get(RId::Indexable), BuiltInTypes::ObjectType);
    ArrayType = new ValueClass(VMId::get(RId::VArray), BuiltInTypes::IndexableType);
    MD_ArrayType = new ValueClass(VMId::get(RId::MD_Array), BuiltInTypes::ObjectType);
    MapType = new ValueClass(VMId::get(RId::VMap), BuiltInTypes::IndexableType);
    StringType = new StringClass();
    SpriteType = new SpriteClass(VMId::get(RId::Sprite));
    FileType = NULL;
    RawFileType = new ValueClass(VMId::get(RId::RawFile), BuiltInTypes::RawType);
    WindowType = new ValueClass(VMId::get(RId::Window), BuiltInTypes::ObjectType);
    RefType = new ValueClass(VMId::get(RId::Reference), BuiltInTypes::ObjectType);
    FieldRefType = new ValueClass(VMId::get(RId::FieldReference), BuiltInTypes::ObjectType);
    ArrayRefType = new ValueClass(VMId::get(RId::ArrayReference), BuiltInTypes::ObjectType);
    NullType = new ValueClass(VMId::get(RId::NullType), BuiltInTypes::ObjectType);
    NullType->equality = eq_null;
    ChannelType = new ValueClass(VMId::get(RId::Channel), BuiltInTypes::ObjectType);
    QObjectType = new ValueClass(VMId::get(RId::QObject), BuiltInTypes::ObjectType);
    QObjectType->equality = eq_qobject;
    LambdaType = new ValueClass("%lambda", BuiltInTypes::ObjectType);

    IntType->equality = eq_int;
    LongType->equality = eq_long;
    DoubleType->equality = eq_double;
    BoolType->equality = eq_bool;
    StringType->equality = eq_str;

    c_int = new ValueClass(VMId::get(RId::c_int32), BuiltInTypes::ObjectType);
    c_long = new ValueClass(VMId::get(RId::c_long), BuiltInTypes::ObjectType);
    c_float = new ValueClass(VMId::get(RId::c_float), BuiltInTypes::ObjectType);
    c_double = new ValueClass(VMId::get(RId::c_double), BuiltInTypes::ObjectType);
    c_char = new ValueClass(VMId::get(RId::c_char), BuiltInTypes::ObjectType);
    c_asciiz = new ValueClass(VMId::get(RId::c_ascii), BuiltInTypes::ObjectType);
    c_wstr = new ValueClass(VMId::get(RId::c_wstr), BuiltInTypes::ObjectType);
    c_ptr= new ValueClass(VMId::get(RId::c_pointer), BuiltInTypes::RawType);

    c_void = new ValueClass(VMId::get(RId::c_void), BuiltInTypes::ObjectType);
}

Value *Value::NullValue = NULL;

Value::Value()
{
    mark = 0;
    heapNext = NULL;
}

bool Value::intEqualsMe(int)
{
    return false;
}

bool Value::doubleEqualsMe(double)
{
    return false;
}

bool Value::longEqualsMe(long)
{
    return false;
}

// Destructors for various types of values

// In the cases of RefVal, ArrayVal and StringVal
// we _always_ assume the Value owns the stored pointer.

// In the case of RawVal, we assume the external code that passed the RawVal
// to the 'Value' object owns the passed RawVal.

ObjVal::~ObjVal()
{
    delete v;
}

ArrayVal::~ArrayVal()
{
    delete[] v->Elements;
    delete v;
}

MapVal::~MapVal()
{
  //todo: leak here?
}

MultiDimensionalArrayVal::~MultiDimensionalArrayVal()
{
     delete v;
}

RefVal::~RefVal()
{
    delete v;
}

ChannelVal::~ChannelVal()
{
    delete v;
}

QObjVal::~QObjVal()
{
    // todo: should we delete QObject values?
    // delete v;
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
    for(QMap<VBox, Value *>::const_iterator i=map->Elements.begin(); i!=map->Elements.end(); ++i)
    {
        const Value *key = i.key().v;
        const Value *v = i.value();
        lst.append(QString("%1=%2").arg(key->toString()).arg(v->toString()));
    }

    return QString("{%1}").arg(lst.join(", "));
}

QString IntVal::toString() const
{
    QLocale loc(QLocale::English, QLocale::UnitedStates);
    loc.setNumberOptions(QLocale::OmitGroupSeparator);
    return loc.toString(v);
}

bool IntVal::equals(Value *v2)
{
    return v2->intEqualsMe(v);
}

bool IntVal::intEqualsMe(int v1)
{
    return v1 == v;
}

bool IntVal::doubleEqualsMe(double d1)
{
    return d1 == v;
}

bool IntVal::longEqualsMe(long l1)
{
    return l1 == v;
}

QString LongVal::toString() const
{
    QLocale loc(QLocale::English, QLocale::UnitedStates);
    loc.setNumberOptions(QLocale::OmitGroupSeparator);
    return loc.toString((qlonglong) v);
}

bool LongVal::equals(Value *v2)
{
    return v2->longEqualsMe(v);
}

bool LongVal::intEqualsMe(int v1)
{
    return v1 == v;
}

bool LongVal::doubleEqualsMe(double d1)
{
    return d1 == v;
}

bool LongVal::longEqualsMe(long l1)
{
    return l1 == v;
}

QString DoubleVal::toString() const
{
    QLocale loc(QLocale::English, QLocale::UnitedStates);
    loc.setNumberOptions(QLocale::OmitGroupSeparator);
    return QString("%1").arg(v);
}

bool DoubleVal::equals(Value *v2)
{
    return v2->doubleEqualsMe(v);
}

bool DoubleVal::intEqualsMe(int v1)
{
    return v1 == v;
}

bool DoubleVal::doubleEqualsMe(double d1)
{
    return d1 == v;
}

bool DoubleVal::longEqualsMe(long l1)
{
    return l1 == v;
}

QString NullVal::toString() const
{
    return QString("<%1>").arg(VMId::get(RId::NullValue));
}

bool NullVal::equals(Value *v2)
{
    return v2->type == BuiltInTypes::NullType;
}

QString ObjVal::toString() const
{
    return QString("<%1>").arg(type->getName());
}

bool ObjVal::equals(Value *v2)
{
    return v2 == this;
}

QString StringVal::toString() const
{
    return v;
}

bool StringVal::equals(Value *v2)
{
    return v2->type == BuiltInTypes::StringType
            && v == unboxStr(v2);
}

QString RawVal::toString() const
{
    return QString("<raw %1>").arg((long) v);
}

bool RawVal::equals(Value *v2)
{
    return v2->type == this->type
            && v == unboxRaw(v2);
}

QString RefVal::toString() const
{
    return "<a reference>";
}

bool RefVal::equals(Value *v2)
{
    return v2->type == this->type
            && v == unboxRef(v2);
}

QString ArrayVal::toString() const
{
    return ArrayToString(v);
}

bool ArrayVal::equals(Value *v2)
{
    return this == v2;
}

QString MultiDimensionalArrayVal::toString() const
{
    QVector<int> dims;
    QStringList elems;
    dims = v->dimensions;

    for(QVector<int>::const_iterator i=dims.begin(); i != dims.end(); ++i)
        elems.append(QString("%1").arg(*i));
    return VMId::get(RId::ArrayWithDims1).arg(elems.join(", "));
}

bool MultiDimensionalArrayVal::equals(Value *v2)
{
    return this == v2;
}

QString MapVal::toString() const
{
    return MapToString(v);
}

bool MapVal::equals(Value *v2)
{
    return this == v2;
}

QString BoolVal::toString() const
{
    return v? VMId::get(RId::TrueValue) : VMId::get(RId::FalseValue);
}

bool BoolVal::equals(Value *v2)
{
    return this == v2;
}

QString ChannelVal::toString() const
{
    return QString("<%1 %2>").arg(VMId::get(RId::ChannelValue)).arg((long) v);
}

bool ChannelVal::equals(Value *v2)
{
    return this == v2;
}

QString QObjVal::toString() const
{
    return QString("<%1>").arg(v->metaObject()->className());
}

bool QObjVal::equals(Value *v2)
{
    return v2->type == this->type && v == unboxQObj(v2);
}

bool VArray::keyCheck(Value *key, VMError &err)
{
    if(key->type != BuiltInTypes::IntType)
    {
        err = VMError(SubscribtMustBeInteger);
        return false;
    }
    int i = unboxInt(key);
    if(!(i>=1 && i<=this->count()))
    {
        err = VMError(SubscriptOutOfRange2).arg(str(i)).arg(str(this->count()));
        return false;
    }
    return true;
}

Value *VArray::get(Value *key)
{
    int i = unboxInt(key);
    return this->Elements[i-1];
}

void VArray::set(Value *key, Value *v)
{
    int i = unboxInt(key);
    this->Elements[i-1] = v;
}

bool VMap::keyCheck(Value *key, VMError &err)
{
    if(key->type == BuiltInTypes::IntType ||
       key->type == BuiltInTypes::StringType ||
       key->type == BuiltInTypes::LongType)
        return true;
    if(key->type == BuiltInTypes::ArrayType)
    {
        VArray *arr = unboxArray(key);
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
    VBox vb(key);
    if(Elements.contains(vb))
        return this->Elements[vb];
    return NULL;
}

void VMap::set(Value *key, Value *v)
{
    allKeys.append(key);
    VBox vb(key);
    Elements[vb] = v;
}

bool compareRef(Value *v1, Value *v2)
{
    return unboxObj(v1) == unboxObj(v2);
}

ValueClass::ValueClass(QString name, IClass *baseClass)
{
    this->name = name;
    if(baseClass !=NULL)
        this->BaseClasses.append(baseClass);
    if(baseClass != NULL)
        this->equality = baseClass->equality;
    else
        this->equality = compareRef;
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
        return (IMethod *) unboxObj(methods[name]);
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
    this->equality = compareRef;
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
        Value *v1 = arr1->Elements[i];
        Value *v2 = arr2->Elements[i];
        VBox vb1(v1), vb2(v2);
        if(vb1 < vb2)
            return true;
        else if(vb1 == vb2)
            continue;
        else
            return false;

    }
    return false;
}

inline bool operator<(const VBox &v1, const VBox &v2)
{
    // Only int, long, string, and arrays of [comparable value] are comparable values
    // Since we need a partial order, we will assume ints <long < string < array
    if(v1.v->type == BuiltInTypes::IntType && v2.v->type == BuiltInTypes::IntType)
        return unboxInt(v1.v) < unboxInt(v2.v);

    if(v1.v->type == BuiltInTypes::LongType && v2.v->type == BuiltInTypes::LongType)
        return unboxLong(v1.v) < unboxLong(v2.v);

    if(v1.v->type == BuiltInTypes::StringType && v2.v->type == BuiltInTypes::StringType)
        return unboxStr(v1.v) < unboxStr(v2.v);

    if(v1.v->type == BuiltInTypes::ArrayType && v2.v->type == BuiltInTypes::ArrayType)
        return LexicographicLessThan(unboxArray(v1.v), unboxArray(v2.v));

    if(v1.v->type == BuiltInTypes::IntType &&
            (v2.v->type == BuiltInTypes::LongType ||
             v2.v->type == BuiltInTypes::StringType ||
             v2.v->type == BuiltInTypes::ArrayType))
    {
        return true;
    }

    if(v1.v->type == BuiltInTypes::LongType &&
            (v2.v->type == BuiltInTypes::StringType ||
             v2.v->type == BuiltInTypes::ArrayType))
    {
        return true;
    }

    if(v1.v->type == BuiltInTypes::StringType && v2.v->type == BuiltInTypes::ArrayType)
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
        Value *v1 = arr1->Elements[i];
        Value *v2 = arr2->Elements[i];
        VBox vb1(v1), vb2(v2);
        if(!(vb1 == vb2))
            return false;
    }
    return true;
}

inline bool operator==(const VBox &v1, const VBox &v2)
{
    // The == function is only used as a helper for LexicographicLessThan, defined above
    // Only int, string, and arrays of [comparable value] are comparable values

    if(v1.v->type != v2.v->type)
        return false;

    if(v1.v->type == BuiltInTypes::IntType)
        return unboxInt(v1.v) == unboxInt(v2.v);

    if(v1.v->type == BuiltInTypes::StringType)
        return unboxStr(v1.v) == unboxStr(v2.v);

    if(v1.v->type == BuiltInTypes::ArrayType)
        return ElementWiseCompare(unboxArray(v1.v), unboxArray(v2.v));

    return false;
}
