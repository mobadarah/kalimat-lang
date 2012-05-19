/**************************************************************************
**   The Kalimat programming language
**   Copyright (C) 2010 Mohamed Samy Ali - samy2004@gmail.com
**   This project is released under the Apache License version 2.0
**   as described in the included license.txt file
**************************************************************************/

#ifndef FRAME_H
#define FRAME_H

#ifndef INSTRUCTION_H
    #include "instruction.h"
#endif

#ifndef METHOD_H
    #include "method.h"
#endif

#ifndef EASYFOREIGNCLASS_H
    #include "easyforeignclass.h"
#endif

#include <QStack>
#include <QMap>

struct FrameClass : public EasyForeignClass
{
    Allocator *allocator;

    FrameClass(QString className);
    virtual Value *dispatch(int id, QVector<Value *>args);
    virtual IObject *newValue(Allocator *allocator);
};

struct Frame : public IObject
{
    Frame *caller;
    Method *currentMethod;
    int ip;
    bool returnReferenceIfRefMethod;
    Frame();
    Frame(Method *method, Frame *caller);
    Frame(Method *method, int ip, Frame *caller);
    Instruction getPreviousRunningInstruction();

    QStack<Value *> OperandStack;
    QMap<QString, Value *> Locals;

    // IObject
    virtual bool hasSlot(QString name);
    virtual QList<QString> getSlotNames();
    virtual Value *getSlotValue(QString name);
    virtual void setSlotValue(QString name, Value *val);
    virtual QString toString();
};
#endif // FRAME_H
