#ifndef CHANNEL_H
#define CHANNEL_H

#ifndef VALUE_H
    #include "value.h"
#endif

#include <QQueue>

struct Process;

class Channel
{
    QQueue<Process *> send_q;
    QQueue<Process *> recv_q;
    QMap<Process *, Value *> data;
public:
    Channel();
    void send(Value *v, Process *proc);
    void receive(Process *proc);
};

#endif // CHANNEL_H
