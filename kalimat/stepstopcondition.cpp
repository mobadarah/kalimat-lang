#include "stepstopcondition.h"
#include "mainwindow.h"

bool SingleStepCondition::stopNow(int offset, Frame *frame, Process *proc)
{
    if(proc->isFinished())
        return false;
    const Instruction &i = frame->currentMethod->Get(offset);
    if(mw->PositionInfo.contains(i.extra))
    {
        CodePosition &pos = mw->PositionInfo[i.extra];
        if(pos.ast->getPos().Line != startingLine
                || pos.doc != this->doc
                || frame != this->startingFrame)
        {
            return true;
        }
        else
        {
            return false;
        }

    }
    else
    {
        return false;
    }
}

bool StepOverCondition::stopNow(int offset, Frame *frame, Process *)
{
    const Instruction &i = frame->currentMethod->Get(offset);
    if(mw->PositionInfo.contains(i.extra))
    {
        CodePosition &pos = mw->PositionInfo[i.extra];
        if(pos.ast->getPos().Line != startingLine
                && pos.doc == this->doc
                && frame == this->startingFrame)
        {
            return true;
        }
        else
        {
            return false;
        }

    }
    else
    {
        return false;
    }
}
