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
    Int, Long, Double, Boolean, ObjectVal, NullVal, StringVal, RawVal,
    RefVal, ArrayVal, MultiDimensionalArrayVal, MapVal, ChannelVal, QObjectVal
};

struct VIndexable
{
    virtual bool keyCheck(Value *key, VMError &err) = 0;
    virtual void set(Value *key, Value *v) = 0;
    virtual Value *get(Value *key) = 0;
    virtual int count() = 0;
};

struct VArray : public VIndexable
{
    Value **Elements;
    int _count;
    bool keyCheck(Value *key, VMError &err);
    void set(Value *key, Value *v);
    Value *get(Value *key);
    int count() { return _count; }
};

struct VMap : public VIndexable
{
    QVector<Value *> allKeys; //We need this for the GC, since the map itself stores the values as keys, not their pointers
    QMap<Value, Value *> Elements;
    bool keyCheck(Value *key, VMError &err);
    void set(Value *key, Value *v);
    Value *get(Value *key);
    int count() { return Elements.count(); }
};

union ValueItem
{
    int intVal;
    long longVal;
    double doubleVal;
    bool boolVal;
    IObject *objVal;
    void *rawVal;
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
    // not in the union because QStrings have a constuctor
    // and a pointer to QString in the union would mean a lot of
    // allocation and copying. this adds 4 bytes to *every* value
    // though
    QString vstrVal;
    Value();
    ~Value();
    inline int unboxInt() const { return v.intVal; }
    inline long unboxLong() const { return v.longVal; }
    inline double unboxDouble()  const { return v.doubleVal; }
    inline bool unboxBool() const { return v.boolVal; }
    inline IObject *unboxObj()  const { return v.objVal; }
    inline IClass *unboxClass() const { return dynamic_cast<IClass *>(unboxObj()); }
    inline VArray *unboxArray() const { return v.arrayVal; }
    inline MultiDimensionalArray<Value *> *unboxMultiDimensionalArray() const
    {
        return v.multiDimensionalArrayVal;
    }
    inline VMap *unboxMap()  const { return v.mapVal; }
    inline void *unboxRaw()  const { return v.rawVal; }
    inline QString unboxStr() const { return vstrVal; }
    inline Reference *unboxRef() const { return v.refVal; }
    inline Channel *unboxChan() const { return v.channelVal; }
    inline QObject *unboxQObj() const { return v.qobjVal; }
    QString toString() const;

    inline double unboxNumeric()
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

    inline VIndexable *unboxIndexable() const
    {
        if(tag == ArrayVal)
            return unboxArray();
        if(tag == MapVal)
            return unboxMap();
        // This should not be called
        return NULL;
    }
    static Value *NullValue;
};

// So that we can add (some types of) values to QMap
inline bool operator<(const Value &v1, const Value &v2);
inline bool operator==(const Value &v1, const Value &v2);

class BuiltInTypes
{
public:
    static ValueClass *ObjectType; // Must be declared before all of the other types, since their
                                   // Initialization depends on it

    static ValueClass *NumericType;
    static ValueClass *IntType;
    static ValueClass *LongType;
    static ValueClass *DoubleType;
    static ValueClass *BoolType;
    static IClass *MethodType;
    static ValueClass *ExternalMethodType;
    static ValueClass *ExternalLibrary;
    static MetaClass  *ClassType;
    static ValueClass *IndexableType;
    static ValueClass *ArrayType;
    static ValueClass *MapType;
    static ValueClass *StringType;
    static IClass *SpriteType;
    static ValueClass *FileType;
    static ValueClass *RawFileType;
    static ValueClass *WindowType;
    static ValueClass *RefType;
    static ValueClass *FieldRefType;
    static ValueClass *ArrayRefType;
    static ValueClass *NullType;
    static ValueClass *ChannelType;
    static ValueClass *QObjectType;
    static ValueClass *LambdaType;
    static IClass *ActivationFrameType;

    // For FFI
    static ValueClass *c_int;
    static ValueClass *c_long;
    static ValueClass *c_float;
    static ValueClass *c_double;
    static ValueClass *c_char;
    static ValueClass *c_asciiz; // Your standard C ascii null-terminated string
    static ValueClass *c_wstr;  // C wide string, pointer to wchar_t
    static ValueClass *c_void;
    static ValueClass *c_ptr;

    static void init();
};

#endif // VALUE_H
