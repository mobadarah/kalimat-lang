#ifndef VMCLIENT_H
#define VMCLIENT_H

#include "../debugger.h"
class RunWindow;
class VM;

#include "../vmerror.h"

class VMClient : public Debugger
{
public:
    virtual bool isWonderfulMonitorEnabled() = 0;
    virtual int wonderfulMonitorDelay() = 0;
    virtual void programStopped(RunWindow *) = 0;

    virtual void markCurrentInstruction(VM *vm, Process *proc, int *pos, int *length)=0;
    virtual void postMarkCurrentInstruction(VM *vm, Process *proc, int *pos, int *length)=0;
    virtual void handleVMError(VMError err) = 0;
};

class NullaryVMClient : public VMClient
{
public:
    bool isWonderfulMonitorEnabled() { return false; }
    int wonderfulMonitorDelay() { return 0; }
    void programStopped(RunWindow *) { }

    void postMarkCurrentInstruction(VM *, Process *, int *, int *) { }
    void markCurrentInstruction(VM *, Process *, int *, int *) { }
    void handleVMError(VMError err) { }

    void Break(int offset, Frame *frame, Process *process) { }
    void postBreak(int offset, Frame *frame, Process *process) { }
    void setDebuggedProcess(Process *) { }
    bool currentBreakCondition(int offset, Frame *frame, Process *process)
    {
        return false;
    }
};


#endif // VMCLIENT_H
