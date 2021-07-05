/**************************************************************************
**   The Kalimat programming language
**   Copyright (C) 2010 Mohamed Samy Ali - samy2004@gmail.com
**   This project is released under the Apache License version 2.0
**   as described in the included license.txt file
**************************************************************************/

#include "method.h"
#include "vm.h"
#include "vmerror.h"
#include "runtime_identifiers.h"

Method::Method(QString name,int arity)
{
    this->name = name;
    this->arity = arity;
    this->numReturnValues = 0;
    this->returnsReference = false;
    receiver = NULL; // 'tis a global method, not a member of a class
}

Method::Method(QString name,int arity, int numReturnValues)
{
    this->name = name;
    this->arity = arity;
    this->numReturnValues = numReturnValues;
    this->returnsReference = false;
    receiver = NULL; // 'tis a global method, not a member of a class
}

Method::Method(QString name, int arity, int numReturnValues, bool returnsReference, ValueClass *receiver)
{
    this->name = name;
    this->arity = arity;
    this->numReturnValues = numReturnValues;
    this->returnsReference = returnsReference;
    this->receiver = receiver;
}

void Method::prepareInstruction(int index)
{
    Instruction &i = instructions[index];

    if(index > 0)
    {
        instructions[index-1].next = &i;
    }


    if(i.opcode == PushLocal || i.opcode == PopLocal)
    {
        int index = localsInterner.labelOf(i.SymRef);
        i.SymRefLabel = index;
        Locals[i.SymRef] = index;
    }
}

void Method::Add(Instruction i)
{
    instructions.append(i);
    prepareInstruction(instructions.count()-1);
}

void Method::Add(Instruction i, QString label)
{
    instructions.append(i);
    prepareInstruction(instructions.count()-1);

    if(label != "")
    {
        labels[label] = instructions.count() -1;
    }
}

Instruction &Method::Add(Instruction i, QString label, int extraInfo)
{
    instructions.append(i.wExtra(extraInfo));
    int index = instructions.count()-1;
    prepareInstruction(index);
    if(label != "")
    {
        labels[label] = index;
    }
    return instructions[index];
}

void Method::Set(int ip, Instruction i)
{
    instructions[ip] = i;
    prepareInstruction(ip);
}

void Method::setLabelsInInstructions()
{
    for(int i=0; i<instructions.count(); ++i)
    {
        Instruction &in = instructions[i];

        if(in.opcode == Jmp)
        {
            in.SymRefLabel = labels[in.SymRef];
        }
    }
}

void Method::optimize()
{
    static QMap<Opcode, Opcode> comparisons;
    if(comparisons.empty())
    {
        comparisons[Eq] = JmpIfNe;
        comparisons[Ne] = JmpIfEq;
    }

    for(int i=0; i<instructions.count(); ++i)
    {
        Instruction &in = instructions[i];

        if(in.opcode == PushVal
                && in.Arg->type == BuiltInTypes::IntType
                && unboxInt(in.Arg) == 1
                && ((i+1) < instructions.count()))
        {
            Instruction &in1 = instructions[i+1];

            // if it's pushv 1, add make it inc
            if(in1.opcode == ::Add)
            {
                in.opcode = Increment;
                in1.opcode = Nop;
            }
            // if it's pushv 1, sub, make it dec
            else if(in1.opcode == Sub)
            {
                in.opcode = Decrement;
                in1.opcode = Nop;
            }
        }
        if(comparisons.contains(in.opcode) && i+2 < instructions.count())
        {
            Instruction &in1 = instructions[i+1];
            Instruction &in2 = instructions[i+2];

            if(in1.opcode == If)
            {
                in.opcode = comparisons[in.opcode];
                in.SymRef = in2.SymRef;
                in.SymRefLabel = in2.SymRefLabel;

                in1.opcode = Nop;
                in2.opcode = Nop;
            }
        }
    }
}

void Method::buildCFlowGraph()
{
    if(instructions.count() == 0)
        return;

    QSet<int> leaders;
    leaders.insert(0);
    for(int i=0; i<instructions.count(); ++i)
    {
        Instruction &in = instructions[i];
        if((in.opcode == Jmp
            ||
            in.opcode == JmpVal
            ||
            in.opcode == Ret
            ) && (i+1) < instructions.count())
        {
            leaders.insert(i+1);
        }

        if(labels.values().contains(i))
            leaders.insert(i);
    }

    QList<int> sortedLeader = leaders.toList();
    qSort(sortedLeader);
    for(int i=0; i<sortedLeader.count(); ++i)
    {
        int i1 = sortedLeader[i];
        int i2;
        if((i+1) < sortedLeader.count())
            i2 = sortedLeader[i+1];
        else
            i2 = instructions.count() -1;

        cflow.addBasicBlock(i1, i2);
    }

    for(int i=0; i<cflow.basicBlockCount(); ++i)
    {
        BasicBlock &bb = cflow.basicBlock(i);
        int ix = cflow.basicBlockStartingAt(bb.to+1);
        if(ix != -1)
        {
            cflow.addEdge(i, ix);
        }
    }
}

void Method::computeMaxStack(VM *vm)
{
    for(int i=0; i<cflow.basicBlockCount(); ++i)
    {
        computeStackDiff(cflow.basicBlock(i), vm);
    }
}

void Method::computeStackDiff(BasicBlock &bb, VM *vm)
{
    int d = 0;
    for(int i=bb.from; i<=bb.to; ++i)
    {
        int delta;
        Instruction &in = instructions[i];
        if(in.opcode == Call || in.opcode == CallRef)
        {
            // IMethod *method = vm->GetMethod(in.SymRef);
            // delta += -method->Arity() + 1; // consume args, push (possibly) return val

            delta +=1; // we won't bother to find arity for now, todo:
        }
        else if(in.opcode == CallExternal)
        {
            // IMethod *method = vm->GetMethod(in.SymRef);
            // delta += -method->Arity() + 1;

            delta +=1; // we won't bother to find arity for now, todo:
        }
        else if(in.opcode == CallMethod)
        {
            delta +=1; // we won't bother to find arity for now, todo:
        }
        else
        {
#define VM_INSTRUCTION(opCode, take, put) \
    if(in.opcode == opCode) { \
        delta += (put - take) }
#undef VM_INSTRUCTION
        }
        d+=delta;
    }
    bb.stackChange = d;

}

QString Method::getName()
{
    return this->name;
}

bool Method::hasSlot(QString name)
{
    return false;
}

QList<QString> Method::getSlotNames()
{
    return QList<QString>();
}

Value *Method::getSlotValue(QString name)
{
    return NULL;
}

void Method::setSlotValue(QString name, Value *val)
{

}

QString Method::toString()
{
    return getName();
}

IMethod *MethodClass::Apply = NULL;

IMethod *MethodClass::lookupMethod(QString name)
{
    if(name == VMId::get(RId::Invoke))
        return MethodClass::Apply;
    return NULL;
}

IObject *MethodClass::newValue(Allocator *allocator)
{
    throw VMError(InternalError1).arg("Cannot create a method directly");
}
