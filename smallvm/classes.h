#ifndef CLASSES_H
#define CLASSES_H

#include <QString>
#include <QMap>
#include <QVector>
#include <QSet>
#include <QStack>

class Value;
class Allocator;

struct IObject
{
    virtual bool hasSlot(QString name)=0;
    virtual QList<QString> getSlotNames()=0;
    virtual Value *getSlotValue(QString name)=0;
    virtual void setSlotValue(QString name, Value *val)=0;
    virtual QString toString() =0;
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
    virtual Value *invoke(QVector<Value *> args)=0;
};

struct IClass : public IObject
{
    virtual QString getName() = 0;
    virtual bool hasField(QString name) = 0;
    virtual IClass *baseClass() = 0;
    virtual bool subclassOf(IClass *c)=0; // Interpreted as "subclass of or equals"
    virtual IMethod *lookupMethod(QString name)=0;
    virtual IObject *newValue(Allocator *allocator)=0;
    virtual bool getFieldAttribute(QString attr, Value *&ret, Allocator *allocator)=0;
};

struct ValueClass : public IClass
{
    ValueClass(QString name, ValueClass *baseClass);
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
    QString toString();
private:
    void InitObjectLayout(Object *object, Allocator *allocator);
public:
    static void InitObjectLayoutHelper(ValueClass *_class, Object *object, Allocator *allocator);
public:
    QString name;
    QSet<QString> fields;
    QVector<QString> fieldNames; // In order of definition
    QVector<ValueClass*> BaseClasses;
    QMap<QString, Value*> methods;
    QMap<QString, Value *> fieldAttributes;
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

struct FFILibraryClass : public IClass
{
    FFILibraryClass(QString name);
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
    QString toString();

public:
    QString name;
private:
    static IMethod *registerFFIMethod;
    static IMethod *ffiMethodProxy;
};

#endif // CLASSES_H
