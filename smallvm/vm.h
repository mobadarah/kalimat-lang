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
#include <QMutex>

class VM
{
    Labeller constantPoolLabeller;
    QHash<int, Value*> constantPool;

    QQueue<Process *> running;
    QQueue<Process *> sleeping;
    QQueue<Process *> timerWaiting;
    QQueue<Process *> newProcesses;
    QQueue<Process *> guiProcesses; // Schedule processes that need to interact with Qt
                                    // to run in the GUI thread. See RunWindow::Run() and VM::RunStep()

public:
    Process *runningNow;
    QQueue<Process *> &getGUIProcesses() { return guiProcesses;}
    static Translation<ArgErr::ArgError> argumentErrors;
private:
    // The allocator must be declared after the 'constantPool' and 'stack'
    // members, since it's initialized with them in VMs constructor initializer list!!
    Allocator allocator;

    bool _isRunning;
    VMError _lastError;
    Frame *_globalFrame;
    QMap<QString, QString> registeredEventHandlers;
private:
    // QStack<Frame> &stack();
public: // todo:temp
    /*inline Frame *currentFrame()
    {
        if(stack().empty())
            return NULL;
        return &stack().top();
    }
    */
private:
    Frame &globalFrame();

    friend class Process;
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

    void SetGlobal(QString symRef, Value *v);
    Value *GetGlobal(QString symRef);

    Value *GetType(QString symref);

    void ActivateEvent(QString evName, QVector<Value *> args);
    bool schedule();
    void RunStep(bool singleInstruction=false);
    void finishUpRunningProcess();
    Allocator &GetAllocator();
    void gc();

    Frame *launchProcess(Method *method);
    Frame *launchProcess(Method *method, Process *&proc);
    Frame *launchProcessAsInterrupt(Method *method);
    void awaken(Process *proc);
    void sleepify(Process *proc);

    inline Process *currentProcess() { return running.front(); }
    void makeItSleep(Process *proc, int ms);
    bool hasRegisteredEventHandler(QString evName);
    bool hasInterrupts();
    void setDebugger(Debugger *);
    void clearAllBreakPoints();
    void setBreakPoint(QString methodName, int offset);
    void clearBreakPoint(QString methodName, int offset);

    // returns true if the process is in a valid running position
    bool getCodePos(Process *proc, QString &method, int &offset, Frame *&frame);

    void signal(Process *proc, VMErrorType toSignal);
    void signal(Process *proc, VMErrorType toSignal, QString arg0);
    void signal(Process *proc, VMErrorType toSignal, QString arg0, QString arg1);
    void signal(Process *proc, VMErrorType toSignal, QString arg0, QString arg1, QString arg2);

    void signalWithStack(Process *proc, VMError err);
    QString toStr(int);

    bool isRunning();
    bool isDone();
    void reactivate();
    Value *__top();

    void assert(Process *proc, bool cond, VMErrorType toSignal);
    void assert(Process *proc, bool cond, VMErrorType toSignal, QString arg0);
    void assert(Process *proc, bool cond, VMErrorType toSignal, QString arg0, QString arg1);
    void assert(Process *proc, bool cond, VMErrorType toSignal, IClass *arg0, IClass *arg1);
    void assert(Process *proc, bool cond, VMErrorType toSignal, QString arg0, QString arg1, QString arg2);

    QSet<QQueue<Process *> *> getCallStacks();



    VMError GetLastError();
private:

    void patchupInheritance(QMap<ValueClass *, QString> inheritanceList);

public:

};

#endif // VM_H

