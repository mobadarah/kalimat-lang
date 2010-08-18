#include "scope.h"

Scope::Scope()
{
    parent = NULL;
}
Scope::Scope(Scope *parent)
{
    this->parent = parent;
}
Scope::define(QString name)
{
    bindings[name] = NULL;
}
Scope::define(QString name, void *value)
{
    bindings[name] = value;
}
Scope::contains(QString name)
{
    if(bindings.contains(name))
        return true;
    if(parent == NULL)
        return false;
    return parent->contains(name);
}
Scope::lookup(QString name)
{
    if(bindings.contains(name))
        return bindings[name];
    if(parent == NULL)
        return NULL;
    return parent->lookup(name);
}
Scope::define(QString name)
{
    return bindings.contains(name);
}
