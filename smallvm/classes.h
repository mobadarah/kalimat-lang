#ifndef CLASSES_H
#define CLASSES_H

#include <QString>
#include <QMap>
#include <QVector>
#include <QSet>

class Value;

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

};

struct IClass : public IObject
{
    virtual QString getName() = 0;
    virtual bool hasField(QString name) = 0;
    virtual IClass *baseClass() = 0;
    virtual bool subclassOf(IClass *c)=0; // Interpreted as "subclass of or equals"
    virtual IMethod *lookupMethod(QString name)=0;
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

    QString toString();
//private:
public:
    QString name;
    QSet<QString> fields;
    QVector<QString> fieldNames; // In order of definition
    QVector<ValueClass*> BaseClasses;
    QMap<QString, Value*> methods;
};

#endif // CLASSES_H
