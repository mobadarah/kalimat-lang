#ifndef REFERENCES_H
#define REFERENCES_H

#include "value.h"
#include "multidimensionalarray.h"

struct Reference
{
    virtual void Set(Value *val) = 0;
    virtual Value *Get() = 0;
};

struct FieldReference : public Reference
{
    IObject *object;
    QString SymRef;

    FieldReference(IObject *_object, QString fieldName);
    void Set(Value *val);
    Value *Get();
};

struct ArrayReference : public Reference
{
    VArray *array;
    int index;

    void Set(Value *val);
    Value *Get();
    ArrayReference(VArray *arr, int ind);
};

struct MultiDimensionalArrayReference : public Reference
{
    MultiDimensionalArray<Value *> *array;
    QVector<int> index;

    void Set(Value *val);
    Value *Get();
};

#endif // REFERENCES_H
