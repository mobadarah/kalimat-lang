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
    sendNoLock(v, proc);
}

void Channel::sendNoLock(Value *v, Process *proc)
{
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
        else
        {
            nullProcessQ.push_back(v);
            send_q.push_back(NULL);
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
    receiveNoLock(proc);
}

void Channel::receiveNoLock(Process *proc)
{
    if(!send_q.empty())
    {
        Process *sender = send_q.front();
        send_q.pop_front();

        if(sender)
        {
            Value *v = data[sender];
            proc->currentFrame()->OperandStack.push(v);
            sender->awaken();
        }
        else
        {
            // The channel class allows sending from C++ code with no SmallVM 'sender'
            // process. In that case the sender stored in send_q is null
            // and the value is stored in nullProcessQ
            Value *v = nullProcessQ.front();
            nullProcessQ.pop_front();
            proc->currentFrame()->OperandStack.push(v);
        }

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
