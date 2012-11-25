/**************************************************************************
**   The Kalimat programming language
**   Copyright (C) 2010 Mohamed Samy Ali - samy2004@gmail.com
**   This project is released under the Apache License version 2.0
**   as described in the included license.txt file
**************************************************************************/

#ifndef METHOD_H
#define METHOD_H

#include <QObject>
#include <QMap>
#include <QString>
#include <QVector>
#include <QSet>

#ifndef VALUE_H
    #include "value.h"
#endif

#ifndef INSTRUCTION_H
    #include "instruction.h"
#endif

class Method : public IMethod
{
private:
    QVector<Instruction> instructions;
    QMap<QString, int> labels;
    int arity;
    int numReturnValues; // 0, 1 or -1 meaning don't check
    bool returnsReference;
    ValueClass *receiver;
    QString name;
public:
    Method(QString name, int arity, int numReturnValues, bool returnsReference, ValueClass *receiver);
    Method(QString name, int arity, int numReturnValues);
    Method(QString name, int arity);

    void Add(Instruction i);
    void Add(Instruction i, QString label);
    void Add(Instruction i, QString label, int extraInfo);
    int GetIp(QString label);
    int Arity();
    int InstructionCount();
    Instruction &Get(QString label);
    Instruction &Get(int ip);
    void Set(int ip, Instruction i);
    bool HasInstruction(int ip);
    bool IsReturningReference();
    int NumReturnValues();
    QString getName();

    // Implementing IObject
    virtual bool hasSlot(QString name);
    virtual QList<QString> getSlotNames();
    virtual Value *getSlotValue(QString name);
    virtual void setSlotValue(QString name, Value *val);
    virtual QString toString() ;
};

class MethodClass : public IClass
{
    QString name;
    IClass *base;
    static IMethod *Apply;
public:

    MethodClass(QString name, IClass *base) : name(name), base(base) { }
    QString getName() { return name;}
    virtual bool hasField(QString name) { return false;}
    virtual IClass *baseClass() { return base;}
    virtual bool subclassOf(IClass *c) { return c == this || base->subclassOf(c); }
    virtual IMethod *lookupMethod(QString name);

    virtual IObject *newValue(Allocator *allocator);
    virtual bool getFieldAttribute(QString attr, Value *&ret, Allocator *allocator) { return false; }
    // TODO: we need a relection API with attributes, similar to e.g .net
    // instead of these ad-hoc solutions
    virtual QVector<PropertyDesc> getProperties() { return QVector<PropertyDesc>(); }

    virtual bool hasSlot(QString name) { return base->hasSlot(name); }
    virtual QList<QString> getSlotNames() { return base->getSlotNames(); }
    virtual Value *getSlotValue(QString name) { return base->getSlotValue(name); }
    virtual void setSlotValue(QString name, Value *val) { base->setSlotValue(name, val);}
    virtual QString toString() { return QString("<%1>").arg(name);}
};

class ApplyM : public Method
{
public:
    ApplyM(): Method("apply", 2, -1)
    {
        Add(Instruction(Apply));
        Add(Instruction(Ret));
    }
};

#endif // METHOD_H
