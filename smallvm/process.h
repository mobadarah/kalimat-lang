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

/*
  Processes are a fundamental component in SmallVM. A process has a call stack, runs instructions, and can communicate with
  other processes via channels.

  Each process is owned by exactly one Scheduler which is responsible for executing it, but it can migrate to another
  scheduler if needed.

  A process has one of three states:

  - Awake, meaning it is either running now or waiting to be run in the 'running' queue of some scheduler

  - Sleeping, meaning it is blocking for a channel communication like send or receive. Meanwhile it will be in the
    'sleeping' queue of its scheduler

  - TimerWaiting, meaning the wait(ms) function is called and the process is in the 'timerWaiting' queue of some scheduler, to be
    reactivated later.
  */

enum ProcessState
{
    SleepingProcess,
    TimerWaitingProcess,
    AwakeProcess
};

class Channel;
class VM;
class Scheduler;

inline int getInstructionArity(const Instruction &i)
{
    if(i.Arg == NULL)
        return -1;
    return i.Arg->unboxInt();
}

struct Process
{
    ProcessState state;
    Frame *stack;

    // allChans and nsend are used in select(...) operations
    QVector<Channel *> allChans;
    int nsend;

    Scheduler *owner;
    VM *vm;
    clock_t timeToWake;
    QString administrator;
    bool interrupt;
    int timeSlice;
    QString starterProcedureName;
public:
    Scheduler *wannaMigrateTo;
public:
    Process(Scheduler *owner);
    ~Process();
    void awaken();
    void sleep();
    void select(QVector<Channel *> allChans,
                QVector<Value *> parametes,
                int nsend);

    void successfullSelect(Channel *chan);
public:
    inline bool isFinished()
    {
        if(stack == NULL)
        {
            return true;
        }

        //*
        const  Frame *frame = stack;
        if(frame->currentMethod == NULL)
        {
            return true;
        }

        if(!frame->currentMethod->HasInstruction(frame->ip))
        {
            return true;
        }
        //*/

        return false;
    }
    inline void pushFrame(Frame *f) {
        f->next = stack;
        stack = f;
    }

    inline Frame *popFrame() {
        if(stack == NULL)
            signal(InternalError1, "Accessing frame on empty call stack");
        Frame *temp = stack;
        stack = stack->next;
        return temp;
    }
    inline Frame *currentFrame() { return stack; }

public:
    const Instruction &getCurrentInstruction();
    void RunTimeSlice(int slice, VM *vm, Scheduler *caller);
    void RunSingleInstruction();
    void migrateTo(Scheduler *scheduler);
public:
    void signal(VMErrorType toSignal);
    void signal(VMErrorType toSignal, QString arg0);
    void signal(VMErrorType toSignal, QString arg0, QString arg1);
    void signal(VMErrorType toSignal, IClass *arg0, IClass *arg1);
    void signal(VMErrorType toSignal, QString arg0, QString arg1, QString arg2);
    void signalWithStack(VMError err);

    //*
    void assert(bool cond, VMErrorType toSignal);
    void assert(bool cond, VMErrorType toSignal, QString arg0);
    void assert(bool cond, VMErrorType toSignal, QString arg0, QString arg1);
    void assert(bool cond, VMErrorType toSignal, IClass *arg0, IClass *arg1);
    void assert(bool cond, VMErrorType toSignal, QString arg0, QString arg1, QString arg2);
    //*/
private:
    inline QMap<QString, Value *> &globalFrame();
    inline QHash<int, Value*> &constantPool();
    inline Allocator &allocator();
public:
    void DoPushVal(Value *Arg);
    void DoPushLocal(const QString &SymRef, int SymRefLabel);
    void DoPushGlobal(const QString &SymRef);
    void DoPushConstant(QString SymRef, int SymRefLabel);
    void DoPopLocal(const QString &SymRef, int SymRefLabel);
    void DoPopGlobal(const QString &SymRef);
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
    void DoJmp(const QString &label, int fastLabel);
    void DoJmpVal();
    void DoIf(const QString &trueLabel, const QString &falseLabel, int fastTrueLabel, int fastFalseLabel);
    void DoLt();
    void DoGt();
    void DoEq();
    void DoNe();
    void DoLe();
    void DoGe();
    void DoCall(const QString &symRef, int SymRefLabel, int arity, CallStyle callStyle);
    void DoCallRef(const QString &symRef, int SymRefLabel, int arity, CallStyle callStyle);
    void DoCallMethod(const QString &SymRef, int SymRefLabel, int arity, CallStyle callStyle);
    void DoRet();
    void DoApply();
    void DoCallExternal(const QString &symRef, int SymRefLabel, int arity);
    void DoSetField(const QString &SymRef);
    void DoGetField(const QString &SymRef);
    void DoGetFieldRef(const QString &SymRef);
    void DoGetArr();
    void DoSetArr();
    void DoGetArrRef();
    void DoNew(const QString &SymRef, int SymRefLabel);
    void DoNewArr();
    void DoArrLength();
    void DoNewMD_Arr();
    void DoGetMD_Arr();
    void DoSetMD_Arr();
    void DoGetMD_ArrRef();
    void DoMD_ArrDimensions();
    void DoRegisterEvent(Value *evname, QString SymRef);
    void DoIsa(const QString &SymRef, int SymRefLabel);
    void DoSend();
    void DoReceive();
    void DoSelect();
    void DoBreak();
    void DoTick();

    void CallImpl(const QString &sym, int SymRefLabel, bool wantValueNotRef, int arity, CallStyle callStyle);
    void CallImpl(Method *method, bool wantValueNotRef, int arity, CallStyle callStyle);
    void CallSpecialMethod(IMethod *method, QVector<Value *> args);
    void test(bool, const QString &, const QString &, int fastTrueLabel, int fastFalseLabel);
    bool coercion(Value *v1, Value *v2, Value *&newV1, Value *&newV2);
    Value *_div(Value *, Value *);
    void Pop_Md_Arr_and_indexes(MultiDimensionalArray<Value *> *&theArray, QVector<int> &indexes);

    void BuiltInArithmeticOp(const QString &opName, int (*intFunc)(int,int),
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
    inline Value *popValue()
    {
        if(currentFrame()->OperandStack.empty())
            signal(InternalError1, "Empty operand stack");
        return currentFrame()->OperandStack.pop();
    }

    int popIntOrCoercedDouble();
    double popDoubleOrCoercedInt();
    IMethod *popMethod();
    VArray *popArray();
    inline Value *__top() {
        return currentFrame()->OperandStack.top();
    }
};

#endif // PROCESS_H
