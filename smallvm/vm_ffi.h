#ifndef VM_FFI_H
#define VM_FFI_H

#ifndef VM_H
#include "vm_incl.h"
#include "vm.h"
#endif
#include "libffi/include/ffi.h"

struct ClosureData
{
    Process *proc;
    VM *vm;
    Value *funcObj;
    IClass *funcObjType;
    FunctionClass *funcClass;

    ClosureData(Process *proc,
                VM *vm,
                Value *funcObj,
                FunctionClass *funcClass)
        :proc(proc), vm(vm), funcObj(funcObj), funcObjType(funcObjType), funcClass(funcClass)
    {

    }
};

void kalimat_to_ffi_type(IClass *kalimatType, ffi_type *&type, VM *vm);
void kalimat_to_ffi_value(IClass *kalimatType, Value *v, ffi_type *type, void *&value, Process *proc, VM *vm);
void default_C_Type_Of(IClass *kalimatType, IClass *&c_KalimatType);
void toKalimatType(IClass *kalimatType, ffi_type *type, Value *&value, void *v, VM *vm);

ffi_status PrepareCif(ffi_cif *cif, ffi_type **&ffi_argTypes, ffi_type *&c_retType, VM *vm, IClass *retType, QVector<IClass *> argTypes);

Value *CallForeign(void *funcPtr, QVector<Value *> argz, IClass *retType, QVector<IClass *> argTypes, bool guessTypes, Process *proc, VM *vm);

#endif // VM_FFI_H
