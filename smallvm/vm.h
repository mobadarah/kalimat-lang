/**************************************************************************
**   The Kalimat programming language
**   Copyright (C) 2010 Mohamed Samy Ali - samy2004@gmail.com
**   This project is released under the Apache License version 2.0
**   as described in the included license.txt file
**************************************************************************/

#ifndef VM_H
#define VM_H

#ifndef PROCESS_H
    #include "process.h"
#endif

#ifndef ALLOCATOR_H
    #include "allocator.h"
#endif

#ifndef VMERROR_H
    #include "vmerror.h"
#endif

#ifndef EXTERNALMETHOD_H
    #include "externalmethod.h"
#endif

#ifndef DEBUGGER_H
    #include "debugger.h"
#endif

#include "utils.h"

#include <QQueue>

template <typename T> bool isa(void * obj)
{
    T value = dynamic_cast<T>(obj);
    return value != NULL;
}

template<class T> class QVectorMap
{
    QVector<T> _map;
public:
    T &operator[](int idx) {
        if(idx < _map.size())
            return _map[idx];
        else
        {
            _map.resize(idx+10);
            return _map[idx];
        }
    }
    bool contains(int key) {
        return _map[key] != NULL;
    }
};

class VM
{
    Labeller constantPoolLabeller;
    QHash<int, Value*> constantPool;
    // Maps from original QObject-based class name
    // to name in SmallVM (English or Arabic)
    QMap<QString, QString> translatedQbjClasses;
    QQueue<Process *> processes;
    QQueue<Process *> running;
    QQueue<Process *> timerWaiting;
    QQueue<Process *> newProcesses;


    /* An 'administrator' is a special process queue
       that can be used to run processes in sequence (each of them
       must wait for the previous one to finish. If you call
       launchAdministeredProcess with a name of an administrator,
       the VM scheduler would lauch the process as suspended
       (except if the queue is empty) and resume the next process
       in the queue once the current is done.

       You must register an administrator with
       "registerProcessAdministrator" before using it.
       */
    QMap<QString, QQueue<Process *> > processAdministrators;

    // The allocator must be declared after the 'constantPool' and 'stack'
    // members, since it's initialized with them in VMs constructor initializer list!!
    Allocator allocator;

    bool _isRunning;
    VMError _lastError;
    Frame *_globalFrame;
    QMap<QString, QString> registeredEventHandlers;
private:
    QStack<Frame> &stack();
public: // todo:temp
    inline Frame *currentFrame()
    {
        if(stack().empty())
            return NULL;
        return &stack().top();
    }

private:
    Frame &globalFrame();
private:
    // Map Method name -> offset -> instruction
    // This will store the original instruction before we replace it with a 'break'
    QMap<QString, QMap<int, Instruction> > breakPoints;
    Debugger *debugger;
    Process *_mainProcess;
public:
    VM();
    void Init();
    static void LoadCallInstruction(Opcode type, QString arg, Allocator &allocator,
                                    Method *method, QString label,
                                    int extraInfo, CallStyle callStyle,
                                    Labeller &constantPoolLabeller);
    void Load(QString assemblyCode);

    void Register(QString symRef, ExternalMethod *method);
    void RegisterType(QString typeName, IClass *type);
    void DefineStringConstant(QString symRef, QString strValue);

    void registerProcessAdministrator(QString name);
    Value *GetType(QString symref);

    /*
      Creates & defines a (SmallVM) class which acts as a wrapper
      for a QObject. If the QObject's metaclass has already been
      created it will be reused. Then wraps the 'obj' parameter
      in an instance of that class & returns it.

      The translations map gives an Arabic name for the English
      property or method name. If the Map doesn't contain a name
      that means we don't want it in the wrapper, unless 'wrapAll'
      is true, then if there is no translation in the map the English
    */
    Value *wrapQObject(QObject *obj, QString newClassName,QMap<QString, QString> translations, bool wrapAll);
    Value *wrapQObject(IClass *cls, QObject *obj);
    IClass *classForQtClass(QString className);

    void ActivateEvent(QString evName, QVector<Value *> args);
    void RunStep(bool singleInstruction=false);
    void RunSingleInstruction(Process *process);
    Allocator &GetAllocator();
    void gc();

    Frame *launchProcess(Method *method);
    Frame *launchProcess(Method *method, Process *&proc);
    Frame *launchAdministeredProcess(Method *method, QString administrator);
    bool hasRunningInstruction();
    inline bool processIsFinished(Process *process)
    {
        if(process->stack.isEmpty())
        {
            return true;
        }

        //*
        Frame &frame = process->stack.top();
        if(frame.currentMethod == NULL)
        {
            return true;
        }
        if(!frame.currentMethod->HasInstruction(currentFrame()->ip))
        {
            return true;
        }
        //*/

        return false;
    }

    Instruction getCurrentInstruction();
    inline Process *currentProcess() { return running.front(); }
    void makeItSleep(Process *proc, int ms);
    bool hasRegisteredEventHandler(QString evName);
    void setDebugger(Debugger *);
    void clearAllBreakPoints();
    void setBreakPoint(QString methodName, int offset);
    void clearBreakPoint(QString methodName, int offset);

    // returns true if the process is in a valid running position
    bool getCodePos(Process *proc, QString &method, int &offset, Frame *&frame);

    void signal(VMErrorType toSignal);
    void signal(VMErrorType toSignal, QString arg0);
    void signal(VMErrorType toSignal, QString arg0, QString arg1);
    void signal(VMErrorType toSignal, QString arg0, QString arg1, QString arg2);

    void signalWithStack(VMError err);
    QString toStr(int);

    bool isRunning();
    bool isDone();
    void reactivate();
    Value *__top();

    void assert(bool cond, VMErrorType toSignal);
    void assert(bool cond, VMErrorType toSignal, QString arg0);
    void assert(bool cond, VMErrorType toSignal, QString arg0, QString arg1);
    void assert(bool cond, VMErrorType toSignal, IClass *arg0, IClass *arg1);
    void assert(bool cond, VMErrorType toSignal, QString arg0, QString arg1, QString arg2);

    QQueue<Process *> &getCallStacks();

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
    void DoPushParserBacktrack();
    void DoIgnoreParserBacktrack();

    void CallImpl(QString sym, int SymRefLabel, bool wantValueNotRef, int arity, CallStyle callStyle);
    void CallSpecialMethod(IMethod *method, QVector<Value *> args);
    void test(bool, QString, QString);
    bool coercion(Value *v1, Value *v2, Value *&newV1, Value *&newV2);
    Value *_div(Value *, Value *);
    void Pop_Md_Arr_and_indexes(MultiDimensionalArray<Value *> *&theArray, QVector<int> &indexes);

    VMError GetLastError();
private:

    void patchupInheritance(QMap<ValueClass *, QString> inheritanceList);
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
public:
    int popIntOrCoercedDouble();
    double popDoubleOrCoercedInt();

};

#endif // VM_H

