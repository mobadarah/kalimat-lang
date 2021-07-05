#ifndef CLASSES_H
#define CLASSES_H

#include <QString>
#include <QMap>
#include <QVector>
#include <QSet>
#include <QStack>

class Value;
class VArray;
class Allocator;
class Process;
class VM;

struct IObject
{
    virtual bool hasSlot(QString name)=0;
    virtual QList<QString> getSlotNames()=0;
    virtual Value *getSlotValue(QString name)=0;
    virtual void setSlotValue(QString name, Value *val)=0;
    virtual QString toString() =0;
    virtual ~IObject() {}
};

struct Object : public IObject
{
    virtual bool hasSlot(QString name);
    virtual QList<QString> getSlotNames();
    virtual Value *getSlotValue(QString name);
    virtual void setSlotValue(QString name, Value *val);
    virtual QString toString();
public:
    QMap<QString, Value *> _slots;
    // For when we need to traverse slots in order of definition
    // todo: this should be in the class, to save memory
    QVector<QString> slotNames;
    virtual ~Object() {}
};

struct IMethod : public IObject
{
    virtual int Arity()=0;
};

struct IForeignMethod : public IMethod
{
    virtual bool hasSlot(QString name) { return false;}
    virtual QList<QString> getSlotNames() { return QList<QString>();}
    virtual Value *getSlotValue(QString name) { return NULL; }
    virtual void setSlotValue(QString name, Value *val) {}
    virtual QString toString()=0;
    // Args are last to first
    virtual Value *invoke(Process *proc, QVector<Value *> args)=0;
};

struct PropertyDesc
{
    QString name;
    bool readOnly;
};

typedef bool (*EqualityFuncSameType)(Value *v1, Value *v2);
bool compareRef(Value *v1, Value *v2);

struct IClass : public IObject
{
    virtual QString getName() = 0;
    virtual bool hasField(QString name) = 0;
    virtual IClass *baseClass() = 0;
    virtual bool subclassOf(IClass *c)=0; // Interpreted as "subclass of or equals"
    virtual IMethod *lookupMethod(QString name)=0;
    virtual IObject *newValue(Allocator *allocator)=0;
    virtual bool getFieldAttribute(QString attr, Value *&ret, Allocator *allocator)=0;

    virtual QString toString();
    // TODO: we need a relection API with attributes, similar to e.g .net
    // instead of these ad-hoc solutions
    virtual QVector<PropertyDesc> getProperties()=0;
    EqualityFuncSameType equality;

    virtual int compareTo(Value *v1, Value *v2);
    virtual int compareIntToMe(int v1, Value *v2);
    virtual int compareDoubleToMe(double v1,  Value *v2);
    virtual int compareLongToMe(long v1,  Value *v2);
    virtual int compareStringToMe(QString v1,  Value *v2);

    virtual bool isNumeric() { return false;}

    virtual Value *addTo(Value *v1, Value *v2, Allocator *);
    virtual Value* addIntToMe(int v1, Value *v2, Allocator *);
    virtual Value *addDoubleToMe(double v1,  Value *v2, Allocator *);
    virtual Value *addLongToMe(long v1,  Value *v2, Allocator *);
    virtual Value *addStringToMe(QString v1,  Value *v2, Allocator *);
    virtual Value *addArrayToMe(VArray *v1, Value *v2, Allocator *);

    virtual Value *minus(Value *v1, Value *v2, Allocator *);
    virtual Value* intMinusMe(int v1, Value *v2, Allocator *);
    virtual Value *doubleMinusMe(double v1,  Value *v2, Allocator *);
    virtual Value *longMinusMe(long v1,  Value *v2, Allocator *);

};

struct ArrayClass : public IClass
{
    QMap<QString, IMethod *> externalMethods;
    void attachVmMethod(VM *vm, QString methodName);
    // IObject
    virtual bool hasSlot(QString name) { return false; }
    virtual QList<QString> getSlotNames() { return QList<QString>(); }
    virtual Value *getSlotValue(QString name) { return NULL; }
    virtual void setSlotValue(QString name, Value *val) { }

    //IClass
    virtual QString getName();
    bool hasField(QString name) { return false;}
    IClass *baseClass();
    virtual bool subclassOf(IClass *c);
    IMethod *lookupMethod(QString name);
    IObject *newValue(Allocator *allocator);
    virtual bool getFieldAttribute(QString attr, Value *&ret, Allocator *allocator) { return false;}
    virtual QVector<PropertyDesc> getProperties() { return QVector<PropertyDesc>(); }
    QString toString() { return getName();}

    // Addition
    Value *addTo(Value *v1, Value *v2, Allocator *);
    Value *addArrayToMe(VArray *v1, Value *v2, Allocator *allocator);
};

struct MapClass : public IClass
{
    QMap<QString, IMethod *> externalMethods;
    void attachVmMethod(VM *vm, QString methodName);
    // IObject
    virtual bool hasSlot(QString name) { return false; }
    virtual QList<QString> getSlotNames() { return QList<QString>(); }
    virtual Value *getSlotValue(QString name) { return NULL; }
    virtual void setSlotValue(QString name, Value *val) { }

    //IClass
    virtual QString getName();
    bool hasField(QString name) { return false;}
    IClass *baseClass();
    virtual bool subclassOf(IClass *c);
    IMethod *lookupMethod(QString name);
    IObject *newValue(Allocator *allocator);
    virtual bool getFieldAttribute(QString attr, Value *&ret, Allocator *allocator) { return false;}
    virtual QVector<PropertyDesc> getProperties() { return QVector<PropertyDesc>(); }
    QString toString() { return getName();}
};

struct ComparableClass : public IClass
{
    // virtual bool equals(Value *v) = 0;
};

struct StringClass : public ComparableClass
{
    QMap<QString, IMethod *> externalMethods;
    void attachVmMethod(VM *vm, QString methodName);
    // IObject
    virtual bool hasSlot(QString name) { return false; }
    virtual QList<QString> getSlotNames() { return QList<QString>(); }
    virtual Value *getSlotValue(QString name) { return NULL; }
    virtual void setSlotValue(QString name, Value *val) { }

    //IClass
    virtual QString getName();
    bool hasField(QString name) { return false;}
    IClass *baseClass();
    virtual bool subclassOf(IClass *c);
    IMethod *lookupMethod(QString name);
    IObject *newValue(Allocator *allocator);
    virtual bool getFieldAttribute(QString attr, Value *&ret, Allocator *allocator) { return false;}
    virtual QVector<PropertyDesc> getProperties() { return QVector<PropertyDesc>(); }
    QString toString() { return getName();}

    // Comparison
    int compareTo(Value *v1, Value *v2);
    int compareStringToMe(QString v1,  Value *v2);

    // Addition
    Value *addTo(Value *v1, Value *v2, Allocator *);
    Value *addStringToMe(QString v1,  Value *v2, Allocator *);
};

struct NumericClass : public ComparableClass
{
    // IObject
    virtual bool hasSlot(QString name) { return false; }
    virtual QList<QString> getSlotNames() { return QList<QString>(); }
    virtual Value *getSlotValue(QString name) { return NULL; }
    virtual void setSlotValue(QString name, Value *val) { }

    //IClass
    virtual QString getName();
    bool hasField(QString name) { return false;}
    IClass *baseClass();
    virtual bool subclassOf(IClass *c);
    IMethod *lookupMethod(QString name);
    IObject *newValue(Allocator *allocator);
    virtual bool getFieldAttribute(QString attr, Value *&ret, Allocator *allocator) { return false;}
    virtual QVector<PropertyDesc> getProperties() { return QVector<PropertyDesc>(); }
    //QString toString() { return getName();}

    bool isNumeric() { return true;}
};

struct IntClass : public NumericClass
{
    QString getName();
    IClass *baseClass();

    int compareTo(Value *v1, Value *v2);
    int compareIntToMe(int v1, Value *v2);
    int compareDoubleToMe(double v1,  Value *v2);
    int compareLongToMe(long v1,  Value *v2);

    Value *addTo(Value *v1, Value *v2, Allocator *);
    Value* addIntToMe(int v1, Value *v2, Allocator *);
    Value *addDoubleToMe(double v1,  Value *v2, Allocator *);
    Value *addLongToMe(long v1,  Value *v2, Allocator *);

    Value *minus(Value *v1, Value *v2, Allocator *);
    Value* intMinusMe(int v1, Value *v2, Allocator *);
    Value *doubleMinusMe(double v1,  Value *v2, Allocator *);
    Value *longMinusMe(long v1,  Value *v2, Allocator *);
};

struct DoubleClass : public NumericClass
{
    QString getName();
    IClass *baseClass();

    int compareTo(Value *v1, Value *v2);
    int compareIntToMe(int v1, Value *v2);
    int compareDoubleToMe(double v1,  Value *v2);
    int compareLongToMe(long v1,  Value *v2);

    Value *addTo(Value *v1, Value *v2, Allocator *);
    Value* addIntToMe(int v1, Value *v2, Allocator *);
    Value *addDoubleToMe(double v1,  Value *v2, Allocator *);
    Value *addLongToMe(long v1,  Value *v2, Allocator *);

    Value *minus(Value *v1, Value *v2, Allocator *);
    Value* intMinusMe(int v1, Value *v2, Allocator *);
    Value *doubleMinusMe(double v1,  Value *v2, Allocator *);
    Value *longMinusMe(long v1,  Value *v2, Allocator *);
};

struct LongClass : public NumericClass
{
    QString getName();
    IClass *baseClass();

    int compareTo(Value *v1, Value *v2);
    int compareIntToMe(int v1, Value *v2);
    int compareDoubleToMe(double v1,  Value *v2);
    int compareLongToMe(long v1,  Value *v2);

    Value *addTo(Value *v1, Value *v2, Allocator *);
    Value* addIntToMe(int v1, Value *v2, Allocator *);
    Value *addDoubleToMe(double v1,  Value *v2, Allocator *);
    Value *addLongToMe(long v1,  Value *v2, Allocator *);

    Value *minus(Value *v1, Value *v2, Allocator *);
    Value* intMinusMe(int v1, Value *v2, Allocator *);
    Value *doubleMinusMe(double v1,  Value *v2, Allocator *);
    Value *longMinusMe(long v1,  Value *v2, Allocator *);
};

struct ValueClass : public IClass
{
    ValueClass(QString name, IClass *baseClass);
    virtual ~ValueClass();

    // IObject
    virtual bool hasSlot(QString name);
    virtual QList<QString> getSlotNames();
    virtual Value *getSlotValue(QString name);
    virtual void setSlotValue(QString name, Value *val);

    //IClass
    QString getName();
    bool hasField(QString name);
    IClass *baseClass();
    virtual bool subclassOf(IClass *c);
    IMethod *lookupMethod(QString name);
    IObject *newValue(Allocator *allocator);
    virtual bool getFieldAttribute(QString attr, Value *&ret, Allocator *allocator);
    virtual QVector<PropertyDesc> getProperties();
    QString toString();
private:
    void InitObjectLayout(Object *object, Allocator *allocator);
public:
    static void InitObjectLayoutHelper(ValueClass *_class, Object *object, Allocator *allocator);
public:
    QString name;
    QSet<QString> fields;
    QVector<QString> fieldNames; // In order of definition
    QVector<IClass *> BaseClasses;
    QMap<QString, Value*> methods;
    QMap<QString, Value *> fieldAttributes;
    QVector<PropertyDesc> properties;
};

struct ForeignClass : public IClass
{
    ForeignClass(QString name);
    // IObject
    virtual bool hasSlot(QString name);
    virtual QList<QString> getSlotNames();
    virtual Value *getSlotValue(QString name);
    virtual void setSlotValue(QString name, Value *val);

    //IClass
    QString getName();
    virtual bool hasField(QString name)=0;
    IClass *baseClass();
    virtual bool subclassOf(IClass *c);
    virtual IMethod *lookupMethod(QString name)=0;
    virtual IObject *newValue(Allocator *allocator)=0;
    QString toString();

public:
    QString name;

};

/*
  Represents pointer types for ffi
  The type can be reified by using the 'typefromid' external method and using an
  ID starting with an asterisk

  pushc %str1;  *c_int
  typefromid

  would have a type of pointer to c_int on the stack
*/

struct PointerClass : public IClass
{
    IClass *pointee;
    PointerClass(IClass *pointee);
    // IObject
    virtual bool hasSlot(QString name);
    virtual QList<QString> getSlotNames();
    virtual Value *getSlotValue(QString name);
    virtual void setSlotValue(QString name, Value *val);

    //IClass
    QString getName();
    virtual bool hasField(QString name);
    IClass *baseClass();
    virtual bool subclassOf(IClass *c);
    virtual IMethod *lookupMethod(QString name);
    virtual IObject *newValue(Allocator *allocator);
    virtual bool getFieldAttribute(QString attr, Value *&ret, Allocator *allocator);
    virtual QVector<PropertyDesc> getProperties();
    QString toString();
};

/*
  Represents lambda types for ffi

  The type can be reified by using the 'typefromid' external method and using an
  ID int the form ^(t1,t2,t3)->retType
  The parser for those types of ids is very sensetive: use exact format with no spaces!

  pushc %str1;  ^(c_int,c_int)->c_long
  typefromid

  would have the corresponding function type on the stack
*/

struct FunctionClass : public IClass
{
    IClass *retType;
    QVector<IClass *> argTypes;
    FunctionClass(IClass *retType, QVector<IClass *> argTypes);

    // IObject
    virtual bool hasSlot(QString name);
    virtual QList<QString> getSlotNames();
    virtual Value *getSlotValue(QString name);
    virtual void setSlotValue(QString name, Value *val);

    //IClass
    QString getName();
    virtual bool hasField(QString name);
    IClass *baseClass();
    virtual bool subclassOf(IClass *c);
    virtual IMethod *lookupMethod(QString name);
    virtual IObject *newValue(Allocator *allocator);
    virtual bool getFieldAttribute(QString attr, Value *&ret, Allocator *allocator);
    virtual QVector<PropertyDesc> getProperties();
    QString toString();
};

struct ChannelClass : public IClass
{
    QMap<QString, IMethod *> externalMethods;
    void attachVmMethod(VM *vm, QString methodName);
    // IObject
    virtual bool hasSlot(QString name) { return false; }
    virtual QList<QString> getSlotNames() { return QList<QString>(); }
    virtual Value *getSlotValue(QString name) { return NULL; }
    virtual void setSlotValue(QString name, Value *val) { }

    //IClass
    virtual QString getName();
    bool hasField(QString name) { return false;}
    IClass *baseClass();
    virtual bool subclassOf(IClass *c);
    IMethod *lookupMethod(QString name);
    IObject *newValue(Allocator *allocator);
    virtual bool getFieldAttribute(QString attr, Value *&ret, Allocator *allocator) { return false;}
    virtual QVector<PropertyDesc> getProperties() { return QVector<PropertyDesc>(); }
    QString toString() { return getName();}
};

#endif // CLASSES_H
