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

#include "value.h"
#include "instruction.h"


class Method : public Object
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
};

#endif // METHOD_H
