#ifndef SPRITECLASS_H
#define SPRITECLASS_H

#include "../easyforeignclass.h"

class SpriteClass : public EasyForeignClass
{
public:
    SpriteClass(QString name);
    virtual Value *dispatch(Process *proc, int id, QVector<Value *>args);
    virtual IObject *newValue(Allocator *allocator);
};

#endif // SPRITECLASS_H
