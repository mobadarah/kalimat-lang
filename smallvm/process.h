#ifndef PROCESS_H
#define PROCESS_H

#ifndef FRAME_H
    #include "frame.h"
#endif

#include <QStack>

struct Process
{
    QStack<Frame> stack;
};

#endif // PROCESS_H
