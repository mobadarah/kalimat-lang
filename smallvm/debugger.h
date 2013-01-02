#ifndef DEBUGGER_H
#define DEBUGGER_H

#include <QString>
#ifndef FRAME_H
    #include "frame.h"
#endif

struct Debugger
{
    virtual bool currentBreakCondition(Frame *frame, Process *process) =0;
    virtual void postBreak(int offset, Frame *frame, Process *process)=0;
    virtual void Break(int offset, Frame *frame, Process *process) = 0;
    virtual void setDebuggedProcess(Process *) = 0;

};

#endif // DEBUGGER_H
