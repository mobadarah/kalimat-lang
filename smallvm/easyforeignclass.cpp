#include "easyforeignclass.h"
#include "value.h"
#include "vm_ffi.h"

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

EasyForeignClass::EasyForeignClass(QString name)
    :ForeignClass(name)
{
}

bool EasyForeignClass::hasField(QString name)
{
    return fields.contains(name);
}

IMethod *EasyForeignClass::lookupMethod(QString name)
{
    if(!methodIds.contains(name))
        return NULL;
    return new ForeignMethodProxy(name, this, methodIds[name], methodArities[name]);
}
