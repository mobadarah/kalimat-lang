/**************************************************************************
**   The Kalimat programming language
**   Copyright (C) 2010 Mohamed Samy Ali - samy2004@gmail.com
**   This project is released under the Apache License version 2.0
**   as described in the included license.txt file
**************************************************************************/

#include "vm_incl.h"
#include "vm.h"
#include "utils.h"
#include "vm_ffi.h"
//#include <iostream>
#include <QLibrary>
#include <QDateTime>
#include <time.h>
#include "runtime/parserengine.h"
#include <QtDebug>

using namespace std;

Translation<ArgErr::ArgError> VM::argumentErrors(":/runlib_errors.txt");

QSet<QQueue<Process *> *> setOf(QQueue<Process *> *q1, QQueue<Process *> * q2, QQueue<Process *> *q3,
                                QQueue<Process *> *q4, QQueue<Process *> *q5)
{
    QSet<QQueue<Process *> *> ret;
    ret.insert(q1);
    ret.insert(q2);
    ret.insert(q3);
    ret.insert(q4);
    ret.insert(q5);
    return ret;
}

VM::VM()
    :allocator(&constantPool, setOf(&running, &sleeping, &timerWaiting, &newProcesses, &guiProcesses))
{
}

void VM::Init()
{
    BuiltInTypes::ClassType->setAllocator(&this->allocator);

    int _main = constantPoolLabeller.labelOf("main");
    if(!constantPool.contains(_main))
        signal(NULL, InternalError1 ,VM::argumentErrors[ArgErr::NoMainFuncToExecute]);
    Method *method = dynamic_cast<Method *>(constantPool[_main]->unboxObj());
    int malaf = constantPoolLabeller.labelOf("%file");
    if(constantPool.contains(malaf))
        BuiltInTypes::FileType = (ValueClass *) constantPool[malaf]->unboxObj();

    _globalFrame = new Frame();
    allocator.addOtherFrameAsRoot(_globalFrame);
    launchProcess(method);

    // launchProcess() puts the just created main function
    // into the newProcesses queue, while the rest of the code
    // assumes it's in 'running' queue
    // so we manually move it
    running.push_front(newProcesses.takeFirst());

    _mainProcess = running.front();
    _isRunning = true;
    debugger = NULL;
}

Frame *VM::launchProcess(Method *method)
{
    Process *dummy;
    return launchProcess(method, dummy);
}

Frame *VM::launchProcess(Method *method, Process *&proc)
{
    Process *p = new Process(this);
    p->stack.push(Frame(method, NULL));
    newProcesses.push_back(p);
    Frame *ret = &p->stack.top();
    return ret;
}

Frame *VM::launchProcessAsInterrupt(Method *method)
{
    Process *p = new Process(this);
    p->interrupt = true; // <-
    p->stack.push(Frame(method, NULL));
    newProcesses.push_front(p); // front, not back!
    Frame *ret = &p->stack.top();
    return ret;
}

void VM::awaken(Process *proc)
{
    sleeping.removeOne(proc);

    running.push_front(proc);
}

void VM::sleepify(Process *proc)
{
    running.removeOne(proc);

    sleeping.push_back(proc); // todo: what's more optimal, push_back or push_front?

}

void VM::assert(Process *proc, bool cond, VMErrorType toSignal)
{
    if(!cond)
        signal(proc, toSignal);
}

void VM::assert(Process *proc, bool cond, VMErrorType toSignal, QString arg0)
{
    if(!cond)
        signal(proc, toSignal, arg0);
}

void VM::assert(Process *proc, bool cond, VMErrorType toSignal, QString arg0, QString arg1)
{
    if(!cond)
        signal(proc, toSignal, arg0, arg1);
}

void VM::assert(Process *proc, bool cond, VMErrorType toSignal, IClass *arg0, IClass *arg1)
{
    assert(proc, cond, toSignal, arg0->getName(), arg1->getName());
}

void VM::assert(Process *proc, bool cond, VMErrorType toSignal, QString arg0, QString arg1, QString arg2)
{
    if(!cond)
        signal(proc, toSignal, arg0, arg1, arg2);
}

QSet<QQueue<Process *> *> VM::getCallStacks()
{
    return setOf(&running, &sleeping, &timerWaiting, &newProcesses, &guiProcesses);
}

void VM::signalWithStack(Process *proc, VMError err)
{
    _isRunning = false;
    if(!running.empty())
        err.callStack = proc->stack;
    throw err;
}

void VM::signal(Process *proc, VMErrorType err)
{
    _isRunning = false;
    if(proc)
        _lastError = VMError(err, proc->stack);
    else
        _lastError = VMError(err);

    throw _lastError;
}
void VM::signal(Process *proc, VMErrorType err, QString arg0)
{
    _isRunning = false;
    if(proc)
        _lastError = VMError(err, proc->stack).arg(arg0);
    else
        _lastError = VMError(err).arg(arg0);
    throw _lastError;
}

void VM::signal(Process *proc, VMErrorType err, QString arg0, QString arg1)
{
    _isRunning = false;
    if(proc)
        _lastError = VMError(err, proc->stack).arg(arg0).arg(arg1);
    else
        _lastError = VMError(err).arg(arg0).arg(arg1);
    throw _lastError;
}

void VM::signal(Process *proc, VMErrorType err, QString arg0, QString arg1, QString arg2)
{
    _isRunning = false;
    if(proc)
        _lastError = VMError(err, proc->stack).arg(arg0).arg(arg1).arg(arg2);
    else
        _lastError = VMError(err).arg(arg0).arg(arg1).arg(arg2);
    throw _lastError;
}

QString VM::toStr(int i)
{
    return QString("%1").arg(i);
}

VMError VM::GetLastError()
{
    return _lastError;
}

/*
QStack<Frame> &VM::stack()
{
    return running.front()->stack;
}
*/

void VM::makeItSleep(Process *proc, int ms)
{
    proc->state = TimerWaitingProcess;

    proc->timeToWake = clock() + ms * CLOCKS_PER_SEC / 1000;
    running.removeOne(proc);
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

bool VM::hasInterrupts()
{
    return running.count()>0 &&
            running.front()->interrupt;
}

bool VM::hasRegisteredEventHandler(QString evName)
{
    return registeredEventHandlers.contains(evName);
}

Frame &VM::globalFrame()
{
    return *_globalFrame;
}

void VM::setDebugger(Debugger *d)
{
    debugger = d;
    if(d)
    {
        d->setDebuggedProcess(_mainProcess);
    }
}

void VM::clearAllBreakPoints()
{
    breakPoints.clear();
}

void VM::setBreakPoint(QString methodName, int offset)
{
    /*
      The method in which we want to break may not exist in the VM image
      if we put a breakpoint in a CodeDoc that is not the program to be run
      or a module included by it. Therefore we check first if the method exists.
      TODO: Check how the debugging system behaves with multiple open programs and modules
    */

    int methodLabel = constantPoolLabeller.labelOf(methodName);
    if(!constantPool.contains(methodLabel))
        return;

    Method *method = dynamic_cast<Method*>(constantPool[methodLabel]->v.objVal);
    if(method == NULL)
        return;
    Instruction newI = method->Get(offset);
    breakPoints[methodName][offset] = newI;

    newI.opcode = Break;
    method->Set(offset, newI);
}

void VM::clearBreakPoint(QString methodName, int offset)
{
    int methodLabel = constantPoolLabeller.labelOf(methodName);
    Method *method = (Method*) constantPool[methodLabel]->v.objVal;
    method->Set(offset, breakPoints[methodName][offset]);
}

bool VM::getCodePos(Process *proc, QString &method, int &offset, Frame *&frame)
{
    if(proc->isFinished())
    {
        return false;
    }
    frame = &proc->stack.top();
    method = frame->currentMethod->getName();
    offset = frame->ip;
    return true;
}

void VM::Register(QString symRef, ExternalMethod *method)
{
    constantPool[constantPoolLabeller.labelOf(symRef)] =
            allocator.newObject(method, BuiltInTypes::ExternalMethodType);
}

void VM::RegisterType(QString typeName, IClass *type)
{
    constantPool[constantPoolLabeller.labelOf(typeName)] =
            allocator.newObject(type, BuiltInTypes::ClassType);
}

void VM::ActivateEvent(QString evName, QVector<Value *>args)
{
    //assert(registeredEventHandlers.contains(evName), NoSuchEvent, evName);
    if(!registeredEventHandlers.contains(evName))
        return;
    QString procName = registeredEventHandlers[evName];
    int procLabel = constantPoolLabeller.labelOf(procName);
    assert(NULL, constantPool.contains(procLabel), NoSuchProcedureOrFunction1, procName);
    Method *method = (Method *) constantPool[procLabel]->unboxObj();
    assert(NULL, args.count() == method->Arity(), WrongNumberOfArguments3, procName, toStr(args.count()), toStr(method->Arity()));

    //Frame *newFrame = launchAdministeredProcess(method, "evQ");
    Frame *newFrame = launchProcessAsInterrupt(method);
    for(int i=args.count()-1; i>=0; i--)
    {
        newFrame->OperandStack.push(args[i]);
    }
    _isRunning = true;
}

void VM::DefineStringConstant(QString symRef, QString strValue)
{
    Value *v = allocator.newString(strValue);
    constantPool[constantPoolLabeller.labelOf(symRef)] = v;
    //cout << "Defined string constant " << symRef.toStdString() << endl;
    //cout.flush();
}

void VM::SetGlobal(QString symRef, Value *v)
{
    globalFrame().Locals[symRef] = v;
}

Value *VM::GetGlobal(QString symRef)
{
    return globalFrame().Locals[symRef];
}

Value *VM::GetType(QString vmTypeId)
{
    if(vmTypeId.startsWith("*"))
    {
        IClass *pointee = (IClass *) GetType(vmTypeId.mid(1))->unboxObj();
        // todo: Currently we do not allow the GC to claim our new pointer type
        // should we unload it at some point?
        // todo: intern pointer types
        return allocator.newObject(new PointerClass(pointee), BuiltInTypes::ClassType, false);
    }
    else
    {
        int vmTypeIdLabel = constantPoolLabeller.labelOf(vmTypeId);
        if(!constantPool.contains(vmTypeIdLabel))
            signal(NULL, InternalError1, QString("VM::GetType Cannot find type:%1").arg(vmTypeId));
        Value *v = constantPool[vmTypeIdLabel];
        if(!(v->type == BuiltInTypes::ClassType))
            signal(NULL, InternalError1, QString("VM::GetType: Constant pool entry '%1' not a type").arg(vmTypeId));
        return v;
    }
}

Allocator &VM::GetAllocator()
{
    return allocator;
}

void VM::gc()
{
    allocator.gc();
}

bool VM::schedule()
{
    clock_t qt = clock();

    bool awokeAtimer = false;
    if(timerWaiting.count() > 0)
    {
        while(timerWaiting.count() >0 && timerWaiting.front()->timeToWake < qt)
        {
            awokeAtimer = true;
            Process *proc = timerWaiting.takeFirst();
            proc->awaken();
        }
    }

    if(!awokeAtimer && newProcesses.count() > 0)
    {
        Process *proc = newProcesses.takeFirst();
        running.push_front(proc);
    }


    if(running.empty() && timerWaiting.empty() && sleeping.empty())
    {
        _isRunning = false;
        return false;
    }

    if(running.empty())
    {
        return false;
    }
    return true;
}

void VM::RunStep(bool singleInstruction)
{
    int random = rand();
    int n = singleInstruction? 1 : random % 30;

    // Bring a process to the front of 'running' queue

    if(!schedule())
        return;

    runningNow = running.front();

    runningNow->RunTimeSlice(n, this);

    if(runningNow->state != MovedToGuiThread)
        finishUpRunningProcess();

    _isRunning = !isDone();
}

void VM::finishUpRunningProcess()
{
    // Notice that it may be already removed, e.g by a call to receive
    // or sleep
    if(runningNow->isFinished() || runningNow->state != AwakeProcess)
    {
        running.removeOne(runningNow);
    }

    if(runningNow->isFinished())
    {
        delete runningNow; // since it will not be put in any queue
        runningNow = NULL;

    }
}


/*
proc PrintLine(x, y):
   if(x+y)<5 then:
       print x
   else
       print y
   done
end

.method PrintLine 2
  popl x
  popl y
  pushl x
  pushl y
  add
  pushv 5
  lt
  if less,not_less
  less:
    push x
    Call print
    ret
  not_less:
    push y
    call print,1
    ret
.endmethod

*/

// This is a helper function for the next function - VM::Load()
void VM::LoadCallInstruction(Opcode type, QString arg, Allocator &allocator,
                             Method *method, QString label, int extraInfo,
                             CallStyle callStyle,
                             Labeller &constantPoolLabeller)
{
    if(arg.contains(","))
    {
        QStringList ref_arity= arg.split(",", QString::SkipEmptyParts, Qt::CaseInsensitive);
        ref_arity[0] = ref_arity[0].trimmed();
        ref_arity[1] = ref_arity[1].trimmed();
        Instruction i = Instruction(type)
                .wRef(ref_arity[0], constantPoolLabeller)
                .wArgParse(ref_arity[1], &allocator)
                .wCallStyle(callStyle);
        method->Add(i, label, extraInfo);
    }
    else
    {
        Instruction i = Instruction(type).wRef(arg, constantPoolLabeller).wCallStyle(callStyle);
        method->Add(i, label, extraInfo);
    }
}


void VM::Load(QString assemblyCode)
{
    QStringList lines = assemblyCode.split("\n", QString::SkipEmptyParts, Qt::CaseSensitive);
    QMap<ValueClass *, QString> inheritanceList;

    QString curMethodName = "";
    Method *curMethod = NULL;

    QString curClassName = "";
    ValueClass *curClass = NULL;

    CallStyle callStyle = NormalCall;
    for(int i =0; i<lines.count(); i++)
    {

        QString line = lines[i];
        int  extraInfo = -1;
        line = line.replace("\t"," ",Qt::CaseSensitive).replace("\r","",Qt::CaseSensitive).trimmed();
        if(line == "" || line.startsWith("//"))
            continue;

        if(line.contains("@",Qt::CaseSensitive))
        {
            QStringList parts = line.split("@", QString::KeepEmptyParts, Qt::CaseSensitive);
            line = parts[0].trimmed();
            QString extraInfoTxt = parts[1].trimmed();
            bool ok;
            extraInfo = extraInfoTxt.toInt(&ok, 10);
        }
        QStringList lineParts = line.split(" ", QString::SkipEmptyParts, Qt::CaseSensitive);

        QString label = "";
        QString opcode;
        QString arg ;
        if(lineParts[0].endsWith(":"))
        {
            label = lineParts[0];
            label = label.left(label.length()-1);
            if(lineParts.count() == 1)
            {
                opcode = "nop";
                arg = "";
            }
            else if(lineParts.count() == 2)
            {
                opcode = lineParts[1];
                arg = "";
            }
            else
            {
                opcode = lineParts[1];
                arg = lineParts[2];
            }
        }
        else // no label, just an instruction
        {
            label = "";
            opcode = lineParts[0];
            arg = arg = lineParts.count()>1? lineParts[1] : "";
        }
        if(opcode == ".strconst")
        {
            if(lineParts.count() == 3)
            {
                QString &sym = arg;
                QString data = base64Decode(lineParts[2]);
                DefineStringConstant(sym, data);
            }
            else
            {
                //cout << "in strconst lineParts.count is " << lineParts.count() << endl;
                //cout.flush();
                signal(NULL, InternalError1, QString("Malformed strconst input: '%1'").arg(line));
            }
        }
        else if(opcode == ".method")
        {
            curMethodName = arg.trimmed();
            int arity = -1;
            int numReturnValues = -1;
            bool returnsReference = false;
            if(lineParts.count()==3)
            {
                arity = lineParts[2].trimmed().toInt();
            }
            if(lineParts.count() == 4)
            {
                arity = lineParts[2].trimmed().toInt();
                numReturnValues = lineParts[3].trimmed().toInt();
            }
            if(lineParts.count() == 5)
            {
                arity = lineParts[2].trimmed().toInt();
                numReturnValues = lineParts[3].trimmed().toInt();
                returnsReference = lineParts[4].trimmed()== "ref";
            }
            curMethod = new Method(curMethodName, arity, numReturnValues, returnsReference, curClass); // curClass can be null, in which case the method is global
            Value *methodVal = allocator.newObject(curMethod, BuiltInTypes::MethodType);
            if(curClass == NULL)
            {
                int methodLabel = constantPoolLabeller.labelOf(curMethodName);
                if(constantPool.contains(methodLabel))
                    signal(NULL, ElementAlreadyDefined1, curMethodName);
                else
                    constantPool[methodLabel] = methodVal;
            }
            else
            {
                if(curClass->methods.contains(curMethodName))
                    signal(NULL, MethodAlreadyDefinedInClass, curMethodName, curClassName);
                else
                    curClass->methods[curMethodName] = methodVal;
            }
        }
        else if(opcode == ".endmethod")
        {
            curMethod = NULL;
            curMethodName = "";
        }
        else if(opcode == ".class")
        {
            curClassName = arg.trimmed();
            curClass = new ValueClass(curClassName, BuiltInTypes::ObjectType);
            int classLabel = constantPoolLabeller.labelOf(curClassName);
            if(constantPool.contains(classLabel))
                signal(NULL, ElementAlreadyDefined1, curClassName);
            else
                constantPool[classLabel] = allocator.newObject(curClass, BuiltInTypes::ClassType);
        }
        else if(opcode == ".endclass")
        {
            curClass = NULL;
            curClassName = "";
        }
        else if(opcode == ".field")
        {
            if(curClass->fields.contains(arg))
                signal(NULL, MethodAlreadyDefinedInClass, arg, curClassName);
            else
            {
                curClass->fieldNames.append(arg);
                curClass->fields.insert(arg);
            }
            if(lineParts.count()==3)
            {
                QString metaData = lineParts[2].trimmed();
                QStringList kvPairs = metaData.split(";", QString::SkipEmptyParts);
                for(QStringList::Iterator j= kvPairs.begin(); j !=kvPairs.end(); ++j)
                {
                    QString pair = *j;
                    QStringList kv = pair.split("=", QString::SkipEmptyParts);
                    if(kv.count() == 2)
                    {
                        QString key = kv[0];
                        QString val = kv[1];
                        //todo: make sure GC doesn't erase this
                        //when it's still needed
                        curClass->fieldAttributes[key] = allocator.newString(val);
                    }
                }
            }
        }
        else if(opcode == ".extends")
        {
            inheritanceList[curClass] = arg;
        }
        else if(opcode == "pushv")
        {
            Instruction i = Instruction(PushV).wArgParse(arg, &allocator);
            curMethod->Add(i, label, extraInfo);
        }
        else if(opcode == "pushl")
        {
            Instruction i = Instruction(PushLocal).wRef(arg, constantPoolLabeller);
            curMethod->Add(i, label, extraInfo);
        }
        else if(opcode == "pushg")
        {
            Instruction i = Instruction(PushGlobal).wRef(arg, constantPoolLabeller);
            curMethod->Add(i, label, extraInfo);
        }
        else if(opcode == "pushc")
        {
            Instruction i = Instruction(PushConstant).wRef(arg, constantPoolLabeller);
            curMethod->Add(i, label, extraInfo);
        }
        else if(opcode == "popl")
        {
            Instruction i = Instruction(PopLocal).wRef(arg, constantPoolLabeller);
            curMethod->Add(i, label, extraInfo);
        }
        else if(opcode == "popg")
        {
            Instruction i = Instruction(PopGlobal).wRef(arg, constantPoolLabeller);
            curMethod->Add(i, label, extraInfo);
        }
        else if(opcode == "pushnull")
        {
            Instruction i = Instruction(PushNull);
            curMethod->Add(i, label, extraInfo);
        }
        else if(opcode == "getref")
        {
            Instruction i = Instruction(GetRef);
            curMethod->Add(i, label, extraInfo);
        }
        else if(opcode == "setref")
        {
            Instruction i = Instruction(SetRef);
            curMethod->Add(i, label, extraInfo);
        }
        else if(opcode == "add")
        {
            Instruction i = Instruction(Add);
            curMethod->Add(i, label, extraInfo);
        }
        else if(opcode == "sub")
        {
            Instruction i = Instruction(Sub);
            curMethod->Add(i, label, extraInfo);
        }
        else if(opcode == "mul")
        {
            Instruction i = Instruction(Mul);
            curMethod->Add(i, label, extraInfo);
        }
        else if(opcode == "div")
        {
            Instruction i = Instruction(Div);
            curMethod->Add(i, label, extraInfo);
        }
        else if(opcode == "neg")
        {
            Instruction i = Instruction(Neg);
            curMethod->Add(i, label, extraInfo);
        }
        else if(opcode == "and")
        {
            Instruction i = Instruction(And);
            curMethod->Add(i, label, extraInfo);
        }
        else if(opcode == "or")
        {
            Instruction i = Instruction(Or);
            curMethod->Add(i, label, extraInfo);
        }
        else if(opcode == "not")
        {
            Instruction i = Instruction(Not);
            curMethod->Add(i, label, extraInfo);
        }
        else if(opcode == "jmp")
        {
            Instruction i = Instruction(Jmp).wLabels(arg, NULL);
            curMethod->Add(i, label, extraInfo);
        }
        else if(opcode == "jmpv")
        {
            Instruction i = Instruction(JmpVal);
            curMethod->Add(i, label, extraInfo);
        }
        else if(opcode == "if")
        {
            QStringList two_labels = arg.split(",", QString::SkipEmptyParts, Qt::CaseInsensitive);
            Instruction i = Instruction(If).wLabels(two_labels[0].trimmed(), two_labels[1].trimmed());
            curMethod->Add(i, label, extraInfo);
        }
        else if(opcode == "lt")
        {
            Instruction i = Instruction(Lt);
            curMethod->Add(i, label, extraInfo);
        }
        else if(opcode == "gt")
        {
            Instruction i = Instruction(Gt);
            curMethod->Add(i, label, extraInfo);
        }
        else if(opcode == "eq")
        {
            Instruction i = Instruction(Eq);
            curMethod->Add(i, label, extraInfo);
        }
        else if(opcode == "ne")
        {
            Instruction i = Instruction(Ne);
            curMethod->Add(i, label, extraInfo);
        }
        else if(opcode == "ge")
        {
            Instruction i = Instruction(Ge);
            curMethod->Add(i, label, extraInfo);
        }
        else if(opcode == "le")
        {
            Instruction i = Instruction(Le);
            curMethod->Add(i, label, extraInfo);
        }
        else if(opcode == "tail")
        {
            callStyle = TailCall;
        }
        else if(opcode == "launch")
        {
            callStyle = LaunchCall;
        }
        else if(opcode == "call")
        {
            LoadCallInstruction(Call, arg, allocator, curMethod, label, extraInfo, callStyle, constantPoolLabeller);
            callStyle = NormalCall;
        }
        else if(opcode == "callr")
        {
            LoadCallInstruction(CallRef, arg, allocator, curMethod, label, extraInfo, callStyle, constantPoolLabeller);
            callStyle = NormalCall;
        }
        else if(opcode == "ret")
        {
            Instruction i = Instruction(Ret);
            curMethod->Add(i, label, extraInfo);
        }
        else if(opcode == "apply")
        {
            Instruction i = Instruction(Apply);
            curMethod->Add(i, label, extraInfo);
        }
        else if(opcode == "callex")
        {
            LoadCallInstruction(CallExternal, arg, allocator, curMethod, label, extraInfo, NormalCall, constantPoolLabeller);
        }
        else if(opcode == "nop")
        {
            Instruction i = Instruction(Nop);
            curMethod->Add(i, label, extraInfo);
        }
        else if(opcode == "setfld")
        {
            Instruction i = Instruction(SetField).wRef(arg, constantPoolLabeller);
            curMethod->Add(i, label, extraInfo);
        }
        else if(opcode == "getfld")
        {
            Instruction i = Instruction(GetField).wRef(arg, constantPoolLabeller);
            curMethod->Add(i, label, extraInfo);
        }
        else if(opcode == "getfldref")
        {
            Instruction i = Instruction(GetFieldRef).wRef(arg, constantPoolLabeller);
            curMethod->Add(i, label, extraInfo);
        }
        else if(opcode == "setarr")
        {
            Instruction i = Instruction(SetArr);
            curMethod->Add(i, label, extraInfo);
        }
        else if(opcode == "getarr")
        {
            Instruction i = Instruction(GetArr);
            curMethod->Add(i, label, extraInfo);
        }
        else if(opcode == "getarrref")
        {
            Instruction i = Instruction(GetArrRef);
            curMethod->Add(i, label, extraInfo);
        }
        else if(opcode == "callm")
        {
            LoadCallInstruction(CallMethod, arg, allocator, curMethod, label, extraInfo, callStyle, constantPoolLabeller);
            callStyle = NormalCall;
        }
        else if(opcode == "new")
        {
            Instruction i = Instruction(New).wRef(arg, constantPoolLabeller);
            curMethod->Add(i, label, extraInfo);
        }
        else if(opcode == "newarr")
        {
            Instruction i = Instruction(NewArr);
            curMethod->Add(i, label, extraInfo);
        }
        else if(opcode == "arrlength")
        {
            Instruction i = Instruction(ArrLength);
            curMethod->Add(i, label, extraInfo);
        }
        else if(opcode == "newmdarr")
        {
            Instruction i = Instruction(New_MD_Arr);
            curMethod->Add(i, label, extraInfo);
        }
        else if(opcode == "getmdarr")
        {
            Instruction i = Instruction(Get_MD_Arr);
            curMethod->Add(i, label, extraInfo);
        }
        else if(opcode == "setmdarr")
        {
            Instruction i = Instruction(Set_MD_Arr);
            curMethod->Add(i, label, extraInfo);
        }
        else if(opcode == "getmdarrref")
        {
            Instruction i = Instruction(Get_MD_ArrRef);
            curMethod->Add(i, label, extraInfo);
        }
        else if(opcode == "mdarrdimensions")
        {
            Instruction i = Instruction(MD_ArrDimensions);
            curMethod->Add(i, label, extraInfo);
        }
        else if(opcode == "regev")
        {
            QStringList ev_and_proc = arg.split(",", QString::SkipEmptyParts, Qt::CaseInsensitive);
            QString evName = ev_and_proc[0].trimmed();
            QString procSymRef = ev_and_proc[1].trimmed();

            // evName is allocated but not referenced in the
            // constant pool, therefore it doesn't exist in the
            // root, and could be GC'd, hence we allocate
            // it with gcMonitor=false
            Value *name = allocator.newString(evName, false);
            Instruction i = Instruction(RegisterEvent)
                    .wArg(name)
                    .wRef(procSymRef, constantPoolLabeller);
            curMethod->Add(i, label, extraInfo);
        }
        else if(opcode == "isa")
        {
            Instruction i = Instruction(Isa).wRef(arg, constantPoolLabeller);
            curMethod->Add(i, label, extraInfo);
        }
        else if(opcode == "send")
        {
            Instruction i = Instruction(Send);
            curMethod->Add(i, label, extraInfo);
        }
        else if(opcode == "receive")
        {
            Instruction i = Instruction(Receive);
            curMethod->Add(i, label, extraInfo);
        }
        else if(opcode == "select")
        {
            Instruction i = Instruction(Select);
            curMethod->Add(i, label, extraInfo);
        }
        else if(opcode == "break")
        {
            Instruction i = Instruction(Break);
            curMethod->Add(i, label, extraInfo);
        }
        else if(opcode == "tick")
        {
            Instruction i = Instruction(Tick);
            curMethod->Add(i, label, extraInfo);
        }
        else
        {
            signal(NULL, UnrecognizedMnemonic2,opcode,toStr(i));
        }

    } // end for(lines)
    patchupInheritance(inheritanceList);
}

void VM::patchupInheritance(QMap<ValueClass *, QString> inheritanceList)
{
    // todo: walking the inheritanceList is unordered!
    // base classes might not be added in order or inheritance in code!!
    for(QMap<ValueClass *, QString>::iterator i=inheritanceList.begin(); i!= inheritanceList.end(); ++i)
    {
        ValueClass *c = i.key();
        QString parentName = i.value();
        int parentNameLabel = constantPoolLabeller.labelOf(parentName);
        Value *classObj = (Value*) constantPool[parentNameLabel];
        ValueClass *parent = dynamic_cast<ValueClass *>(classObj->v.objVal);

        // In VM::Load we always initialize a class with a base of BuiltInTypes::ObjectType
        // now that we found a class's real parent, we don't need this default
        // value any more.
        // This code assumes only one base class at a time
        // TODO: either change BaseClasses from a vector to a single class
        // or remove the assumption in this code
        if(c->BaseClasses.contains(BuiltInTypes::ObjectType))
        {
            c->BaseClasses.clear();
        }
        c->BaseClasses.append(parent);
    }
}


bool VM::isRunning()
{
    return _isRunning;
}

bool VM::isDone()
{
    return running.empty() && timerWaiting.empty() && newProcesses.empty() && sleeping.empty() && guiProcesses.empty();
}

void VM::reactivate()
{
    _isRunning = true;
}
