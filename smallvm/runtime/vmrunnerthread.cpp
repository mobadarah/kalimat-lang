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
        while(rw->state == rwNormal || rw->state ==rwTextInput)
        {

            vm->mainScheduler.waitRunning(100);
            if(vm->mainScheduler.RunStep())
            {
                //rw->redrawWindow();
            }
            /*
                if(vm->isDone())
                    rw->client->programStopped(rw);
                */
        }
        rw->update();// Final update, in case the last instruction didn't update things in time.
    }
    catch(VMError err)
    {
        rw->emitErrorEvent(err);
    }
}
