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

#ifndef RUNTIMEIDENTIFIERS_H
    #include "runtime_identifiers.h"
#endif

#ifndef FRAMEPOOL_H
    #include "framepool.h"
#endif

#ifndef OPERANDSTACK_H
    #include "operandstack.h"
#endif

#ifndef DEBUGGER_H
    #include "debugger.h"
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
    return unboxInt(i.Arg);
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
    FramePool framePool;

    bool _isFinished;
    VOperandStack OperandStack;
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

        /*
        const  Frame *frame = stack;

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
    inline bool frameHasOperands()
    {
        return OperandStack.count() > currentFrame()->operandStackLevel;
        //return true;
    }
    inline void pushOperand(Value *v)
    {
        OperandStack.push(v);
    }

    inline Value *popOperand() { return popValue();}
private:
    int currentLabel;
public:
    const Instruction &getCurrentInstruction();
    void RunTimeSlice(VM *vm);
    void FastRunTimeSlice();
    void RunSingleInstruction(VM *vm);
    void RunUntilReturn();
    void migrateTo(Scheduler *scheduler);
    inline void exitTimeSlice() { currentLabel = 1; }
    //inline void exitTimeSlice() { timeSlice = 0; }
    void Break(BreakSource::Src s) { BreakImpl(s); }
private:
    void startMigrationToGui();
    void migrateBackFromGui();
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
    inline QHash<int, Value *> &externalMethods();
    inline Allocator &allocator();
public:
#define VM_INSTRUCTION(x, __t, __p) \
    inline void Do##x(Instruction *);
#include "instruction_defs.h"
#undef VM_INSTRUCTION

    void BreakImpl(BreakSource::Src source);
    inline void JmpImpl(int label);
    inline void CallImpl(Instruction *executedInstruction);
    inline void verifyArity(int arity, Method *method)
    {
        int marity = method->Arity();
        if(arity != -1 && marity !=-1 && arity != marity)
        {
           signal(WrongNumberOfArguments3,
                method->getName(), str(arity), str(method->Arity()));
        }
    }

    void CallImpl(Method *method, CallStyle callStyle);
    void copyArgs(VOperandStack &source, VOperandStack &dest, int marity);
    void CallSpecialMethod(IMethod *method, int arity);
    bool coercion(Value *v1, Value *v2, Value *&newV1, Value *&newV2);
    Value *_div(Value *, Value *);
    void Pop_Md_Arr_and_indexes(MultiDimensionalArray<Value *> *&theArray, QVector<int> &indexes);

    inline void BuiltInArithmeticOp(RId::RuntimeId opName, int (*intFunc)(int,int),
                             long (*longFunc)(long, long),
                             double (*doubleFunc)(double,double));
    void BinaryLogicOp(bool (*boolFunc)(bool, bool));
    void UnaryLogicOp(bool (*boolFunc)(bool));
private:
    inline Value *popValue()
    {
        if(!frameHasOperands())
            signal(InternalError1, "Empty operand stack");
        return OperandStack.pop();
    }

    int popIntOrCoercedDouble();
    inline int popInt();
    inline bool popBool();
    double popDoubleOrCoercedInt();
    IMethod *popIMethod();
    Method *popMethod();
    VArray *popArray();
    inline Value *__top() {
        return OperandStack.top();
    }
};

#endif // PROCESS_H
