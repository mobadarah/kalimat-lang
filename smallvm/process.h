#ifndef PROCESS_H
#define PROCESS_H

#ifndef FRAME_H
    #include "frame.h"
#endif

#ifndef VALUE_H
    #include "value.h"
#endif

#ifndef CHANNEL_H
    #include "channel.h"
#endif

#ifndef VMERROR_H
    #include "vmerror.h"
#endif

#include <QStack>
#include <QVector>
#include <QMap>
#include <time.h>

enum ProcessState
{
    SleepingProcess,
    TimerWaitingProcess,
    AwakeProcess,
    MovedToGuiThread
};

class Channel;
class VM;

struct Process
{
    ProcessState state;
    QStack<Frame> stack;

    QVector<Channel *> allChans;
    int nsend;
    VM *owner;
    clock_t timeToWake;
    QString administrator;
    bool interrupt;
public:
    Process(VM *owner);
    void awaken();
    void sleep();
    void select(QVector<Channel *> allChans,
                QVector<Value *> parametes,
                int nsend);

    void successfullSelect(Channel *chan);
public:
    inline bool isFinished()
    {
        if(stack.isEmpty())
        {
            return true;
        }

        //*
        Frame &frame = stack.top();
        if(frame.currentMethod == NULL)
        {
            return true;
        }
        if(!frame.currentMethod->HasInstruction(frame.ip))
        {
            return true;
        }
        //*/

        return false;
    }
public:
    const Instruction &getCurrentInstruction();
    void RunTimeSlice(int slice, VM *vm);
    void RunSingleInstruction();
public:
    void signal(VMErrorType toSignal);
    void signal(VMErrorType toSignal, QString arg0);
    void signal(VMErrorType toSignal, QString arg0, QString arg1);
    void signal(VMErrorType toSignal, QString arg0, QString arg1, QString arg2);
    void signalWithStack(VMError err);

    void assert(bool cond, VMErrorType toSignal);
    void assert(bool cond, VMErrorType toSignal, QString arg0);
    void assert(bool cond, VMErrorType toSignal, QString arg0, QString arg1);
    void assert(bool cond, VMErrorType toSignal, IClass *arg0, IClass *arg1);
    void assert(bool cond, VMErrorType toSignal, QString arg0, QString arg1, QString arg2);
private:
    inline Frame *currentFrame() { return &stack.top(); }
    inline Frame &globalFrame();
    inline QHash<int, Value*> &constantPool();
    inline Allocator &allocator();

    void DoPushVal(Value *Arg);
    void DoPushLocal(QString SymRef);
    void DoPushGlobal(QString SymRef);
    void DoPushConstant(QString SymRef, int SymRefLabel);
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
    void DoJmpVal();
    void DoIf(QString trueLabel, QString falseLabel);
    void DoLt();
    void DoGt();
    void DoEq();
    void DoNe();
    void DoLe();
    void DoGe();
    void DoCall(QString symRef, int SymRefLabel, int arity, CallStyle callStyle);
    void DoCallRef(QString symRef, int SymRefLabel, int arity, CallStyle callStyle);
    void DoCallMethod(QString SymRef, int SymRefLabel, int arity, CallStyle callStyle);
    void DoRet();
    void DoApply();
    void DoCallExternal(QString symRef, int SymRefLabel, int arity);
    void DoSetField(QString SymRef);
    void DoGetField(QString SymRef);
    void DoGetFieldRef(QString SymRef);
    void DoGetArr();
    void DoSetArr();
    void DoGetArrRef();
    void DoNew(QString SymRef, int SymRefLabel);
    void DoNewArr();
    void DoArrLength();
    void DoNewMD_Arr();
    void DoGetMD_Arr();
    void DoSetMD_Arr();
    void DoGetMD_ArrRef();
    void DoMD_ArrDimensions();
    void DoRegisterEvent(Value *evname, QString SymRef);
    void DoIsa(QString SymRef, int SymRefLabel);
    void DoSend();
    void DoReceive();
    void DoSelect();
    void DoBreak();
    void DoTick();

    void CallImpl(QString sym, int SymRefLabel, bool wantValueNotRef, int arity, CallStyle callStyle);
    void CallImpl(Method *method, bool wantValueNotRef, int arity, CallStyle callStyle);
    void CallSpecialMethod(IMethod *method, QVector<Value *> args);
    void test(bool, QString, QString);
    bool coercion(Value *v1, Value *v2, Value *&newV1, Value *&newV2);
    Value *_div(Value *, Value *);
    void Pop_Md_Arr_and_indexes(MultiDimensionalArray<Value *> *&theArray, QVector<int> &indexes);

    void BuiltInArithmeticOp(QString opName, int (*intFunc)(int,int),
                             long (*longFunc)(long, long),
                             double (*doubleFunc)(double,double));
    void BuiltInComparisonOp(bool  (*intFunc)(int,int),
                             bool (*longFunc)(long, long),
                             bool (*doubleFunc)(double,double),
                             bool (*strFunc)(QString, QString));
    void BuiltInAddOp(int (*intFunc)(int,int),
                      long (*longFunc)(long, long),
                      double (*doubleFunc)(double,double),
                      QString (*strFunc)(QString ,QString));
    void EqualityRelatedOp(bool  (*intFunc)(int,int),
                           bool (*longFunc)(long, long),
                           bool  (*doubleFunc)(double,double),
                           bool  (*boolFunc)(bool, bool),
                           bool  (*objFunc)(IObject *, IObject *),
                           bool  (*strFunc)(QString, QString),
                           bool  (*rawFunc)(void *, void *),
                           bool  (*differentTypesFunc)(Value *, Value *),
                           bool  (*nullFunc)());
    void BinaryLogicOp(bool (*boolFunc)(bool, bool));
    void UnaryLogicOp(bool (*boolFunc)(bool));
private:
    int popIntOrCoercedDouble();
    double popDoubleOrCoercedInt();
    IMethod *popMethod();
    VArray *popArray();
    inline Value *__top();
};

#endif // PROCESS_H
