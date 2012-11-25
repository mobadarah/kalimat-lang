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
        while((rw->state == rwNormal || rw->state ==rwTextInput)&& vm->isRunning())
        {
            vm->RunStep();
        }
        if(vm->isDone())
            rw->client->programStopped(rw);
        rw->update();// Final update, in case the last instruction didn't update things in time.
    }
    catch(VMError err)
    {
        rw->reportError(err);
        rw->close();
    }
}

void VMRunthread::doGUI(QObject *control, QString method)
{
    emit callGUI(control, method);
}

void VMRunthread::createNew(ObjContainer *box, OBJ_MAKER maker)
{
    emit callNew(box, maker);
}
