#ifndef PROCESS_H
#define PROCESS_H

#ifndef FRAME_H
    #include "frame.h"
#endif

#ifndef VALUE_H
    #include "value.h"
#endif

#ifndef CHANNEL_H
    #include "channel.h"
#endif

#include <QStack>
#include <QVector>
#include <QMap>
#include <QTime>

enum ProcessState
{
    SleepingProcess,
    AwakeProcess
};

class Channel;
class VM;

struct Process
{
    ProcessState state;
    QStack<Frame> stack;

    QVector<Channel *> allChans;
    int nsend;
    VM *owner;
    QTime timeToWake;
public:
    Process(VM *owner);
    void awaken();
    void sleep();
    void select(QVector<Channel *> allChans,
                QVector<Value *> parametes,
                int nsend);

    void successfullSelect(Channel *chan);
};

#endif // PROCESS_H
