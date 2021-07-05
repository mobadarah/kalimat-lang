#include "debugger.h"

#include <cstddef>

NullaryDebugger *NullaryDebugger::instance()
{
    static NullaryDebugger *inst = NULL;
    if(inst == NULL)
        inst = new NullaryDebugger();
    return inst;
}

