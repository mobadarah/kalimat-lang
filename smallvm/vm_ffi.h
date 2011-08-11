#ifndef VM_FFI_H
#define VM_FFI_H

#ifndef VM_H
#include "vm_incl.h"
#include "vm.h"
#endif
#include "libffi/include/ffi.h"

void kalimat_to_ffi_type(IClass *kalimatType, ffi_type *&type, VM *vm);
void kalimat_to_ffi_value(IClass *kalimatType, Value *v, ffi_type *type, void *&value, VM *vm);

Value *CallForeign(void *funcPtr, QVector<Value *> argz, IClass *retType, QVector<IClass *> argTypes, bool guessTypes, VM *vm);

#endif // VM_FFI_H
