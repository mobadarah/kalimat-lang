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

#include <QStack>
#include <QMap>
struct Frame
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
};
#endif // FRAME_H
