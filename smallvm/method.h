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
    Method(QString name, int arity);
    void Add(Instruction i);
    void Add(Instruction i, QString label);
    void Add(Instruction i, QString label, int extraInfo);
    int GetIp(QString label);
    int Arity();
    int InstructionCount();
    Instruction &Get(QString label);
    Instruction &Get(int ip);
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

#endif // METHOD_H
