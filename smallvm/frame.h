/**************************************************************************
**   The Kalimat programming language
**   Copyright (C) 2010 Mohamed Samy Ali - samy2004@gmail.com
**   This project is released under the Apache License version 2.0
**   as described in the included license.txt file
**************************************************************************/

#ifndef FRAME_H
#define FRAME_H

#include "instruction.h"
#include <QStack>
#include <QMap>
struct Frame
{
    Method *currentMethod;
    int ip;
    bool returnReferenceIfRefMethod;
    Frame();
    Frame(Method *method);
    Frame(Method *method, int ip);
    Instruction getPreviousRunningInstruction();

    QStack<Value *> OperandStack;
    QMap<QString, Value *> Locals;
};
#endif // FRAME_H
