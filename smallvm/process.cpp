#include "process.h"

Process::Process()
{
    state = AwakeProcess;
}

void Process::sleep()
{
    state = SleepingProcess;
}

void Process::awaken()
{
    state = AwakeProcess;
}
