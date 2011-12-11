#ifndef QOBJECTFOREIGNCLASS_H
#define QOBJECTFOREIGNCLASS_H

#ifndef VM_H
#include "vm.h"
#endif

class QObjectForeignClass : public EasyForeignClass
{
    VM *vm;
    const QMetaObject *qClass;
    bool wrapAll;
    QMap<QString, QString> translations;
    QMap<int, QMetaMethod> methodInvokations;
public:
    QObjectForeignClass(VM *vm,
                        QString className,
                        const QMetaObject *qClass,
                        QMap<QString, QString> translations,
                        bool wrapAll);
    virtual Value *dispatch(int id, QVector<Value *>args);
    virtual IObject *newValue(Allocator *allocator);
    virtual bool getFieldAttribute(QString attr, Value *&ret, Allocator *allocator) {return false;}
    virtual QVector<PropertyDesc> getProperties() { return properties; }
};

#endif // QOBJECTFOREIGNCLASS_H
