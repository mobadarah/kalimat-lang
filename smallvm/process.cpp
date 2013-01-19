#include "process.h"
#include "scheduler.h"
#include "vm.h"
#include "value.h"
#include "utils.h"
#include "runtime/parserengine.h"
#include <QDebug>

#define assert0(cond, toSignal) if(!(cond)) { signal(toSignal); }
#define assert1(cond, toSignal, arg0) if(!(cond)) { signal(toSignal, arg0); }
#define assert2(cond, toSignal, arg0, arg1) if(!(cond)) { signal(toSignal, arg0, arg1); }
#define assert3(cond, toSignal, arg0, arg1, arg2) if(!(cond)) { signal(toSignal, arg0, arg1, arg2); }

Process::Process(Scheduler *owner)
{
    vm = owner->vm;
    state = AwakeProcess;
    this->owner = owner;
    interrupt = false;
    wannaMigrateTo = NULL;
    stack = NULL;
    _isFinished = false;
    OperandStack.reserve(OperandStackChunkSize);
}

Process::~Process()
{
    while(stack != NULL)
    {
        Frame *temp = stack;
        stack = stack->next;
        framePool.free(temp);
    }
    // the destructor of FramePool will now take care
    // of deallocation of all frames in the freelist
}

void Process::sleep()
{
    state = SleepingProcess;
    owner->sleepify(this);
}

void Process::awaken()
{
    state = AwakeProcess;
    owner->awaken(this);
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
            allChans[index]->sendNoLock(parameters[index], this);
        }
        else
        {
            allChans[index]->receiveNoLock(this);
        }
    }
    else
    {
        for(int i=0; i<nsend; i++)
        {
            allChans[i]->sendNoLock(parameters[i], this);
        }
        for(int i=nsend; i<allChans.count(); i++)
        {
            allChans[i]->receiveNoLock(this);
        }
    }
}

void Process::successfullSelect(Channel *chan)
{
    int successIndex = -1;
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

    if(!allChans.empty())
    {
        // If this was a select, not a simple receive or send
        this->pushOperand(vm->GetAllocator().newInt(successIndex));
    }
    allChans.clear();
}

const Instruction &Process::getCurrentInstruction()
{
    //todo: use this to retrieve the current instruction in RunStep()
    Frame *f = currentFrame();
    const Instruction &i= f->currentMethod->Get(f->ip);
    return i;
}

IMethod *Process::popIMethod()
{
    Value *v = popValue();
    if(!v->type->subclassOf(BuiltInTypes::IMethodType))
        this->signal(InternalError1, QString("Expected IMethod object on the stack, got %1").arg(v->type->toString()));
    return (IMethod *) unboxObj(v);
}

Method *Process::popMethod()
{
    Value *v = popValue();
    if(!v->type->subclassOf(BuiltInTypes::MethodType))
        this->signal(InternalError1, QString("Expected method object on the stack, got %1").arg(v->type->toString()));

    return (Method *) unboxObj(v);
}

VArray *Process::popArray()
{
    Value *v = popValue();
    if(!(v->type == BuiltInTypes::ArrayType))
        this->signal(InternalError1, QString("Expected array on the stack, got %1").arg(v->type->toString()));

    return unboxArray(v);
}

int Process::popIntOrCoercedDouble()
{
    Value *v = popValue();

    if(v->type == BuiltInTypes::IntType)
        return unboxInt(v);
    else if(v->type == BuiltInTypes::DoubleType)
        return (int) unboxDouble(v);
    else
        this->signal(TypeError2, BuiltInTypes::NumericType->getName(), v->type->getName());
    return 0;
}

inline int Process::popInt()
{
    Value *v = popValue();
    if(v->type == BuiltInTypes::IntType)
        return unboxInt(v);
    else
        this->signal(TypeError2, BuiltInTypes::NumericType->getName(), v->type->getName());
    return 0;
}

inline bool Process::popBool()
{
    Value *v = popValue();
    if(v->type != BuiltInTypes::BoolType)
        this->signal(TypeError2, BuiltInTypes::BoolType->getName(), v->type->getName());

    return unboxBool(v);
}

double Process::popDoubleOrCoercedInt()
{
    Value *v = popValue();
    double ret = 0.0;
    if(v->type == BuiltInTypes::DoubleType)
        ret = unboxDouble(v);
    else if(v->type == BuiltInTypes::IntType)
        ret = (double) unboxInt(v);
    else
        this->signal(TypeError2, BuiltInTypes::NumericType->getName(), v->type->getName());

    return ret;
}

void Process::RunTimeSlice(VM *vm)
{
    while(timeSlice > 0)
    {
        RunSingleInstruction();
        vm->debugger->currentBreakCondition(this);
        timeSlice--;
    }
}

void Process::FastRunTimeSlice(VM *vm)
{
    while(timeSlice > 0)
    {
        RunSingleInstruction();
        timeSlice--;
    }
}

void Process::RunUntilReturn()
{
    /* Todo:

     * This code runs only the current process until it returns. If the process is, say, waiting to
     * receive from a channel; it would be waiting forever if the process sending to the channel runs
     * on the same scheduler. We need to run the _virtual machine_, and not just the process, until
     * the process returns.
     */
    assert1(stack->next, InternalError1, "RunUntilReturn: must have something to return to");
    Frame *prev  = stack->next;
    while(stack != prev)
    {
        if(this->isFinished())
            break;
        RunSingleInstruction();
    }
}

void Process::migrateTo(Scheduler *scheduler)
{
    if(owner == scheduler)
        return;

    if(this->state != AwakeProcess)
        signal(InternalError1, "Cannot migrate process unless it's active");

    scheduler->lock.lock();
    //qDebug("locked other in migrate");

    timeSlice = 0;
    owner->detachProcess(this);
    owner = scheduler;
    scheduler->attachProcess(this);

    scheduler->lock.unlock();
    //qDebug("unlocked other in migrate");

    /*
    if(owner == &vm->mainScheduler)
    {
        //qDebug() << "migrated " << (long) this << " to runner thread";
    }
    else
    {
        //qDebug() << "migrated " << (long) this << " to GUI thread";
        vm->guiScheduler.restartOwner();
    }
    //*/
}

void Process::RunSingleInstruction()
{
    Frame *frame = currentFrame();
    executedInstruction = (Instruction *) &frame->currentMethod->Get(frame->ip);

    /*
    //if(vm->traceInstructions)
    {
    qDebug() << QString("Running '%1', IP=%2, method=%3, process=%4, scheduler =%5")
             .arg(InstructionToString(i))
             .arg(frame->ip)
             .arg(frame->currentMethod->getName())
             .arg((long) this)
             .arg((owner ==(&vm->mainScheduler))?QString("main"): QString("gui"));
    }
    //*/
    frame->ip++;

    (this->*executedInstruction->runner)();
}

void Process::signal(VMErrorType toSignal)
{
    vm->signal(this, toSignal);
}


void Process::signal(VMErrorType toSignal, QString arg0)
{
    vm->signal(this, toSignal, arg0);
}

void Process::signal(VMErrorType toSignal, QString arg0, QString arg1)
{
    vm->signal(this, toSignal, arg0, arg1);
}

void Process::signal(VMErrorType toSignal, IClass *arg0, IClass *arg1)
{
    vm->signal(this, toSignal, arg0->getName(), arg1->getName());
}

void Process::signal(VMErrorType toSignal, QString arg0, QString arg1, QString arg2)
{
    vm->signal(this, toSignal, arg0, arg1, arg2);
}

void Process::signalWithStack(VMError err)
{
    vm->signalWithStack(this, err);
}

//*
void Process::assert(bool cond, VMErrorType toSignal)
{
    vm->assert(this, cond, toSignal);
}

void Process::assert(bool cond, VMErrorType toSignal, QString arg0)
{
    vm->assert(this, cond, toSignal, arg0);
}

void Process::assert(bool cond, VMErrorType toSignal, QString arg0, QString arg1)
{
    vm->assert(this, cond, toSignal, arg0, arg1);
}

void Process::assert(bool cond, VMErrorType toSignal, IClass *arg0, IClass *arg1)
{
    vm->assert(this, cond, toSignal, arg0, arg1);
}

void Process::assert(bool cond, VMErrorType toSignal, QString arg0, QString arg1, QString arg2)
{
    vm->assert(this, cond, toSignal, arg0, arg1, arg2);
}
//*/

inline QMap<QString, Value *> &Process::globalFrame()
{
    return vm->globalFrame();
}

inline QHash<int, Value*> &Process::constantPool()
{
    return vm->constantPool;
}

inline Allocator &Process::allocator()
{
    return vm->allocator;
}

void Process::DoPushVal()
{
    pushOperand(executedInstruction->Arg);
}

void Process::DoPushLocal()
{
    Value *v = currentFrame()->fastLocals[executedInstruction->SymRefLabel];
    assert1(v, NoSuchVariable1, executedInstruction->SymRef);
    pushOperand(v);
}

void Process::DoPushGlobal()
{
    Value *v = globalFrame().value(executedInstruction->SymRef, NULL);
    assert1(v != NULL, NoSuchVariable1, executedInstruction->SymRef);
    pushOperand(v);
}

void Process::DoPushConstant()
{
    Value *v = constantPool().value(executedInstruction->SymRefLabel, NULL);
    if(v != NULL)
        pushOperand(v);
    else
    {
        signal(InternalError1,
               QString("pushc: Constant pool doesn't contain key '%1'").arg(executedInstruction->SymRef));
    }
}

void Process::DoPopLocal()
{
    if(!frameHasOperands())
        signal(InternalError1, "Empty operand stack when reading value in 'popl'");
    int SymRefLabel = executedInstruction->SymRefLabel;
    assert1(SymRefLabel != -1, NoSuchVariable1, executedInstruction->SymRef);
    Value *v = popValue();
    currentFrame()->fastLocals[SymRefLabel] = v;
}

void Process::DoPopGlobal()
{
    if(!frameHasOperands())
        signal(InternalError1, "Empty operand stack when reading value in 'popg'");
    Value *v = popValue();
    globalFrame()[executedInstruction->SymRef] = v;
}

void Process::DoPushNull()
{
    pushOperand(allocator().null());
}

void Process::DoGetRef()
{
    // ... ref => ... val
    Reference *ref = unboxRef(popValue());
    pushOperand(ref->Get());
}

void Process::DoSetRef()
{
    // ...ref val => ...
    Value *v = popValue();
    Reference *ref = unboxRef(popValue());

    ref->Set(v);
}

int add_int(int a, int b) { return a + b;}
long add_long(long a, long b) { return a + b;}
double add_double(double a, double b) { return a + b;}
QString add_str(QString a, QString b){ return a + b;}

int sub_int(int a, int b) { return a - b;}
long sub_long(long a, long b) { return a - b;}
double sub_double(double a, double b) { return a - b;}

int mul_int(int a, int b) { return a * b;}
long mul_long (long a, long b) { return a * b;}
double mul_double(double a, double b) { return a * b;}

int div_int(int a, int b) { return a / b;}
long div_long(long a, long b) { return a / b;}
double div_double(double a, double b) { return a / b;}

bool _and(bool a, bool b) { return a && b;}
bool _or(bool a, bool b) { return a || b;}
bool _not(bool a) { return !a;}

bool lt_int(int a, int b) { return a<b;}
bool lt_long(long a, long b) { return a<b;}
bool lt_double(double a, double b) { return a<b;}
bool lt_str(QString a, QString b) { return a < b; }

bool gt_int(int a, int b) { return a>b;}
bool gt_long(long a, long b) { return a>b;}
bool gt_double(double a, double b) { return a>b;}
bool gt_str(QString a, QString b) { return a > b; }

bool le_int(int a, int b) { return a<=b;}
bool le_long(long a, long b) { return a<=b;}
bool le_double(double a, double b) { return a<=b;}
bool le_str(QString a, QString b) { return a <= b; }

bool ge_int(int a, int b) { return a>=b;}
bool ge_long(long a, long b) { return a>=b;}
bool ge_double(double a, double b) { return a>=b;}
bool ge_str(QString a, QString b) { return a >= b; }

void Process::DoAdd()
{
    //BuiltInAddOp(add_int, add_long, add_double, add_str);

    Value *v2 = this->popValue();
    Value *v1 = this->popValue();

    Value *v3 = v1->type->addTo(v1, v2, &allocator());
    pushOperand(v3);
}

void Process::DoSub()
{
    // BuiltInArithmeticOp(RId::Subtraction, sub_int, sub_long, sub_double);

    Value *v2 = this->popValue();
    Value *v1 = this->popValue();

    Value *v3 = v1->type->minus(v1, v2, &allocator());
    pushOperand(v3);
}

void Process::DoMul()
{
    BuiltInArithmeticOp(RId::Multiplication, mul_int, mul_long, mul_double);
}

void Process::DoDiv()
{
    // can't convert till we can handle div by zero situation :(
    Value *v2 = popValue();
    Value *v1 = popValue();

    Value *v3 = _div(v1, v2);

    pushOperand(v3);
}

void Process::DoNeg()
{
    Value *v1 = popValue();
    Value *v2 = NULL;

    assert2(v1->type == BuiltInTypes::IntType ||
            v1->type == BuiltInTypes::LongType ||
            v1->type == BuiltInTypes::DoubleType ,
            NumericOperationOnNonNumber2, "-",v1->type->toString());
    if(v1->type == BuiltInTypes::IntType)
        v2 = allocator().newInt(-unboxInt(v1));
    if(v1->type == BuiltInTypes::LongType)
        v2 = allocator().newLong(-unboxLong(v1));
    if(v1->type == BuiltInTypes::DoubleType)
        v2 = allocator().newDouble(-unboxDouble(v1));

    pushOperand(v2);
}

void Process::DoAnd()
{
    BinaryLogicOp(_and);
}

void Process::DoOr()
{
    BinaryLogicOp(_or);
}

void Process::DoNot()
{
    UnaryLogicOp(_not);
}

void Process::DoJmp()
{
    int fastLabel = executedInstruction->SymRefLabel;
    assert1(fastLabel != -1, JumpingToNonExistentLabel1, executedInstruction->SymRef);
    JmpImpl(fastLabel);
}

void Process::DoJmpIfEq()
{
    Value *v2 = popValue();
    Value *v1 = popValue();
    bool result = v1->equals(v2);
    if(result)
    {
        DoJmp();
    }
    else
    {
        currentFrame()->ip+=2;
    }
}

void Process::DoJmpIfNe()
{
    Value *v2 = popValue();
    Value *v1 = popValue();
    bool result = !v1->equals(v2);
    if(result)
    {
        DoJmp();
    }
    else
    {
        currentFrame()->ip+=2;
    }
}

inline void Process::JmpImpl(int label)
{
    currentFrame()->ip = label;
}

void Process::DoJmpVal()
{
    Frame *f = currentFrame();
    Value *v = popValue();
    assert1(v->type == BuiltInTypes::IntType ||
            v->type == BuiltInTypes::StringType, IncorrectJumpLabel1, v->type->toString());
    QString label;
    if(v->type == BuiltInTypes::IntType)
        label = QString("%1").arg(unboxInt(v));
    else
        label = unboxStr(v);
    JmpImpl(f->currentMethod->GetIp(label));
}

void Process::DoIf()
{
    /*
     An If instruction has no operands, and is always followed by a jmp instruction
     together the two instructions mean "jump if false".
     */
    //currentFrame()->ip += popBoolOffset();
    bool v = popBool();
    if(v)
    {
        currentFrame()->ip++; // skip the following jump
    }
    else
    {
        // directly execute the following jump
        // instead of waiting for the next instruction cycle
        // we didn't increment the IP since the jmp is going to change it anyway
        const Instruction &in = currentFrame()->currentMethod->Get(currentFrame()->ip);
        JmpImpl(in.SymRefLabel);
    }
}

void Process::DoLt()
{
    BuiltInComparisonOp(lt_int,lt_long, lt_double, lt_str);
}

void Process::DoGt()
{
    BuiltInComparisonOp(gt_int, gt_long, gt_double , gt_str);
}

void Process::DoEq()
{
    Value *v2 = popValue();
    Value *v1 = popValue();

    Value *v = allocator().newBool(v1->equals(v2));
    pushOperand(v);
}

void Process::DoNe()
{
    Value *v2 = popValue();
    Value *v1 = popValue();

    Value *v = allocator().newBool(!v1->equals(v2));
    pushOperand(v);
}

void Process::DoLe()
{
    BuiltInComparisonOp(le_int, le_long, le_double, le_str);
}

void Process::DoGe()
{
    BuiltInComparisonOp(ge_int, ge_long, ge_double, ge_str);
}

void Process::DoNop()
{

}

void Process::DoCall()
{
    Value *v = constantPool().value(executedInstruction->SymRefLabel, NULL);
    assert1(v != NULL, NoSuchProcedureOrFunction1, executedInstruction->SymRef);
    assert1(v->type == BuiltInTypes::MethodType, NoSuchProcedureOrFunction1, executedInstruction->SymRef);
    Method *method = (Method *) unboxObj(v);

    int arity = getInstructionArity(*executedInstruction);
    int marity = method->Arity();
    assert3(arity == -1 || marity ==-1 || arity == marity, WrongNumberOfArguments3,
            method->getName(), str(arity), str(method->Arity()));

    // Cache the method object (since it's resolved statically it can be cached)
    // and change how the instruction runs
    executedInstruction->Arg = v;
    executedInstruction->runner = &Process::DoCallCached;

    CallImpl(method, executedInstruction->callStyle);
}

void Process::DoCallCached()
{
    Method *method = (Method *) unboxObj(executedInstruction->Arg);
    CallImpl(method, executedInstruction->callStyle);
}

void Process::DoCallRef()
{
    CallImpl();
    if(executedInstruction->callStyle != LaunchCall)
    {
        currentFrame()->returnReferenceIfRefMethod = true;
    }
}

inline void Process::CallImpl()
{
    Value *v = constantPool().value(executedInstruction->SymRefLabel, NULL);
    assert1(v != NULL, NoSuchProcedureOrFunction1, executedInstruction->SymRef);
    assert1(v->type == BuiltInTypes::MethodType, NoSuchProcedureOrFunction1, executedInstruction->SymRef);
    Method *method = (Method *) unboxObj(v);

    int arity = getInstructionArity(*executedInstruction);
    verifyArity(arity, method);

    CallImpl(method, executedInstruction->callStyle);
}

void Process::CallImpl(Method *method, CallStyle callStyle)
{
    // call expects the arguments on the operand stack in reverse order,
    // but the callee pops them in the right order
    // so f(a, b, c) will be like this:
    // CALLER
    // -------
    // push c
    // push b
    // push a
    // call f
    // CALLEE
    // ------
    // pop a
    // pop b
    // pop c
    // ...code
    int marity = method->Arity();
    int stackSize = OperandStack.count() - marity;

    assert1(stackSize >=0, InternalError1, "Not enough operands for function call");

    // Frame *frame = NULL;
    if(callStyle == NormalCall)
    {
        pushFrame(framePool.allocate(method, stackSize));
       // frame = currentFrame();
    }
    else if(callStyle == TailCall)
    {
        Frame *oldFrame = popFrame();
        framePool.free(oldFrame);
        pushFrame(framePool.allocate(method, stackSize));
        //frame = currentFrame();
    }
    else if(callStyle == LaunchCall)
    {
        Process *newProc;
        //frame = owner->launchProcess(method, newProc);
        owner->launchProcess(method, newProc);
        copyArgs(OperandStack, newProc->OperandStack, marity);
    }
}

void Process::copyArgs(VOperandStack &source, VOperandStack &dest, int marity)
{
    Value *argsFast[10];
    Value** args = NULL;
    if(marity <=10)
    {
        for(int i=0; i<marity; i++)
        {
            Value *v = source.pop();
            argsFast[i] = v;
        }
        for(int i=marity-1; i>=0; i--)
        {
            Value *v = argsFast[i];
            dest.push(v);
        }
    }
    else
    {
        args = new Value*[marity];
        for(int i=0; i<marity; i++)
        {
            Value *v = source.pop();
            args[i] = v;
        }
        for(int i=marity-1; i>=0; i--)
        {
            Value *v = args[i];
            dest.push(v);
        }
        delete[] args;
    }
}

void Process::DoCallMethod()
{
    // callm expects the arguments in reverse order, and the last pushed argument is 'this'
    // but the execution site pops them in the correct order, i.e the first popped is 'this'
    // followed by the first normal argument...and so on.
    Value *receiver = popValue();
    assert0(receiver->type != BuiltInTypes::NullType, CallingMethodOnNull);
    assert0(receiver->isObject(), CallingMethodOnNonObject);

    QString SymRef = executedInstruction->SymRef;
    int arity = getInstructionArity(*executedInstruction);
    CallStyle callStyle = executedInstruction->callStyle;
    IMethod *_method = receiver->type->lookupMethod(SymRef);


    assert2(_method!=NULL, NoSuchMethod2, SymRef, receiver->type->getName());

    Method *method = dynamic_cast<Method *>(_method);

    // put the receiver back on the operand stack
    pushOperand(receiver);
    int marity = _method->Arity();
    assert3(arity == -1 || marity ==-1 || arity == marity, WrongNumberOfArguments3,
            SymRef, str(arity), str(marity));

    int stackSize = OperandStack.count() - marity;
    assert1(stackSize >=0, InternalError1, "Not enough operands for function call");

    if(callStyle == TailCall)
    {
        Frame *f = popFrame();
        framePool.free(f);
    }

    if(method == NULL)
    {
        // Since _method is not NULL but method is,
        // therefore we have a special method (i.e not a collection of bytecode)
        // in our hands.
        CallSpecialMethod(_method, marity);
        return;
    }
    else
    {
        //Frame *frame = NULL;
        if(callStyle == NormalCall || callStyle == TailCall)
        {
            pushFrame(framePool.allocate(method, stackSize));
          //  frame = currentFrame();
        }
        else if(callStyle == LaunchCall)
        {
            Process *newProc;
            //frame =
                    owner->launchProcess(method, newProc);
            copyArgs(OperandStack, newProc->OperandStack, marity);
        }
    }
}

void Process::CallSpecialMethod(IMethod *method, int arity)
{
    QVector<Value *> args;
    args.resize(arity);
    for(int i=0; i<arity; ++i)
    {
        args[i] = popValue();
    }

    IForeignMethod *fm = dynamic_cast<IForeignMethod *>(method);
    if(fm != NULL)
    {
        Value *ret = fm->invoke(this, args);
        if(ret != NULL)
            pushOperand(ret);

        return;
    }
    ApplyM *apply = dynamic_cast<ApplyM *>(method);
    if(apply)
    {
        // stack is:
        // methodObj, [args], ...

        // todo:
        //IMethod *calee = popMethod();
        //VArray *argList = popArray();
        //if(owner != &vm->mainScheduler)
        //{
        //   // qDebug() << "CallExternal finished";
        //   migrateBackFromGui();
        //}
    }

    assert1(false, InternalError, "CallSpecialMethod is not implemented for this type of method");
}

void Process::DoRet()
{
    int toReturn = currentFrame()->currentMethod->NumReturnValues();
    bool getReferredVal = currentFrame()->currentMethod->IsReturningReference() &&!currentFrame()->returnReferenceIfRefMethod;

    int leftValCount = OperandStack.count() - currentFrame()->operandStackLevel;
    if(toReturn != -1)
    {
        if(toReturn == 1 && !frameHasOperands())
            signal(FunctionDidntReturnAValue1, currentFrame()->currentMethod->getName());

        else if(toReturn != leftValCount)
        {
            // qDebug() << OperandStack.pop()->toString();
            // qDebug() << OperandStack.pop()->toString();
            signal(InternalError1,
                   QString("Values left on stack (%1) do not match declared return value count (%2) for method '%3'")
                   .arg(leftValCount)
                   .arg(toReturn)
                   .arg(currentFrame()->currentMethod->getName()));
        }
    }

    Frame *f = popFrame();
    framePool.free(f);
    if(!(stack == NULL))
    {
        if((leftValCount == 1) && getReferredVal)
            DoGetRef();
    }
    else
    {
        // All calls on the stack have returned, nothing to do
        timeSlice = 0;
    }
}

void Process::DoApply()
{
    // Stack is:
    // methodObj, [args], ...
    Method *method = dynamic_cast<Method *>(popIMethod());
    assert1(method != NULL, InternalError1, "Apply has been passed a non-standard method");
    VArray *args = popArray();

    verifyArity(args->count(), method);

    for(int i=args->count()-1; i>=0; i--)
        pushOperand(args->Elements[i]);

    // todo: how does this interfere with tail calls or concurrency
    // in complex cases?
    CallImpl(method, NormalCall);
}

void Process::startMigrationToGui()
{
    Frame *frame = currentFrame();
    frame->ip--;
    this->wannaMigrateTo = &vm->guiScheduler;
    this->timeSlice = 0;
}

void Process::migrateBackFromGui()
{
    this->wannaMigrateTo = &vm->mainScheduler;
    this->timeSlice = 0;
}

void Process::DoCallExternal()
{
    Value *v = constantPool().value(executedInstruction->SymRefLabel, NULL);
    assert1(v, NoSuchExternalMethod1, executedInstruction->SymRef);
    int arity = getInstructionArity(*executedInstruction);
    ExternalMethod *method = (ExternalMethod*) unboxObj(v);
    assert3(arity == -1 || method->Arity() ==-1 || arity == method->Arity(), WrongNumberOfArguments3,
            executedInstruction->SymRef, str(arity), str(method->Arity()));

    if(method->mustRunInGui && owner != &vm->guiScheduler)
    {
        startMigrationToGui();
        return;
    }

    (*method)(OperandStack, this);
    if(method->mustRunInGui && owner != &vm->mainScheduler)
    {
        // qDebug() << "CallExternal finished";
        migrateBackFromGui();
    }
}

void Process::DoSetField()
{
    // ...obj val  => ...
    Value *v = popValue();
    Value *objVal = popValue();
    QString SymRef = executedInstruction->SymRef;
    assert0(objVal->type != BuiltInTypes::NullType, SettingFieldOnNull);
    assert1(objVal->isObject(), SettingFieldOnNonObject1, objVal->type->toString());
    assert2(objVal->type->hasField(SymRef), NoSuchField2, SymRef, objVal->type->getName());

    IObject *obj = unboxObj(objVal);
    obj->setSlotValue(SymRef, v);
}

void Process::DoGetField()
{
    // ...object => val
    Value *objVal = popValue();
    QString SymRef = executedInstruction->SymRef;
    assert0(objVal->type != BuiltInTypes::NullType, GettingFieldOnNull);
    assert1(objVal->isObject(), GettingFieldOnNonObject1, objVal->type->toString());
    assert2(objVal->type->hasField(SymRef), NoSuchField2, SymRef, objVal->type->getName());

    IObject *obj = unboxObj(objVal);
    Value *v = obj->getSlotValue(SymRef);
    pushOperand(v);
}

void Process::DoGetFieldRef()
{
    // ...object => ...fieldRef
    Value *objVal = popValue();
    QString SymRef = executedInstruction->SymRef;
    assert0(objVal->type != BuiltInTypes::NullType, GettingFieldOnNull);
    assert1(objVal->isObject(), GettingFieldOnNonObject1, objVal->type->getName());
    assert2(objVal->type->hasField(SymRef), NoSuchField2, SymRef, objVal->type->getName());
    Value *ref = allocator().newFieldReference(unboxObj(objVal), SymRef);
    pushOperand(ref);
}

void Process::DoSetArr()
{
    // ...arr index val => ...
    /*
    Value *v = currentFrame()->OperandStack.pop();
    Value *index = currentFrame()->OperandStack.pop();
    Value *arrVal = currentFrame()->OperandStack.pop();

    assert(arrVal->tag == ArrayVal, SubscribingNonArray);
    assert(index->tag == Int, SubscribtMustBeInteger);
    int i = index->unboxInt();
    VArray *arr = arrVal->unboxArray();

    assert(i>=1 && i<=arr->count, SubscriptOutOfRange2, str(i), str(arr->count));

    arr->Elements[i-1] = v;
    */

    Value *v = popValue();
    Value *index = popValue();
    Value *arrVal = popValue();

    assert0(arrVal->type == BuiltInTypes::ArrayType
            || arrVal->type == BuiltInTypes::MapType
            || arrVal->type == BuiltInTypes::StringType,
            SubscribingNonArray);

    if(arrVal->type != BuiltInTypes::StringType)
    {
        VIndexable *container = unboxIndexable(arrVal);
        VMError err;
        bool b = container->keyCheck(index, err);
        if(!b)
            signalWithStack(err);

        container->set(index, v);
    }
    else
    {
        assert0(index->type == BuiltInTypes::IntType, SubscribtMustBeInteger);
        assert2(v->type == BuiltInTypes::StringType, TypeError2, BuiltInTypes::StringType, v->type);
        QString arr = unboxStr(arrVal);
        QString sv = unboxStr(v);
        assert1(sv.length() ==1, ArgumentError,
                VM::argumentErrors.get(ArgErr::GivenStringMustBeOneCharacter1, sv));
        int i = unboxInt(index) -1;
        arr[i] = sv[0];
    }
}

void Process::DoGetArr()
{
    // ...arr index => ...value
    /*
    Value *index = popValue();
    Value *arrVal= popValue();

    assert(arrVal->tag == ArrayVal, SubscribingNonArray);
    assert(index->tag == Int, SubscribtMustBeInteger);
    int i = index->unboxInt();
    VArray *arr = arrVal->unboxArray();

    assert(i>=1 && i<=arr->count, SubscriptOutOfRange2, str(i), str(arr->count));

    Value *v = arr->Elements[i-1];
    pushOperand(v);
    */
    Value *index = popValue();
    Value *arrVal= popValue();

    assert0(arrVal->type == BuiltInTypes::ArrayType
            || arrVal->type == BuiltInTypes::MapType,
            SubscribingNonArray);

    if(arrVal->type != BuiltInTypes::StringType)
    {
        VMError err;
        VIndexable *container = unboxIndexable(arrVal);
        bool b = container->keyCheck(index, err);
        if(!b)
            signalWithStack(err);
        Value *v = container->get(index);
        if(!v)
            assert1(false, IndexableNotFound1, index->toString());
        pushOperand(v);
    }
    else
    {
        assert0(index->type == BuiltInTypes::IntType, SubscribtMustBeInteger);
        QString arr = unboxStr(arrVal);
        int i = unboxInt(index) -1;
        Value *v = allocator().newString(arr.mid(i,1));
        pushOperand(v);
    }
}

void Process::DoGetArrRef()
{
    // todo: Sync DoGetArrRef with moving to VIndexable, support strings
    // ...arr index => ...arrref
    Value *index  = popValue();
    Value *arrVal= popValue();

    assert0(arrVal->type == BuiltInTypes::ArrayType, SubscribingNonArray);
    assert0(index->type == BuiltInTypes::IntType, SubscribtMustBeInteger);
    int i = unboxInt(index);
    VArray *arr = unboxArray(arrVal);

    assert2(i>=1 && i<=arr->count(), SubscriptOutOfRange2, str(i), str(arr->count()));

    Value *ref = allocator().newArrayReference(arr, i-1);
    pushOperand(ref);
}

void Process::DoNew()
{
    Value *classObj = constantPool().value(executedInstruction->SymRefLabel, NULL);
    assert1(classObj != NULL, NoSuchClass1, executedInstruction->SymRef);

    IClass *theClass = dynamic_cast<IClass *>(unboxObj(classObj));
    assert1(theClass != NULL, NameDoesntIndicateAClass1, executedInstruction->SymRef);

    ForeignClass *fc = dynamic_cast<ForeignClass *>(theClass);

    if(fc && owner != &vm->guiScheduler)
    {
        startMigrationToGui();
        return;
    }

    Value *newObj = allocator().newObject(theClass);
    pushOperand(newObj);

    if(fc && owner != &vm->mainScheduler)
    {
        migrateBackFromGui();
        return;
    }
}

void Process::DoNewArr()
{
    assert2(__top()->type == BuiltInTypes::IntType, TypeError2, BuiltInTypes::IntType, __top()->type);
    int size = popInt();

    Value *newArr = allocator().newArray(size);
    pushOperand(newArr);
}

void Process::DoArrLength()
{
    // ... arr => ... length
    assert2(__top()->type == BuiltInTypes::ArrayType
            || __top()->type == BuiltInTypes::MapType
            || __top()->type == BuiltInTypes::StringType, TypeError2, BuiltInTypes::IndexableType, __top()->type);

    Value *arrVal= popValue();
    if(arrVal->type == BuiltInTypes::StringType)
    {
        Value *len = allocator().newInt(unboxStr(arrVal).length());
        pushOperand(len);
    }
    else
    {
        VIndexable *arr = unboxIndexable(arrVal);
        Value *len = allocator().newInt(arr->count());
        pushOperand(len);
    }
}

void Process::DoNewMD_Arr()
{
    // ... dimensions => ... md_arr
    VArray *arr = popArray();
    QVector<int> dimensions;
    for(int i=0; i<arr->count(); i++)
    {
        assert2(arr->Elements[i]->type == BuiltInTypes::IntType, TypeError2, BuiltInTypes::IntType, arr->Elements[i]->type);
        int z = unboxInt(arr->Elements[i]);
        assert1(z>=1, ArgumentError, VM::argumentErrors.get(ArgErr::ZeroSizeAtDimention1, str(i)));
        dimensions.append(z);
    }
    Value *mdarr = allocator().newMultiDimensionalArray(dimensions);
    pushOperand(mdarr);
}

void Process::Pop_Md_Arr_and_indexes(MultiDimensionalArray<Value *> *&theArray, QVector<int> &indexes)
{

    // ... arr index => ...
    assert2(__top()->type == BuiltInTypes::ArrayType, TypeError2, BuiltInTypes::ArrayType, __top()->type);
    Value *arrVal= popValue();
    VArray *boxedIndexes= unboxArray(arrVal);

    assert2(__top()->type == BuiltInTypes::MD_ArrayType, TypeError2, BuiltInTypes::MD_ArrayType, __top()->type);
    Value *md_arrVal= popValue();
    theArray= unboxMultiDimensionalArray(md_arrVal);

    assert0(theArray->dimensions.count()== boxedIndexes->count(), MD_IndexingWrongNumberOfDimensions);

    for(int i=0; i<boxedIndexes->count(); i++)
    {
        assert2(boxedIndexes->Elements[i]->type == BuiltInTypes::IntType, TypeError2, BuiltInTypes::IntType, boxedIndexes->Elements[i]->type);
        int n = unboxInt(boxedIndexes->Elements[i]);
        assert3(n>=1 && n<=theArray->dimensions[i], SubscriptOutOfRange3, str(i+1), str(n), str(theArray->dimensions[i]));
        indexes.append(n-1); // We're one-based, remember
    }
}

void Process::DoGetMD_Arr()
{
    // ... md_arr indexes => ... value
    MultiDimensionalArray<Value *> *theArray;
    QVector<int> indexes;

    Pop_Md_Arr_and_indexes(theArray, indexes);
    Value *v = theArray->get(indexes);
    pushOperand(v);
}

void Process::DoSetMD_Arr()
{
    // ... md_arr indexes value => ...
    MultiDimensionalArray<Value *> *theArray;
    QVector<int> indexes;

    Value *v = popValue();
    Pop_Md_Arr_and_indexes(theArray, indexes);

    theArray->set(indexes, v);
}

void Process::DoGetMD_ArrRef()
{
    // ... md_arr indexes => ... md_arrref
    MultiDimensionalArray<Value *> *theArray;
    QVector<int> indexes;

    Pop_Md_Arr_and_indexes(theArray, indexes);
    Value *v = allocator().newMultiDimensionalArrayReference(theArray, indexes);

    pushOperand(v);
}

void Process::DoMD_ArrDimensions()
{
    // ... md_arr => ... dimensions
    assert2(__top()->type == BuiltInTypes::MD_ArrayType, TypeError2, BuiltInTypes::MD_ArrayType, __top()->type);
    Value *arrVal= popValue();
    MultiDimensionalArray<Value *> *arr = unboxMultiDimensionalArray(arrVal);
    Value *v = allocator().newArray(arr->dimensions.count());
    VArray *internalArr = unboxArray(v);
    for(int i=0; i<arr->dimensions.count(); i++)
    {

        internalArr->Elements[i] = allocator().newInt(arr->dimensions[i]);
    }

    pushOperand(v);
}

void Process::DoRegisterEvent()
{
    int SymRefLabel = executedInstruction->SymRefLabel;
    Value *evname = executedInstruction->Arg;
    assert2(evname->type == BuiltInTypes::StringType, TypeError2, BuiltInTypes::StringType, evname->type);

    // todo: verify symref is actually a procedure
    assert1(constantPool().contains(SymRefLabel), NoSuchProcedure1, executedInstruction->SymRef);
    QString evName = unboxStr(evname);
    vm->registeredEventHandlers[evName] = executedInstruction->SymRef;
}

void Process::DoIsa()
{
    // ...value => ...bool
    Value *v = popValue();
    IClass *cls = NULL;
    Value *classObj = NULL;
    int SymRefLabel = executedInstruction->SymRefLabel;
    if(constantPool().contains(SymRefLabel))
    {
        classObj = constantPool()[SymRefLabel];
        if(classObj->type == BuiltInTypes::ClassType)
        {
            cls = dynamic_cast<IClass *>(unboxObj(classObj));
        }
    }
    assert1(cls != NULL, NoSuchClass1, executedInstruction->SymRef);
    bool b = v->type->subclassOf(cls);
    pushOperand(allocator().newBool(b));
}

void Process::DoSend()
{
    // ... chan val => ...
    Value *v = popValue();
    Value *chan = popValue();
    assert2(chan->type == BuiltInTypes::ChannelType, TypeError2, BuiltInTypes::ChannelType, chan->type);
    Channel *channel = unboxChan(chan);

    channel->send(v, this);
}

void Process::DoReceive()
{
    // ... chan => ... val
    Value *chan = popValue();
    assert2(chan->type == BuiltInTypes::ChannelType, TypeError2, BuiltInTypes::ChannelType, chan->type);
    Channel *channel = unboxChan(chan);

    channel->receive(this);
}

void insertion_sort(QVector<Channel *> &chans)
{
    // sort the cases by Hchan address to get the locking order.
    for(int i=0; i<chans.count(); i++)
    {
        int j;
        Channel *c = chans[i];
        for(j=i; j>0 && chans[j-1] >= c; j--)
            chans[j] = chans[j-1];
        chans[j] = c;
    }
}

void lock_select(const QVector<Channel *> &chans)
{
    Channel *c = NULL;
    for(int i=0; i<chans.count(); ++i)
    {
        if(c != chans[i])
        {
            c = chans[i];
            c->lock.lock();
        }
    }
}

void unlock_select(const QVector<Channel *> &chans)
{
    Channel *c = NULL;
    for(int i=chans.count()-1; i>=0; --i)
    {
        if(c != chans[i])
        {
            c = chans[i];
            c->lock.unlock();
        }
    }
}

void Process::DoSelect()
{
    // ... arr sendcount => ... ret? activeIndex

    assert0(__top()->type == BuiltInTypes::IntType, InternalError);
    int nsend = unboxInt(popValue());

    assert0(__top()->type == BuiltInTypes::ArrayType, InternalError);
    VArray *varr = unboxArray(popValue());

    QVector<Channel *> allChans;
    QVector<Value *> args;
    for(int i=0; i<varr->count(); i+=2)
    {
        // Typecheck that it's actually a chan
        Value *v = varr->Elements[i];
        assert2(v->type == BuiltInTypes::ChannelType,
               TypeError2, v->type->getName(),
                BuiltInTypes::ChannelType->getName());

        allChans.append(unboxChan(varr->Elements[i]));
        args.append(varr->Elements[i+1]);
    }
    QVector<Channel *> lockOrder;
    // todo: It's slow to lock a copy of the allChans array instead of the array itself
    // but currently the select() method depends on the order of channels in the array (to separate
    // receivers from senders) and thus we can't sort allChans
    lockOrder += allChans;
    insertion_sort(lockOrder);
    lock_select(lockOrder);
    this->select(allChans, args, nsend);
    unlock_select(lockOrder);
}

void Process::DoBreak()
{
    timeSlice = 0;
    vm->destroyTheWorld(owner);

    // Restore the original instruction for when/if we resume

    int ip = currentFrame()->ip;
    QString curMethodName = currentFrame()->currentMethod->getName();
    if(vm->breakPoints.contains(curMethodName) &&
            vm->breakPoints[curMethodName].contains(ip-1))
    {

        currentFrame()->ip--;
        ip--;
        Instruction i = vm->breakPoints[curMethodName][ip];
        currentFrame()->currentMethod->Set(ip, i);
    }
    else
    {
        //signal(InternalError);
    }

    int offset = currentFrame()->ip;
    if(vm->isBreakpointOneShot(curMethodName, offset))
    {
        vm->clearBreakPoint(curMethodName, offset);
    }

    if(vm->debugger)
    {
        if(owner == &vm->guiScheduler)
            vm->debugger->Break(offset, currentFrame(), this);
        else
            vm->debugger->postBreak(offset, currentFrame(), this);
    }
}

void Process::DoTick()
{
    //clock_t t = clock();
    //double span = ((double ) t)* 1000.0 / (double) CLOCKS_PER_SEC;
    long ts = get_time();
    pushOperand(allocator().newLong(ts/1000));
}

bool Process::coercion(Value *v1, Value *v2, Value *&newV1, Value *&newV2)
{
    bool ret = false;
    if(v1->type == v2->type)
        return ret;
    //todo: corecion leaks
    if (v1->type == BuiltInTypes::IntType && v2->type == BuiltInTypes::DoubleType)
    {
        //*
        // Why do we allocate a new value instead of just
        // reusing the pointer in v2? Because the call
        // to the allocator could GC v2, and thus we return an invalid
        // pointer!
        // todo: we need to make sure any call to allocation
        // is not followed by code that makes use of values
        // that are not guaranteed reachable!

        // notice how we quickly save the old
        // unboxed values before any call to allocation
        int oldv1 = unboxInt(v1);
        double oldv2 = unboxDouble(v2);

        // We first make newV1 not gcMonitored until
        // newV2 is allocated, because otherwise
        // the allocation of newV2 might GC newV1
        newV1 = allocator().newDouble(oldv1,false);
        newV2 = allocator().newDouble(oldv2);
        allocator().makeGcMonitored(newV1);
        ret = true;
        //*/
        /*
         // Experimental, need to review race conditions
         // if the GC runs in another thread while this code is running

        int unboxed_v1 = v1->unboxInt();
        allocator().stopGcMonitoring(v2);
        newV1 = allocator().newDouble(unboxed_v1);
        allocator().makeGcMonitored(v2);
        newV2 = v2;
        */
    }
    else if(v1->type == BuiltInTypes::IntType && v2->type == BuiltInTypes::LongType)
    {
        int oldv1 = unboxInt(v1);
        long oldv2 = unboxLong(v2);
        newV1 = allocator().newLong(oldv1,false);
        newV2 = allocator().newLong(oldv2);
        allocator().makeGcMonitored(newV1);
        ret = true;
    }
    else if(v1->type == BuiltInTypes::LongType && v2->type == BuiltInTypes::DoubleType)
    {
        long oldv1 = unboxLong(v1);
        double oldv2 = unboxDouble(v2);
        newV1 = allocator().newDouble(oldv1, false);
        newV2 = allocator().newDouble(oldv2);
        allocator().makeGcMonitored(newV1);
        ret = true;
    }
    else if(v1->type == BuiltInTypes::LongType && v2->type == BuiltInTypes::IntType)
    {
        long oldv1 = unboxLong(v1);
        int oldv2 = unboxInt(v2);
        newV1 = allocator().newLong(oldv1, false);
        newV2 = allocator().newLong(oldv2);
        allocator().makeGcMonitored(newV1);
        ret = true;
    }
    else if(v1->type == BuiltInTypes::DoubleType && v2->type == BuiltInTypes::LongType)
    {
        double oldv1 = unboxDouble(v1);
        long oldv2 = unboxLong(v2);
        newV1 = allocator().newDouble(oldv1, false);
        newV2 = allocator().newDouble(oldv2);
        allocator().makeGcMonitored(newV1);
        ret = true;
    }
    else if(v1->type == BuiltInTypes::DoubleType && v2->type == BuiltInTypes::IntType)
    {
        double oldv1 = unboxDouble(v1);
        int oldv2 = unboxInt(v2);
        newV1 = allocator().newDouble(oldv1, false);
        newV2 = allocator().newDouble(oldv2);
        allocator().makeGcMonitored(newV1);
        ret = true;
    }
    return ret;
}

void Process::BuiltInAddOp(int (*intFunc)(int,int),
                           long (*longFunc)(long, long),
                           double (*doubleFunc)(double,double),
                           QString (*strFunc)(QString, QString))
{

    Value *v2 = this->popValue();
    Value *v1 = this->popValue();

    Value *newV1, *newV2;
    if(coercion(v1, v2, newV1, newV2))
    {
        v1 = newV1;
        v2 = newV2;
    }

    assert2( v1->type == v2->type &&
             (v1->type == BuiltInTypes::IntType ||
              v1->type == BuiltInTypes::StringType||
              v1->type == BuiltInTypes::DoubleType||
              v1->type == BuiltInTypes::LongType ||
              v1->type == BuiltInTypes::ArrayType), BuiltInOperationOnNonBuiltn2, v1->type->toString(), v2->type->toString());

    Value *v3 = NULL;

    if(v1->type == BuiltInTypes::IntType)
        v3 = allocator().newInt(intFunc(unboxInt(v1), unboxInt(v2)));
    else if(v1->type == BuiltInTypes::StringType)
        v3 = allocator().newString(strFunc(unboxStr(v1), unboxStr(v2)));
    else if(v1->type == BuiltInTypes::LongType)
        v3 = allocator().newLong(longFunc(unboxLong(v1), unboxLong(v2)));
    else if(v1->type == BuiltInTypes::DoubleType)
        v3 = allocator().newDouble(doubleFunc(unboxDouble(v1), unboxDouble(v2)));
    else if(v1->type == BuiltInTypes::ArrayType)
    {
        VArray *arr1 = unboxArray(v1);
        VArray *arr2 = unboxArray(v2);
        v3 = allocator().newArray(arr1->count() + arr2->count());
        int c = 0;
        for(int i=0; i<arr1->count(); i++)
            unboxArray(v3)->Elements[c++] = arr1->Elements[i];
        for(int i=0; i<arr2->count(); i++)
            unboxArray(v3)->Elements[c++] = arr2->Elements[i];
    }
    pushOperand(v3);
}

inline void Process::BuiltInArithmeticOp(RId::RuntimeId opName,
                                         int (*intFunc)(int,int),
                                         long (*longFunc)(long, long),
                                         double (*doubleFunc)(double,double))
{
    Value *v2 = popValue();
    Value *v1 = popValue();

    Value *newV1, *newV2;
    if(coercion(v1, v2, newV1, newV2))
    {
        v1 = newV1;
        v2 = newV2;
    }

    assert3((v1->type == v2->type) &&
            (v1->type == BuiltInTypes::IntType ||
             v1->type == BuiltInTypes::LongType||
             v1->type == BuiltInTypes::DoubleType),
            NumericOperationOnNonNumber3 , VMId::get(opName), v1->type->toString(), v2->type->toString());

    Value *v3 = NULL;

    if(v1->type == BuiltInTypes::IntType)
        v3 = allocator().newInt(intFunc(unboxInt(v1), unboxInt(v2)));
    else if(v1->type == BuiltInTypes::LongType)
    {
        v3 = allocator().newLong(longFunc(unboxLong(v1), unboxLong(v2)));
    }
    else if(v1->type == BuiltInTypes::DoubleType)
        v3 = allocator().newDouble(doubleFunc(unboxDouble(v1), unboxDouble(v2)));

    pushOperand(v3);
}

void Process::BuiltInComparisonOp(bool (*intFunc)(int,int),
                                  bool (*longFunc)(long, long),
                                  bool (*doubleFunc)(double,double),
                                  bool (*strFunc)(QString, QString))
{
    Value *v2 = popValue();
    Value *v1 = popValue();

    Value *newV1, *newV2;

    if(coercion(v1, v2, newV1, newV2))
    {
        v1 = newV1;
        v2 = newV2;
    }

    assert2(v1->type == v2->type &&
            (v1->type == BuiltInTypes::IntType ||
             v1->type == BuiltInTypes::LongType ||
             v1->type == BuiltInTypes::DoubleType ||
             v1->type == BuiltInTypes::StringType),
            BuiltInOperationOnNonBuiltn2, v1->type->toString(), v1->type->toString());

    Value *v3 = NULL;
    bool result = false;

    if(v1->type == BuiltInTypes::IntType)
        result = intFunc(unboxInt(v1), unboxInt(v2));
    else if(v1->type == BuiltInTypes::LongType)
        result = longFunc(unboxLong(v1), unboxLong(v2));
    else if(v1->type == BuiltInTypes::DoubleType)
        result = doubleFunc(unboxDouble(v1), unboxDouble(v2));
    else if(v1->type == BuiltInTypes::StringType)
        result = strFunc(unboxStr(v1), unboxStr(v2));

    v3 = allocator().newBool(result);
    pushOperand(v3);
}

void Process::BinaryLogicOp(bool (*boolFunc)(bool, bool))
{
    Value *v2 = popValue();
    Value *v1 = popValue();
    Value *v3 = NULL;

    assert0(v1->type == BuiltInTypes::BoolType &&
            v2->type == BuiltInTypes::BoolType, LogicOperationOnNonBoolean);
    v3 = allocator().newBool(boolFunc(unboxBool(v1), unboxBool(v2)));

    pushOperand(v3);
}

void Process::UnaryLogicOp(bool(*boolFunc)(bool))
{
    Value *v1 = popValue();
    Value *v2 = NULL;

    assert0(v1->type == BuiltInTypes::BoolType, LogicOperationOnNonBoolean);
    v2 = allocator().newBool(boolFunc(unboxBool(v1)));

    pushOperand(v2);
}

Value *Process::_div(Value *v1, Value *v2)
{
    Value *newV1 = NULL, *newV2 = NULL;

    if(coercion(v1, v2, newV1, newV2))
    {
        v1 = newV1;
        v2 = newV2;
    }

    assert3((v1->type == v2->type) &&
            ( v1->type == BuiltInTypes::DoubleType ||
              v1->type == BuiltInTypes::LongType ||
              v1->type == BuiltInTypes::IntType),
            NumericOperationOnNonNumber3, VMId::get(RId::Division), v1->type->toString(), v2->type->toString());

    if(v1->type == BuiltInTypes::IntType)
    {
        assert0(unboxInt(v2) != 0, DivisionByZero);
        double result = ((double)(unboxInt(v1))) / ((double) (unboxInt(v2)));
        return allocator().newDouble(result);
    }
    else if(v1->type == BuiltInTypes::DoubleType)
    {
        assert0(unboxDouble(v2) != 0.0, DivisionByZero);
        return allocator().newDouble(unboxDouble(v1) / unboxDouble(v2));
    }
    else if(v1->type == BuiltInTypes::LongType)
    {
        assert0(unboxLong(v2) != 0L, DivisionByZero);
        return allocator().newDouble(((double) unboxLong(v1)) /
                                     ((double)unboxLong(v2)));
    }
    return NULL;
}
