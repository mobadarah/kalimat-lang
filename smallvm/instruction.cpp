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
#include "process.h"
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
        this->Arg = allocator->newBool(true);
    }
    else if(argS.trimmed() == "false")
    {
        this->Arg = allocator->newBool(false);
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

Instruction &Instruction::wLabels(QString l1)
{
    this->SymRef = l1;
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
        runner = &Process::DoPushVal;
        break;
    case PushLocal:
        runner = &Process::DoPushLocal;
        break;
    case PopLocal:
        runner = &Process::DoPopLocal;
        break;
    case PushGlobal:
        runner = &Process::DoPushGlobal;
        break;
    case PopGlobal:
        runner = &Process::DoPopGlobal;
        break;
    case PushNull:
        runner = &Process::DoPushNull;
        break;
    case GetRef:
        runner = &Process::DoGetRef;
        break;
    case SetRef:
        runner = &Process::DoSetRef;
        break;
    case Add:
        runner = &Process::DoAdd;
        break;
    case Sub:
        runner = &Process::DoSub;
        break;
    case Mul:
        runner = &Process::DoMul;
        break;
    case Div:
        runner = &Process::DoDiv;
        break;
    case And:
        runner = &Process::DoAnd;
        break;
    case Or:
        runner = &Process::DoOr;
        break;
    case Not:
        runner = &Process::DoNot;
        break;
    case Jmp:
        runner = &Process::DoJmp;
        break;
    case JmpVal:
        runner = &Process::DoJmpVal;
        break;
    case If:
        runner = &Process::DoIf;
        break;
    case Lt:
        runner = &Process::DoLt;
        break;
    case Gt:
        runner = &Process::DoGt;
        break;
    case Eq:
        runner = &Process::DoEq;
        break;
    case Ne:
        runner = &Process::DoNe;
        break;
    case Le:
        runner = &Process::DoLe;
        break;
    case Ge:
        runner = &Process::DoGe;
        break;
    case Tail:
        runner = &Process::DoNop;
        break;
    case Call:
        runner = &Process::DoCall;
        break;
    case CallMethod:
        runner = &Process::DoCallMethod;
        break;
    case CallRef:
        runner = &Process::DoCallRef;
        break;
    case Ret:
        runner = &Process::DoRet;
        break;
    case Apply:
        runner = &Process::DoApply;
        break;
    case CallExternal:
        runner = &Process::DoCallExternal;
        break;
    case Nop:
        runner = &Process::DoNop;
        break;
    case SetField:
        runner = &Process::DoSetField;
        break;
    case GetField:
        runner = &Process::DoGetField;
        break;
    case GetFieldRef:
        runner = &Process::DoGetFieldRef;
        break;
    case GetArr:
        runner = &Process::DoGetArr;
        break;
    case SetArr:
        runner = &Process::DoSetArr;
        break;
    case GetArrRef:
        runner = &Process::DoGetArrRef;
        break;
    case New:
        runner = &Process::DoNew;
        break;
    case NewArr:
        runner = &Process::DoNewArr;
        break;
    case ArrLength:
        runner = &Process::DoArrLength;
        break;
    case New_MD_Arr:
        runner = &Process::DoNewMD_Arr;
        break;
    case Get_MD_Arr:
        runner = &Process::DoGetMD_Arr;
        break;
    case Set_MD_Arr:
        runner = &Process::DoSetMD_Arr;
        break;
    case Get_MD_ArrRef:
        runner = &Process::DoGetMD_ArrRef;
        break;
    case MD_ArrDimensions:
        runner = &Process::DoMD_ArrDimensions;
        break;
    case PushConstant:
        runner = &Process::DoPushConstant;
        break;
    case Neg:
        runner = &Process::DoNeg;
        break;
    case RegisterEvent:
        runner = &Process::DoRegisterEvent;
        break;
    case Isa:
        runner = &Process::DoIsa;
        break;
    case Send:
        runner = &Process::DoSend;
        break;
    case Receive:
        runner = &Process::DoReceive;
        break;
    case Select:
        runner = &Process::DoSelect;
        break;
    case Break:
        runner = &Process::DoBreak;
        break;
    case Tick:
        runner = &Process::DoTick;
        break;
    case Jeq:
        runner = &Process::DoJmpIfEq;
        break;
    case Jne:
        runner = &Process::DoJmpIfNe;
        break;
    default:
        throw VMError(InternalError1).arg(QString("Cannot assign runner to instrusction %1").arg(InstructionToString(*this)));
    }
}
