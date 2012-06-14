#include "spriteclass.h"
#include "../allocator.h"
#include "sprite.h"
SpriteClass::SpriteClass(QString name):
    EasyForeignClass(name)
{
    methodIds[_ws(L"خزن.القيمة")] =
            1;

    methodArities[_ws(L"خزن.القيمة")]
            = 2;

    methodIds[_ws(L"القيمة.المخزنة")] =
            2;

    methodArities[_ws(L"القيمة.المخزنة")]
            = 1;

    methodIds[_ws(L"امنع.التصادم")] =
            3;

    methodArities[_ws(L"امنع.التصادم")]
            = 1;

    methodIds[_ws(L"اسمح.بالتصادم")] =
            4;

    methodArities[_ws(L"اسمح.بالتصادم")]
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

Value *SpriteClass::dispatch(int id, QVector<Value *> args)
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
