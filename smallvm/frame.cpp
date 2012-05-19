/**************************************************************************
**   The Kalimat programming language
**   Copyright (C) 2010 Mohamed Samy Ali - samy2004@gmail.com
**   This project is released under the Apache License version 2.0
**   as described in the included license.txt file
**************************************************************************/

#include "frame.h"
#include "vmerror.h"
#include "allocator.h"

FrameClass::FrameClass(QString className)
    : EasyForeignClass(className)
{
    _method(L"قيمة.المتغير.المحلي",
            0, 1);
    _method(L"حدد.قيمة.المتغير.المحلي",
            1, 2);
}

IObject *FrameClass::newValue(Allocator *allocator)
{
    this->allocator = allocator;
    Frame *f= new Frame();
    return f;
}

Value *FrameClass::dispatch(int id, QVector<Value *>args)
{
    IObject *receiver = args[0]->unboxObj();
    Frame *frm = dynamic_cast<Frame *>(receiver);
    QString str;
    switch(id)
    {
        case 0: // قيمة.المتغير.المحلي
        str = args[1]->unboxStr();
        if(frm->Locals.contains(str))
            return frm->Locals[str];
        else
            return allocator->null();
        case 1: // حدد.قيمة.المتغير.المحلي
        str = args[1]->unboxStr();
        frm->Locals[str] = args[2];
        return NULL;
    default:
        throw VMError(InternalError1).arg(QString::fromStdWString(L"لا توجد هذه الاستجابة"));
    }
}

Frame::Frame()
{

}

Frame::Frame(Method *method, Frame *caller)
{
    currentMethod = method;
    this->caller = caller;
    ip = 0;
    returnReferenceIfRefMethod = true;
}
Frame::Frame(Method *method, int ip, Frame *caller)
{
    currentMethod = method;
    this->caller = caller;
    this->ip = ip;
    returnReferenceIfRefMethod = true;
}

Instruction Frame::getPreviousRunningInstruction()
{
    int n = currentMethod->InstructionCount();
    if(ip>=1 && ip-1 < n)
        return currentMethod->Get(this->ip-1);
    else
        return currentMethod->Get(n-1);
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
    return QString::fromStdWString(L"<إطار.تفعيل>");
}
