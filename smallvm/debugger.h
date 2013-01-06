#ifndef DEBUGGER_H
#define DEBUGGER_H

class Frame;
class Process;

struct Debugger
{
    virtual bool currentBreakCondition(Frame *frame, Process *process) =0;
    virtual void postBreak(int offset, Frame *frame, Process *process)=0;
    virtual void Break(int offset, Frame *frame, Process *process) = 0;
    virtual void setDebuggedProcess(Process *) = 0;
};

struct NullaryDebugger : public Debugger
{
    virtual bool currentBreakCondition(Frame *frame, Process *process)
    {
        return false;
    }

    void postBreak(int offset, Frame *frame, Process *process) { }
    void Break(int offset, Frame *frame, Process *process) { }
    void setDebuggedProcess(Process *) { }
    static NullaryDebugger *instance();
};

#endif // DEBUGGER_H
