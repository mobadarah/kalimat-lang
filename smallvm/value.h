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

/*
enum Tag
{
    Int, Long, Double, Boolean, ObjectVal, NullVal, StringVal, RawVal,
    RefVal, ArrayVal, MultiDimensionalArrayVal, MapVal, ChannelVal, QObjectVal
};
*/

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

struct VBox
{
    Value *v;
    VBox(Value *v) :v(v) { }
};

struct VMap : public VIndexable
{
    QVector<Value *> allKeys; //We need this for the GC, since the map itself stores the values as keys, not their pointers
    QMap<VBox, Value *> Elements;
    bool keyCheck(Value *key, VMError &err);
    void set(Value *key, Value *v);
    Value *get(Value *key);
    int count() { return Elements.count(); }
};

union ValueItem
{
    int intValue;
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

class BuiltInTypes
{
public:
    static ValueClass *ObjectType; // Must be declared before all of the other types, since their
                                   // Initialization depends on it

    static IClass *NumericType;
    static IClass *IntType;
    static IClass *LongType;
    static IClass *DoubleType;
    static ValueClass *BoolType;
    static ValueClass *RawType;
    static IClass *IMethodType;
    static IClass *MethodType;
    static ValueClass *ExternalMethodType;
    static ValueClass *ExternalLibrary;
    static MetaClass  *ClassType;
    static ValueClass *IndexableType;
    static ArrayClass *ArrayType;
    static ValueClass *MD_ArrayType;
    static MapClass *MapType;
    static StringClass *StringType;
    static IClass *SpriteType;
    static ValueClass *FileType;
    static ValueClass *RawFileType;
    static ValueClass *WindowType;
    static ValueClass *RefType;
    static ValueClass *FieldRefType;
    static ValueClass *ArrayRefType;
    static ValueClass *NullType;
    static ChannelClass *ChannelType;
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

struct Value
{
    char mark;
    IClass *type;
    Value *heapNext;
    Value();
    virtual ~Value() { }
    static Value *NullValue;
    virtual QString toString() const =0;
    virtual bool isObject() { return false; }

    virtual bool equals(Value *v2) =0;

    virtual bool intEqualsMe(int);
    virtual bool doubleEqualsMe(double);
    virtual bool longEqualsMe(long);
};

struct IntVal : public Value
{
    int v;
    IntVal(int v): v(v) { }
    IntVal()  { }
    QString toString() const;

    bool equals(Value *v2);

    bool intEqualsMe(int);
    bool doubleEqualsMe(double);
    bool longEqualsMe(long);
};

struct DoubleVal : public Value
{
    double v;
    DoubleVal(double v): v(v) { }
    QString toString() const;
    bool equals(Value *v2);

    bool intEqualsMe(int);
    bool doubleEqualsMe(double);
    bool longEqualsMe(long);
};

struct LongVal : public Value
{
    long v;
    LongVal(long v): v(v) { }
    QString toString() const;
    bool equals(Value *v2);

    bool intEqualsMe(int);
    bool doubleEqualsMe(double);
    bool longEqualsMe(long);
};

struct BoolVal : public Value
{
    bool v;
    int ifOffset;
    BoolVal(bool v): v(v)
    {
        if(v)
            ifOffset = 1;
        else
            ifOffset = 0;
    }
    QString toString() const;

    bool equals(Value *v2);

    bool intEqualsMe(int v1, Value *v2);
    bool doubleEqualsMe(double d1, Value *v);
    bool longEqualsMe(long l1, Value *v);
};

struct StringVal : public Value
{
    QString v;
    StringVal(QString v): v(v) { }
    QString toString() const;
    bool equals(Value *v2);

    bool intEqualsMe(int v1, Value *v2);
    bool doubleEqualsMe(double d1, Value *v);
    bool longEqualsMe(long l1, Value *v);
};

struct ObjVal : public Value
{
    IObject *v;
    ObjVal(IObject* v): v(v) { }
    ~ObjVal();
    QString toString() const;
    bool isObject() { return true;}
    bool equals(Value *v2);

    bool intEqualsMe(int v1, Value *v2);
    bool doubleEqualsMe(double d1, Value *v);
    bool longEqualsMe(long l1, Value *v);
};

struct NullVal : public Value
{
    NullVal() { }
    QString toString() const;
    bool equals(Value *v2);

    bool intEqualsMe(int v1, Value *v2);
    bool doubleEqualsMe(double d1, Value *v);
    bool longEqualsMe(long l1, Value *v);
};

struct RawVal : public Value
{
    void *v;
    RawVal(void *v): v(v) { }
    QString toString() const;
    bool equals(Value *v);

    bool intEqualsMe(int v1, Value *v2);
    bool doubleEqualsMe(double d1, Value *v);
    bool longEqualsMe(long l1, Value *v);
};

struct RefVal : public Value
{
    Reference *v;
    RefVal(Reference *v): v(v) { }
    ~RefVal();
    QString toString() const;
    bool equals(Value *v);

    bool intEqualsMe(int v1, Value *v2);
    bool doubleEqualsMe(double d1, Value *v);
    bool longEqualsMe(long l1, Value *v);
};

struct ArrayVal : public Value
{
    VArray *v;
    ArrayVal(VArray *v): v(v) { }
    ~ArrayVal();
    QString toString() const;
    bool equals(Value *v);

    bool intEqualsMe(int v1, Value *v2);
    bool doubleEqualsMe(double d1, Value *v);
    bool longEqualsMe(long l1, Value *v);
};

struct MultiDimensionalArrayVal : public Value
{
    MultiDimensionalArray<Value *> *v;
    MultiDimensionalArrayVal(MultiDimensionalArray<Value *> *v) : v(v) { }
    ~MultiDimensionalArrayVal();
    QString toString() const;
    bool equals(Value *v);

    bool intEqualsMe(int v1, Value *v2);
    bool doubleEqualsMe(double d1, Value *v);
    bool longEqualsMe(long l1, Value *v);
};

struct MapVal : public Value
{
    VMap *v;
    MapVal(VMap *v): v(v) { }
    ~MapVal();
    QString toString() const;
    bool equals(Value *v);

    bool intEqualsMe(int v1, Value *v2);
    bool doubleEqualsMe(double d1, Value *v);
    bool longEqualsMe(long l1, Value *v);
};

struct ChannelVal : public Value
{
    Channel *v;
    ChannelVal(Channel *v): v(v) { }
    ~ChannelVal();
    QString toString() const;
    bool equals(Value *v);

    bool intEqualsMe(int v1, Value *v2);
    bool doubleEqualsMe(double d1, Value *v);
    bool longEqualsMe(long l1, Value *v);
};

struct QObjVal : public Value
{
    QObject *v;
    QObjVal(QObject *v): v(v) { }
    ~QObjVal();
    QString toString() const;
    bool equals(Value *v);

    bool intEqualsMe(int v1, Value *v2);
    bool doubleEqualsMe(double d1, Value *v);
    bool longEqualsMe(long l1, Value *v);
};

inline int unboxInt(const Value *v)
{
    return ((const IntVal *)v)->v;
}

inline double unboxDouble(const Value *v)
{
    return ((const DoubleVal *) v)->v;
}

inline long unboxLong(const Value *v)
{
    return ((const LongVal *) v)->v;
}

inline bool unboxBool(const Value *v)
{
    return ((const BoolVal *) v)->v;
}

inline IObject *unboxObj(const Value *v)
{
    return ((const ObjVal *) v)->v;
}

inline IClass *unboxClass(const Value *v)
{
    return dynamic_cast<IClass *>(unboxObj(v));
}

inline VArray *unboxArray(const Value *v)
{
    return ((const ArrayVal *) v)->v;
}

inline MultiDimensionalArray<Value *> *unboxMultiDimensionalArray(const Value *v)
{
    return ((const MultiDimensionalArrayVal *) v)->v;
}

inline VMap *unboxMap(const Value *v)
{
    return ((const MapVal*) v)->v;
}
inline void *unboxRaw(const Value *v)
{
    return ((const RawVal*) v)->v;
}

inline QString unboxStr(const Value *v)
{
    return ((const StringVal*) v)->v;
}

inline Reference *unboxRef(const Value *v)
{
    return ((const RefVal*) v)->v;
}

inline Channel *unboxChan(const Value *v)
{
    return ((const ChannelVal*) v)->v;
}

inline QObject *unboxQObj(const Value *v)
{
    return ((const QObjVal*) v)->v;
}

inline double unboxNumeric(const Value *v)
{
    if(v->type == BuiltInTypes::IntType)
        return unboxInt(v);
    if(v->type == BuiltInTypes::DoubleType)
        return unboxDouble(v);
    if(v->type == BuiltInTypes::LongType)
        return unboxLong(v);
    // This should not be called
    return 0.0;
}

inline VIndexable *unboxIndexable(const Value *v)
{
    if(v->type == BuiltInTypes::ArrayType)
        return unboxArray(v);
    if(v->type == BuiltInTypes::MapType)
        return unboxMap(v);
    // This should not be called
    return NULL;
}

// So that we can add (some types of) values to QMap
inline bool operator<(const VBox &v1, const VBox &v2);
inline bool operator==(const VBox &v1, const VBox &v2);

#endif // VALUE_H
