#ifndef ASTREIFIER_H
#define ASTREIFIER_H

#ifndef VALUE_H
    #include "kalimatast.h"
    #include "../smallvm/value.h"
    #include "../smallvm/allocator.h"
#endif

class ASTReifier
{
public:
    Value *reify(AST *t, Allocator *allocator);
};

#endif // ASTREIFIER_H
