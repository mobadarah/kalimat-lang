#include "vmrunnerthread.h"
#include "runwindow.h"

VMRunthread::VMRunthread(VM *vm, RunWindow *rw)
 :QThread(), vm(vm), rw(rw)
{

}

void VMRunthread::run()
{
    try
    {
        int pos,  len, oldPos = -1, oldLen = -1;
        while(rw->state == rwNormal || rw->state ==rwTextInput)
        {
            if(vm->destroyTheWorldFlag)
            {
                if(vm->destroyer != &vm->mainScheduler)
                    vm->worldDestruction.release();
                break;
            }
            bool visualize = client->isWonderfulMonitorEnabled();

            vm->mainScheduler.RunStep(visualize);

            if(visualize && vm->getMainProcess() && vm->getMainProcess()->state == AwakeProcess)
            {
                client->postMarkCurrentInstruction(vm, vm->getMainProcess(), &pos, &len);
                if((oldPos != pos ) && (oldLen != len))
                {
                    mySleep(client->wonderfulMonitorDelay());
                }
            }
            oldPos = pos;
            oldLen = len;
        }
        rw->update();// Final update, in case the last instruction didn't update things in time.
    }
    catch(VMError err)
    {
        rw->emitErrorEvent(err);
    }
}
