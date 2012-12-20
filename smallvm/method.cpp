/**************************************************************************
**   The Kalimat programming language
**   Copyright (C) 2010 Mohamed Samy Ali - samy2004@gmail.com
**   This project is released under the Apache License version 2.0
**   as described in the included license.txt file
**************************************************************************/

#include "method.h"
#include "vmerror.h"
#include "runtime_identifiers.h"

Method::Method(QString name,int arity)
{
    this->name = name;
    this->arity = arity;
    this->numReturnValues = 0;
    this->returnsReference = false;
    receiver = NULL; // 'tis a global method, not a member of a class
}

Method::Method(QString name,int arity, int numReturnValues)
{
    this->name = name;
    this->arity = arity;
    this->numReturnValues = numReturnValues;
    this->returnsReference = false;
    receiver = NULL; // 'tis a global method, not a member of a class
}

Method::Method(QString name, int arity, int numReturnValues, bool returnsReference, ValueClass *receiver)
{
    this->name = name;
    this->arity = arity;
    this->numReturnValues = numReturnValues;
    this->returnsReference = returnsReference;
    this->receiver = receiver;
}

void Method::prepareInstruction(int index)
{
    Instruction &i = instructions[index];

    if(index > 0)
    {
        instructions[index-1].next = &i;
    }

    i.assignRunner();

    if(i.opcode == PushLocal || i.opcode == PopLocal)
    {
        int index = localsInterner.labelOf(i.SymRef);
        i.SymRefLabel = index;
        Locals[i.SymRef] = index;
    }

}

void Method::Add(Instruction i)
{
    instructions.append(i);
    prepareInstruction(instructions.count()-1);
}

void Method::Add(Instruction i, QString label)
{
    instructions.append(i);
    prepareInstruction(instructions.count()-1);

    if(label != "")
    {
        labels[label] = instructions.count() -1;
        fastLabels[labelInterner.labelOf(label)] = instructions.count() -1;
    }
}

void Method::Add(Instruction i, QString label, int extraInfo)
{

    instructions.append(i.wExtra(extraInfo));
    prepareInstruction(instructions.count()-1);

    if(label != "")
    {
        labels[label] = instructions.count() -1;
        fastLabels[labelInterner.labelOf(label)] = instructions.count() -1;
    }
}

void Method::Set(int ip, Instruction i)
{
    instructions[ip] = i;
    prepareInstruction(ip);
}

QString Method::getName()
{
    return this->name;
}

int Method::localVarCount()
{
    return Locals.count();
}

bool Method::hasSlot(QString name)
{
    return false;
}

QList<QString> Method::getSlotNames()
{
    return QList<QString>();
}

Value *Method::getSlotValue(QString name)
{
    return NULL;
}

void Method::setSlotValue(QString name, Value *val)
{

}

QString Method::toString()
{
    return getName();
}

IMethod *MethodClass::Apply = new ApplyM();

IMethod *MethodClass::lookupMethod(QString name)
{
    if(name == VMId::get(RId::Invoke))
        return MethodClass::Apply;
    return NULL;
}

IObject *MethodClass::newValue(Allocator *allocator)
{
    throw VMError(InternalError1).arg("Cannot create a method directly");
}
