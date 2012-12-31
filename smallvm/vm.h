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

#include "runtime_identifiers.h"
#include "utils.h"

#include "scheduler.h"
#include "runtime/vmrunnerthread.h"
#include <QQueue>
#include <QMutex>

class VM
{
    Labeller constantPoolLabeller;
    QHash<int, Value*> constantPool;

public:
    static Translation<ArgErr::ArgError> argumentErrors;
    Scheduler mainScheduler;
    Scheduler guiScheduler;
    VMRunthread *vmThread;

    // If two threads try to migrate processes to each other at the same time
    // we would have a deadlock since each process locks itself, then calls migrateTo()
    // which locks the other.
    // Therefore we will use a global mutext to ensure only one thread is migrating
    // a process of theirs at a time
    bool traceInstructions;
    QSemaphore gcCanStart;
    QSemaphore worldDestruction;
    bool destroyTheWorldFlag;
    Scheduler *destroyer;
private:

    // The allocator must be declared after the 'constantPool', 'stack', and 'mainScheduler'
    // members, since it's initialized with them in VMs constructor initializer list!!
    Allocator allocator;

    VMError _lastError;
    QMap<QString, Value *> _globalFrame;
    QMap<QString, QString> registeredEventHandlers;

private:
    QMap<QString, Value *> &globalFrame();

    friend class Process;
private:
    // Map Method name -> offset -> instruction
    // This will store the original instruction before we replace it with a 'break'
    QMap<QString, QMap<int, Instruction> > breakPoints;
    QMap<QString, QMap<int, bool> > _isBreakpointOneShot;
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
    IMethod *GetMethod(QString symRef);
    IClass *parseTypeId(QString typeId, int &pos);

    void ActivateEvent(QString evName, QVector<Value *> args);

    Allocator &GetAllocator();
    inline Process *getMainProcess() { return _mainProcess; }
    void gc();

    Frame *launchProcess(Method *method);
    Frame *launchProcess(Method *method, Process *&proc);
    Frame *launchProcessAsInterrupt(Method *method);

    bool hasRegisteredEventHandler(QString evName);
    void setDebugger(Debugger *);
    void clearAllBreakPoints();
    void setBreakPoint(QString methodName, int offset, bool oneShot=false);
    void clearBreakPoint(QString methodName, int offset);
    bool isBreakpointOneShot(QString methodName, int offset);

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

    VMError GetLastError();
private:

    void patchupInheritance(QMap<ValueClass *, QString> inheritanceList);

public:
    void stopTheWorld();
    void startTheWorld();
    void destroyTheWorld(Scheduler *owner);
};

#endif // VM_H

