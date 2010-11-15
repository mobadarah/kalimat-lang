/**************************************************************************
**   The Kalimat programming language
**   Copyright (C) 2010 Mohamed Samy Ali - samy2004@gmail.com
**   This project is released under the Apache License version 2.0
**   as described in the included license.txt file
**************************************************************************/

#ifndef INSTRUCTION_H
#define INSTRUCTION_H

class Allocator;
enum Opcode
{
    PushV, PushLocal, PopLocal, PushGlobal, PopGlobal, PushNull, GetRef, SetRef,
    Add, Sub, Mul, Div, And, Or, Not, Jmp, If, Lt, Gt, Eq, Ne, Le, Ge,
    Call, CallMethod, CallRef, Ret, CallExternal, Nop,
    SetField, GetField, GetFieldRef, GetArr, SetArr, GetArrRef, New, NewArr, ArrLength,
    New_MD_Arr, Get_MD_Arr, Set_MD_Arr, Get_MD_ArrRef, MD_ArrDimensions,
    PushConstant, Neg, // todo!
    RegisterEvent, Isa
};

struct Instruction
{
    Opcode opcode;
    Value *Arg;
    QString SymRef;
    QString True, False;

    int extra;

    Instruction();
    Instruction(Opcode opcode);
    Instruction &wArg(Value *);
    Instruction &wArgParse(QString, Allocator *allocator);
    Instruction &wLabels(QString, QString);
    Instruction &wRef(QString);
    Instruction &wExtra(int info);
};



#endif // INSTRUCTION_H
