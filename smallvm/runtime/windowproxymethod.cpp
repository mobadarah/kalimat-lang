#include "windowproxymethod.h"

WindowProxyMethod::WindowProxyMethod(RunWindow *parent, VM *vm, VM_PROC proc, bool mustRunInGui)
{
    this->vm = vm;
    this->parent = parent;
    this->proc = proc;
    this->mustRunInGui = mustRunInGui;
}

void WindowProxyMethod::operator ()(VOperandStack &operandStack, Process *process)
{
    proc(operandStack, process, parent, vm);
}
