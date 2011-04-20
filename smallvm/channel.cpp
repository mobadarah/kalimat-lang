#include "channel.h"
#include "process.h"

Channel::Channel()
{
}

bool Channel::canSend()
{
    return !recv_q.empty();
}

void Channel::send(Value *v, Process *proc)
{
    if(!recv_q.empty())
    {
        Process *receiver = recv_q.front();
        recv_q.pop_front();
        receiver->stack.top().OperandStack.push(v);
        receiver->awaken();
        receiver->successfullSelect(this);
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
    return !recv_q.empty();
}

void Channel::receive(Process *proc)
{
    if(!send_q.empty())
    {
        Process *sender = send_q.front();
        send_q.pop_front();
        Value *v = data[sender];
        proc->stack.top().OperandStack.push(v);
        sender->awaken();
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
