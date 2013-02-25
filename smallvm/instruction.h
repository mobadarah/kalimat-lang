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

class Allocator;
class Process;
class Instruction;

typedef void(Process::*InstructionRunner)();

enum Opcode
{
#define VM_INSTRUCTION(x, __t, __p) \
    x,
#include "instruction_defs.h"
#undef VM_INSTRUCTION
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

    CallStyle callStyle;
    int extra;

    Instruction *next;

    Instruction();
    Instruction(Opcode opcode);
    Instruction &wArg(Value *);
    Instruction &wArgParse(QString, Allocator *allocator);
    Instruction &wLabels(QString);
    Instruction &wRef(QString, Labeller &lblr);
    Instruction &wExtra(int info);
    Instruction &wCallStyle(CallStyle);
};

QString InstructionToString(const Instruction &i);

#endif // INSTRUCTION_H
