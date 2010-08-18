/**************************************************************************
**   The Kalimat programming language
**   Copyright (C) 2010 Mohamed Samy Ali - samy2004@gmail.com
**   This project is released under the Apache License version 2.0
**   as described in the included license.txt file
**************************************************************************/

#include "vm_incl.h"
#include "vm.h"

VM::VM()
    :allocator(&constantPool, &stack)
{
    _lastError = NULL;
}
void VM::Init()
{
    Method *method = (Method *) constantPool["main"]->unboxObj();
    QString malaf = "%file";
    Value::FileType = (ValueClass *) constantPool[malaf]->unboxObj();
    stack.clear();
    stack.push(Frame(method));
    _isRunning = true;
}

void VM::assert(bool cond, VMErrorType toSignal)
{
    if(!cond)
        signal(toSignal);

}

void VM::assert(bool cond, VMErrorType toSignal, QString arg0)
{
    if(!cond)
        signal(toSignal, arg0);
}

void VM::assert(bool cond, VMErrorType toSignal, QString arg0, QString arg1)
{
    if(!cond)
        signal(toSignal, arg0, arg1);
}

void VM::assert(bool cond, VMErrorType toSignal, QString arg0, QString arg1, QString arg2)\
{
    if(!cond)
        signal(toSignal, arg0, arg1, arg2);
}

QStack<Frame> &VM::getCallStack()
{
    return this->stack;
}

void VM::signal(VMErrorType err)
{
    _isRunning = false;
    _lastError = new VMError(err, stack);
    throw _lastError;
}
void VM::signal(VMErrorType err, QString arg0)
{
    _isRunning = false;
    _lastError = (new VMError(err, stack))->arg(arg0);
    throw _lastError;
}
void VM::signal(VMErrorType err, QString arg0, QString arg1)
{
    _isRunning = false;
    _lastError = (new VMError(err, stack))->arg(arg0)->arg(arg1);
    throw _lastError;
}
void VM::signal(VMErrorType err, QString arg0, QString arg1, QString arg2)
{
    _isRunning = false;
    _lastError = (new VMError(err, stack))->arg(arg0)->arg(arg1)->arg(arg2);
    throw _lastError;
}
QString VM::toStr(int i)
{
    return QString("%1").arg(i);
}

VMError *VM::GetLastError()
{
    return _lastError;
}

Frame *VM::currentFrame()
{
    if(stack.empty())
        return NULL;
    return &stack.top();
}
Frame &VM::globalFrame()
{
    return stack[0];
}

void VM::Register(QString symRef, ExternalMethod *method)
{
    constantPool[symRef] = allocator.newObject(method, Value::ExternalMethodType);
}
void VM::ActivateEvent(QString evName, QVector<Value *>args)
{
    //assert(registeredEventHandlers.contains(evName), NoSuchEvent, evName);
    if(!registeredEventHandlers.contains(evName))
        return;
    QString procName = registeredEventHandlers[evName];
    /*
    if(stack.empty())
    {
        // If the event happened after main() is done, there
        // will be no caller stack frame to push the arguments on
        // for now we'll solve this by creating a dummy frame
        DoCall("%emptymethod");
    }
    */
    for(int i=args.count()-1; i>=0; i--)
    {
        DoPushVal(args[i]);
    }
    DoCall(procName, args.count());
    _isRunning = true;

}

void VM::DefineStringConstant(QString symRef, QString strValue)
{
    QString *str = new QString(strValue);
    Value *v = allocator.newString(str);
    constantPool[symRef] = v;
}

bool VM::hasRunningInstruction()
{
    //todo: use this instead of the initial checks in RunStep()
    if(stack.isEmpty())
    {
        return false;
    }
    if(currentFrame()->currentMethod == NULL)
    {
        return false;
    }

    if(!currentFrame()->currentMethod->HasInstruction(currentFrame()->ip))
    {
        return false;
    }
    return true;

}
Instruction VM::getCurrentInstruction()
{
    //todo: use this to retrieve the current instruction in RunStep()
    Instruction i= currentFrame()->currentMethod->Get(currentFrame()->ip);
    return i;
}
Allocator &VM::GetAllocator()
{
    return allocator;
}
void VM::gc()
{
    allocator.gc();
}
int VM::popIntOrCoercedDouble()
{
    QStack<Value *> &stack = currentFrame()->OperandStack;
    if(stack.empty())
        this->signal(InternalError);
    Value *v = stack.top();
    if(v->tag != Int && v->tag != Double)
        this->signal(TypeError);
    v = stack.pop();
    int ret;
    if(v->tag == Double)
        ret = (int) v->unboxDouble();
    else
        ret = v->unboxInt();
    return ret;
}

double VM::popDoubleOrCoercedInt()
{
    QStack<Value *> &stack = currentFrame()->OperandStack;
    if(stack.empty())
        this->signal(InternalError);
    Value *v = stack.top();
    if(v->tag != Int && v->tag != Double)
        this->signal(TypeError);
    v = stack.pop();
    double ret;
    if(v->tag == Double)
        ret = v->unboxDouble();
    else
        ret = (double) v->unboxInt();
    return ret;
}

int getInstructionArity(Instruction &i)
{
    if(i.Arg == NULL)
        return -1;
    return i.Arg->unboxInt();
}

void VM::RunStep()
{
    if(stack.isEmpty())
    {
           _isRunning = false;
        return;
    }
    if(currentFrame()->currentMethod == NULL)
    {
           _isRunning = false;
        return;
    }

    if(!currentFrame()->currentMethod->HasInstruction(currentFrame()->ip))
    {
        _isRunning = false;
        return;
    }

    Instruction i= currentFrame()->currentMethod->Get(currentFrame()->ip);
    currentFrame()->ip++;

    Value *Arg;
    QString SymRef;
    QString True, False;

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
        this->DoPushConstant(i.SymRef);
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
        this->DoCall(i.SymRef, getInstructionArity(i));
        break;
    case CallRef:
        this->DoCallRef(i.SymRef, getInstructionArity(i));
        break;
    case CallMethod:
        this->DoCallMethod(i.SymRef, getInstructionArity(i));
        break;
    case Ret:
        this->DoRet();
        break;
    case CallExternal:
        this->DoCallExternal(i.SymRef, getInstructionArity(i));
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
        this->DoNew(i.SymRef);
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
    default:
        signal(UnrecognizedInstruction);
        break;

    } // switch i.opcode
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
void VM::LoadCallInstruction(Opcode type, QString arg, Allocator &allocator, Method *method, QString label, int extraInfo)
{
    if(arg.contains(","))
    {
        QStringList ref_arity= arg.split(",", QString::SkipEmptyParts, Qt::CaseInsensitive);
        ref_arity[0] = ref_arity[0].trimmed();
        ref_arity[1] = ref_arity[1].trimmed();
        Instruction i = Instruction(type).wRef(ref_arity[0]).wArgParse(ref_arity[1], &allocator);
        method->Add(i, label, extraInfo);
    }
    else
    {
        Instruction i = Instruction(type).wRef(arg);
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
    for(int i =0; i<lines.count(); i++)
    {
        QString line = lines[i];
        int  extraInfo = -1;
        line = line.replace("\t"," ",Qt::CaseSensitive).replace("\r","",Qt::CaseSensitive).trimmed();
        if(line == "")
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
        if(opcode == ".method")
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
            Value *methodVal = allocator.newObject(curMethod, Value::MethodType);
            if(curClass == NULL)
            {
                if(constantPool.contains(curMethodName))
                    signal(ElementAlreadyDefined, curMethodName);
                else
                    constantPool[curMethodName] = methodVal;
            }
            else
            {
                if(curClass->methods.contains(curMethodName))
                    signal(MethodAlreadyDefinedInClass, curMethodName, curClassName);
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
            curClass = new ValueClass(curClassName, Value::ObjectType);
            if(constantPool.contains(curClassName))
                signal(ElementAlreadyDefined, curClassName);
            else
                constantPool[curClassName] = allocator.newObject(curClass, Value::ClassType);
        }
        else if(opcode == ".endclass")
        {
            curClass = NULL;
            curClassName = "";
        }
        else if(opcode == ".field")
        {
            if(curClass->fields.contains(arg))
                signal(MethodAlreadyDefinedInClass, arg, curClassName);
            else
                curClass->fields.insert(arg);
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
            Instruction i = Instruction(PushLocal).wRef(arg);
            curMethod->Add(i, label, extraInfo);
        }
        else if(opcode == "pushg")
        {
            Instruction i = Instruction(PushGlobal).wRef(arg);
            curMethod->Add(i, label, extraInfo);
        }
        else if(opcode == "pushc")
        {
            Instruction i = Instruction(PushConstant).wRef(arg);
            curMethod->Add(i, label, extraInfo);
        }
        else if(opcode == "popl")
        {
            Instruction i = Instruction(PopLocal).wRef(arg);
            curMethod->Add(i, label, extraInfo);
        }
        else if(opcode == "popg")
        {
            Instruction i = Instruction(PopGlobal).wRef(arg);
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
        else if(opcode == "if")
        {
            QStringList two_labels = arg.split(",", QString::SkipEmptyParts, Qt::CaseInsensitive);
            Instruction i = Instruction(If).wLabels(two_labels[0].trimmed(), two_labels[1].trimmed());
            curMethod->Add(i, label, extraInfo);
        }
        else if(opcode == "lt")
        {
            QStringList two_labels = arg.split(",", QString::SkipEmptyParts, Qt::CaseInsensitive);
            Instruction i = Instruction(Lt);
            curMethod->Add(i, label, extraInfo);
        }
        else if(opcode == "gt")
        {
            QStringList two_labels = arg.split(",", QString::SkipEmptyParts, Qt::CaseInsensitive);
            Instruction i = Instruction(Gt);
            curMethod->Add(i, label, extraInfo);
        }
        else if(opcode == "eq")
        {
            QStringList two_labels = arg.split(",", QString::SkipEmptyParts, Qt::CaseInsensitive);
            Instruction i = Instruction(Eq);
            curMethod->Add(i, label, extraInfo);
        }
        else if(opcode == "ne")
        {
            QStringList two_labels = arg.split(",", QString::SkipEmptyParts, Qt::CaseInsensitive);
            Instruction i = Instruction(Ne);
            curMethod->Add(i, label, extraInfo);
        }
        else if(opcode == "ge")
        {
            QStringList two_labels = arg.split(",", QString::SkipEmptyParts, Qt::CaseInsensitive);
            Instruction i = Instruction(Ge);
            curMethod->Add(i, label, extraInfo);
        }
        else if(opcode == "le")
        {
            QStringList two_labels = arg.split(",", QString::SkipEmptyParts, Qt::CaseInsensitive);
            Instruction i = Instruction(Le);
            curMethod->Add(i, label, extraInfo);
        }
        else if(opcode == "call")
        {
            LoadCallInstruction(Call, arg, allocator, curMethod, label, extraInfo);
        }
        else if(opcode == "callr")
        {
            LoadCallInstruction(CallRef, arg, allocator, curMethod, label, extraInfo);
        }
        else if(opcode == "ret")
        {
            Instruction i = Instruction(Ret);
            curMethod->Add(i, label, extraInfo);
        }
        else if(opcode == "callex")
        {
            LoadCallInstruction(CallExternal, arg, allocator, curMethod, label, extraInfo);
        }
        else if(opcode == "nop")
        {
            Instruction i = Instruction(Nop);
            curMethod->Add(i, label, extraInfo);
        }
        else if(opcode == "setfld")
        {
            Instruction i = Instruction(SetField).wRef(arg);
            curMethod->Add(i, label, extraInfo);
        }
        else if(opcode == "getfld")
        {
            Instruction i = Instruction(GetField).wRef(arg);
            curMethod->Add(i, label, extraInfo);
        }
        else if(opcode == "getfldref")
        {
            Instruction i = Instruction(GetFieldRef).wRef(arg);
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
            LoadCallInstruction(CallMethod, arg, allocator, curMethod, label, extraInfo);
        }
        else if(opcode == "new")
        {
            Instruction i = Instruction(New).wRef(arg);
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
            Value *name = allocator.newString(new QString(evName));
            Instruction i = Instruction(RegisterEvent).wArg(name).wRef(procSymRef);
            curMethod->Add(i, label, extraInfo);
        }
        else
        {
            signal(UnrecognizedMnemonic,opcode,toStr(i));
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
        Value *classObj = (Value*) constantPool[parentName];
        ValueClass *parent = dynamic_cast<ValueClass *>(classObj->v.objVal);

        // In VM::Load we always initialize a class with a base of Value::ObjectType
        // now that we found a class's real parent, we don't need this default
        // value any more.
        // This code assumes only one base class at a time
        // TODO: either change BaseClasses from a vector to a single class
        // or remove the assumption in this code
        if(c->BaseClasses.contains(Value::ObjectType))
        {
            c->BaseClasses.clear();
        }
        c->BaseClasses.append(parent);
    }
}

void VM::DoPushVal(Value *Arg)
{
    currentFrame()->OperandStack.push(Arg);
}
void VM::DoPushLocal(QString SymRef)
{
    assert(currentFrame()->Locals.contains(SymRef), NoSuchVariable);
    currentFrame()->OperandStack.push(currentFrame()->Locals[SymRef]);
}
void VM::DoPushGlobal(QString SymRef)
{
    assert(globalFrame().Locals.contains(SymRef), NoSuchVariable);
    currentFrame()->OperandStack.push(globalFrame().Locals[SymRef]);
}
void VM::DoPushConstant(QString SymRef)
{
    currentFrame()->OperandStack.push(constantPool[SymRef]);
}
void VM::DoPopLocal(QString SymRef)
{
    Value *v = currentFrame()->OperandStack.pop();
    currentFrame()->Locals[SymRef] = v;
}
void VM::DoPopGlobal(QString SymRef)
{
    Value *v = currentFrame()->OperandStack.pop();
    globalFrame().Locals[SymRef] = v;
}
void VM::DoPushNull()
{
    currentFrame()->OperandStack.push(allocator.null());
}
void VM::DoGetRef()
{
    // ... ref => ... val
    Reference *ref = currentFrame()->OperandStack.pop()->unboxRef();
    currentFrame()->OperandStack.push(ref->Get());
}
void VM::DoSetRef()
{
    // ...ref val => ...
    Value *v = currentFrame()->OperandStack.pop();
    Reference *ref = currentFrame()->OperandStack.pop()->unboxRef();

    ref->Set(v);
}

int add_int(int a, int b) { return a + b;}
double add_double(double a, double b) { return a + b;}
QString *add_str(QString *a, QString *b)
{
    return new QString((*a) + (*b));
}
int sub_int(int a, int b) { return a - b;}
double sub_double(double a, double b) { return a - b;}
int mul_int(int a, int b) { return a * b;}
double mul_double(double a, double b) { return a * b;}
int div_int(int a, int b) { return a / b;}
double div_double(double a, double b) { return a / b;}

int _and(int a, int b) { return a && b;}
int _or(int a, int b) { return a || b;}
int _not(int a) { return !a;}

int lt_int(int a, int b) { return a<b;}
int lt_double(double a, double b) { return a<b;}
int lt_str(QString *a, QString *b)
{
    return (*a) < (*b);
}
int gt_int(int a, int b) { return a>b;}
int gt_double(double a, double b) { return a>b;}
int gt_str(QString *a, QString *b)
{
    return (*a) > (*b);
}
int le_int(int a, int b) { return a<=b;}
int le_double(double a, double b) { return a<=b;}
int le_str(QString *a, QString *b)
{
    return (*a) <= (*b);
}
int ge_int(int a, int b) { return a>=b;}
int ge_double(double a, double b) { return a>=b;}
int ge_str(QString *a, QString *b)
{
    return (*a) >= (*b);
}
int eq_int(int a, int b) { return a==b;}
int eq_double(double a, double b) { return a==b;}
int eq_obj(Object *a, Object *b){ return a==b;}
int eq_raw(void *a, void *b){ return a==b;}
int eq_str(QString *a, QString *b)
{
    QString *s1 = (QString *) a;
    QString *s2 = (QString *) b;
    return QString::compare(*s1, *s2, Qt::CaseSensitive) == 0;
}


int ne_int(int a, int b) { return a!=b;}
int ne_double(double a, double b) { return a!=b;}
int ne_obj(Object *a, Object *b) { return a!=b;}
int ne_raw(void *a, void *b) { return a!=b;}
int ne_str(QString*a, QString*b)
{
    QString *s1 = (QString *) a;
    QString *s2 = (QString *) b;
    return QString::compare(*s1, *s2, Qt::CaseSensitive) != 0;
}

void VM::DoAdd()
{
    BuiltInBinaryOp(add_int, add_double, add_str);
}
void VM::DoSub()
{
    NumericBinaryOp(sub_int, sub_double);
}
void VM::DoMul()
{
    NumericBinaryOp(mul_int, mul_double);
}
void VM::DoDiv()
{
    // can't convert till we can handle div by zero situation :(
    Value *v2 = currentFrame()->OperandStack.pop();
    Value *v1 = currentFrame()->OperandStack.pop();

    Value *v3 = _div(v1, v2);

    currentFrame()->OperandStack.push(v3);
}
void VM::DoNeg()
{
    Value *v1 = currentFrame()->OperandStack.pop();
    Value *v2 = NULL;

    assert(v1->tag == Int || v1->tag == Double, NumericOperationOnNonNumber);
    if(v1->tag == Int)
        v2 = allocator.newInt(-v1->unboxInt());
    if(v1->tag == Double)
        v2 = allocator.newDouble(-v1->unboxDouble());

    currentFrame()->OperandStack.push(v2);
}

void VM::DoAnd()
{
    BinaryLogicOp(_and);
}
void VM::DoOr()
{
    BinaryLogicOp(_or);
}
void VM::DoNot()
{
    UnaryLogicOp(_not);
}
void VM::DoJmp(QString label)
{
    currentFrame()->ip = currentFrame()->currentMethod->GetIp(label);
}
void VM::DoIf(QString trueLabel, QString falseLabel)
{
    Value *v = currentFrame()->OperandStack.pop();
    test(!(v->tag == Int && v->v.intVal == 0), trueLabel, falseLabel);
}
void VM::DoLt()
{
    BuiltInBinaryBoolOp(lt_int, lt_double, lt_str);
}
void VM::DoGt()
{
    BuiltInBinaryBoolOp(gt_int, gt_double , gt_str);
}
void VM::DoEq()
{
    BinaryBoolOp(eq_int, eq_double, eq_obj, eq_str, eq_raw);
}
void VM::DoNe()
{
    BinaryBoolOp(ne_int, ne_double, ne_obj, ne_str, ne_raw);
}
void VM::DoLe()
{
    BuiltInBinaryBoolOp(le_int, le_double, le_str);
}
void VM::DoGe()
{
    BuiltInBinaryBoolOp(ge_int, ge_double, ge_str);
}
void VM::DoCall(QString symRef, int arity)
{
    CallImpl(symRef, true, arity);
}
void VM::DoCallRef(QString symRef, int arity)
{
    CallImpl(symRef, false, arity);
}
void VM::CallImpl(QString symRef, bool wantValueNotRef, int arity)
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
    assert(constantPool.contains(symRef), NoSuchProcedureOrFunction, symRef);
    Method *method = (Method *) constantPool[symRef]->v.objVal;


    assert(arity == -1 || method->Arity() ==-1 || arity == method->Arity(), WrongNumberOfArguments);
    QVector<Value *> args;
    for(int i=0; i<method->Arity(); i++)
    {
        Value *v = currentFrame()->OperandStack.pop();
        args.append(v);
    }

    stack.push(Frame(method));
    currentFrame()->returnReferenceIfRefMethod = !wantValueNotRef;

    for(int i=args.count()-1; i>=0; i--)
    {
        Value *v = args[i];
        currentFrame()->OperandStack.push(v);
    }
}

void VM::DoCallMethod(QString SymRef, int arity)
{
    // callm expects the arguments in reverse order, and the last pushed argument is 'this'
    // but the execution site pops them in the correct order, i.e the first popped is 'this'
    // followed by the first normal argument...and so on.
    QVector<Value *> args;
    Value *receiver = currentFrame()->OperandStack.pop();

    assert(receiver ->tag == ObjectVal, CallingMethodOnNonObject);
    Method *method = receiver->type->lookupMethod(SymRef);

    assert(method!=NULL, NoSuchMethod,SymRef);
    assert(arity == -1 || method->Arity() ==-1 || arity == method->Arity(), WrongNumberOfArguments);

    args.append(receiver);

    for(int i=0; i<method->Arity()-1; i++)
    {
        Value *v = currentFrame()->OperandStack.pop();
        args.append(v);

    }

    stack.push(Frame(method));
    // When popped and pushed, the arguments
    // will be in the right order in the new frame
    // so e.g in calling x.print(a, b)
    // the new frame will have a stack like this:
    // [x, a , b |...]
    for(int i= args.count()-1; i>=0; i--)
    {
        Value *v = args[i];
        currentFrame()->OperandStack.push(v);
    }
}
void VM::DoRet()
{
    Value *v = NULL;
    int toReturn = currentFrame()->currentMethod->NumReturnValues();
    bool getReferredVal = currentFrame()->currentMethod->IsReturningReference() &&! currentFrame()->returnReferenceIfRefMethod;

    if(toReturn != -1)
    {
        if(toReturn == 1 && currentFrame()->OperandStack.empty())
            signal(FunctionDidntReturnAValue);
        else if(toReturn != currentFrame()->OperandStack.count())
            signal(InternalError);
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
void VM::DoCallExternal(QString symRef, int arity)
{
    assert(constantPool.contains(symRef), NoSuchExternalMethod, symRef);

    ExternalMethod *method = (ExternalMethod*) constantPool[symRef]->v.objVal;
    assert(arity == -1 || method->Arity() ==-1 || arity == method->Arity(), WrongNumberOfArguments);
    (*method)(currentFrame()->OperandStack);
}
void VM::DoSetField(QString SymRef)
{
    // ...obj val  => ...
    Value *v = currentFrame()->OperandStack.pop();
    Value *objVal = currentFrame()->OperandStack.pop();


    assert(objVal->tag == ObjectVal, SettingFieldOnNonObject);
    assert(objVal->type->fields.contains(SymRef), NoSuchField, SymRef);

    Object *obj = objVal->unboxObj();
    obj->setSlotValue(SymRef, v);
}
void VM::DoGetField(QString SymRef)
{
    // ...object => val
    Value *objVal = currentFrame()->OperandStack.pop();

    assert(objVal->tag == ObjectVal, GettingFieldOnNonObject);
    assert(objVal->type->fields.contains(SymRef), NoSuchField, SymRef);

    Object *obj = objVal->v.objVal;
    Value *v = obj->getSlotValue(SymRef);
    currentFrame()->OperandStack.push(v);
}
void VM::DoGetFieldRef(QString SymRef)
{
    // ...object => ...fieldRef
    Value *objVal = currentFrame()->OperandStack.pop();
    assert(objVal->tag == ObjectVal, GettingFieldOnNonObject);
    assert(objVal->type->fields.contains(SymRef), NoSuchField, SymRef);
    Value *ref = allocator.newFieldReference(objVal->unboxObj(), SymRef);
    currentFrame()->OperandStack.push(ref);
}
void VM::DoSetArr()
{
    // ...arr index val => ...
    Value *v = currentFrame()->OperandStack.pop();
    Value *index = currentFrame()->OperandStack.pop();
    Value *arrVal = currentFrame()->OperandStack.pop();

    assert(arrVal->tag == ArrayVal, SubscribingNonArray);
    assert(index->tag == Int, SubscribtMustBeInteger);
    int i = index->unboxInt();
    VArray *arr = arrVal->unboxArray();

    assert(i>=1 && i<=arr->count, SubscriptOutOfRange);

    arr->Elements[i-1] = v;

}
void VM::DoGetArr()
{
    // ...arr index => ...value
    Value *index = currentFrame()->OperandStack.pop();
    Value *arrVal= currentFrame()->OperandStack.pop();

    assert(arrVal->tag == ArrayVal, SubscribingNonArray);
    assert(index->tag == Int, SubscribtMustBeInteger);
    int i = index->unboxInt();
    VArray *arr = arrVal->unboxArray();

    assert(i>=1 && i<=arr->count, SubscriptOutOfRange);

    Value *v = arr->Elements[i-1];
    currentFrame()->OperandStack.push(v);
}
void VM::DoGetArrRef()
{
    // ...arr index => ...arrref
    Value *index  = currentFrame()->OperandStack.pop();
    Value *arrVal= currentFrame()->OperandStack.pop();



    assert(arrVal->tag == ArrayVal, SubscribingNonArray);
    assert(index->tag == Int, SubscribtMustBeInteger);
    int i = index->unboxInt();
    VArray *arr = arrVal->unboxArray();

    assert(i>=1 && i<=arr->count, SubscriptOutOfRange);

    Value *ref = allocator.newArrayReference(arr, i-1);
    currentFrame()->OperandStack.push(ref);
}

void VM::DoNew(QString SymRef)
{
    assert(constantPool.contains(SymRef), NoSuchClass, SymRef);
    Value *classObj = (Value*) constantPool[SymRef];
    ValueClass *theClass = dynamic_cast<ValueClass *>(classObj->v.objVal);
    assert(theClass != NULL, NameDoesntIndicateAClass, SymRef);
    Value *newObj = allocator.newObject(theClass);
    currentFrame()->OperandStack.push(newObj);
}
void VM::DoNewArr()
{
    assert(__top()->tag == Int, TypeError);
    int size = currentFrame()->OperandStack.pop()->unboxInt();

    Value *newArr = allocator.newArray(size);
    currentFrame()->OperandStack.push(newArr);
}

void VM::DoArrLength()
{
    // ... arr => ... length
    assert(__top()->tag == ArrayVal, TypeError);
    Value *arrVal= currentFrame()->OperandStack.pop();
    VArray *arr = arrVal->unboxArray();
    Value *len = allocator.newInt(arr->count);
    currentFrame()->OperandStack.push(len);
}
void VM::DoNewMD_Arr()
{
    // ... dimensions => ... md_arr
    assert(__top()->tag == ArrayVal, TypeError);
    Value *arrVal= currentFrame()->OperandStack.pop();
    VArray *arr = arrVal->unboxArray();
    QVector<int> dimensions;
    for(int i=0; i<arr->count; i++)
    {
        assert(arr->Elements[i]->tag == Int, TypeError, "Multidimensional array index must be an integer");
        dimensions.append(arr->Elements[i]->unboxInt());
    }
    Value *mdarr = allocator.newMultiDimensionalArray(dimensions);
    currentFrame()->OperandStack.push(mdarr);
}
void VM::Pop_Md_Arr_and_indexes(MultiDimensionalArray<Value *> *&theArray, QVector<int> &indexes)
{

    // ... arr index => ...
    assert(__top()->tag == ArrayVal, TypeError);
    Value *arrVal= currentFrame()->OperandStack.pop();
    VArray *boxedIndexes= arrVal->unboxArray();

    assert(__top()->tag == MultiDimensionalArrayVal, TypeError);
    Value *md_arrVal= currentFrame()->OperandStack.pop();
    theArray= md_arrVal->unboxMultiDimensionalArray();



    assert(theArray->dimensions.count()== boxedIndexes->count, MD_IndexingWrongNumberOfDimensions);

    for(int i=0; i<boxedIndexes->count; i++)
    {
        assert(boxedIndexes->Elements[i]->tag == Int, TypeError, "Multidimensional array index must be an integer");
        int n = boxedIndexes->Elements[i]->unboxInt();
        assert(n>=1 && n<=theArray->dimensions[i], SubscriptOutOfRange, QString("%1 of 1..%2").arg(n).arg(theArray->dimensions[i]));
        indexes.append(n-1); // We're one-based, remember
    }
}

void VM::DoGetMD_Arr()
{
    // ... md_arr indexes => ... value
    MultiDimensionalArray<Value *> *theArray;
    QVector<int> indexes;

    Pop_Md_Arr_and_indexes(theArray, indexes);
    Value *v = theArray->get(indexes);
    currentFrame()->OperandStack.push(v);
}

void VM::DoSetMD_Arr()
{
    // ... md_arr indexes value => ...
    MultiDimensionalArray<Value *> *theArray;
    QVector<int> indexes;

    Value *v = currentFrame()->OperandStack.pop();
    Pop_Md_Arr_and_indexes(theArray, indexes);


    theArray->set(indexes, v);
}

void VM::DoGetMD_ArrRef()
{
    // ... md_arr indexes => ... md_arrref
    MultiDimensionalArray<Value *> *theArray;
    QVector<int> indexes;

    Pop_Md_Arr_and_indexes(theArray, indexes);
    Value *v = allocator.newMultiDimensionalArrayReference(theArray, indexes);

    currentFrame()->OperandStack.push(v);
}

void VM::DoMD_ArrDimensions()
{
    // ... md_arr => ... dimensions
    assert(__top()->tag == MultiDimensionalArrayVal, TypeError);
    Value *arrVal= currentFrame()->OperandStack.pop();
    MultiDimensionalArray<Value *> *arr = arrVal->unboxMultiDimensionalArray();
    Value *v = allocator.newArray(arr->dimensions.count());
    VArray *internalArr = v->v.arrayVal;
    for(int i=0; i<arr->dimensions.count(); i++)
    {

        internalArr->Elements[i] = allocator.newInt(arr->dimensions[i]);
    }

    currentFrame()->OperandStack.push(v);
}

void VM::DoRegisterEvent(Value *evname, QString SymRef)
{
    assert(evname->type == Value::StringType, TypeError);
    assert(constantPool.contains(SymRef), NoSuchProcedure);
    QString *evName = evname->unboxStr();
    registeredEventHandlers[*evName] = SymRef;
}

void VM::test(bool cond, QString trueLabel, QString falseLabel)
{
    int newIp;
    if(cond)
        newIp = currentFrame()->currentMethod->GetIp(trueLabel);
    else
        newIp = currentFrame()->currentMethod->GetIp(falseLabel);

    currentFrame()->ip = newIp;
}
void VM::BuiltInBinaryOp(int (*intFunc)(int,int), double (*doubleFunc)(double,double), QString *(*strFunc)(QString *,QString *))
{
    Value *v2 = currentFrame()->OperandStack.pop();
    Value *v1 = currentFrame()->OperandStack.pop();

    if (v1->tag == Int && v2->tag == Double)
    {
        v1 = allocator.newDouble(v1->v.intVal);
    }
    else if(v1->tag == Double && v2->tag == Int)
    {
        v2 = allocator.newDouble(v2->v.intVal);
    }

    assert( v1->tag == v2->tag &&
            (v1->tag == Double || v1->tag == Int || v1->tag == StringVal), BuiltInOperationOnNonBuiltn);

    Value *v3 = NULL;

    if(v1->tag == StringVal)
        v3 = allocator.newString(strFunc(v1->unboxStr(), v2->unboxStr()));
    else if(v1->tag == Int)
        v3 = allocator.newInt(intFunc(v1->unboxInt(), v2->unboxInt()));
    else if(v1->tag == Double )
        v3 = allocator.newDouble(doubleFunc(v1->unboxDouble(), v2->unboxDouble()));

    currentFrame()->OperandStack.push(v3);
}

void VM::NumericBinaryOp(int (*intFunc)(int,int), double (*doubleFunc)(double,double))
{
    Value *v2 = currentFrame()->OperandStack.pop();
    Value *v1 = currentFrame()->OperandStack.pop();

    if (v1->tag == Int && v2->tag == Double)
    {
        v1 = allocator.newDouble(v1->v.intVal);
    }
    else if(v1->tag == Double && v2->tag == Int)
    {
        v2 = allocator.newDouble(v2->v.intVal);
    }

    assert((v1->tag == v2->tag) && (v1->tag == Int || v1->tag == Double), NumericOperationOnNonNumber);

    Value *v3 = NULL;

    if(v1->tag == Int && v2->tag == Int)
        v3 = allocator.newInt(intFunc(v1->unboxInt(), v2->unboxInt()));
    else if(v1->tag == Double && v2->tag == Double)
        v3 = allocator.newDouble(doubleFunc(v1->unboxDouble(), v2->unboxDouble()));

    currentFrame()->OperandStack.push(v3);
}
void VM::BuiltInBinaryBoolOp(int (*intFunc)(int,int), int (*doubleFunc)(double,double),int (*strFunc)(QString *, QString *))
{
    Value *v2 = currentFrame()->OperandStack.pop();
    Value *v1 = currentFrame()->OperandStack.pop();

    if (v1->tag == Int && v2->tag == Double)
    {
        v1 = allocator.newDouble(v1->v.intVal);
    }
    else if(v1->tag == Double && v2->tag == Int)
    {
        v2 = allocator.newDouble(v2->v.intVal);
    }

    assert(v1->tag == v2->tag && (v1->tag == Int || v1->tag == Double || v1->tag == StringVal), BuiltInOperationOnNonBuiltn);

    Value *v3 = NULL;
    int result;

    if(v1->tag == Int)
        result = intFunc(v1->v.intVal, v2->v.intVal);
    else if(v1->tag == Double)
        result = doubleFunc(v1->v.doubleVal, v2->v.doubleVal);
    else if(v1->tag == StringVal)
        result = strFunc(v1->v.strVal, v2->v.strVal);

    v3 = allocator.newInt(result);
    currentFrame()->OperandStack.push(v3);
}
void VM::BinaryBoolOp(int (*intFunc)(int,int),
                      int (*doubleFunc)(double,double),
                      int (*objFunc)(Object *, Object *),
                      int (*strFunc)(QString *, QString *),
                      int (*rawFunc)(void *, void *))
{
    Value *v2 = currentFrame()->OperandStack.pop();
    Value *v1 = currentFrame()->OperandStack.pop();

    if (v1->tag == Int && v2->tag == Double)
    {
        v1 = allocator.newDouble(v1->v.intVal);
    }
    else if(v1->tag == Double && v2->tag == Int)
    {
        v2 = allocator.newDouble(v2->v.intVal);
    }
    assert(v1->tag == v2->tag
           && (v1->tag == Int || v1->tag == Double || v1->tag == ObjectVal || v1->tag == StringVal || v1->tag == ObjectVal || v1->tag == RawVal), TypeError);

    Value *v3 = NULL;
    int result;

    if(v1->tag == Int)
        result = intFunc(v1->v.intVal, v2->v.intVal);
    if(v1->tag == Double)
        result = doubleFunc(v1->v.doubleVal, v2->v.doubleVal);
    if(v1->tag == ObjectVal)
        result = objFunc(v1->v.objVal, v2->v.objVal);
    if(v1->tag == StringVal)
        result = strFunc(v1->unboxStr(), v2->unboxStr());
    if(v1->tag == RawVal)
        result = rawFunc(v1->unboxRaw(), v2->unboxRaw());
    v3 = allocator.newInt(result);
    currentFrame()->OperandStack.push(v3);
}
void VM::BinaryLogicOp(int (*intFunc)(int,int))
{
    Value *v2 = currentFrame()->OperandStack.pop();
    Value *v1 = currentFrame()->OperandStack.pop();
    Value *v3 = NULL;

    assert(v1->tag == Int && v2->tag == Int, LogicOperationOnNonBoolean);
    v3 =allocator.newInt(intFunc(v1->v.intVal, v2->v.intVal));

    currentFrame()->OperandStack.push(v3);
}
void VM::UnaryLogicOp(int (*intFunc)(int))
{
    Value *v1 = currentFrame()->OperandStack.pop();
    Value *v2 = NULL;

    assert(v1->tag == Int, LogicOperationOnNonBoolean);
    v2 =allocator.newInt(intFunc(v1->v.intVal));

    currentFrame()->OperandStack.push(v2);
}
Value *VM::_div(Value *v1, Value *v2)
{
    if (v1->tag == Int && v2->tag == Double)
    {
        v1 = allocator.newDouble(v1->v.intVal);
    }
    else if(v1->tag == Double && v2->tag == Int)
    {
        v2 = allocator.newDouble(v2->v.intVal);
    }
    assert((v1->tag == v2->tag) &&( v1->tag == Double || v1->tag == Int ), NumericOperationOnNonNumber);

    if(v1->tag == Int && v2->tag == Int)
    {
        assert(v2->v.intVal != 0, DivisionByZero);
        double result = ((double)(v1->unboxInt())) / ((double) (v2->unboxInt()));
        return allocator.newDouble(result);
    }
    else if(v1->tag == Double && v2->tag == Double)
    {
        assert(v2->v.doubleVal != 0.0, DivisionByZero);
        return allocator.newDouble(v1->v.doubleVal / v2->v.doubleVal);
    }
    return NULL;
}
Value *VM::__top()
{
    assert(!stack.empty(), InternalError);
    return currentFrame()->OperandStack.top();
}
bool VM::isRunning()
{
    return _isRunning;
}
