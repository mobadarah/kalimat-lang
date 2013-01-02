/**************************************************************************
**   The Kalimat programming language
**   Copyright (C) 2010 Mohamed Samy Ali - samy2004@gmail.com
**   This project is released under the Apache License version 2.0
**   as described in the included license.txt file
**************************************************************************/

#include "frame.h"
#include "vmerror.h"
#include "allocator.h"
#include "runtime_identifiers.h"
#include <memory>

FrameClass::FrameClass(QString className, VM *vm)
    : EasyForeignClass(className, vm)
{
    _method(VMId::get(RId::LocalVar),
            0, 1);
    _method(VMId::get(RId::SetLocalVar),
            1, 2);
}

IObject *FrameClass::newValue(Allocator *allocator)
{
    this->allocator = allocator;
    Frame *f= new Frame();
    return f;
}

Value *FrameClass::dispatch(Process *proc, int id, QVector<Value *> args)
{
    IObject *receiver = args[0]->unboxObj();
    Frame *frm = dynamic_cast<Frame *>(receiver);
    QString str;
    switch(id)
    {
        case 0: // قيمة.المتغير.المحلي
        str = args[1]->unboxStr();
        if(frm->currentMethod->Locals.contains(str))
            return frm->fastLocals[frm->currentMethod->Locals[str]];
        else
            return allocator->null();
        case 1: // حدد.قيمة.المتغير.المحلي
        str = args[1]->unboxStr();
        frm->fastLocals[frm->currentMethod->Locals[str]] = args[2];
        return NULL;
    default:
        throw VMError(NoSuchForeignMethod2).arg(::str(id)).arg(this->getName());
    }
}

Frame::Frame()
{
    next = NULL;
    fastLocals = NULL;
    fastLocalCount = 0;
}

Frame::Frame(Method *method)
    :currentMethod(method),
      ip(0),
      returnReferenceIfRefMethod(true),
      next(NULL)
{
    prepareFastLocals();
}

Frame::Frame(Method *method, int ip)
    :currentMethod(method),
      ip(ip),
      returnReferenceIfRefMethod(true),
      next(NULL)
{
    prepareFastLocals();
}

void Frame::Init(Method *method)
{
    Init(method, 0);
}

void Frame::Init(Method *method, int ip)
{
    OperandStack.clear();
    currentMethod = method;
    this->ip = ip;
    returnReferenceIfRefMethod = true;
    next = NULL;
    if(fastLocals && fastLocals != fastLocalsStatic)
    {
        delete[] fastLocals;
    }
    prepareFastLocals();
}



Frame::~Frame()
{
    if(fastLocals && fastLocals != fastLocalsStatic)
    {
        //qDebug("~Frame calling delete[]");
        delete[] fastLocals;
    }
}

const Instruction &Frame::getPreviousRunningInstruction()
{
    const int n = currentMethod->InstructionCount();
    if(ip>=1 && ip-1 < n)
        return currentMethod->Get(this->ip-1);
    else
        return currentMethod->Get(n-1);
}

void Frame::prepareFastLocals()
{
    int n = currentMethod->localVarCount();
    fastLocalCount = n;
    if(n != 0)
    {
        if(n<=fast_local_static_size)
        {
            fastLocals = fastLocalsStatic;
        }
        else
        {
            fastLocals = new Value *[n];
        }
        for(int i=0; i<n; i++)
        {
            fastLocals[i] = NULL;
        }

        memset(fastLocals, 0, n *sizeof(Value *));
    }
    else
    {
        fastLocals = NULL;
    }
}

bool Frame::hasSlot(QString)
{
    return false;
}

QList<QString> Frame::getSlotNames()
{
    return QList<QString>();
}

Value *Frame::getSlotValue(QString)
{
    return NULL;
}

void Frame::setSlotValue(QString, Value *)
{

}

QString Frame::toString()
{
    return QString("<%1>").arg(VMId::get(RId::ActivationRecord));
}
