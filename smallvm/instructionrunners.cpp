#include "instructionrunners.h"
#include "process.h"

Instruction *run_PushV(Instruction *i, Process *proc)
{
    proc->DoPushVal(i->Arg);
    return i->next;
}

Instruction *run_PushLocal(Instruction *i, Process *proc)
{
    proc->DoPushLocal(i->SymRef, i->SymRefLabel);
    return i->next;
}

Instruction *run_PopLocal(Instruction *i, Process *proc)
{
    proc->DoPopLocal(i->SymRef, i->SymRefLabel);
    return i->next;
}

Instruction *run_PushGlobal(Instruction *i, Process *proc)
{
    proc->DoPushGlobal(i->SymRef);
    return i->next;
}

Instruction *run_PopGlobal(Instruction *i, Process *proc)
{
    proc->DoPopGlobal(i->SymRef);
    return i->next;
}

Instruction *run_PushNull(Instruction *i, Process *proc)
{
    proc->DoPushNull();
    return i->next;
}

Instruction *run_GetRef(Instruction *i, Process *proc)
{
    proc->DoGetRef();
    return i->next;
}

Instruction *run_SetRef(Instruction *i, Process *proc)
{
    proc->DoSetRef();
    return i->next;
}

Instruction *run_Add(Instruction *i, Process *proc)
{
    proc->DoAdd();
    return i->next;
}

Instruction *run_Sub(Instruction *i, Process *proc)
{
    proc->DoSub();
    return i->next;
}

Instruction *run_Mul(Instruction *i, Process *proc)
{
    proc->DoMul();
    return i->next;
}

Instruction *run_Div(Instruction *i, Process *proc)
{
    proc->DoDiv();
    return i->next;
}

Instruction *run_And(Instruction *i, Process *proc)
{
    proc->DoAnd();
    return i->next;
}

Instruction *run_Or(Instruction *i, Process *proc)
{
    proc->DoOr();
    return i->next;
}

Instruction *run_Not(Instruction *i, Process *proc)
{
    proc->DoNot();
    return i->next;
}

Instruction *run_Jmp(Instruction *i, Process *proc)
{
    proc->DoJmp(i->True, i->fastTrue);
    return i->next;
}

Instruction *run_JmpVal(Instruction *i, Process *proc)
{
    proc->DoJmpVal();
    return i->next;
}

Instruction *run_If(Instruction *i, Process *proc)
{
    proc->DoIf(i->True, i->False, i->fastTrue, i->fastFalse);
    return i->next;
}

Instruction *run_Lt(Instruction *i, Process *proc)
{
    proc->DoLt();
    return i->next;
}

Instruction *run_Gt(Instruction *i, Process *proc)
{
    proc->DoGt();
    return i->next;
}

Instruction *run_Eq(Instruction *i, Process *proc)
{
    proc->DoEq();
    return i->next;
}

Instruction *run_Ne(Instruction *i, Process *proc)
{
    proc->DoNe();
    return i->next;
}

Instruction *run_Le(Instruction *i, Process *proc)
{
    proc->DoLe();
    return i->next;
}

Instruction *run_Ge(Instruction *i, Process *proc)
{
    proc->DoGe();
    return i->next;
}

Instruction *run_Tail(Instruction *i, Process *proc)
{
    return i->next;
}

Instruction *run_Call(Instruction *i, Process *proc)
{
    proc->DoCall(i->SymRef, i->SymRefLabel, getInstructionArity(*i), i->callStyle);
    return i->next;
}

Instruction *run_CallMethod(Instruction *i, Process *proc)
{
    proc->DoCallMethod(i->SymRef, i->SymRefLabel, getInstructionArity(*i), i->callStyle);
    return i->next;
}

Instruction *run_CallRef(Instruction *i, Process *proc)
{
    proc->DoCallRef(i->SymRef, i->SymRefLabel, getInstructionArity(*i), i->callStyle);
    return i->next;
}

Instruction *run_Ret(Instruction *i, Process *proc)
{
    proc->DoRet();
    return i->next;
}

Instruction *run_Apply(Instruction *i, Process *proc)
{
    proc->DoApply();
    return i->next;
}

Instruction *run_CallExternal(Instruction *i, Process *proc)
{
    proc->DoCallExternal(i->SymRef, i->SymRefLabel, getInstructionArity(*i));
    return i->next;
}

Instruction *run_Nop(Instruction *i, Process *proc)
{
    return i->next;
}

Instruction *run_SetField(Instruction *i, Process *proc)
{
    proc->DoSetField(i->SymRef);
    return i->next;
}

Instruction *run_GetField(Instruction *i, Process *proc)
{
    proc->DoGetField(i->SymRef);
    return i->next;
}

Instruction *run_GetFieldRef(Instruction *i, Process *proc)
{
    proc->DoGetFieldRef(i->SymRef);
    return i->next;
}

Instruction *run_GetArr(Instruction *i, Process *proc)
{
    proc->DoGetArr();
    return i->next;
}

Instruction *run_SetArr(Instruction *i, Process *proc)
{
    proc->DoSetArr();
    return i->next;
}

Instruction *run_GetArrRef(Instruction *i, Process *proc)
{
    proc->DoGetArrRef();
    return i->next;
}

Instruction *run_New(Instruction *i, Process *proc)
{
    proc->DoNew(i->SymRef, i->SymRefLabel);
    return i->next;
}

Instruction *run_NewArr(Instruction *i, Process *proc)
{
    proc->DoNewArr();
    return i->next;
}

Instruction *run_ArrLength(Instruction *i, Process *proc)
{
    proc->DoArrLength();
    return i->next;
}

Instruction *run_New_MD_Arr(Instruction *i, Process *proc)
{
    proc->DoNewMD_Arr();
    return i->next;
}

Instruction *run_Get_MD_Arr(Instruction *i, Process *proc)
{
    proc->DoGetMD_Arr();
    return i->next;
}

Instruction *run_Set_MD_Arr(Instruction *i, Process *proc)
{
    proc->DoSetMD_Arr();
    return i->next;
}

Instruction *run_Get_MD_ArrRef(Instruction *i, Process *proc)
{
    proc->DoGetMD_ArrRef();
    return i->next;
}

Instruction *run_MD_ArrDimensions(Instruction *i, Process *proc)
{
    proc->DoMD_ArrDimensions();
    return i->next;
}

Instruction *run_PushConstant(Instruction *i, Process *proc)
{
    proc->DoPushConstant(i->SymRef, i->SymRefLabel);
    return i->next;
}

Instruction *run_Neg(Instruction *i, Process *proc)
{
    proc->DoNeg();
    return i->next;
}

Instruction *run_RegisterEvent(Instruction *i, Process *proc)
{
    proc->DoRegisterEvent(i->Arg, i->SymRef);
    return i->next;
}

Instruction *run_Isa(Instruction *i, Process *proc)
{
    proc->DoIsa(i->SymRef, i->SymRefLabel);
    return i->next;
}

Instruction *run_Send(Instruction *i, Process *proc)
{
    proc->DoSend();
    return i->next;
}

Instruction *run_Receive(Instruction *i, Process *proc)
{
    proc->DoReceive();
    return i->next;
}

Instruction *run_Select(Instruction *i, Process *proc)
{
    proc->DoSelect();
    return i->next;
}

Instruction *run_Break(Instruction *i, Process *proc)
{
    proc->DoBreak();
    return i->next;
}

Instruction *run_Tick(Instruction *i, Process *proc)
{
    proc->DoTick();
    return i->next;
}

