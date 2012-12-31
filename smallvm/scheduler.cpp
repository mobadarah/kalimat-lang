#include "scheduler.h"
#include "process.h"
#include "vm.h"
#include "runtime/runwindow.h"

#include <queue>
using namespace std;
#include <QApplication>
Scheduler::Scheduler(VM *vm)
    :vm(vm)
{
    stopForGc = false;
    releasedGcSemaphore = false;
    runningNow = NULL;
}

void Scheduler::restartOwner()
{
    //runWindow->EmitGuiSchedule();
    qApp->processEvents(QEventLoop::AllEvents);
}

void Scheduler::attachProcess(Process *proc)
{
    switch(proc->state)
    {
    case AwakeProcess:
        running.push_back(proc);
        break;
    case SleepingProcess:
        sleeping.push_back(proc);
        break;
    case TimerWaitingProcess:
        timerWaiting.push_back(proc);
        break;
    }
}

void Scheduler::detachProcess(Process *proc)
{
    /*
    switch(proc->state)
    {
    case AwakeProcess:
        running.removeOne(proc);
        break;
    case SleepingProcess:
        sleeping.removeOne(proc);
        break;
    case TimerWaitingProcess:
        timerWaiting.removeOne(proc);
        break;
    }
    */
}

QString Scheduler::getFriendlyName()
{
    if(this == &vm->guiScheduler)
        return "GUI";
    else
        return "Execution";
}

int Scheduler::activateElapsedTimers()
{
    clock_t qt = clock();
    int ntimer = timerWaiting.count();
    if(ntimer)
    {
        while(ntimer > 0 && timerWaiting.front()->timeToWake < qt)
        {
            ntimer--;
            Process *proc = timerWaiting.takeFirst();
            proc->state = AwakeProcess;
            running.push_front(proc);
        }
    }
    return ntimer;
}

bool Scheduler::schedule()
{

    if(stopForGc)
    {
        if(!releasedGcSemaphore)
        {
            releasedGcSemaphore = true;
            vm->gcCanStart.release();
        }
        return false;
    }

    // _isRunning should be true if we have have processes in the running queue or timerWaiting queue
    int ntimer = activateElapsedTimers();
    _isRunning = ntimer;

    runningNow = NULL;
    if(!running.empty())
    {
        _isRunning = 1; // (*).... we check the running queue and find it non-empty
        return running.TryDequeue(runningNow, 30);
    }

    if(!sleeping.empty())
        _isRunning = 1;

    return false;
}

bool Scheduler::RunStep(bool singleInstruction, int maxtimeSclice)
{
    int random = rand();
    int n = singleInstruction? 1 : random % maxtimeSclice;

    // Bring a process to the front of 'running' queue

    if(!schedule())
    {
        return false;
    }

    runningNow->RunTimeSlice(n, vm, this);

    // We shall wait until a sleeping or timerWaiting process
    // awakens before deleting it, since other parts of the
    // VM still have business to do with that process
    if(runningNow->isFinished() && runningNow->state == AwakeProcess)
    {
        // If it's finished then it doesn't need to be put back in any queues, doesn't
        // need to migrate to another scheduler, and should be completely removed

        // setting runningNow to null is important because the GC looks in runningNow in
        // addition to the various queues for root objects
        delete runningNow;
        runningNow = NULL;
    }
    else if(runningNow->wannaMigrateTo != NULL && runningNow->wannaMigrateTo != this)
    {
        //vm->migrationToken.lock();
        Scheduler *temp = runningNow->wannaMigrateTo;
        runningNow->wannaMigrateTo = NULL;
        runningNow->migrateTo(temp);
        //vm->migrationToken.unlock();
    }
    else
    {
        if(runningNow->owner != this)
        {
            // this is a bug, a process is running outside its owner
            throw VMError(InternalError1, runningNow, this, runningNow->stack).arg(QString("Process %1/%2 running from sheduler %3 but owned by %4")
                          .arg((long)runningNow))
                    .arg(runningNow->starterProcedureName)
                    .arg(this->getFriendlyName())
                    .arg(runningNow->owner->getFriendlyName());
        }

        // If a process has just been run and is not asleep, waiting, or finished
        // return it to the running queue
        if(runningNow->state == AwakeProcess)
        {
            lock.lock();
            running.push_back(runningNow);
            lock.unlock();
        }

    }

    runningNow = NULL;

    return true;
}

void Scheduler::finishUpRunningProcess()
{
    if(runningNow->isFinished())
    {

    }
}

ProcessIterator *Scheduler::getProcesses()
{
    // Don't forget to delete at call site
    // todo: consider chaching it for each GC cycle
    return new SchedulerProcessIterator(*this);
}

Frame *Scheduler::launchProcess(Method *method)
{
    Process *dummy;
    return launchProcess(method, dummy);
}

Frame *Scheduler::launchProcess(Method *method, Process *&proc)
{
    Process *p = new Process(this);
    p->starterProcedureName = method->getName();
    proc = p;
    p->pushFrame(new Frame(method));
    running.push_front(p);
    Frame *ret = p->stack;
    _isRunning = 1;
    return ret;
}

Frame *Scheduler::launchProcessAsInterrupt(Method *method)
{
    Process *proc;
    Frame *ret = launchProcess(method, proc);
    proc->interrupt = true;
    return ret;
}

void Scheduler::awaken(Process *proc)
{
    lock.lock();
    sleeping.removeOne(proc);
    running.push_front(proc);
    lock.unlock();
}

void Scheduler::sleepify(Process *proc)
{
    sleeping.push_back(proc); // todo: what's more optimal, push_back or push_front?
    proc->timeSlice = 0;
}

void Scheduler::makeItWaitTimer(Process *proc, int ms)
{
    proc->state = TimerWaitingProcess;
    proc->timeSlice = 0;
    proc->timeToWake = clock() + (clock_t) (((double) ms / 1000.0) * CLOCKS_PER_SEC);
    bool added = false;
    int posToInsertAt= 0;

    // We now want to insert proc just before the proc
    // with least timeToWake that is larger than
    // proc's timeToWake
    for(int i=0; i<timerWaiting.count(); i++)
    {
        if(timerWaiting[i]->timeToWake > proc->timeToWake)
        {
            posToInsertAt = i;
            added = true;
        }
    }
    if(!added)
        timerWaiting.append(proc);
    else
        timerWaiting.insert(posToInsertAt, proc);
}

bool Scheduler::hasInterrupts()
{
    //return running.count()>0 &&
    //        running.front()->interrupt;
    return false;
}


bool Scheduler::isDone()
{
    lock.lock();
    bool ret = running.empty() && timerWaiting.empty() && sleeping.empty();
    lock.unlock();
    return ret;
}

bool Scheduler::isRunning()
{
    return _isRunning;
}
