#ifndef METACLASS_H
#define METACLASS_H

#ifndef EASYFOREIGNCLASS_H
    #include "easyforeignclass.h"
#endif

class MetaClass : public EasyForeignClass
{
    Allocator *allocator;
public:
    MetaClass(QString name, Allocator *allocator);
    virtual IObject *newValue(Allocator *allocator);
    virtual Value *dispatch(Process *proc, int id, QVector<Value *>args);
    virtual bool getFieldAttribute(QString attr, Value *&ret, Allocator *allocator);
    virtual QVector<PropertyDesc> getProperties();
    // We need to set this before the VM starts
    // because a metaclass can be used to create new objects
    // and thus needs an allocator
    void setAllocator(Allocator *allocator);
};

#endif // METACLASS_H
