#include "easyforeignclass.h"

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

void ForeignMethodProxy::invoke(QVector<Value *> args)
{
    owner->dispatch(id, args);
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
