#include "vm_ffi.h"

#include <stdint.h>
#include <QDebug>

/* Compute the member offsets in a struct relative to the given base
   pointer. */
// Taken from source of the Pure programming language
// code http://code.google.com/p/pure-lang
static void offsets(void *data, unsigned n, ffi_type **types, QVector<void *> &offsets)
{
  size_t ofs = 0;
  unsigned i;
  for (i = 0; i < n; i++)
  {
    unsigned short a = ofs % types[i]->alignment;
    if (a != 0) ofs += types[i]->alignment-a;
    offsets.append((char *) data+ofs);
    ofs += types[i]->size;
  }
}


void makeStructFromKalimatClass(IClass *kalimatType, ffi_type *&type, VM *vm)
{
    static QMap<IClass *, ffi_type *> internedTypes;

    if(internedTypes.contains(kalimatType))
    {
        type= internedTypes.value(kalimatType);
        return;
    }

    ValueClass *vc = (ValueClass *) kalimatType;
    QVector<ffi_type *> fieldCTypes;
    for(int i=0; i<vc->fieldNames.count(); i++)
    {
        Value *fieldMarshallingType;
        if(vc->getFieldAttribute("marshalas", fieldMarshallingType, NULL))
        {
            QString str = fieldMarshallingType->unboxStr();
            if(vm->GetType(str) == NULL)
                vm->signal(NULL, InternalError1, QString("Marshalling type '%1' does not exist").arg(str));

            IClass *fieldClass = (IClass *) vm->GetType(str)->unboxObj();
            ffi_type *fieldFfiType;
            kalimat_to_ffi_type(fieldClass, fieldFfiType, vm);
            fieldCTypes.append(fieldFfiType);
        }

    } // for
    type = new ffi_type;
    type->size = 0;
    type->alignment = 0;
    type->type = FFI_TYPE_STRUCT;

    ffi_type **type_elements = new ffi_type*[fieldCTypes.count()+1];
    for(int i=0; i<fieldCTypes.count(); i++)
    {
        type_elements[i] = fieldCTypes[i];
    }
    type_elements[fieldCTypes.count()] = NULL;
    type->elements = type_elements;
    /* Type information hasn't been filled in yet; do a dummy call to
          ffi_prep_cif to do that now. */
    ffi_cif cif;
    if (ffi_prep_cif(&cif, FFI_DEFAULT_ABI, 0, type, 0) != FFI_OK)
        vm->signal(NULL, InternalError1, QString("ffi_cif failed to make a struct"));
    internedTypes[kalimatType] = type;
}

void ffi_callback_dispatcher(ffi_cif *cif, void *ret, void** args,
                      void *data)
{
    // to return something use *ret = ...;
    ClosureData *cdata = (ClosureData *) data;
    qDebug() << "Callback invoked: " << (long) cdata->funcObj << ".";
    Method *method = dynamic_cast<Method *>(cdata->funcObj->unboxObj());
    if(method)
    {
        cdata->proc->CallImpl(method, true, cdata->funcClass->argTypes.count(), NormalCall);
        cdata->proc->RunUntilReturn();
    }
    else
    {
        Object *obj = dynamic_cast<Object *>(cdata->funcObj->unboxObj());
        if(!obj)
            return;
        IMethod *m= cdata->funcObj->type->lookupMethod(QString::fromStdWString(L"تنفيذها"));
        if(!m)
            return;
        method = dynamic_cast<Method *>(m);
        if(!method)
            return;

        for(int i=0; i<cdata->funcClass->argTypes.count(); ++i)
        {
            Value *v;
            toKalimatType(cdata->funcClass->argTypes[i], cif->arg_types[i],
                          v, args[i], cdata->vm);
            cdata->proc->stack->OperandStack.push(v);
        }
        cdata->proc->stack->OperandStack.push(cdata->funcObj);
        cdata->proc->CallImpl(method, true, cdata->funcClass->argTypes.count()+1, NormalCall);
        cdata->proc->RunUntilReturn();
        if(cif->rtype != &ffi_type_void)
        {
            if(cdata->proc->stack->OperandStack.empty())
            {
                throw VMError(InternalError1).arg("FFI callback did not return a value");
            }
            Value *v = cdata->proc->stack->OperandStack.pop();
            kalimat_to_ffi_value(cdata->funcClass->retType, v,
                                 cif->rtype, ret, cdata->proc, cdata->vm);
        }
    }

}


void makeClosureFromKalimatClass(IClass *kalimatType,
                                 ffi_type *&type,
                                 Value *funcObj,
                                 void *&funcPtr,
                                 Process *proc,
                                 VM *vm)
{
    FunctionClass *funcClass = (FunctionClass *) kalimatType;
    void *closureCode;
    ffi_closure *closureObj = (ffi_closure *) ffi_closure_alloc(sizeof(ffi_closure), &closureCode);
    if(!closureObj)
        throw VMError(InternalError1).arg(QString("Cannot allocate closure for type: %1").arg(kalimatType->toString()));

    ffi_cif *cif = new ffi_cif;
    ffi_type **argTypes;
    ffi_type *retType;
    ffi_status success = PrepareCif(cif,argTypes, retType,vm,
                                    funcClass->retType, funcClass->argTypes);
    if(success != FFI_OK)
        throw VMError(InternalError1).arg(QString("Cannot prepare CIF for closure for type: %1").arg(kalimatType->toString()));
    success = ffi_prep_closure_loc(closureObj, cif, ffi_callback_dispatcher,
                                   new ClosureData(proc, vm, funcObj, (FunctionClass *) kalimatType), closureCode);
    if(success != FFI_OK)
        throw VMError(InternalError1).arg(QString("Cannot initialize closure for type: %1").arg(kalimatType->toString()));
    funcPtr = closureCode;
}

void kalimat_to_ffi_type(IClass *kalimatType, ffi_type *&type, VM *vm)
{
    if(kalimatType == BuiltInTypes::c_int)
    {
        type = &ffi_type_sint32;
    }
    else if(kalimatType == BuiltInTypes::c_long)
    {
        type = &ffi_type_slong;
    }
    else if(kalimatType == BuiltInTypes::c_float)
    {
        type = &ffi_type_float;
    }
    else if(kalimatType == BuiltInTypes::c_double)
    {
        type = &ffi_type_double;
    }
    else if(kalimatType == BuiltInTypes::c_char)
    {
        type = &ffi_type_schar;
    }
    else if(kalimatType == BuiltInTypes::c_asciiz)
    {
        type =&ffi_type_pointer;
    }
    else if(kalimatType == BuiltInTypes::c_wstr)
    {
        type =&ffi_type_pointer;
    }
    else if(kalimatType == BuiltInTypes::c_ptr)
    {
        type =&ffi_type_pointer;
    }
    else if(kalimatType == BuiltInTypes::c_void)
    {
        type = &ffi_type_void;
    }
    else if(dynamic_cast<PointerClass *>(kalimatType) != NULL)
    {
        type = &ffi_type_pointer;
    }
    else if(dynamic_cast<FunctionClass*>(kalimatType) != NULL)
    {
        type = &ffi_type_pointer;
    }
    else if(dynamic_cast<ValueClass *>(kalimatType) != NULL)
    {
        makeStructFromKalimatClass(kalimatType, type, vm);
    }
    else
    {
        type = NULL;
    }
}

void kalimat_to_ffi_value(IClass *kalimatType, Value *v, ffi_type *type, void *&value, Process *proc, VM *vm)
{
    if(type == &ffi_type_sint32)
    {
        *((int32_t *)value) = (int32_t) v->unboxNumeric();
    }
    else if(type == &ffi_type_slong)
    {
        *((long *)value) = (long) v->unboxNumeric();
    }
    else if(type == &ffi_type_float)
    {
        *((float *)value) = (float) v->unboxNumeric();
    }
    else if(type == &ffi_type_double)
    {
        *((double *)value) = (double) v->unboxNumeric();
    }
    else if(type == &ffi_type_schar)
    {
        *((char *)value) = 'a';
    }
    else if(type == &ffi_type_pointer)
    {
        if(kalimatType == BuiltInTypes::c_asciiz)
        {
            std::string str_c = v->unboxStr().toStdString();
            char *str = new char[str_c.length()+1];
            for(unsigned int i=0;i<str_c.length(); i++)
            {
                str[i] = str_c[i];
            }
            str[str_c.length()] = 0;

            *((char **) value) = str;
        }
        else if(kalimatType == BuiltInTypes::c_wstr)
        {
            std::wstring str_c = v->unboxStr().toStdWString();
            wchar_t *str = new wchar_t[str_c.length()+1];
            for(unsigned int i=0;i<str_c.length(); i++)
            {
                str[i] = str_c[i];
            }
            str[str_c.length()] = 0;
            *((wchar_t **) value) = str;
        }
        else if(kalimatType == BuiltInTypes::c_ptr)
        {
            *((void **)value) = (void *) v->unboxInt();
        }
        else if(dynamic_cast<FunctionClass *>(kalimatType) != NULL)
        {
            void *cval;
            makeClosureFromKalimatClass(kalimatType, type, v, cval, proc, vm);
            *((void **)value) = cval;
        }
        else if(dynamic_cast<PointerClass *>(kalimatType) != NULL)
        {
            void *ptr = v->unboxRaw();
            *((void **) value) = ptr;
        }
    }
    else if(type->type ==FFI_TYPE_STRUCT && dynamic_cast<ValueClass *>(kalimatType) != NULL)
    {
        ValueClass *vc = (ValueClass *) kalimatType;
        IObject *obj = v->unboxObj();
        QVector<IClass *> fieldKalimatTypes;
        QVector<Value *> fieldKalimatValues;
        for(int i=0; i<vc->fieldNames.count(); i++)
        {
            Value *fieldMarshallingType;
            if(vc->getFieldAttribute("marshalas", fieldMarshallingType, NULL))
            {
                QString str = fieldMarshallingType->unboxStr();
                if(vm->GetType(str) == NULL)
                    vm->signal(NULL, InternalError1, QString("Marshalling type '%1' does not exist").arg(str));

                IClass *fieldClass = (IClass *) vm->GetType(str)->unboxObj();

                fieldKalimatTypes.append(fieldClass);
                fieldKalimatValues.append(obj->getSlotValue(vc->fieldNames[i]));
            }

        } // for

        QVector<void *> offs;
        void *struct_val = value;
        offsets(struct_val, fieldKalimatTypes.count(), type->elements, offs);
        for(int i=0; i<fieldKalimatTypes.count(); i++)
        {
            kalimat_to_ffi_value(fieldKalimatTypes[i], fieldKalimatValues[i], type->elements[i], offs[i], proc, vm);
        }
    }
}

void ffi_free(IClass *kalimatType, ffi_type *type, void *&value)
{
    if(type == &ffi_type_sint32)
    {
        free(value);
    }
    else if(type == &ffi_type_slong)
    {
        free(value);
    }
    else if(type == &ffi_type_float)
    {
        free(value);
    }
    else if(type == &ffi_type_double)
    {
        free(value);
    }
    else if(type == &ffi_type_schar)
    {
        free(value);
    }
    else if(type == &ffi_type_pointer)
    {
        if(kalimatType == BuiltInTypes::c_asciiz)
        {
            delete[] *((char **) value);
            free(value);
        }
        else if(kalimatType == BuiltInTypes::c_wstr)
        {
            delete[] *((wchar_t **) value);
            free(value);
        }
        else if(kalimatType == BuiltInTypes::c_ptr)
        {

        }
        else if(dynamic_cast<FunctionClass *>(kalimatType) != NULL)
        {
            // todo: closures
        }
        else if(dynamic_cast<PointerClass *>(kalimatType) != NULL)
        {
            // todo: raw?
        }
    }
    else if(type->type ==FFI_TYPE_STRUCT && dynamic_cast<ValueClass *>(kalimatType) != NULL)
    {
        // todo: struct
    }
}

void toKalimatType(IClass *kalimatType, ffi_type *type, Value *&value, void *v, VM *vm)
{
    Allocator *allocator = &(vm->GetAllocator());
    if(kalimatType == BuiltInTypes::c_int)
    {
        value = allocator->newInt(*((uint32_t *)v));
    }
    else if(kalimatType == BuiltInTypes::c_long)
    {
        value = allocator->newInt(*((ulong *)v));
    }
    else if(kalimatType == BuiltInTypes::c_float)
    {
        value = allocator->newDouble(*((float *)v));
    }
    else if(kalimatType == BuiltInTypes::c_double)
    {
        value = allocator->newDouble(*((double *)v));
    }
    else if(kalimatType == BuiltInTypes::c_char)
    {
        char x[2];
        x[0] = *((char *) v);
        x[1] = '\0';
        QString s = QString("%1").arg(x);
        value = allocator->newString(s);
    }
    else if(kalimatType == BuiltInTypes::c_asciiz)
    {
        char *str = *((char **) v);
        QString s = QString("%1").arg(str);
        value = allocator->newString(s);
    }
    else if(kalimatType == BuiltInTypes::c_wstr)
    {
        wchar_t *str = *((wchar_t **) v);
        QString s = QString::fromWCharArray(str);
        value = allocator->newString(s);
    }
    else if(kalimatType == BuiltInTypes::c_ptr)
    {
        value = allocator->newInt(*((uint32_t *)v));
    }
    else if(dynamic_cast<PointerClass *>(kalimatType) != NULL)
    {
        PointerClass *pc = dynamic_cast<PointerClass *>(kalimatType);
        void * pointee = *((void **) v);
        ffi_type *pointee_c_type;
        kalimat_to_ffi_type(pc->pointee, pointee_c_type, vm);
        toKalimatType(pc->pointee, pointee_c_type, value, pointee, vm);
    }
    else if(dynamic_cast<ValueClass *>(kalimatType) && type->type == FFI_TYPE_STRUCT)
    {
        ValueClass *vc = (ValueClass *) kalimatType;
        IObject *obj = vc->newValue(allocator);
        QVector<IClass *> fieldKalimatTypes;
        QVector<QString> fieldKalimatNames;
        for(int i=0; i<vc->fieldNames.count(); i++)
        {
            Value *fieldMarshallingType;
            if(vc->getFieldAttribute("marshalas", fieldMarshallingType, NULL))
            {
                QString str = fieldMarshallingType->unboxStr();
                if(vm->GetType(str) == NULL)
                    vm->signal(NULL, InternalError1, QString("Marshalling type '%1' does not exist").arg(str));

                IClass *fieldClass = (IClass *) vm->GetType(str)->unboxObj();

                fieldKalimatTypes.append(fieldClass);
                fieldKalimatNames.append(vc->fieldNames[i]);
            }

        } // for

        QVector<void *> offs;
        void *struct_val = v;
        offsets(struct_val, fieldKalimatTypes.count(), type->elements, offs);
        for(int i=0; i<fieldKalimatTypes.count(); i++)
        {
            Value *val;
            toKalimatType(fieldKalimatTypes[i], type->elements[i], val, offs[i], vm);
            obj->setSlotValue(fieldKalimatNames[i], val);
        }
        value = allocator->newObject(obj, vc);
    }
}

void default_C_Type_Of(IClass *kalimatType, IClass *&c_KalimatType)
{
    if(kalimatType == BuiltInTypes::IntType)
    {
        c_KalimatType = BuiltInTypes::c_int;
    }
    else if(kalimatType == BuiltInTypes::DoubleType)
    {
        c_KalimatType = BuiltInTypes::c_double;
    }
    else if(kalimatType == BuiltInTypes::LongType)
    {
        c_KalimatType = BuiltInTypes::c_long;
    }
    else
    {
        c_KalimatType = kalimatType;
    }
}

void guessType(Value *v, ffi_type *&type, void *&ret)
{
    if(v->type->subclassOf(BuiltInTypes::IntType))
    {
        type = &ffi_type_uint32;
        ret = new uint32_t(v->unboxInt());
    }
    else if(v->type->subclassOf(BuiltInTypes::StringType))
    {
        type = &ffi_type_pointer;
        ret = (void *) v->unboxStr().toStdString().c_str();
    }
}

ffi_status PrepareCif(ffi_cif *cif, ffi_type **&ffi_argTypes, ffi_type *&c_retType, VM *vm,
                      IClass *retType, QVector<IClass *> argTypes)
{
    int n = argTypes.count();
    ffi_argTypes = new ffi_type*[n];
    c_retType = NULL;
    for(int i=0; i<n; i++)
    {
        kalimat_to_ffi_type(argTypes[i], ffi_argTypes[i], vm);
    }
    kalimat_to_ffi_type(retType, c_retType, vm);

    return ffi_prep_cif(cif, FFI_DEFAULT_ABI, n,
                        c_retType, ffi_argTypes);
}

Value *CallForeign(void *funcPtr, QVector<Value *> argz, IClass *retType, QVector<IClass *> argTypes, bool _guessTypes,
                   Process *proc, VM *vm)
{
    //*
    int n = argz.count();
    bool autoConvert = _guessTypes;

    ffi_cif cif;
    ffi_type **ffi_argTypes = new ffi_type*[n];
    ffi_type *c_retType = NULL;
    void **values = new void*[n];

    //Initialize the argument info vectors
    for(int i=0; i<n; i++)
    {
        if(autoConvert)
        {
            guessType(argz[i], ffi_argTypes[i], values[i]);
        }
        else
        {
            //mapType(argTypes[i], argz[i], ffi_argTypes[i], values[i], vm);
            kalimat_to_ffi_type(argTypes[i], ffi_argTypes[i], vm);
            void *ptr = malloc(ffi_argTypes[i]->size);
            kalimat_to_ffi_value(argTypes[i], argz[i], ffi_argTypes[i], ptr, proc, vm);
            values[i] = ptr;
        }
    }

    kalimat_to_ffi_type(retType, c_retType, vm);
    void *retVal = malloc(c_retType->size);

    // Initialize the cif
    if (ffi_prep_cif(&cif, FFI_DEFAULT_ABI, n,
            c_retType, ffi_argTypes) == FFI_OK)
    {
      ffi_call(&cif, (void (*)()) funcPtr, retVal , values);
    }
    //*/
    Value *ret = NULL;
    if(c_retType != &ffi_type_void)
    {
        toKalimatType(retType, c_retType, ret, retVal, vm);
    }

    // free the arguments
    for(int i=0; i<argz.count(); ++i)
    {
        ffi_free(argTypes[i], ffi_argTypes[i], values[i]);
    }

    return ret;
}
