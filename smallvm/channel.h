#ifndef CHANNEL_H
#define CHANNEL_H

#ifndef VALUE_H
    #include "value.h"
#endif

#include <QQueue>
#include <QMutex>

struct Process;

class Channel
{
    QQueue<Process *> send_q;
    QQueue<Process *> recv_q;
    QMap<Process *, Value *> data;
public:
    QMutex lock;
    Channel();
    void send(Value *v, Process *proc);
    void receive(Process *proc);

    bool canSend();
    bool canReceive();
    void removeFromSendQ(Process *p);
    void removeFromRecvQ(Process *p);
};

#endif // CHANNEL_H
