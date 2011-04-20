#ifndef PROCESS_H
#define PROCESS_H

#ifndef FRAME_H
    #include "frame.h"
#endif

#include <QStack>

enum ProcessState
{
    SleepingProcess,
    AwakeProcess
};

struct Process
{
    Process();
    ProcessState state;
    QStack<Frame> stack;
    void awaken();
    void sleep();
};

#endif // PROCESS_H
