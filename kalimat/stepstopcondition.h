#ifndef STEPSTOPCONDITION_H
#define STEPSTOPCONDITION_H

class Frame;
class Process;
class CodeDocument;
class MainWindow;
struct StepStopCondition
{
    virtual bool stopNow(Frame *frame, Process *process)=0;
};

struct SingleStepCondition : public StepStopCondition
{
    CodeDocument *doc;
    int startingLine;
    Frame *startingFrame;
    MainWindow *mw;
    SingleStepCondition(CodeDocument *doc, int line, Frame *startingFrame, MainWindow *mw)
        :doc(doc), startingLine(line), startingFrame(startingFrame), mw(mw)
    {

    }
    bool stopNow(Frame *frame, Process *);
};

struct StepOverCondition : public StepStopCondition
{
    CodeDocument *doc;
    int startingLine;
    Frame *startingFrame;
    MainWindow *mw;
    StepOverCondition(CodeDocument *doc, int line, Frame *startingFrame, MainWindow *mw)
        :doc(doc), startingLine(line), startingFrame(startingFrame), mw(mw)
    {

    }
    bool stopNow(Frame *frame, Process *);
};


#endif // STEPSTOPCONDITION_H
