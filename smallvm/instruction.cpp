/**************************************************************************
**   The Kalimat programming language
**   Copyright (C) 2010 Mohamed Samy Ali - samy2004@gmail.com
**   This project is released under the Apache License version 2.0
**   as described in the included license.txt file
**************************************************************************/

#include <QObject>
#include <QString>
#include <QSet>
#include <QMap>
#include <QStack>
#include <QVector>
#include "value.h"
#include "instruction.h"
#include "frame.h"
#include "allocator.h"
#include "vmerror.h"

Instruction::Instruction()
{
    this->Arg = NULL;
    this->extra = -1;
    this->callStyle = NormalCall;
    this->SymRefLabel = -1;
}

Instruction::Instruction(Opcode opcode)
{
    this->opcode = opcode;
    this->extra = -1;
    this->Arg = NULL;
    this->callStyle = NormalCall;
    this->SymRefLabel = -1;
}

Instruction &Instruction::wArg(Value *arg)
{
    this->Arg = arg;
    return *this;
}

Instruction &Instruction::wArgParse(QString argS, Allocator *allocator)
{
    bool ok;
    int iVal;
    double dVal;

    if(argS.trimmed() == "true")
    {
        this->Arg = allocator->newBool(true, false);
    }
    else if(argS.trimmed() == "false")
    {
        this->Arg = allocator->newBool(false, false);
    }
    else
    {
    iVal = argS.toInt(&ok, 10);
        if(ok)
        {
            // We don't want the GC to remove the values
            // embedded in an instruction's opcode!
            // TODO: We should manually destroy the 'Arg' member if it isn't GCd
            this->Arg = allocator->newInt(iVal, false);
        }
        else
        {
            dVal = argS.toDouble(&ok);
            if(ok)
            {
                // We don't want the GC to remove the values
                // embedded in an instruction's opcode!
                // TODO: We should manually destroy the 'Arg' member if it isn't GCd
                this->Arg = allocator->newDouble(dVal, false);
            }
            else
            {
                throw VMError(InternalError1).arg("Error parsing value in instruction");
            }
        }
    }

    return *this;
}

Instruction &Instruction::wLabels(QString l1, QString l2)
{
    this->True = l1;
    this->False = l2;
    return *this;
}

Instruction &Instruction::wRef(QString ref, Labeller &lblr)
{
    this->SymRef = ref;
    this->SymRefLabel = lblr.labelOf(ref);
    return *this;
}

Instruction &Instruction::wExtra(int info)
{
    this->extra = info;
    return *this;
}

Instruction &Instruction::wCallStyle(CallStyle style)
{
    this->callStyle = style;
    return *this;
}
