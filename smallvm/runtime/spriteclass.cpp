#include "spriteclass.h"
#include "../allocator.h"

SpriteClass::SpriteClass(QString name):
    EasyForeignClass(name)
{
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
    return NULL;
}
