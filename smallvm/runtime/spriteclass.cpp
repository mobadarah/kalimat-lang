#include "spriteclass.h"
#include "../allocator.h"
#include "sprite.h"
#include "../runtime_identifiers.h"

SpriteClass::SpriteClass(QString name):
    EasyForeignClass(name)
{
    methodIds[VMId::get(RId::SetStoredValue)] =
            1;

    methodArities[VMId::get(RId::SetStoredValue)]
            = 2;

    methodIds[VMId::get(RId::GetStoredValue)] =
            2;

    methodArities[VMId::get(RId::GetStoredValue)]
            = 1;

    methodIds[VMId::get(RId::DisableCollision)] =
            3;

    methodArities[VMId::get(RId::DisableCollision)]
            = 1;

    methodIds[VMId::get(RId::EnableCollision)] =
            4;

    methodArities[VMId::get(RId::EnableCollision)]
            = 1;

    fields.insert("_handle");
    fields.insert("_storedValue");
}

IObject *SpriteClass::newValue(Allocator *allocator)
{
    Object *newObj = new Object();
    newObj->slotNames.append("_handle");
    newObj->slotNames.append("_storedValue");
    newObj->setSlotValue("_storedValue", allocator->null());
    return newObj;
}

Value *SpriteClass::dispatch(Process *proc, int id, QVector<Value *> args)
{
    IObject *receiver = args[0]->unboxObj();
    Sprite *sprite = reinterpret_cast<Sprite *>(
                receiver->getSlotValue("_handle")->unboxRaw());
    Value *arg1 = NULL;
    switch(id)
    {
    case 1: // خزن.القيمة
        arg1 = args[1];
        receiver->setSlotValue("_storedValue", arg1);
        return NULL;
    case 2: // القيمة.المخزنة
        return receiver->getSlotValue("_storedValue");
    case 3: // امنع.التصادم
        sprite->enableColission(false);
        return NULL;
    case 4: // اسمح.بالتصادم
        sprite->enableColission(true);
        return NULL;

    }

    return NULL;
}
