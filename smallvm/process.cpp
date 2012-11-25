#include "process.h"
#include "vm.h"
#include "value.h"
#include "utils.h"
#include "runtime/parserengine.h"


Process::Process(VM *owner)
{
    state = AwakeProcess;
    this->owner = owner;
    interrupt = false;
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
        this->stack.top().OperandStack.push(owner->GetAllocator().newInt(successIndex));
    }
    allChans.clear();
}

const Instruction &Process::getCurrentInstruction()
{
    //todo: use this to retrieve the current instruction in RunStep()
    const Instruction &i= stack.top().currentMethod->Get(stack.top().ip);
    return i;
}

IMethod *Process::popMethod()
{
    QStack<Value *> &stack = this->stack.top().OperandStack;
    if(stack.empty())
        this->signal(InternalError1, "Empty operand stack when reading integer or integer-coercible value");
    Value *v = stack.top();
    if(!v->type->subclassOf(BuiltInTypes::MethodType))
        this->signal(InternalError1, QString("Expected method object on the stack, got %1").arg(v->type->toString()));
    stack.pop();

    return (IMethod *) v->unboxObj();
}

VArray *Process::popArray()
{
    QStack<Value *> &stack = this->stack.top().OperandStack;
    if(stack.empty())
        this->signal(InternalError1, "Empty operand stack when reading integer or integer-coercible value");
    Value *v = stack.top();
    if(!(v->tag == ArrayVal))
        this->signal(InternalError1, QString("Expected array on the stack, got %1").arg(v->type->toString()));
    stack.pop();

    return v->unboxArray();
}

int Process::popIntOrCoercedDouble()
{
    QStack<Value *> &stack = this->stack.top().OperandStack;
    if(stack.empty())
        this->signal(InternalError1, "Empty operand stack when reading integer or integer-coercible value");
    Value *v = stack.top();
    if(v->tag != Int && v->tag != Double)
        this->signal(TypeError2, QString::fromStdWString(L"عدد"), v->type->getName());
    v = stack.pop();
    int ret;
    if(v->tag == Double)
        ret = (int) v->unboxDouble();
    else
        ret = v->unboxInt();
    return ret;
}

double Process::popDoubleOrCoercedInt()
{
    QStack<Value *> &stack = this->stack.top().OperandStack;
    if(stack.empty())
        this->signal(InternalError1, "Empty operand stack when reading value in 'popDoubleOrCoercedInt'");
    Value *v = stack.top();
    if(v->tag != Int && v->tag != Double)
        this->signal(TypeError2, QString::fromStdWString(L"عدد"), v->type->getName());
    v = stack.pop();
    double ret;
    if(v->tag == Double)
        ret = v->unboxDouble();
    else
        ret = (double) v->unboxInt();
    return ret;
}

int getInstructionArity(const Instruction &i)
{
    if(i.Arg == NULL)
        return -1;
    return i.Arg->unboxInt();
}

void Process::RunTimeSlice(int slice, VM *vm)
{
    while(slice--)
    {
        if(this->isFinished())
        {
            break;
        }
        if(this->state == SleepingProcess)
        {
            break;
        }
        if(this->state == TimerWaitingProcess)
        {
            break;
        }
        if(this->state == MovedToGuiThread)
        {
            vm->getGUIProcesses().push_back(this);
            break;
        }

        RunSingleInstruction();
        // A breakpoint might pause/stop the VM, we then must stop this function at once
        if(!vm->isRunning())
            return;
    }
}

void Process::RunSingleInstruction()
{
    Frame &frame = stack.top();
    const Instruction &i = frame.currentMethod->Get(frame.ip);

    /*
    qDebug() << QString("Running '%1', IP=%2, method=%3, process=%4\n")
             .arg(InstructionToString(i))
             .arg(frame.ip)
             .arg(frame.currentMethod->getName())
             .arg((long) process);
    //*/
    frame.ip++;

    Value *Arg;

    switch(i.opcode)
    {
    case PushV:
        Arg = i.Arg;
        this->DoPushVal(Arg);
        break;
    case PushLocal:
        this->DoPushLocal(i.SymRef);
        break;
    case PushGlobal:
        this->DoPushGlobal(i.SymRef);
        break;
    case PushConstant:
        this->DoPushConstant(i.SymRef, i.SymRefLabel);
        break;
    case PopLocal:
        this->DoPopLocal(i.SymRef);
        break;
    case PopGlobal:
        this->DoPopGlobal(i.SymRef);
        break;
    case PushNull:
        this->DoPushNull();
        break;
    case SetRef:
        this->DoSetRef();
        break;
    case GetRef:
        this->DoGetRef();
        break;
    case Add:
        this->DoAdd();
        break;
    case Sub:
        this->DoSub();
        break;
    case Mul:
        this->DoMul();
        break;
    case Div:
        this->DoDiv();
        break;
    case Neg:
        this->DoNeg();
        break;
    case And:
        this->DoAnd();
        break;
    case Or:
        this->DoOr();
        break;
    case Not:
        this->DoNot(); // tee hee
        break;
    case Jmp:
        this->DoJmp(i.True);
        break;
    case JmpVal:
        this->DoJmpVal();
        break;
    case If:
        this->DoIf(i.True, i.False);
        break;
    case Lt:
        this->DoLt();
        break;
    case Gt:
        this->DoGt();
        break;
    case Eq:
        this->DoEq();
        break;
    case Ne:
        this->DoNe();
        break;
    case Le:
        this->DoLe();
        break;
    case Ge:
        this->DoGe();
        break;
    case Call:
        this->DoCall(i.SymRef, i.SymRefLabel, getInstructionArity(i), i.callStyle);
        break;
    case CallRef:
        this->DoCallRef(i.SymRef, i.SymRefLabel, getInstructionArity(i), i.callStyle);
        break;
    case CallMethod:
        this->DoCallMethod(i.SymRef, i.SymRefLabel, getInstructionArity(i), i.callStyle);
        break;
    case Ret:
        this->DoRet();
        break;
    case Apply:
        this->DoApply();
        break;
    case CallExternal:
        this->DoCallExternal(i.SymRef, i.SymRefLabel, getInstructionArity(i));
        break;
    case Nop:
        break;
    case SetField:
        this->DoSetField(i.SymRef);
        break;
    case GetField:
        this->DoGetField(i.SymRef);
        break;
    case GetFieldRef:
        this->DoGetFieldRef(i.SymRef);
        break;
    case SetArr:
        this->DoSetArr();
        break;
    case GetArr:
        this->DoGetArr();
        break;
    case GetArrRef:
        this->DoGetArrRef();
        break;
    case New:
        this->DoNew(i.SymRef, i.SymRefLabel);
        break;
    case NewArr:
        this->DoNewArr();
        break;
    case ArrLength:
        this->DoArrLength();
        break;
    case New_MD_Arr:
        this->DoNewMD_Arr();
        break;
    case Get_MD_Arr:
        this->DoGetMD_Arr();
        break;
    case Set_MD_Arr:
        this->DoSetMD_Arr();
        break;
    case Get_MD_ArrRef:
        this->DoGetMD_ArrRef();
        break;
    case MD_ArrDimensions:
        this->DoMD_ArrDimensions();
        break;
    case RegisterEvent:
        this->DoRegisterEvent(i.Arg, i.SymRef);
        break;
    case Isa:
        this->DoIsa(i.SymRef, i.SymRefLabel);
        break;
    case Send:
        this->DoSend();
        break;
    case Receive:
        this->DoReceive();
        break;
    case Select:
        this->DoSelect();
        break;
    case Break:
        this->DoBreak();
        break;
    case Tick:
        this->DoTick();
        break;
    default:
        signal(UnrecognizedInstruction);
        break;

    } // switch i.opcode
}

void Process::signal(VMErrorType toSignal)
{
    owner->signal(this, toSignal);
}


void Process::signal(VMErrorType toSignal, QString arg0)
{
    owner->signal(this, toSignal, arg0);
}

void Process::signal(VMErrorType toSignal, QString arg0, QString arg1)
{
    owner->signal(this, toSignal, arg0, arg1);
}

void Process::signal(VMErrorType toSignal, QString arg0, QString arg1, QString arg2)
{
    owner->signal(this, toSignal, arg0, arg1, arg2);
}

void Process::signalWithStack(VMError err)
{
    owner->signalWithStack(this, err);
}

void Process::assert(bool cond, VMErrorType toSignal)
{
    owner->assert(this, cond, toSignal);
}

void Process::assert(bool cond, VMErrorType toSignal, QString arg0)
{
    owner->assert(this, cond, toSignal, arg0);
}

void Process::assert(bool cond, VMErrorType toSignal, QString arg0, QString arg1)
{
    owner->assert(this, cond, toSignal, arg0, arg1);
}

void Process::assert(bool cond, VMErrorType toSignal, IClass *arg0, IClass *arg1)
{
    owner->assert(this, cond, toSignal, arg0, arg1);
}

void Process::assert(bool cond, VMErrorType toSignal, QString arg0, QString arg1, QString arg2)
{
    owner->assert(this, cond, toSignal, arg0, arg1, arg2);
}

inline Frame &Process::globalFrame()
{
    return owner->globalFrame();
}

inline QHash<int, Value*> &Process::constantPool()
{
    return owner->constantPool;
}

inline Allocator &Process::allocator()
{
    return owner->allocator;
}

void Process::DoPushVal(Value *Arg)
{
    currentFrame()->OperandStack.push(Arg);
}

void Process::DoPushLocal(QString SymRef)
{
    assert(currentFrame()->Locals.contains(SymRef), NoSuchVariable1, SymRef);
    currentFrame()->OperandStack.push(currentFrame()->Locals[SymRef]);
}

void Process::DoPushGlobal(QString SymRef)
{
    assert(globalFrame().Locals.contains(SymRef), NoSuchVariable1, SymRef);
    currentFrame()->OperandStack.push(globalFrame().Locals[SymRef]);
}

void Process::DoPushConstant(QString SymRef, int SymRefLabel)
{
    if(constantPool().contains(SymRefLabel))
        currentFrame()->OperandStack.push(constantPool()[SymRefLabel]);
    else
    {
        signal(InternalError1, QString("pushc: Constant pool doesn't contain key '%1'").arg(SymRef));
    }
}

void Process::DoPopLocal(QString SymRef)
{
    if(currentFrame()->OperandStack.empty())
        signal(InternalError1, "Empty operand stack when reading value in 'popl'");
    Value *v = currentFrame()->OperandStack.pop();
    currentFrame()->Locals[SymRef] = v;
}

void Process::DoPopGlobal(QString SymRef)
{
    if(currentFrame()->OperandStack.empty())
        signal(InternalError1, "Empty operand stack when reading value in 'popg'");
    Value *v = currentFrame()->OperandStack.pop();
    globalFrame().Locals[SymRef] = v;
}

void Process::DoPushNull()
{
    currentFrame()->OperandStack.push(allocator().null());
}

void Process::DoGetRef()
{
    // ... ref => ... val
    Reference *ref = currentFrame()->OperandStack.pop()->unboxRef();
    currentFrame()->OperandStack.push(ref->Get());
}

void Process::DoSetRef()
{
    // ...ref val => ...
    Value *v = currentFrame()->OperandStack.pop();
    Reference *ref = currentFrame()->OperandStack.pop()->unboxRef();

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

bool eq_int(int a, int b) { return a==b;}
bool eq_long(long a, long b) { return a==b;}
bool eq_double(double a, double b) { return a==b;}
bool eq_bool(bool a, bool b) { return a==b;}
bool eq_obj(IObject *a, IObject *b){ return a==b;}
bool eq_raw(void *a, void *b){ return a==b;}

bool  eq_str(QString a, QString b)
{
    return QString::compare(a, b, Qt::CaseSensitive) == 0;
}

bool  eq_difftypes(Value *, Value *)
{
    return false;
}

bool  eq_bothnull() { return true;}

bool ne_int(int a, int b) { return a!=b;}
bool ne_long(long a, long b) { return a!=b;}
bool ne_double(double a, double b) { return a!=b;}
bool ne_bool(bool a, bool b) { return a!=b;}
bool ne_obj(IObject *a, IObject *b) { return a!=b;}
bool ne_raw(void *a, void *b) { return a!=b;}

bool  ne_str(QString a, QString b)
{
    return QString::compare(a, b, Qt::CaseSensitive) != 0;
}

bool  ne_difftypes(Value *, Value *)
{
    return true;
}
bool  ne_bothnull() { return false; }

void Process::DoAdd()
{
    BuiltInAddOp(add_int, add_long, add_double, add_str);
}

void Process::DoSub()
{
    BuiltInArithmeticOp(_ws(L"طرح"), sub_int, sub_long, sub_double);
}

void Process::DoMul()
{
    BuiltInArithmeticOp(_ws(L"ضرب"), mul_int, mul_long, mul_double);
}

void Process::DoDiv()
{
    // can't convert till we can handle div by zero situation :(
    Value *v2 = currentFrame()->OperandStack.pop();
    Value *v1 = currentFrame()->OperandStack.pop();

    Value *v3 = _div(v1, v2);

    currentFrame()->OperandStack.push(v3);
}

void Process::DoNeg()
{
    Value *v1 = currentFrame()->OperandStack.pop();
    Value *v2 = NULL;

    assert(v1->tag == Int || v1->tag == Long || v1->tag == Double,
           NumericOperationOnNonNumber2, "-",v1->type->toString());
    if(v1->tag == Int)
        v2 = allocator().newInt(-v1->unboxInt());
    if(v1->tag == Long)
        v2 = allocator().newLong(-v1->unboxLong());
    if(v1->tag == Double)
        v2 = allocator().newDouble(-v1->unboxDouble());

    currentFrame()->OperandStack.push(v2);
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

void Process::DoJmp(QString label)
{
    int ip = currentFrame()->currentMethod->GetIp(label);
    assert(ip != -1, JumpingToNonExistentLabel1, label);
    currentFrame()->ip = ip;
}

void Process::DoJmpVal()
{
    Value *v = currentFrame()->OperandStack.pop();
    assert(v->tag == Int || v->tag == StringVal, IncorrectJumpLabel1, v->type->toString());
    QString label;
    if(v->tag == Int)
        label = QString("%1").arg(v->unboxInt());
    else
        label = v->unboxStr();
    DoJmp(label);
}

void Process::DoIf(QString trueLabel, QString falseLabel)
{
    Value *v = currentFrame()->OperandStack.pop();
    assert(v->tag == Boolean, TypeError2, BuiltInTypes::BoolType, v->type);
    test(v->unboxBool(), trueLabel, falseLabel);
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
    EqualityRelatedOp(eq_int, eq_long, eq_double, eq_bool, eq_obj, eq_str, eq_raw, eq_difftypes, eq_bothnull);
}

void Process::DoNe()
{
    EqualityRelatedOp(ne_int, ne_long, ne_double, ne_bool, ne_obj, ne_str, ne_raw, ne_difftypes, ne_bothnull);
}

void Process::DoLe()
{
    BuiltInComparisonOp(le_int, le_long, le_double, le_str);
}

void Process::DoGe()
{
    BuiltInComparisonOp(ge_int, ge_long, ge_double, ge_str);
}

void Process::DoCall(QString symRef, int SymRefLabel, int arity, CallStyle callStyle)
{
    CallImpl(symRef, SymRefLabel, true, arity, callStyle);
}

void Process::DoCallRef(QString symRef, int SymRefLabel, int arity, CallStyle callStyle)
{
    CallImpl(symRef, SymRefLabel, false, arity, callStyle);
}

void Process::CallImpl(QString symRef, int symRefLabel, bool wantValueNotRef, int arity, CallStyle callStyle)
{
    assert(constantPool().contains(symRefLabel), NoSuchProcedureOrFunction1, symRef);
    Method *method = dynamic_cast<Method *>(constantPool()[symRefLabel]->unboxObj());
    assert(method, NoSuchProcedureOrFunction1, symRef);
    CallImpl(method, wantValueNotRef, arity, callStyle);
}

void Process::CallImpl(Method *method, bool wantValueNotRef, int arity, CallStyle callStyle)
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

    assert(arity == -1 || method->Arity() ==-1 || arity == method->Arity(), WrongNumberOfArguments3,
           method->getName(), str(arity), str(method->Arity()));
    QVector<Value *> args;
    for(int i=0; i<method->Arity(); i++)
    {
        Value *v = currentFrame()->OperandStack.pop();
        args.append(v);
    }

    Frame *frame = NULL;
    if(callStyle == TailCall)
    {
        stack.pop();
        stack.push(Frame(method, currentFrame()));
        frame = currentFrame();
    }
    else if(callStyle == LaunchCall)
    {
        frame = owner->launchProcess(method);
    }
    else if(callStyle == NormalCall)
    {
        stack.push(Frame(method, currentFrame()));
        frame = currentFrame();
    }

    frame->returnReferenceIfRefMethod = !wantValueNotRef;
    for(int i=args.count()-1; i>=0; i--)
    {
        Value *v = args[i];
        frame->OperandStack.push(v);
    }
}

void Process::DoCallMethod(QString SymRef, int SymRefLabel, int arity, CallStyle callStyle)
{
    // callm expects the arguments in reverse order, and the last pushed argument is 'this'
    // but the execution site pops them in the correct order, i.e the first popped is 'this'
    // followed by the first normal argument...and so on.
    Value *receiver = currentFrame()->OperandStack.pop();
    bool noSuchMethodTrapFound = false;
    assert(receiver->tag != NullVal, CallingMethodOnNull);
    assert(receiver ->tag == ObjectVal, CallingMethodOnNonObject);

    IMethod *_method = receiver->type->lookupMethod(SymRef);
    /*
    if(_method == NULL)
    {
        _method = receiver->type->lookupMethod("%nosuchmethod");
        if(_method)
            noSuchMethodTrapFound = true;
    }
    */
    assert(_method!=NULL, NoSuchMethod2,SymRef, receiver->type->getName());

    assert(!noSuchMethodTrapFound || arity !=-1, InternalError1, "Calling with %nosuchmethod needs the arity to be specified in the instruction");
    Method *method = dynamic_cast<Method *>(_method);

    QVector<Value *> args;
    args.append(receiver);

    assert(arity == -1 || _method->Arity() ==-1 || arity == _method->Arity(), WrongNumberOfArguments3,
           SymRef, str(arity), str(_method->Arity()));

    if(!noSuchMethodTrapFound)
    {
        for(int i=0; i<_method->Arity()-1; i++)
        {
            Value *v = currentFrame()->OperandStack.pop();
            args.append(v);
        }
    }
    else
    {
        args.append(allocator().newString(SymRef));
        Value *arr = allocator().newArray(arity);
        for(int i=0; i<arity; i++)
        {
            arr->v.arrayVal->Elements[i] = currentFrame()->OperandStack.pop();
        }
    }


    if(callStyle == TailCall)
    {
        stack.pop();
    }
    if(method == NULL)
    {
        // Since _method is not NULL but method is,
        // therefore we have a special method (i.e not a collection of bytecode)
        // in our hands.
        CallSpecialMethod(_method, args);
        return;
    }
    else
    {
        Frame *frame = NULL;
        if(callStyle == NormalCall || callStyle == TailCall)
        {
            stack.push(Frame(method, currentFrame()));
            frame = currentFrame();
        }
        else if(callStyle == LaunchCall)
        {
            frame = owner->launchProcess(method);
        }

        // When popped and pushed, the arguments
        // will be in the right order in the new frame
        // so e.g in calling x.print(a, b)
        // the new frame will have a stack like this:
        // [x, a , b |...]
        for(int i= args.count()-1; i>=0; i--)
        {
            Value *v = args[i];
            frame->OperandStack.push(v);
        }
    }

}

void Process::CallSpecialMethod(IMethod *method, QVector<Value *> args)
{
    IForeignMethod *fm = dynamic_cast<IForeignMethod *>(method);
    if(fm != NULL)
    {
        Value *ret = fm->invoke(this, args);
        if(ret != NULL)
            currentFrame()->OperandStack.push(ret);
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
    }

    assert(false, InternalError, "CallSpecialMethod is not implemented for this type of method");
}

void Process::DoRet()
{
    Value *v = NULL;
    int toReturn = currentFrame()->currentMethod->NumReturnValues();
    bool getReferredVal = currentFrame()->currentMethod->IsReturningReference() &&! currentFrame()->returnReferenceIfRefMethod;

    if(toReturn != -1)
    {
        if(toReturn == 1 && currentFrame()->OperandStack.empty())
            signal(FunctionDidntReturnAValue1, currentFrame()->currentMethod->getName());
        else if(toReturn != currentFrame()->OperandStack.count())
            signal(InternalError1,
                   QString("Values left on stack (%1) do not match declared return value count (%2) for method '%3'")
                   .arg(currentFrame()->OperandStack.count())
                   .arg(toReturn)
                   .arg(currentFrame()->currentMethod->getName()));
    }
    if(currentFrame()->OperandStack.count()==1)
        v = currentFrame()->OperandStack.pop();
    stack.pop();
    if(!stack.empty())
    {
        if(v!=NULL)
            currentFrame()->OperandStack.push(v);
        if(getReferredVal)
            DoGetRef();
    }
}

void Process::DoApply()
{
    // Stack is:
    // methodObj, [args], ...
    Method *method = dynamic_cast<Method *>(popMethod());
    assert(method != NULL, InternalError1, "Apply has been passed a non-standard method");
    VArray *args = popArray();

    for(int i=args->count()-1; i>=0; i--)
        currentFrame()->OperandStack.push(args->Elements[i]);
    // todo: how does this interfere with tail calls or concurrency
    // in complex cases?
    CallImpl(method, true, args->count(), NormalCall);
}

void Process::DoCallExternal(QString symRef, int SymRefLabel, int arity)
{
    assert(constantPool().contains(SymRefLabel), NoSuchExternalMethod1, symRef);

    ExternalMethod *method = (ExternalMethod*) constantPool()[SymRefLabel]->v.objVal;
    assert(arity == -1 || method->Arity() ==-1 || arity == method->Arity(), WrongNumberOfArguments3,
           symRef, str(arity), str(method->Arity()));
    (*method)(currentFrame()->OperandStack, this);
}

void Process::DoSetField(QString SymRef)
{
    // ...obj val  => ...
    Value *v = currentFrame()->OperandStack.pop();
    Value *objVal = currentFrame()->OperandStack.pop();

    assert(objVal->tag != NullVal, SettingFieldOnNull);
    assert(objVal->tag == ObjectVal, SettingFieldOnNonObject1, objVal->type->toString());
    assert(objVal->type->hasField(SymRef), NoSuchField2, SymRef, objVal->type->getName());

    IObject *obj = objVal->unboxObj();
    obj->setSlotValue(SymRef, v);
}

void Process::DoGetField(QString SymRef)
{
    // ...object => val
    Value *objVal = currentFrame()->OperandStack.pop();

    assert(objVal->tag != NullVal, GettingFieldOnNull);
    assert(objVal->tag == ObjectVal, GettingFieldOnNonObject1, objVal->type->toString());
    assert(objVal->type->hasField(SymRef), NoSuchField2, SymRef, objVal->type->getName());

    IObject *obj = objVal->v.objVal;
    Value *v = obj->getSlotValue(SymRef);
    currentFrame()->OperandStack.push(v);
}

void Process::DoGetFieldRef(QString SymRef)
{
    // ...object => ...fieldRef
    Value *objVal = currentFrame()->OperandStack.pop();

    assert(objVal->tag != NullVal, GettingFieldOnNull);
    assert(objVal->tag == ObjectVal, GettingFieldOnNonObject1, objVal->type->getName());
    assert(objVal->type->hasField(SymRef), NoSuchField2, SymRef, objVal->type->getName());
    Value *ref = allocator().newFieldReference(objVal->unboxObj(), SymRef);
    currentFrame()->OperandStack.push(ref);
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


    Value *v = currentFrame()->OperandStack.pop();
    Value *index = currentFrame()->OperandStack.pop();
    Value *arrVal = currentFrame()->OperandStack.pop();

    assert(arrVal->tag == ArrayVal
           || arrVal->tag == MapVal
           || arrVal->tag == StringVal , SubscribingNonArray);

    if(arrVal->tag != StringVal)
    {
        VIndexable *container = arrVal->unboxIndexable();
        VMError err;
        bool b = container->keyCheck(index, err);
        if(!b)
            signalWithStack(err);

        container->set(index, v);
    }
    else
    {
        assert(index->tag == Int, SubscribtMustBeInteger);
        assert(v->tag == StringVal, TypeError2, BuiltInTypes::StringType, v->type);
        QString arr = arrVal->unboxStr();
        QString sv = v->unboxStr();
        assert(sv.length() ==1, ArgumentError,
               QString::fromStdWString(L"النص المحدد لابد أن يكون طوله رمزا واحدا"));
        int i = index->unboxInt() -1;
        arr[i] = sv[0];
    }
}

void Process::DoGetArr()
{

    // ...arr index => ...value
    /*
    Value *index = currentFrame()->OperandStack.pop();
    Value *arrVal= currentFrame()->OperandStack.pop();

    assert(arrVal->tag == ArrayVal, SubscribingNonArray);
    assert(index->tag == Int, SubscribtMustBeInteger);
    int i = index->unboxInt();
    VArray *arr = arrVal->unboxArray();

    assert(i>=1 && i<=arr->count, SubscriptOutOfRange2, str(i), str(arr->count));

    Value *v = arr->Elements[i-1];
    currentFrame()->OperandStack.push(v);
    */
    Value *index = currentFrame()->OperandStack.pop();
    Value *arrVal= currentFrame()->OperandStack.pop();

    assert(arrVal->tag == ArrayVal
           || arrVal->tag == MapVal,
           SubscribingNonArray);

    if(arrVal->tag != StringVal)
    {
        VMError err;
        VIndexable *container = arrVal->unboxIndexable();
        bool b = container->keyCheck(index, err);
        if(!b)
            signalWithStack(err);
        Value *v = container->get(index);
        if(!v)
            assert(false, IndexableNotFound1, index->toString());
        currentFrame()->OperandStack.push(v);
    }
    else
    {
        assert(index->tag == Int, SubscribtMustBeInteger);
        QString arr = arrVal->unboxStr();
        int i = index->unboxInt() -1;
        Value *v = allocator().newString(arr.mid(i,1));
        currentFrame()->OperandStack.push(v);
    }
}

void Process::DoGetArrRef()
{
    // todo: Sync DoGetArrRef with moving to VIndexable, support strings
    // ...arr index => ...arrref
    Value *index  = currentFrame()->OperandStack.pop();
    Value *arrVal= currentFrame()->OperandStack.pop();

    assert(arrVal->tag == ArrayVal, SubscribingNonArray);
    assert(index->tag == Int, SubscribtMustBeInteger);
    int i = index->unboxInt();
    VArray *arr = arrVal->unboxArray();

    assert(i>=1 && i<=arr->count(), SubscriptOutOfRange2, str(i), str(arr->count()));

    Value *ref = allocator().newArrayReference(arr, i-1);
    currentFrame()->OperandStack.push(ref);
}

void Process::DoNew(QString SymRef, int SymRefLabel)
{
    assert(constantPool().contains(SymRefLabel), NoSuchClass1, SymRef);
    Value *classObj = (Value*) constantPool()[SymRefLabel];
    IClass *theClass = dynamic_cast<IClass *>(classObj->v.objVal);
    assert(theClass != NULL, NameDoesntIndicateAClass1, SymRef);
    Value *newObj = allocator().newObject(theClass);
    currentFrame()->OperandStack.push(newObj);
}

void Process::DoNewArr()
{
    assert(__top()->tag == Int, TypeError2, BuiltInTypes::IntType, __top()->type);
    int size = currentFrame()->OperandStack.pop()->unboxInt();

    Value *newArr = allocator().newArray(size);
    currentFrame()->OperandStack.push(newArr);
}

void Process::DoArrLength()
{
    // ... arr => ... length
    assert(__top()->tag == ArrayVal
           || __top()->tag == MapVal
           || __top()->tag == StringVal, TypeError2, BuiltInTypes::IndexableType, __top()->type);

    Value *arrVal= currentFrame()->OperandStack.pop();
    if(arrVal->tag == StringVal)
    {
        Value *len = allocator().newInt(arrVal->unboxStr().length());
        currentFrame()->OperandStack.push(len);
    }
    else
    {
        VIndexable *arr = arrVal->unboxIndexable();
        Value *len = allocator().newInt(arr->count());
        currentFrame()->OperandStack.push(len);
    }
}

void Process::DoNewMD_Arr()
{
    // ... dimensions => ... md_arr
    assert(__top()->tag == ArrayVal, TypeError2, BuiltInTypes::ArrayType, __top()->type);
    Value *arrVal= currentFrame()->OperandStack.pop();
    VArray *arr = arrVal->unboxArray();
    QVector<int> dimensions;
    for(int i=0; i<arr->count(); i++)
    {
        assert(arr->Elements[i]->tag == Int, TypeError2, BuiltInTypes::IntType, arr->Elements[i]->type);
        int z = arr->Elements[i]->unboxInt();
        assert(z>=1, ArgumentError, QString::fromStdWString(L"حجم المصفوفة لابد أن يكون أكبر من صفر في البعد رقم %1").arg(i));
        dimensions.append(z);
    }
    Value *mdarr = allocator().newMultiDimensionalArray(dimensions);
    currentFrame()->OperandStack.push(mdarr);
}

void Process::Pop_Md_Arr_and_indexes(MultiDimensionalArray<Value *> *&theArray, QVector<int> &indexes)
{

    // ... arr index => ...
    assert(__top()->tag == ArrayVal, TypeError2, BuiltInTypes::ArrayType, __top()->type);
    Value *arrVal= currentFrame()->OperandStack.pop();
    VArray *boxedIndexes= arrVal->unboxArray();

    assert(__top()->tag == MultiDimensionalArrayVal, TypeError2, QString::fromStdWString(L"مصفوفة متعددة"), __top()->type->getName());
    Value *md_arrVal= currentFrame()->OperandStack.pop();
    theArray= md_arrVal->unboxMultiDimensionalArray();



    assert(theArray->dimensions.count()== boxedIndexes->count(), MD_IndexingWrongNumberOfDimensions);

    for(int i=0; i<boxedIndexes->count(); i++)
    {
        assert(boxedIndexes->Elements[i]->tag == Int, TypeError2, BuiltInTypes::IntType, boxedIndexes->Elements[i]->type);
        int n = boxedIndexes->Elements[i]->unboxInt();
        assert(n>=1 && n<=theArray->dimensions[i], SubscriptOutOfRange3, str(i+1), str(n), str(theArray->dimensions[i]));
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
    currentFrame()->OperandStack.push(v);
}

void Process::DoSetMD_Arr()
{
    // ... md_arr indexes value => ...
    MultiDimensionalArray<Value *> *theArray;
    QVector<int> indexes;

    Value *v = currentFrame()->OperandStack.pop();
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

    currentFrame()->OperandStack.push(v);
}

void Process::DoMD_ArrDimensions()
{
    // ... md_arr => ... dimensions
    assert(__top()->tag == MultiDimensionalArrayVal, TypeError2, BuiltInTypes::ArrayType, __top()->type);
    Value *arrVal= currentFrame()->OperandStack.pop();
    MultiDimensionalArray<Value *> *arr = arrVal->unboxMultiDimensionalArray();
    Value *v = allocator().newArray(arr->dimensions.count());
    VArray *internalArr = v->v.arrayVal;
    for(int i=0; i<arr->dimensions.count(); i++)
    {

        internalArr->Elements[i] = allocator().newInt(arr->dimensions[i]);
    }

    currentFrame()->OperandStack.push(v);
}

void Process::DoRegisterEvent(Value *evname, QString SymRef)
{
    int SymRefLabel = owner->constantPoolLabeller.labelOf(SymRef);
    assert(evname->type == BuiltInTypes::StringType, TypeError2, BuiltInTypes::StringType, evname->type);

    // todo: verify symref is actually a procedure
    assert(constantPool().contains(SymRefLabel), NoSuchProcedure1, SymRef);
    QString evName = evname->unboxStr();
    owner->registeredEventHandlers[evName] = SymRef;
}

void Process::DoIsa(QString SymRef, int SymRefLabel)
{
    // ...value => ...bool
    Value *v = currentFrame()->OperandStack.pop();
    ValueClass *cls = NULL;
    Value *classObj = NULL;
    if(constantPool().contains(SymRefLabel))
    {
        classObj = constantPool()[SymRefLabel];
        if(classObj->type == BuiltInTypes::ClassType)
        {
            cls = dynamic_cast<ValueClass *>(classObj->unboxObj());
        }
    }
    assert(cls != NULL, NoSuchClass1, SymRef);
    bool b = v->type->subclassOf(cls);
    currentFrame()->OperandStack.push(allocator().newBool(b));
}

void Process::DoSend()
{
    // ... chan val => ...
    Value *v = currentFrame()->OperandStack.pop();
    Value *chan = currentFrame()->OperandStack.pop();
    assert(chan->type == BuiltInTypes::ChannelType, TypeError2, BuiltInTypes::ChannelType, chan->type);
    Channel *channel = chan->unboxChan();
    channel->send(v, this);
}

void Process::DoReceive()
{
    // ... chan => ... val
    Value *chan = currentFrame()->OperandStack.pop();
    assert(chan->type == BuiltInTypes::ChannelType, TypeError2, BuiltInTypes::ChannelType, chan->type);
    Channel *channel = chan->unboxChan();
    channel->receive(this);
}

void Process::DoSelect()
{
    // ... arr sendcount => ... ret? activeIndex

    assert(__top()->tag == Int, InternalError);
    int nsend = currentFrame()->OperandStack.pop()->unboxInt();

    assert(__top()->tag == ArrayVal, InternalError);
    VArray *varr = currentFrame()->OperandStack.pop()->unboxArray();

    QVector<Channel *> allChans;
    QVector<Value *> args;
    for(int i=0; i<varr->count(); i+=2)
    {
        allChans.append(varr->Elements[i]->unboxChan());
        args.append(varr->Elements[i+1]);
    }
    this->select(allChans, args, nsend);
}

void Process::DoBreak()
{
    owner->_isRunning = false;

    // Restore the original instruction for when/if we resume
    QString curMethodName = currentFrame()->currentMethod->getName();
    currentFrame()->ip--;
    int ip = currentFrame()->ip;
    if(owner->breakPoints.contains(curMethodName) &&
            owner->breakPoints[curMethodName].contains(ip))
    {
        Instruction i = owner->breakPoints[curMethodName][ip];
        currentFrame()->currentMethod->Set(ip, i);
    }
    else
    {
        signal(InternalError);
    }

    if(owner->debugger)
    {
        owner->debugger->Break(curMethodName, currentFrame()->ip, currentFrame(), this);
    }
}

void Process::DoTick()
{
    long t = clock();
    currentFrame()->OperandStack.push(allocator().newLong(t));
}

void Process::test(bool cond, QString trueLabel, QString falseLabel)
{
    int newIp;
    if(cond)
        newIp = currentFrame()->currentMethod->GetIp(trueLabel);
    else
        newIp = currentFrame()->currentMethod->GetIp(falseLabel);

    currentFrame()->ip = newIp;
}

bool Process::coercion(Value *v1, Value *v2, Value *&newV1, Value *&newV2)
{
    bool ret = false;
    //todo: corecion leaks
    if (v1->tag == Int && v2->tag == Double)
    {
        // Why do we allocate a new value instead of just
        // reusing the pointer in v2? Because the call
        // to the allocator could GC v2, and thus we return an invalid
        // pointer!
        // todo: we need to make sure any call to allocation
        // is not followed by code that makes use of values
        // that are not guaranteed reachable!

        // notice how we quickly save the old
        // unboxed values before any call to allocation
        int oldv1 = v1->unboxInt();
        double oldv2 = v2->unboxDouble();

        // We first make newV1 not gcMonitored until
        // newV2 is allocated, because otherwise
        // the allocation of newV2 might GC newV1
        newV1 = allocator().newDouble(oldv1,false);
        newV2 = allocator().newDouble(oldv2);
        allocator().makeGcMonitored(newV1);
        ret = true;
    }
    else if(v1->tag == Int && v2->tag == Long)
    {
        int oldv1 = v1->unboxInt();
        long oldv2 = v2->unboxLong();
        newV1 = allocator().newLong(oldv1,false);
        newV2 = allocator().newLong(oldv2);
        allocator().makeGcMonitored(newV1);
        ret = true;
    }
    else if(v1->tag == Long && v2->tag == Double)
    {
        long oldv1 = v1->unboxLong();
        double oldv2 = v2->unboxDouble();
        newV1 = allocator().newDouble(oldv1, false);
        newV2 = allocator().newDouble(oldv2);
        allocator().makeGcMonitored(newV1);
        ret = true;
    }
    else if(v1->tag == Long && v2->tag == Int)
    {
        long oldv1 = v1->unboxLong();
        int oldv2 = v2->unboxInt();
        newV1 = allocator().newLong(oldv1, false);
        newV2 = allocator().newLong(oldv2);
        allocator().makeGcMonitored(newV1);
        ret = true;
    }
    else if(v1->tag == Double && v2->tag == Long)
    {
        double oldv1 = v1->unboxDouble();
        long oldv2 = v2->unboxLong();
        newV1 = allocator().newDouble(oldv1, false);
        newV2 = allocator().newDouble(oldv2);
        allocator().makeGcMonitored(newV1);
        ret = true;
    }
    else if(v1->tag == Double && v2->tag == Int)
    {
        double oldv1 = v1->unboxDouble();
        int oldv2 = v2->unboxInt();
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

    Value *v2 = currentFrame()->OperandStack.pop();
    Value *v1 = currentFrame()->OperandStack.pop();

    Value *newV1, *newV2;
    if(coercion(v1, v2, newV1, newV2))
    {
        v1 = newV1;
        v2 = newV2;
    }

    assert( v1->tag == v2->tag &&
            (v1->tag == Double || v1->tag == Long || v1->tag == Int || v1->tag == StringVal
             || v1->tag == ArrayVal), BuiltInOperationOnNonBuiltn2, v1->type->toString(), v2->type->toString());

    Value *v3 = NULL;

    if(v1->tag == StringVal)
        v3 = allocator().newString(strFunc(v1->unboxStr(), v2->unboxStr()));
    else if(v1->tag == Int)
        v3 = allocator().newInt(intFunc(v1->unboxInt(), v2->unboxInt()));
    else if(v1->tag == Long)
        v3 = allocator().newLong(longFunc(v1->unboxLong(), v2->unboxLong()));
    else if(v1->tag == Double )
        v3 = allocator().newDouble(doubleFunc(v1->unboxDouble(), v2->unboxDouble()));
    else if(v1->tag == ArrayVal)
    {
        VArray *arr1 = v1->unboxArray();
        VArray *arr2 = v2->unboxArray();
        v3 = allocator().newArray(arr1->count() + arr2->count());
        int c = 0;
        for(int i=0; i<arr1->count(); i++)
            v3->v.arrayVal->Elements[c++] = arr1->Elements[i];
        for(int i=0; i<arr2->count(); i++)
            v3->v.arrayVal->Elements[c++] = arr2->Elements[i];
    }
    currentFrame()->OperandStack.push(v3);
}

void Process::BuiltInArithmeticOp(QString opName,
                             int (*intFunc)(int,int),
                             long (*longFunc)(long, long),
                             double (*doubleFunc)(double,double))
{
    Value *v2 = currentFrame()->OperandStack.pop();
    Value *v1 = currentFrame()->OperandStack.pop();

    Value *newV1, *newV2;
    if(coercion(v1, v2, newV1, newV2))
    {
        v1 = newV1;
        v2 = newV2;
    }

    assert((v1->tag == v2->tag) &&
           (v1->tag == Int || v1->tag == Long || v1->tag == Double),
           NumericOperationOnNonNumber3 , opName, v1->type->toString(), v2->type->toString());

    Value *v3 = NULL;

    if(v1->tag == Int && v2->tag == Int)
        v3 = allocator().newInt(intFunc(v1->unboxInt(), v2->unboxInt()));
    else if(v1->tag == Long && v2->tag == Long)
    {
        v3 = allocator().newLong(longFunc(v1->unboxLong(),v2->unboxLong()));
    }
    else if(v1->tag == Double && v2->tag == Double)
        v3 = allocator().newDouble(doubleFunc(v1->unboxDouble(), v2->unboxDouble()));

    currentFrame()->OperandStack.push(v3);
}

void Process::BuiltInComparisonOp(bool (*intFunc)(int,int),
                             bool (*longFunc)(long, long),
                             bool (*doubleFunc)(double,double),
                             bool (*strFunc)(QString, QString))
{
    Value *v2 = currentFrame()->OperandStack.pop();
    Value *v1 = currentFrame()->OperandStack.pop();

    Value *newV1, *newV2;

    if(coercion(v1, v2, newV1, newV2))
    {
        v1 = newV1;
        v2 = newV2;
    }

    assert(v1->tag == v2->tag && (v1->tag == Int || v1->tag== Long || v1->tag == Double || v1->tag == StringVal),
           BuiltInOperationOnNonBuiltn2, v1->type->toString(), v1->type->toString());

    Value *v3 = NULL;
    bool result = false;

    if(v1->tag == Int)
        result = intFunc(v1->v.intVal, v2->v.intVal);
    else if(v1->tag == Long)
        result = longFunc(v1->v.longVal, v2->v.longVal);
    else if(v1->tag == Double)
        result = doubleFunc(v1->v.doubleVal, v2->v.doubleVal);
    else if(v1->tag == StringVal)
        result = strFunc(v1->vstrVal, v2->vstrVal);

    v3 = allocator().newBool(result);
    currentFrame()->OperandStack.push(v3);
}

void Process::EqualityRelatedOp(bool(*intFunc)(int, int),
                           bool (*longFunc)(long, long),
                           bool(*doubleFunc)(double, double),
                           bool(*boolFunc)(bool, bool),
                           bool(*objFunc)(IObject *, IObject *),
                           bool(*strFunc)(QString, QString),
                           bool(*rawFunc)(void *, void *),
                           bool(*differentTypesFunc)(Value *, Value *),
                           bool(*nullFunc)())
{
    Value *v2 = currentFrame()->OperandStack.pop();
    Value *v1 = currentFrame()->OperandStack.pop();

    Value *newV1, *newV2;

    if(coercion(v1, v2, newV1, newV2))
    {
        v1 = newV1;
        v2 = newV2;
    }

    Value *v3 = NULL;
    bool result = false;

    // If both values are of the same type...
    if(v1->tag == v2->tag
            && (v1->tag == Int || v1->tag == Long || v1->tag == Double || v1->tag == Boolean || v1->tag == ObjectVal
                || v1->tag == StringVal || v1->tag == ObjectVal || v1->tag == RawVal || v1->tag == NullVal))

    {
        if(v1->tag == Int)
            result = intFunc(v1->v.intVal, v2->v.intVal);
        if(v1->tag == Long)
            result = longFunc(v1->v.longVal, v2->v.longVal);
        if(v1->tag == Double)
            result = doubleFunc(v1->v.doubleVal, v2->v.doubleVal);
        if(v1->tag == Boolean)
            result = boolFunc(v1->unboxBool(), v2->unboxBool());
        if(v1->tag == ObjectVal)
            result = objFunc(v1->v.objVal, v2->v.objVal);
        if(v1->tag == StringVal)
            result = strFunc(v1->unboxStr(), v2->unboxStr());
        if(v1->tag == RawVal)
            result = rawFunc(v1->unboxRaw(), v2->unboxRaw());
        if(v1->tag == NullVal)
            result = nullFunc();
    }
    else
    {
        result = differentTypesFunc(v1, v2);
    }
    v3 = allocator().newBool(result);
    currentFrame()->OperandStack.push(v3);
}

void Process::BinaryLogicOp(bool (*boolFunc)(bool, bool))
{
    Value *v2 = currentFrame()->OperandStack.pop();
    Value *v1 = currentFrame()->OperandStack.pop();
    Value *v3 = NULL;

    assert(v1->tag == Boolean && v2->tag == Boolean, LogicOperationOnNonBoolean);
    v3 = allocator().newBool(boolFunc(v1->unboxBool(), v2->unboxBool()));

    currentFrame()->OperandStack.push(v3);
}

void Process::UnaryLogicOp(bool(*boolFunc)(bool))
{
    Value *v1 = currentFrame()->OperandStack.pop();
    Value *v2 = NULL;

    assert(v1->tag == Boolean, LogicOperationOnNonBoolean);
    v2 = allocator().newBool(boolFunc(v1->unboxBool()));

    currentFrame()->OperandStack.push(v2);
}

Value *Process::_div(Value *v1, Value *v2)
{
    Value *newV1 = NULL, *newV2 = NULL;

    if(coercion(v1, v2, newV1, newV2))
    {
        v1 = newV1;
        v2 = newV2;
    }

    assert((v1->tag == v2->tag) &&
           ( v1->tag == Double || v1->tag == Long || v1->tag == Int ),
           NumericOperationOnNonNumber3, _ws(L"قسمة"), v1->type->toString(), v2->type->toString());

    if(v1->tag == Int && v2->tag == Int)
    {
        assert(v2->v.intVal != 0, DivisionByZero);
        double result = ((double)(v1->unboxInt())) / ((double) (v2->unboxInt()));
        return allocator().newDouble(result);
    }
    else if(v1->tag == Double && v2->tag == Double)
    {
        assert(v2->v.doubleVal != 0.0, DivisionByZero);
        return allocator().newDouble(v1->v.doubleVal / v2->v.doubleVal);
    }
    else if(v1->tag == Long && v2->tag == Long)
    {
        assert(v2->v.longVal != 0L, DivisionByZero);
        return allocator().newDouble(((double) v1->v.longVal) /
                                 ((double)v2->v.longVal));
    }
    return NULL;
}

inline Value *Process::__top()
{
    return currentFrame()->OperandStack.top();
}
