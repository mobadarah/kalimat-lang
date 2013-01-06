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

#include "stack.h"
#include <QStack>
#include <QMap>

struct FrameClass : public EasyForeignClass
{
    Allocator *allocator;

    FrameClass(QString className, VM *vm);
    virtual Value *dispatch(Process *proc, int id, QVector<Value *>args);
    virtual IObject *newValue(Allocator *allocator);
};

const int fast_local_static_size = 20;
struct Frame : public IObject
{
    Method *currentMethod;
    int ip;
    bool returnReferenceIfRefMethod;

    Value **fastLocals;
    Value *fastLocalsStatic[fast_local_static_size];
    int fastLocalCount;

    int operandStackLevel;
    Frame *next;

    Frame();
    Frame(Method *method, int operandStackLevel);
    Frame(Method *method, int ip, int operandStackLevel);
    Frame(const Frame &other);

    void Init(Method *method, int operandStackLevel);
    void Init(Method *method, int ip, int operandStackLevel);

    virtual ~Frame();
    const Instruction &getPreviousRunningInstruction();
    void prepareFastLocals();

    inline Value *local(const QString &name)
    {
        return fastLocals[currentMethod->Locals[name]];
    }

    // IObject
    virtual bool hasSlot(QString name);
    virtual QList<QString> getSlotNames();
    virtual Value *getSlotValue(QString name);
    virtual void setSlotValue(QString name, Value *val);
    virtual QString toString();
};
#endif // FRAME_H
