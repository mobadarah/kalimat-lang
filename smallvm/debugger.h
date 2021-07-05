#ifndef DEBUGGER_H
#define DEBUGGER_H

class Frame;
class Process;

namespace BreakSource
{
    enum Src
    {
        FromInstruction,
        FromStepping
    };
}

struct Debugger
{
    virtual void currentBreakCondition(Process *process) =0;
    virtual void postBreak(BreakSource::Src source, int offset, Frame *frame, Process *process)=0;
    virtual void Break(BreakSource::Src source, int offset, Frame *frame, Process *process) = 0;
    virtual void setDebuggedProcess(Process *) = 0;
};

struct NullaryDebugger : public Debugger
{
    virtual void currentBreakCondition(Process *)
    {
        return;
    }

    void postBreak(BreakSource::Src source, int offset, Frame *frame, Process *process) { }
    void Break(BreakSource::Src source, int offset, Frame *frame, Process *process) { }
    void setDebuggedProcess(Process *) { }
    static NullaryDebugger *instance();
};

#endif // DEBUGGER_H
