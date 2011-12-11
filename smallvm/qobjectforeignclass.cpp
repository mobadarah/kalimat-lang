#include "qobjectforeignclass.h"
#include "vm_ffi.h"
#include <QMetaMethod>
#include <QByteArray>

QString metaMethodName(QMetaMethod m)
{
    QString s = m.signature();
    // remove type declaration
    s = s.remove(0, s.indexOf(' ') + 1);
    //remove first open paren and everything after it
    s = s.left(s.indexOf('('));
    return s;
}

QObjectForeignClass::QObjectForeignClass(VM *vm,
                                         QString className,
                                         const QMetaObject *qClass,
                                         QMap<QString, QString> translations,
                                         bool wrapAll)
    :EasyForeignClass(className),
      vm(vm),
      qClass(qClass),
      wrapAll(wrapAll),
      translations(translations)

{
    int methodIdSeq = 0;
    for(int i=0; i< qClass->methodCount(); i++)
    {
        QMetaMethod m = qClass->method(i);
        QString methodName = metaMethodName(m);
        if(wrapAll || translations.contains(methodName))
        {
            if(translations.contains(methodName))
                methodName = translations[methodName];
            int id = methodIdSeq++;
            this->methodIds[methodName] = id;
            this->methodArities[methodName] = m.parameterNames().count() + 1;
            methodInvokations[id] = m;
        }

    }
}

IObject *QObjectForeignClass::newValue(Allocator *allocator)
{
    Object *newObj = new Object();
    newObj->slotNames.append("handle");
    return newObj;
}

QGenericArgument wrap(QByteArray typeName_, Value *val, VM *vm)
{
    void *qVal;
    QString typeName(typeName_);
    QMap<QString, ffi_type *> map;
    map["int"] = &ffi_type_sint;
    map["long"] = &ffi_type_sint;
    map["float"] = &ffi_type_sint;
    map["double"] = &ffi_type_sint;
    map["char"] = &ffi_type_sint;

    if(map.contains(typeName))
    {
        qVal = malloc(map[typeName]->size);
        kalimat_to_ffi_value(val->type, val,
                             map[typeName], qVal, vm);
        return QGenericArgument(typeName.toAscii(), qVal);
    }
    if(typeName == "QChar")
    {
        qVal = new QChar(val->unboxInt());
        return QGenericArgument(typeName.toAscii(), qVal);
    }
    if(typeName == "QString")
    {
        qVal = new QString(*val->unboxStr());
        return QGenericArgument(typeName.toAscii(), qVal);
    }
}

Value *unwrap(QGenericReturnArgument ret, VM *vm)
{
    if(strcmp(ret.name(), "int") == 0)
    {
        return vm->GetAllocator().newInt(*((int *)ret.data()));
    }
    if(strcmp(ret.name(), "long") == 0)
    {
        return vm->GetAllocator().newInt(*((long *)ret.data()));
    }
    if(strcmp(ret.name(), "float") == 0)
    {
        return vm->GetAllocator().newDouble(*((float *)ret.data()));
    }
    if(strcmp(ret.name(), "double") == 0)
    {
        return vm->GetAllocator().newDouble(*((double *)ret.data()));
    }
    if(strcmp(ret.name(), "char") == 0)
    {
        return vm->GetAllocator().newInt(*((char *)ret.data()));
    }
    if(strcmp(ret.name(), "QChar") == 0)
    {
        return vm->GetAllocator().newInt(((QChar *)ret.data())->unicode());
    }
    if(strcmp(ret.name(), "QString") == 0)
    {
        return vm->GetAllocator().newString(
                    ((QString*)ret.data()));
    }


}

Value *QObjectForeignClass::dispatch(int id, QVector<Value *> args)
{
    QMetaMethod m = methodInvokations[id];
    IObject *handle = args[0]->unboxObj();
    QObject *receiver = handle->getSlotValue("handle")->unboxQObj();

    QString sg = m.signature();
    void * ret = QMetaType::construct(QMetaType::type(m.typeName()));
    QGenericReturnArgument qRet(m.typeName(), ret);
    switch(m.parameterNames().count())
    {
    case 0:
        m.invoke(receiver, qRet);
        break;
    case 1:
        m.invoke(receiver, qRet,
                 wrap(m.parameterTypes()[0], args[1], vm));
        break;
    case 2:
        m.invoke(receiver, qRet,
                 wrap(m.parameterTypes()[0], args[1], vm),
                 wrap(m.parameterTypes()[1], args[2], vm));
        break;
    case 3:
        m.invoke(receiver, qRet,
                 wrap(m.parameterTypes()[0], args[1], vm),
                 wrap(m.parameterTypes()[1], args[2], vm),
                 wrap(m.parameterTypes()[2], args[3], vm));
        break;
    case 4:
        m.invoke(receiver, qRet,
                 wrap(m.parameterTypes()[0], args[1], vm),
                 wrap(m.parameterTypes()[1], args[2], vm),
                 wrap(m.parameterTypes()[2], args[3], vm),
                 wrap(m.parameterTypes()[3], args[4], vm));
        break;
    case 5:
        m.invoke(receiver, qRet,
                 wrap(m.parameterTypes()[0], args[1], vm),
                 wrap(m.parameterTypes()[1], args[2], vm),
                 wrap(m.parameterTypes()[2], args[3], vm),
                 wrap(m.parameterTypes()[3], args[4], vm),
                 wrap(m.parameterTypes()[4], args[5], vm));
        break;
    case 6:
        m.invoke(receiver, qRet,
                 wrap(m.parameterTypes()[0], args[1], vm),
                 wrap(m.parameterTypes()[1], args[2], vm),
                 wrap(m.parameterTypes()[2], args[3], vm),
                 wrap(m.parameterTypes()[3], args[4], vm),
                 wrap(m.parameterTypes()[4], args[5], vm),
                 wrap(m.parameterTypes()[5], args[6], vm));
        break;
    case 7:
        m.invoke(receiver, qRet,
                 wrap(m.parameterTypes()[0], args[1], vm),
                 wrap(m.parameterTypes()[1], args[2], vm),
                 wrap(m.parameterTypes()[2], args[3], vm),
                 wrap(m.parameterTypes()[3], args[4], vm),
                 wrap(m.parameterTypes()[4], args[5], vm),
                 wrap(m.parameterTypes()[5], args[6], vm),
                 wrap(m.parameterTypes()[6], args[7], vm));
        break;
    case 8:
        m.invoke(receiver, qRet,
                 wrap(m.parameterTypes()[0], args[1], vm),
                 wrap(m.parameterTypes()[1], args[2], vm),
                 wrap(m.parameterTypes()[2], args[3], vm),
                 wrap(m.parameterTypes()[3], args[4], vm),
                 wrap(m.parameterTypes()[4], args[5], vm),
                 wrap(m.parameterTypes()[5], args[6], vm),
                 wrap(m.parameterTypes()[6], args[7], vm),
                 wrap(m.parameterTypes()[7], args[8], vm));
        break;
    case 9:
        m.invoke(receiver, qRet,
                 wrap(m.parameterTypes()[0], args[1], vm),
                 wrap(m.parameterTypes()[1], args[2], vm),
                 wrap(m.parameterTypes()[2], args[3], vm),
                 wrap(m.parameterTypes()[3], args[4], vm),
                 wrap(m.parameterTypes()[4], args[5], vm),
                 wrap(m.parameterTypes()[5], args[6], vm),
                 wrap(m.parameterTypes()[6], args[7], vm),
                 wrap(m.parameterTypes()[7], args[8], vm),
                 wrap(m.parameterTypes()[8], args[9], vm));
        break;
    case 10:
        m.invoke(receiver, qRet,
                 wrap(m.parameterTypes()[0], args[1], vm),
                 wrap(m.parameterTypes()[1], args[2], vm),
                 wrap(m.parameterTypes()[2], args[3], vm),
                 wrap(m.parameterTypes()[3], args[4], vm),
                 wrap(m.parameterTypes()[4], args[5], vm),
                 wrap(m.parameterTypes()[5], args[6], vm),
                 wrap(m.parameterTypes()[6], args[7], vm),
                 wrap(m.parameterTypes()[7], args[8], vm),
                 wrap(m.parameterTypes()[8], args[9], vm),
                 wrap(m.parameterTypes()[9], args[10], vm));
        break;
    }
    return unwrap(qRet, vm);
}
