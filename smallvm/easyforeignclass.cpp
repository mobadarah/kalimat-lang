#include "easyforeignclass.h"
#include "value.h"
#include "vm_ffi.h"
#include "vm.h"

ForeignMethodProxy::ForeignMethodProxy(QString name, EasyForeignClass *owner, int id, int arity)
{
    this->name = name;
    this->owner = owner;
    this->id = id;
    this->arity = arity;
}

QString ForeignMethodProxy::toString()
{
    return name;
}

Value *ForeignMethodProxy::invoke(Process *proc, QVector<Value *> args)
{
    return owner->dispatch(proc, id, args);
}

int ForeignMethodProxy::Arity()
{
    return arity;
}

EasyForeignClass::EasyForeignClass(QString name, VM *vm)
    :ForeignClass(name), vm(vm)
{
}

EasyForeignClass::EasyForeignClass(QString name)
    :ForeignClass(name), vm(NULL)
{
}

bool EasyForeignClass::hasField(QString name)
{
    return fields.contains(name);
}

void EasyForeignClass::attachVmMethod(QString className, QString methodName)
{
    QString internalName = QString("%%1_%2").arg(className).arg(methodName);
    IMethod *method = vm->GetMethod(internalName);
    vmMethods[methodName] = method;
}

void EasyForeignClass::attachVmMethod(QString methodName)
{
    attachVmMethod(this->getName(), methodName);
}

IMethod *EasyForeignClass::lookupMethod(QString name)
{
    if(methodIds.contains(name))
    {
        return new ForeignMethodProxy(name, this, methodIds[name], methodArities[name]);
    }
    else
    {
        return vmMethods.value(name, NULL);
    }
}
