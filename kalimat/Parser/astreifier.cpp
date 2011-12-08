#include "astreifier.h"
#include "../../smallvm/utils.h"
template<typename T1, typename T2> bool isa(T2 *value)
{
    T1 *test = dynamic_cast<T1 *>(value);
    return test!=NULL;
}

Value *ASTReifier::reify(AST *t, Allocator *allocator)
{

}
