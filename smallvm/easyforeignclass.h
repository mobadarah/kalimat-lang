#ifndef EASYFOREIGNCLASS_H
#define EASYFOREIGNCLASS_H

#ifndef CLASSES_H
    #include "classes.h"
#endif

class EasyForeignClass;

class ForeignMethodProxy : public IForeignMethod
{
    EasyForeignClass *owner;
    int arity;
    int id;
    QString name;
public:
    ForeignMethodProxy(QString name, EasyForeignClass *owner, int id, int arity);
    QString toString();
    Value *invoke(QVector<Value *> args);
    int Arity();
};

class EasyForeignClass : public ForeignClass
{
protected:
    QVector<PropertyDesc> properties;
    QSet<QString> fields;
    QMap<QString, int> methodIds;
    QMap<QString, int> methodArities;
public:
    EasyForeignClass(QString className);
    virtual Value *dispatch(int id, QVector<Value *>args) = 0;
    virtual IObject *newValue(Allocator *allocator) = 0;
    virtual bool getFieldAttribute(QString attr, Value *&ret, Allocator *allocator) {return false;}
    virtual QVector<PropertyDesc> getProperties() { return properties; }
    //IClass
    bool hasField(QString name);
    IMethod *lookupMethod(QString name);
};

#endif // EASYFOREIGNCLASS_H
