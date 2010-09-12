/**************************************************************************
**   The Kalimat programming language
**   Copyright (C) 2010 Mohamed Samy Ali - samy2004@gmail.com
**   This project is released under the Apache License version 2.0
**   as described in the included license.txt file
**************************************************************************/

#ifndef VALUE_H
#define VALUE_H

#include "multidimensionalarray.h"

class Value;
class ValueClass;
class Method;
struct VArray;

enum Tag
{
    Int, Double, ObjectVal, NullVal, StringVal, RawVal,
    RefVal, ArrayVal, MultiDimensionalArrayVal
};

struct IObject
{
    virtual bool hasSlot(QString name)=0;
    virtual Value *getSlotValue(QString name)=0;
    virtual void setSlotValue(QString name, Value *val)=0;
    virtual QString toString() =0;
};

struct Object : public IObject
{
    virtual bool hasSlot(QString name);
    virtual Value *getSlotValue(QString name);
    virtual void setSlotValue(QString name, Value *val);
    virtual QString toString();
public:
    QMap<QString, Value *> _slots;

};
struct Reference
{
    virtual void Set(Value *val) = 0;
    virtual Value *Get() = 0;
};

struct FieldReference : public Reference
{
    Object *object;
    QString SymRef;

    void Set(Value *val);
    Value *Get();
};
struct ArrayReference : public Reference
{
    VArray *array;
    int index;

    void Set(Value *val);
    Value *Get();
};
struct MultiDimensionalArrayReference : public Reference
{
    MultiDimensionalArray<Value *> *array;
    QVector<int> index;

    void Set(Value *val);
    Value *Get();
};

struct IClass : public IObject
{
    virtual QString getName() = 0;
    virtual bool hasField(QString name) = 0;
    virtual IClass *baseClass() = 0;
    virtual bool subclassOf(ValueClass *c)=0; // Interpreted as "subclass of or equals"
    virtual Method *lookupMethod(QString name)=0;
};

struct ValueClass : public IClass, public Object
{
    ValueClass(QString name, ValueClass *baseClass);
    virtual ~ValueClass();

    // IObject
    virtual bool hasSlot(QString name);
    virtual Value *getSlotValue(QString name);
    virtual void setSlotValue(QString name, Value *val);

    //IClass
    QString getName();
    bool hasField(QString name);
    IClass *baseClass();
    virtual bool subclassOf(ValueClass *c);
    Method *lookupMethod(QString name);

    QString toString();
//private:
public:
    QString name;
    QSet<QString> fields;
    QVector<ValueClass*> BaseClasses;
    QMap<QString, Value*> methods;
};

struct VArray
{
    Value **Elements;
    int count;
};

union ValueItem
{
    int intVal;
    double doubleVal;
    Object *objVal;
    void *rawVal;
    QString *strVal;
    Reference *refVal;
    VArray *arrayVal;
    MultiDimensionalArray<Value *> *multiDimensionalArrayVal;
};

struct Value
{
    char mark;
    ValueClass *type;
    Tag tag;
    ValueItem v;

    Value();
    ~Value();
    int unboxInt();
    double unboxDouble();
    Object *unboxObj();
    VArray *unboxArray();
    MultiDimensionalArray<Value *> *unboxMultiDimensionalArray();
    void *unboxRaw();
    QString *unboxStr();
    Reference *unboxRef();

    QString toString();

static Value *NullValue;
};
class BuiltInTypes
{
public:
    static ValueClass *ObjectType; // Must be declare before all of the other types, since their
                                   // Initialization depends on it

    static ValueClass *IntType;
    static ValueClass *DoubleType;
    static ValueClass *MethodType;
    static ValueClass *ExternalMethodType;
    static ValueClass *ClassType;
    static ValueClass *ArrayType;
    static ValueClass *StringType;
    static ValueClass *SpriteType;
    static ValueClass *FileType;
    static ValueClass *RawFileType;
    static ValueClass *RefType;
    static ValueClass *FieldRefType;
    static ValueClass *ArrayRefType;
    static ValueClass *NullType;
};

#endif // VALUE_H
