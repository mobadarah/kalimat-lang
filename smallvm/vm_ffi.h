#ifndef VM_FFI_H
#define VM_FFI_H

#ifndef VM_H
#include "vm_incl.h"
#include "vm.h"
#endif

Value *CallForeign(void *funcPtr, QVector<Value *> argz, IClass *retType, QVector<IClass *> argTypes, bool guessTypes, VM *vm);

#endif // VM_FFI_H
