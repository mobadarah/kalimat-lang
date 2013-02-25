#ifndef STEPSTOPCONDITION_H
#define STEPSTOPCONDITION_H

class Frame;
class Process;
class CodeDocument;
class MainWindow;
struct StepStopCondition
{
    virtual void stopNow(Process *process)=0;
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
    void stopNow(Process *);
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
    void stopNow(Process *);
};

struct NullaryStepStopCondition : public StepStopCondition
{
    void stopNow(Process *)
    {
    }
    static NullaryStepStopCondition *instance();
};

#endif // STEPSTOPCONDITION_H
