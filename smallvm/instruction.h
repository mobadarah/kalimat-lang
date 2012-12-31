/**************************************************************************
**   The Kalimat programming language
**   Copyright (C) 2010 Mohamed Samy Ali - samy2004@gmail.com
**   This project is released under the Apache License version 2.0
**   as described in the included license.txt file
**************************************************************************/

#ifndef INSTRUCTION_H
#define INSTRUCTION_H
#ifndef VALUE_H
    #include "value.h"
#endif

#ifndef UTILS_H
    #include "utils.h"
#endif

#ifndef INSTRUCTIONRUNNERS_H
    #include "instructionrunners.h"
#endif

class Allocator;
enum Opcode
{
    PushV, PushLocal, PopLocal, PushGlobal, PopGlobal, PushNull, GetRef, SetRef,
    Add, Sub, Mul, Div, And, Or, Not, Jmp, JmpVal, If, Lt, Gt, Eq, Ne, Le, Ge,
    Tail, Call, CallMethod, CallRef, Ret, Apply, CallExternal, Nop,
    SetField, GetField, GetFieldRef, GetArr, SetArr, GetArrRef, New, NewArr, ArrLength,
    New_MD_Arr, Get_MD_Arr, Set_MD_Arr, Get_MD_ArrRef, MD_ArrDimensions,
    PushConstant, Neg,
    RegisterEvent, Isa,
    Send, Receive, Select,
    Break, Tick
};

enum CallStyle
{
    NormalCall,
    TailCall,
    LaunchCall
};

struct Instruction
{
    Opcode opcode;
    Value *Arg;
    QString SymRef;
    int SymRefLabel;
     QString True, False;
    int fastTrue , fastFalse;
    CallStyle callStyle;
    int extra;

    Instruction *next;
    InstructionRunner runner;

    Instruction();
    Instruction(Opcode opcode);
    Instruction &wArg(Value *);
    Instruction &wArgParse(QString, Allocator *allocator);
    Instruction &wLabels(QString, QString, int fastL1, int fastL2);
    Instruction &wRef(QString, Labeller &lblr);
    Instruction &wExtra(int info);
    Instruction &wCallStyle(CallStyle);

    void assignRunner();
};

QString InstructionToString(const Instruction &i);

#endif // INSTRUCTION_H
