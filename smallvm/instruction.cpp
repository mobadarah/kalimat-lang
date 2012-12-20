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
    this->next = NULL;
    this->runner = NULL;
}

Instruction::Instruction(Opcode opcode)
{
    this->opcode = opcode;
    this->extra = -1;
    this->Arg = NULL;
    this->callStyle = NormalCall;
    this->SymRefLabel = -1;
    this->next = NULL;
    this->runner = NULL;
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

QString InstructionToString(const Instruction &i)
{
    switch(i.opcode)
    {
    case PushV: return "PushV";
    case PushLocal: return "PushLocal";
    case PopLocal: return "PopLocal";
    case PushGlobal: return "PushGlobal";
    case PopGlobal: return "PopGlobal";
    case PushNull: return "PushNull";
    case GetRef: return "GetRef";
    case SetRef: return "SetRef";
    case Add: return "Add";
    case Sub: return "Sub";
    case Mul: return "Mul";
    case Div: return "Div";
    case And: return "And";
    case Or: return "Or";
    case Not: return "Not";
    case Jmp: return "Jmp";
    case JmpVal: return "JmpVal";
    case If: return "If";
    case Lt: return "Lt";
    case Gt: return "Gt";
    case Eq: return "Eq";
    case Ne: return "Ne";
    case Le: return "Le";
    case Ge: return "Ge";
    case Tail: return "Tail";
    case Call: return "Call";
    case CallMethod: return "CallMethod";
    case CallRef: return "CallRef";
    case Ret: return "Ret";
    case CallExternal: return QString("CallExternal: %1").arg(i.SymRef);
    case Nop: return "Nop";
    case SetField: return "SetField";
    case GetField: return "GetField";
    case GetFieldRef: return "GetFieldRef";
    case GetArr: return "GetArr";
    case SetArr: return "SetArr";
    case GetArrRef: return "GetArrRef";
    case New: return "New";
    case NewArr: return "NewArr";
    case ArrLength: return "ArrLength";
    case New_MD_Arr: return "New_MD_Arr";
    case Get_MD_Arr: return "Get_MD_Arr";
    case Set_MD_Arr: return "Set_MD_Arr";
    case Get_MD_ArrRef: return "Get_MD_ArrRef";
    case MD_ArrDimensions: return "MD_ArrDimensions";
    case PushConstant: return "PushConstant";
    case Neg: return "Neg";
    case RegisterEvent: return "RegisterEvent";
    case Isa: return "Isa";
    case Send: return "Send";
    case Receive: return "Receive";
    case Select: return "Select";
    case Break: return "Break";
    case Tick: return "Tick";
    default: return QString("[Unknown, opcode = %1]").arg(i.opcode);
    }
}

Instruction &Instruction::wLabels(QString l1, QString l2, int fastL1, int fastL2)
{
    this->True = l1;
    this->False = l2;
    this->fastTrue = fastL1;
    this->fastFalse = fastL2;
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

void Instruction::assignRunner()
{
    switch(opcode)
    {
    case PushV:
        runner = run_PushV;
        break;
    case PushLocal:
        runner = run_PushLocal;
        break;
    case PopLocal:
        runner = run_PopLocal;
        break;
    case PushGlobal:
        runner = run_PushGlobal;
        break;
    case PopGlobal:
        runner = run_PopGlobal;
        break;
    case PushNull:
        runner = run_PushNull;
        break;
    case GetRef:
        runner = run_GetRef;
        break;
    case SetRef:
        runner = run_SetRef;
        break;
    case Add:
        runner = run_Add;
        break;
    case Sub:
        runner = run_Sub;
        break;
    case Mul:
        runner = run_Mul;
        break;
    case Div:
        runner = run_Div;
        break;
    case And:
        runner = run_And;
        break;
    case Or:
        runner = run_Or;
        break;
    case Not:
        runner = run_Not;
        break;
    case Jmp:
        runner = run_Jmp;
        break;
    case JmpVal:
        runner = run_JmpVal;
        break;
    case If:
        runner = run_If;
        break;
    case Lt:
        runner = run_Lt;
        break;
    case Gt:
        runner = run_Gt;
        break;
    case Eq:
        runner = run_Eq;
        break;
    case Ne:
        runner = run_Ne;
        break;
    case Le:
        runner = run_Le;
        break;
    case Ge:
        runner = run_Ge;
        break;
    case Tail:
        runner = run_Tail;
        break;
    case Call:
        runner = run_Call;
        break;
    case CallMethod:
        runner = run_CallMethod;
        break;
    case CallRef:
        runner = run_CallRef;
        break;
    case Ret:
        runner = run_Ret;
        break;
    case Apply:
        runner = run_Apply;
        break;
    case CallExternal:
        runner = run_CallExternal;
        break;
    case Nop:
        runner = run_Nop;
        break;
    case SetField:
        runner = run_SetField;
        break;
    case GetField:
        runner = run_GetField;
        break;
    case GetFieldRef:
        runner = run_GetFieldRef;
        break;
    case GetArr:
        runner = run_GetArr;
        break;
    case SetArr:
        runner = run_SetArr;
        break;
    case GetArrRef:
        runner = run_GetArrRef;
        break;
    case New:
        runner = run_New;
        break;
    case NewArr:
        runner = run_NewArr;
        break;
    case ArrLength:
        runner = run_ArrLength;
        break;
    case New_MD_Arr:
        runner = run_New_MD_Arr;
        break;
    case Get_MD_Arr:
        runner = run_Get_MD_Arr;
        break;
    case Set_MD_Arr:
        runner = run_Set_MD_Arr;
        break;
    case Get_MD_ArrRef:
        runner = run_Get_MD_ArrRef;
        break;
    case MD_ArrDimensions:
        runner = run_MD_ArrDimensions;
        break;
    case PushConstant:
        runner = run_PushConstant;
        break;
    case Neg:
        runner = run_Neg;
        break;
    case RegisterEvent:
        runner = run_RegisterEvent;
        break;
    case Isa:
        runner = run_Isa;
        break;
    case Send:
        runner = run_Send;
        break;
    case Receive:
        runner = run_Receive;
        break;
    case Select:
        runner = run_Select;
        break;
    case Break:
        runner = run_Break;
        break;
    case Tick:
        runner = run_Tick;
        break;
    default:
        throw VMError(InternalError1).arg(QString("Cannot assign runner to instrusction %1").arg(InstructionToString(*this)));
    }
}
