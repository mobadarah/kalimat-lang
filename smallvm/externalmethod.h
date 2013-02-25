/**************************************************************************
**   The Kalimat programming language
**   Copyright (C) 2010 Mohamed Samy Ali - samy2004@gmail.com
**   This project is released under the Apache License version 2.0
**   as described in the included license.txt file
**************************************************************************/

#ifndef EXTERNALMETHOD_H
#define EXTERNALMETHOD_H

#include "operandstack.h"

class ExternalMethod : public IMethod
{
    int arity;
public:
    bool mustRunInGui;
public:
    explicit ExternalMethod();
    explicit ExternalMethod(int arity);
    int Arity() { return arity;}
    virtual void operator ()(VOperandStack &operandStack, Process *) =0;

    virtual bool hasSlot(QString name) { return false; }
    virtual QList<QString> getSlotNames() { return QList<QString>(); }
    virtual Value *getSlotValue(QString name) { return NULL; }
    virtual void setSlotValue(QString name, Value *val) { }
    virtual QString toString() { return QString("%1").arg((long)this); }
};

#endif // EXTERNALMETHOD_H
