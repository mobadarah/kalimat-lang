#ifndef TREEUTILS_H
#define TREEUTILS_H

#include "Parser/KalimatAst/kalimatast.h"

template<class T> class TypedTraverser: public Traverser
{
    virtual void doVisit(shared_ptr<T> tree)=0;
    virtual void visit(shared_ptr<PrettyPrintable> tree)
    {
        shared_ptr<T> t = dynamic_pointer_cast<T>(tree);
        if(t)
        {
            doVisit(t);
        }
    }

    virtual void exit(shared_ptr<PrettyPrintable> tree) { }
};

#endif // TREEUTILS_H
