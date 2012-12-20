#include "channel.h"
#include "process.h"

Channel::Channel()
{
}

bool Channel::canSend()
{
    // Warning: Does not lock the channel, use only for internal operations
    return !recv_q.empty();
}

void Channel::send(Value *v, Process *proc)
{
    QMutexLocker locker(&lock);
    if(!recv_q.empty())
    {
        Process *receiver = recv_q.front();
        recv_q.pop_front();
        receiver->currentFrame()->OperandStack.push(v);
        if(proc)
            proc->successfullSelect(this);
        receiver->successfullSelect(this);
        receiver->awaken();
    }
    else
    {
        if(proc)
        {
            data[proc] = v;
            send_q.push_back(proc);
            proc->sleep();
        }
    }
}

bool Channel::canReceive()
{
    // Warning: Does not lock the channel, use only for internal operations
    return !send_q.empty();
}

void Channel::receive(Process *proc)
{
    QMutexLocker locker(&lock);
    if(!send_q.empty())
    {
        Process *sender = send_q.front();
        send_q.pop_front();
        Value *v = data[sender];
        proc->currentFrame()->OperandStack.push(v);
        sender->awaken();
        if(proc)
            proc->successfullSelect(this);
        sender->successfullSelect(this);
    }
    else
    {
        recv_q.push_back(proc);
        proc->sleep();
    }
}

void Channel::removeFromSendQ(Process *p)
{
    send_q.removeOne(p);
}

void Channel::removeFromRecvQ(Process *p)
{
    recv_q.removeOne(p);
}
