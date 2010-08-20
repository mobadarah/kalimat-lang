/**************************************************************************
**   The Kalimat programming language
**   Copyright (C) 2010 Mohamed Samy Ali - samy2004@gmail.com
**   This project is released under the Apache License version 2.0
**   as described in the included license.txt file
**************************************************************************/

#ifndef VM_H
#define VM_H

#include "allocator.h"
template <typename T> bool isa(void * obj)
{
    T value = dynamic_cast<T>(obj);
    return value != NULL;
}


class VM
{

    QMap<QString, Value*> constantPool;
    QStack<Frame> stack;

    // The allocator must be declared after the 'constantPool' and 'stack'
    // members, since it's initialized with them in VMs constructor initializer list!!
    Allocator allocator;
    Frame *currentFrame();
    Frame &globalFrame();



    bool _isRunning;
    VMError _lastError;
    QMap<QString, QString> registeredEventHandlers;
public:
    VM();
    void Init();
    static void LoadCallInstruction(Opcode type, QString arg, Allocator &allocator, Method *method, QString label, int extraInfo);
    void Load(QString assemblyCode);

    void Register(QString symRef, ExternalMethod *method);
    void DefineStringConstant(QString symRef, QString strValue);
    void ActivateEvent(QString evName, QVector<Value *> args);
    void RunStep();
    Allocator &GetAllocator();
    void gc();

    bool hasRunningInstruction();
    Instruction getCurrentInstruction();

    void signal(VMErrorType toSignal);
    void signal(VMErrorType toSignal, QString arg0);
    void signal(VMErrorType toSignal, QString arg0, QString arg1);
    void signal(VMErrorType toSignal, QString arg0, QString arg1, QString arg2);
    QString toStr(int);

    bool isRunning();
    Value *__top();

    void assert(bool cond, VMErrorType toSignal);
    void assert(bool cond, VMErrorType toSignal, QString arg0);
    void assert(bool cond, VMErrorType toSignal, QString arg0, QString arg1);
    void assert(bool cond, VMErrorType toSignal, QString arg0, QString arg1, QString arg2);

    QStack<Frame> &getCallStack();

    void DoPushVal(Value *Arg);
    void DoPushLocal(QString SymRef);
    void DoPushGlobal(QString SymRef);
    void DoPushConstant(QString SymRef);
    void DoPopLocal(QString SymRef);
    void DoPopGlobal(QString SymRef);
    void DoPushNull();
    void DoGetRef();
    void DoSetRef();
    void DoAdd();
    void DoSub();
    void DoMul();
    void DoDiv();
    void DoNeg();
    void DoAnd();
    void DoOr();
    void DoNot();
    void DoJmp(QString label);
    void DoIf(QString trueLabel, QString falseLabel);
    void DoLt();
    void DoGt();
    void DoEq();
    void DoNe();
    void DoLe();
    void DoGe();
    void DoCall(QString symRef, int arity);
    void DoCallRef(QString symRef, int arity);
    void DoCallMethod(QString SymRef, int arity);
    void DoRet();
    void DoCallExternal(QString symRef, int arity);
    void DoSetField(QString SymRef);
    void DoGetField(QString SymRef);
    void DoGetFieldRef(QString SymRef);
    void DoGetArr();
    void DoSetArr();
    void DoGetArrRef();
    void DoNew(QString SymRef);
    void DoNewArr();
    void DoArrLength();
    void DoNewMD_Arr();
    void DoGetMD_Arr();
    void DoSetMD_Arr();
    void DoGetMD_ArrRef();
    void DoMD_ArrDimensions();
    void DoRegisterEvent(Value *evname, QString SymRef);

    void CallImpl(QString sym, bool wantValueNotRef, int arity);
    void test(bool, QString, QString);
    Value *_div(Value *, Value *);
    void Pop_Md_Arr_and_indexes(MultiDimensionalArray<Value *> *&theArray, QVector<int> &indexes);

    VMError GetLastError();
private:

    void patchupInheritance(QMap<ValueClass *, QString> inheritanceList);
    void NumericBinaryOp(int (*intFunc)(int,int), double (*doubleFunc)(double,double));
    void BuiltInBinaryBoolOp(int (*intFunc)(int,int), int (*doubleFunc)(double,double), int (*strFunc)(QString *, QString *));
    void BuiltInBinaryOp(int (*intFunc)(int,int), double (*doubleFunc)(double,double), QString *(*strFunc)(QString *,QString *));
    void BinaryBoolOp(int (*intFunc)(int,int), int (*doubleFunc)(double,double), int (*objFunc)(Object *, Object *),
                      int (*strFunc)(QString *, QString *),
                      int (*rawFunc)(void *, void *));
    void BinaryLogicOp(int (*intFunc)(int,int));
    void UnaryLogicOp(int (*intFunc)(int));
public:
    int popIntOrCoercedDouble();
    double popDoubleOrCoercedInt();

};

#endif // VM_H

