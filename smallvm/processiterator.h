#ifndef PROCESSITERATOR_H
#define PROCESSITERATOR_H

class Process;
struct ProcessIterator
{
    virtual bool hasMoreProcesses() = 0;
    virtual Process *getProcess() = 0;
};

#endif // PROCESSITERATOR_H
