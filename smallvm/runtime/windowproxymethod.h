#ifndef WINDOWPROXYMETHOD_H
#define WINDOWPROXYMETHOD_H

#ifndef VM_H
#include "../smallvm/vm.h"
#endif

#include "../externalmethod.h"

class RunWindow;
class VM;
typedef void (*VM_PROC)(Stack<Value *> &, Process *proc, RunWindow *, VM *);

class WindowProxyMethod : public ExternalMethod
{
    RunWindow *parent;
    VM_PROC proc;
    VM *vm;
public:
    WindowProxyMethod(RunWindow *parent, VM *vm, VM_PROC proc, bool mustRunInGui=true);
    void operator()(Stack<Value *> &operandStack, Process *process);
};

#endif // WINDOWPROXYMETHOD_H
