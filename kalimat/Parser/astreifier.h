#ifndef ASTREIFIER_H
#define ASTREIFIER_H

#ifndef VALUE_H
    #include "KalimatAst/kalimatast_incl.h"
    #include "../smallvm/value.h"
    #include "../smallvm/allocator.h"
#endif

class ASTReifier
{
public:
    Value *reify(AST *t, Allocator *allocator);
};

#endif // ASTREIFIER_H
