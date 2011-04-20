#include "process.h"
#include "vm.h"

Process::Process(VM *owner)
{
    state = AwakeProcess;
    this->owner = owner;
}

void Process::sleep()
{
    state = SleepingProcess;
}

void Process::awaken()
{
    state = AwakeProcess;
}

void Process::select(QVector<Channel *> allChans, QVector<Value *> parameters, int nsend)
{
    this->allChans = allChans;
    this->nsend = nsend;

    QVector<int> readyChans;
    for(int i=0; i<allChans.count(); i++)
    {
        if(i<nsend)
        {
            if(allChans[i]->canSend())
                readyChans.append(i);
        }
        else
        {
            if(allChans[i]->canReceive())
                readyChans.append(i);
        }
    }
    if(!readyChans.empty())
    {
        int choice = rand() % readyChans.count();
        int index = readyChans[choice];
        if(index < nsend)
        {
            allChans[index]->send(parameters[index], this);
        }
        else
        {
            allChans[index]->receive(this);
        }
    }
    else
    {
        for(int i=0; i<nsend; i++)
        {
            allChans[i]->send(parameters[i], this);
        }
        for(int i=nsend; i<allChans.count(); i++)
        {
            allChans[i]->receive(this);
        }
    }
}

void Process::successfullSelect(Channel *chan)
{
    int successIndex;
    for(int i=0; i<allChans.count(); i++)
    {
        if(chan == allChans[i])
        {
            successIndex = i;
            continue;
        }
        if(i<nsend)
            allChans[i]->removeFromSendQ(this);
        else
            allChans[i]->removeFromRecvQ(this);
    }
    allChans.clear();

    this->stack.top().OperandStack.push(owner->GetAllocator().newInt(successIndex));

}
