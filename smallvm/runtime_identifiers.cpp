#include "runtime_identifiers.h"


Translation<RId::RuntimeId> *VMId::runtime = NULL;

void VMId::init()
{
    if(!runtime)
        runtime = new Translation<RId::RuntimeId>(":/runlib_identifiers.txt");
}

QString VMId::get(RId::RuntimeId id)
{
    if(!runtime)
        init();
    return runtime->operator [](id);
}

QString VMId::get(RId::RuntimeId id, QString arg0)
{
    if(!runtime)
        init();
    return runtime->get(id, arg0);
}
