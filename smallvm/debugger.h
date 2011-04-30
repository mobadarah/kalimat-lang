#ifndef DEBUGGER_H
#define DEBUGGER_H

#include <QString>
#ifndef FRAME_H
    #include "frame.h"
#endif

struct Debugger
{
    virtual void Break(QString methodName, int offset, Frame frame) = 0;
};

#endif // DEBUGGER_H
