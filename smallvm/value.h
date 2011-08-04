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

#ifndef CHANNEL_H
    #include "channel.h"
#endif

#ifndef METACLASS_H
    #include "metaclass.h"
#endif


class Value;
class ValueClass;
class Method;
struct VArray;
struct Reference;
class Channel;
class VMError;

enum Tag
{
    Int, Double, Boolean, ObjectVal, NullVal, StringVal, RawVal,
    RefVal, ArrayVal, MultiDimensionalArrayVal, MapVal, ChannelVal, QObjectVal
};

struct VIndexable
{
    virtual bool keyCheck(Value *key, VMError &err) = 0;
    virtual void set(Value *key, Value *v) = 0;
    virtual Value *get(Value *key) = 0;
};

struct VArray : public VIndexable
{
    Value **Elements;
    int count;
    bool keyCheck(Value *key, VMError &err);
    void set(Value *key, Value *v);
    Value *get(Value *key);
};

struct VMap : public VIndexable
{
    QVector<Value *> allKeys; //We need this for the GC, since the map itself stores the values as keys, not their pointers
    QMap<Value, Value *> Elements;
    bool keyCheck(Value *key, VMError &err);
    void set(Value *key, Value *v);
    Value *get(Value *key);
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
    VMap *mapVal;
    Channel *channelVal;
    QObject *qobjVal;
};

struct Value
{
    char mark;
    IClass *type;
    Tag tag;
    ValueItem v;

    Value();
    ~Value();
    int unboxInt() const;
    double unboxDouble()  const;
    bool unboxBool()  const;
    IObject *unboxObj()  const;
    VArray *unboxArray() const;
    MultiDimensionalArray<Value *> *unboxMultiDimensionalArray();
    VMap *unboxMap()  const;
    void *unboxRaw()  const;
    QString *unboxStr() const;
    Reference *unboxRef() const;
    Channel *unboxChan() const;
    QObject *unboxQObj() const;
    QString toString() const;

    double unboxNumeric();
    VIndexable *unboxIndexable() const;
    static Value *NullValue;
};

// So that we can add (some types of) values to QMap
inline bool operator<(const Value &v1, const Value &v2);
inline bool operator==(const Value &v1, const Value &v2);

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
    static ValueClass *ExternalLibrary;
    static MetaClass  *ClassType;
    static ValueClass *IndexableType;
    static ValueClass *ArrayType;
    static ValueClass *MapType;
    static ValueClass *StringType;
    static ValueClass *SpriteType;
    static ValueClass *FileType;
    static ValueClass *RawFileType;
    static ValueClass *WindowType;
    static ValueClass *RefType;
    static ValueClass *FieldRefType;
    static ValueClass *ArrayRefType;
    static ValueClass *NullType;
    static ValueClass *ChannelType;
    static ValueClass *QObjectType;

    // For FFI
    static ValueClass *c_int;
    static ValueClass *c_long;
    static ValueClass *c_float;
    static ValueClass *c_double;
    static ValueClass *c_char;
    static ValueClass *c_asciiz; // Your standard C ascii null-terminated string
    static ValueClass *c_wstr;  // C wide string, pointer to wchar_t
    static ValueClass *c_ptr;
};

#endif // VALUE_H
