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
    bool isOpen;
public:
    QMap<Process *, Value *> data;
    QQueue<Value *> nullProcessQ;
public:
    QMutex lock;
    Channel();

    void close();

    bool closed() { return !isOpen; }
    bool opened() { return isOpen; }

    void send(Value *v, Process *proc);
    void sendNoLock(Value *v, Process *proc);

    void receive(Process *proc);
    void receiveNoLock(Process *proc);

    bool canSend();
    bool canReceive();
    void removeFromSendQ(Process *p);
    void removeFromRecvQ(Process *p);
};

#endif // CHANNEL_H
