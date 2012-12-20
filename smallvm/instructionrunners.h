#ifndef INSTRUCTIONRUNNERS_H
#define INSTRUCTIONRUNNERS_H

class Instruction;
class Process;

typedef Instruction *(*InstructionRunner)(Instruction *i, Process *proc);

Instruction *run_PushV(Instruction *i, Process *proc);
Instruction *run_PushLocal(Instruction *i, Process *proc);
Instruction *run_PopLocal(Instruction *i, Process *proc);
Instruction *run_PushGlobal(Instruction *i, Process *proc);
Instruction *run_PopGlobal(Instruction *i, Process *proc);
Instruction *run_PushNull(Instruction *i, Process *proc);
Instruction *run_GetRef(Instruction *i, Process *proc);
Instruction *run_SetRef(Instruction *i, Process *proc);
Instruction *run_Add(Instruction *i, Process *proc);
Instruction *run_Sub(Instruction *i, Process *proc);
Instruction *run_Mul(Instruction *i, Process *proc);
Instruction *run_Div(Instruction *i, Process *proc);
Instruction *run_And(Instruction *i, Process *proc);
Instruction *run_Or(Instruction *i, Process *proc);
Instruction *run_Not(Instruction *i, Process *proc);
Instruction *run_Jmp(Instruction *i, Process *proc);
Instruction *run_JmpVal(Instruction *i, Process *proc);
Instruction *run_If(Instruction *i, Process *proc);
Instruction *run_Lt(Instruction *i, Process *proc);
Instruction *run_Gt(Instruction *i, Process *proc);
Instruction *run_Eq(Instruction *i, Process *proc);
Instruction *run_Ne(Instruction *i, Process *proc);
Instruction *run_Le(Instruction *i, Process *proc);
Instruction *run_Ge(Instruction *i, Process *proc);
Instruction *run_Tail(Instruction *i, Process *proc);
Instruction *run_Call(Instruction *i, Process *proc);
Instruction *run_CallMethod(Instruction *i, Process *proc);
Instruction *run_CallRef(Instruction *i, Process *proc);
Instruction *run_Ret(Instruction *i, Process *proc);
Instruction *run_Apply(Instruction *i, Process *proc);
Instruction *run_CallExternal(Instruction *i, Process *proc);
Instruction *run_Nop(Instruction *i, Process *proc);
Instruction *run_SetField(Instruction *i, Process *proc);
Instruction *run_GetField(Instruction *i, Process *proc);
Instruction *run_GetFieldRef(Instruction *i, Process *proc);
Instruction *run_GetArr(Instruction *i, Process *proc);
Instruction *run_SetArr(Instruction *i, Process *proc);
Instruction *run_GetArrRef(Instruction *i, Process *proc);
Instruction *run_New(Instruction *i, Process *proc);
Instruction *run_NewArr(Instruction *i, Process *proc);
Instruction *run_ArrLength(Instruction *i, Process *proc);
Instruction *run_New_MD_Arr(Instruction *i, Process *proc);
Instruction *run_Get_MD_Arr(Instruction *i, Process *proc);
Instruction *run_Set_MD_Arr(Instruction *i, Process *proc);
Instruction *run_Get_MD_ArrRef(Instruction *i, Process *proc);
Instruction *run_MD_ArrDimensions(Instruction *i, Process *proc);
Instruction *run_PushConstant(Instruction *i, Process *proc);
Instruction *run_Neg(Instruction *i, Process *proc);
Instruction *run_RegisterEvent(Instruction *i, Process *proc);
Instruction *run_Isa(Instruction *i, Process *proc);
Instruction *run_Send(Instruction *i, Process *proc);
Instruction *run_Receive(Instruction *i, Process *proc);
Instruction *run_Select(Instruction *i, Process *proc);
Instruction *run_Break(Instruction *i, Process *proc);
Instruction *run_Tick(Instruction *i, Process *proc);

#endif // INSTRUCTIONRUNNERS_H
