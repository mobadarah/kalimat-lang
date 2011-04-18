/**************************************************************************
**   The Kalimat programming language
**   Copyright (C) 2010 Mohamed Samy Ali - samy2004@gmail.com
**   This project is released under the Apache License version 2.0
**   as described in the included license.txt file
**************************************************************************/

#ifndef VALUE_H
#define VALUE_H

#ifndef CLASSES_H
    #include "classes.h"
#endif

#ifndef MULTIDIMENSIONALARRAY_H
    #include "multidimensionalarray.h"
#endif

class Value;
class ValueClass;
class Method;
struct VArray;
struct Reference;

enum Tag
{
    Int, Double, Boolean, ObjectVal, NullVal, StringVal, RawVal,
    RefVal, ArrayVal, MultiDimensionalArrayVal
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
    bool boolVal;
    IObject *objVal;
    void *rawVal;
    QString *strVal;
    Reference *refVal;
    VArray *arrayVal;
    MultiDimensionalArray<Value *> *multiDimensionalArrayVal;
};

struct Value
{
    char mark;
    IClass *type;
    Tag tag;
    ValueItem v;

    Value();
    ~Value();
    int unboxInt();
    double unboxDouble();
    bool unboxBool();
    IObject *unboxObj();
    VArray *unboxArray();
    MultiDimensionalArray<Value *> *unboxMultiDimensionalArray();
    void *unboxRaw();
    QString *unboxStr();
    Reference *unboxRef();
    QString toString();

    double unboxNumeric();
    static Value *NullValue;
};

class BuiltInTypes
{
public:
    static ValueClass *ObjectType; // Must be declare before all of the other types, since their
                                   // Initialization depends on it

    static ValueClass *NumericType;
    static ValueClass *IntType;
    static ValueClass *DoubleType;
    static ValueClass *BoolType;
    static ValueClass *MethodType;
    static ValueClass *ExternalMethodType;
    static ValueClass *ClassType;
    static ValueClass *ArrayType;
    static ValueClass *StringType;
    static ValueClass *SpriteType;
    static ValueClass *FileType;
    static ValueClass *RawFileType;
    static ValueClass *RawWindowType;
    static ValueClass *RefType;
    static ValueClass *FieldRefType;
    static ValueClass *ArrayRefType;
    static ValueClass *NullType;
};

#endif // VALUE_H
