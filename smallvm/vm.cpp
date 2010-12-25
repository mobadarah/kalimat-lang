/**************************************************************************
**   The Kalimat programming language
**   Copyright (C) 2010 Mohamed Samy Ali - samy2004@gmail.com
**   This project is released under the Apache License version 2.0
**   as described in the included license.txt file
**************************************************************************/

#include "vm_incl.h"
#include "vm.h"

#define QSTR(x) QString::fromStdWString(x)

QString str(int n)
{
    return QString("%1").arg(n);
}

VM::VM()
    :allocator(&constantPool, &stack)
{
}

void VM::Init()
{
    if(!constantPool.contains("main"))
        signal(InternalError,QSTR(L"لا يوجد برنامج أساسي لينفّذ"));
    Method *method = dynamic_cast<Method *>(constantPool["main"]->unboxObj());
    QString malaf = "%file";
    if(constantPool.contains(malaf))
        BuiltInTypes::FileType = (ValueClass *) constantPool[malaf]->unboxObj();
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

void VM::assert(bool cond, VMErrorType toSignal, ValueClass *arg0, ValueClass *arg1)
{
    assert(cond, toSignal, arg0->getName(), arg1->getName());
}

void VM::assert(bool cond, VMErrorType toSignal, QString arg0, QString arg1, QString arg2)
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
    _lastError = VMError(err, stack);
    throw _lastError;
}
void VM::signal(VMErrorType err, QString arg0)
{
    _isRunning = false;
    _lastError = VMError(err, stack).arg(arg0);
    throw _lastError;
}

void VM::signal(VMErrorType err, QString arg0, QString arg1)
{
    _isRunning = false;
    _lastError = VMError(err, stack).arg(arg0).arg(arg1);
    throw _lastError;
}

void VM::signal(VMErrorType err, QString arg0, QString arg1, QString arg2)
{
    _isRunning = false;
    _lastError = VMError(err, stack).arg(arg0).arg(arg1).arg(arg2);
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
    constantPool[symRef] = allocator.newObject(method, BuiltInTypes::ExternalMethodType);
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
    DoCall(procName, args.count(), false);
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
        this->signal(TypeError2, QString::fromStdWString(L"عدد"), v->type->getName());
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
        this->signal(TypeError2, QString::fromStdWString(L"عدد"), v->type->getName());
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
        this->DoCall(i.SymRef, getInstructionArity(i), i.tailCall);
        break;
    case CallRef:
        this->DoCallRef(i.SymRef, getInstructionArity(i), i.tailCall);
        break;
    case CallMethod:
        this->DoCallMethod(i.SymRef, getInstructionArity(i), i.tailCall);
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
    case Isa:
        this->DoIsa(i.SymRef);
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
void VM::LoadCallInstruction(Opcode type, QString arg, Allocator &allocator,
                             Method *method, QString label, int extraInfo, bool tailCall)
{
    if(arg.contains(","))
    {
        QStringList ref_arity= arg.split(",", QString::SkipEmptyParts, Qt::CaseInsensitive);
        ref_arity[0] = ref_arity[0].trimmed();
        ref_arity[1] = ref_arity[1].trimmed();
        Instruction i = Instruction(type)
                        .wRef(ref_arity[0])
                        .wArgParse(ref_arity[1], &allocator)
                        .wTailCall(tailCall);
        method->Add(i, label, extraInfo);
    }
    else
    {
        Instruction i = Instruction(type).wRef(arg).wTailCall(tailCall);
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
    bool tailCall = false;
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
            Value *methodVal = allocator.newObject(curMethod, BuiltInTypes::MethodType);
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
            curClass = new ValueClass(curClassName, BuiltInTypes::ObjectType);
            if(constantPool.contains(curClassName))
                signal(ElementAlreadyDefined, curClassName);
            else
                constantPool[curClassName] = allocator.newObject(curClass, BuiltInTypes::ClassType);
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
            tailCall = true;
        }
        else if(opcode == "call")
        {
            LoadCallInstruction(Call, arg, allocator, curMethod, label, extraInfo, tailCall);
            tailCall = false;
        }
        else if(opcode == "callr")
        {
            LoadCallInstruction(CallRef, arg, allocator, curMethod, label, extraInfo, tailCall);
            tailCall = false;
        }
        else if(opcode == "ret")
        {
            Instruction i = Instruction(Ret);
            curMethod->Add(i, label, extraInfo);
        }
        else if(opcode == "callex")
        {
            LoadCallInstruction(CallExternal, arg, allocator, curMethod, label, extraInfo, false);
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
            LoadCallInstruction(CallMethod, arg, allocator, curMethod, label, extraInfo, tailCall);
            tailCall = false;
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
        else if(opcode == "isa")
        {
            Instruction i = Instruction(Isa).wRef(arg);
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

void VM::DoPushVal(Value *Arg)
{
    currentFrame()->OperandStack.push(Arg);
}

void VM::DoPushLocal(QString SymRef)
{
    assert(currentFrame()->Locals.contains(SymRef), NoSuchVariable1, SymRef);
    currentFrame()->OperandStack.push(currentFrame()->Locals[SymRef]);
}

void VM::DoPushGlobal(QString SymRef)
{
    assert(globalFrame().Locals.contains(SymRef), NoSuchVariable1, SymRef);
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
QString *add_str(QString *a, QString *b){ return new QString((*a) + (*b));}

int sub_int(int a, int b) { return a - b;}
double sub_double(double a, double b) { return a - b;}

int mul_int(int a, int b) { return a * b;}
double mul_double(double a, double b) { return a * b;}

int div_int(int a, int b) { return a / b;}
double div_double(double a, double b) { return a / b;}

bool _and(bool a, bool b) { return a && b;}
bool _or(bool a, bool b) { return a || b;}
bool _not(bool a) { return !a;}

bool lt_int(int a, int b) { return a<b;}
bool lt_double(double a, double b) { return a<b;}
bool lt_str(QString *a, QString *b) { return (*a) < (*b); }

bool gt_int(int a, int b) { return a>b;}
bool gt_double(double a, double b) { return a>b;}
bool gt_str(QString *a, QString *b) { return (*a) > (*b); }

bool le_int(int a, int b) { return a<=b;}
bool le_double(double a, double b) { return a<=b;}
bool le_str(QString *a, QString *b) { return (*a) <= (*b); }

bool ge_int(int a, int b) { return a>=b;}
bool ge_double(double a, double b) { return a>=b;}
bool ge_str(QString *a, QString *b) { return (*a) >= (*b); }

bool  eq_int(int a, int b) { return a==b;}
bool  eq_double(double a, double b) { return a==b;}
bool  eq_bool(bool a, bool b) { return a==b;}
bool  eq_obj(Object *a, Object *b){ return a==b;}
bool  eq_raw(void *a, void *b){ return a==b;}

bool  eq_str(QString *a, QString *b)
{
    QString *s1 = (QString *) a;
    QString *s2 = (QString *) b;
    return QString::compare(*s1, *s2, Qt::CaseSensitive) == 0;
}

bool  eq_difftypes(Value *, Value *)
{
    return false;
}

bool  eq_bothnull() { return true;}

bool ne_int(int a, int b) { return a!=b;}
bool  ne_double(double a, double b) { return a!=b;}
bool  ne_bool(bool a, bool b) { return a!=b;}
bool  ne_obj(Object *a, Object *b) { return a!=b;}
bool  ne_raw(void *a, void *b) { return a!=b;}

bool  ne_str(QString*a, QString*b)
{
    QString *s1 = (QString *) a;
    QString *s2 = (QString *) b;
    return QString::compare(*s1, *s2, Qt::CaseSensitive) != 0;
}

bool  ne_difftypes(Value *, Value *)
{
    return true;
}
bool  ne_bothnull() { return false; }

void VM::DoAdd()
{
    BuiltInAddOp(add_int, add_double, add_str);
}

void VM::DoSub()
{
    BuiltInArithmeticOp(QSTR(L"طرح"), sub_int, sub_double);
}

void VM::DoMul()
{
    BuiltInArithmeticOp(QSTR(L"ضرب"), mul_int, mul_double);
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

    assert(v1->tag == Int || v1->tag == Double, NumericOperationOnNonNumber1, "-");
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
    assert(v->tag == Boolean, TypeError2, BuiltInTypes::BoolType, v->type);
    test(v->unboxBool(), trueLabel, falseLabel);
}

void VM::DoLt()
{
    BuiltInComparisonOp(lt_int, lt_double, lt_str);
}

void VM::DoGt()
{
    BuiltInComparisonOp(gt_int, gt_double , gt_str);
}

void VM::DoEq()
{
    EqualityRelatedOp(eq_int, eq_double, eq_bool, eq_obj, eq_str, eq_raw, eq_difftypes, eq_bothnull);
}

void VM::DoNe()
{
    EqualityRelatedOp(ne_int, ne_double, ne_bool, ne_obj, ne_str, ne_raw, ne_difftypes, ne_bothnull);
}

void VM::DoLe()
{
    BuiltInComparisonOp(le_int, le_double, le_str);
}

void VM::DoGe()
{
    BuiltInComparisonOp(ge_int, ge_double, ge_str);
}

void VM::DoCall(QString symRef, int arity, bool tailCall)
{
    CallImpl(symRef, true, arity, tailCall);
}

void VM::DoCallRef(QString symRef, int arity, bool tailCall)
{
    CallImpl(symRef, false, arity, tailCall);
}

void VM::CallImpl(QString symRef, bool wantValueNotRef, int arity, bool tailCall)
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
    assert(constantPool.contains(symRef), NoSuchProcedureOrFunction1, symRef);
    Method *method = (Method *) constantPool[symRef]->v.objVal;


    assert(arity == -1 || method->Arity() ==-1 || arity == method->Arity(), WrongNumberOfArguments);
    QVector<Value *> args;
    for(int i=0; i<method->Arity(); i++)
    {
        Value *v = currentFrame()->OperandStack.pop();
        args.append(v);
    }

    if(tailCall)
    {
        stack.pop();
    }

    stack.push(Frame(method));
    currentFrame()->returnReferenceIfRefMethod = !wantValueNotRef;

    for(int i=args.count()-1; i>=0; i--)
    {
        Value *v = args[i];
        currentFrame()->OperandStack.push(v);
    }
}

void VM::DoCallMethod(QString SymRef, int arity, bool tailCall)
{
    // callm expects the arguments in reverse order, and the last pushed argument is 'this'
    // but the execution site pops them in the correct order, i.e the first popped is 'this'
    // followed by the first normal argument...and so on.
    Value *receiver = currentFrame()->OperandStack.pop();

    assert(receiver->tag != NullVal, CallingMethodOnNull);
    assert(receiver ->tag == ObjectVal, CallingMethodOnNonObject);
    Method *method = receiver->type->lookupMethod(SymRef);

    assert(method!=NULL, NoSuchMethod2,SymRef, receiver->type->getName());
    assert(arity == -1 || method->Arity() ==-1 || arity == method->Arity(), WrongNumberOfArguments);

    QVector<Value *> args;
    args.append(receiver);

    for(int i=0; i<method->Arity()-1; i++)
    {
        Value *v = currentFrame()->OperandStack.pop();
        args.append(v);
    }

    if(tailCall)
    {
        stack.pop();
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
    assert(constantPool.contains(symRef), NoSuchExternalMethod1, symRef);

    ExternalMethod *method = (ExternalMethod*) constantPool[symRef]->v.objVal;
    assert(arity == -1 || method->Arity() ==-1 || arity == method->Arity(), WrongNumberOfArguments);
    (*method)(currentFrame()->OperandStack);
}

void VM::DoSetField(QString SymRef)
{
    // ...obj val  => ...
    Value *v = currentFrame()->OperandStack.pop();
    Value *objVal = currentFrame()->OperandStack.pop();

    assert(objVal->tag != NullVal, SettingFieldOnNull);
    assert(objVal->tag == ObjectVal, SettingFieldOnNonObject);
    assert(objVal->type->fields.contains(SymRef), NoSuchField2, SymRef, objVal->type->getName());

    Object *obj = objVal->unboxObj();
    obj->setSlotValue(SymRef, v);
}

void VM::DoGetField(QString SymRef)
{
    // ...object => val
    Value *objVal = currentFrame()->OperandStack.pop();

    assert(objVal->tag != NullVal, GettingFieldOnNull);
    assert(objVal->tag == ObjectVal, GettingFieldOnNonObject);
    assert(objVal->type->fields.contains(SymRef), NoSuchField2, SymRef, objVal->type->getName());

    Object *obj = objVal->v.objVal;
    Value *v = obj->getSlotValue(SymRef);
    currentFrame()->OperandStack.push(v);
}

void VM::DoGetFieldRef(QString SymRef)
{
    // ...object => ...fieldRef
    Value *objVal = currentFrame()->OperandStack.pop();

    assert(objVal->tag != NullVal, GettingFieldOnNull);
    assert(objVal->tag == ObjectVal, GettingFieldOnNonObject);
    assert(objVal->type->fields.contains(SymRef), NoSuchField2, SymRef, objVal->type->getName());
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

    assert(i>=1 && i<=arr->count, SubscriptOutOfRange2, str(i), str(arr->count));

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

    assert(i>=1 && i<=arr->count, SubscriptOutOfRange2, str(i), str(arr->count));

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

    assert(i>=1 && i<=arr->count, SubscriptOutOfRange2, str(i), str(arr->count));

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
    assert(__top()->tag == Int, TypeError2, BuiltInTypes::IntType, __top()->type);
    int size = currentFrame()->OperandStack.pop()->unboxInt();

    Value *newArr = allocator.newArray(size);
    currentFrame()->OperandStack.push(newArr);
}

void VM::DoArrLength()
{
    // ... arr => ... length
    assert(__top()->tag == ArrayVal, TypeError2, BuiltInTypes::ArrayType, __top()->type);
    Value *arrVal= currentFrame()->OperandStack.pop();
    VArray *arr = arrVal->unboxArray();
    Value *len = allocator.newInt(arr->count);
    currentFrame()->OperandStack.push(len);
}

void VM::DoNewMD_Arr()
{
    // ... dimensions => ... md_arr
    assert(__top()->tag == ArrayVal, TypeError2, BuiltInTypes::ArrayType, __top()->type);
    Value *arrVal= currentFrame()->OperandStack.pop();
    VArray *arr = arrVal->unboxArray();
    QVector<int> dimensions;
    for(int i=0; i<arr->count; i++)
    {
        assert(arr->Elements[i]->tag == Int, TypeError2, BuiltInTypes::IntType, arr->Elements[i]->type);
        dimensions.append(arr->Elements[i]->unboxInt());
    }
    Value *mdarr = allocator.newMultiDimensionalArray(dimensions);
    currentFrame()->OperandStack.push(mdarr);
}

void VM::Pop_Md_Arr_and_indexes(MultiDimensionalArray<Value *> *&theArray, QVector<int> &indexes)
{

    // ... arr index => ...
    assert(__top()->tag == ArrayVal, TypeError2, BuiltInTypes::ArrayType, __top()->type);
    Value *arrVal= currentFrame()->OperandStack.pop();
    VArray *boxedIndexes= arrVal->unboxArray();

    assert(__top()->tag == MultiDimensionalArrayVal, TypeError2, QString::fromStdWString(L"مصفوفة متعددة"), __top()->type->getName());
    Value *md_arrVal= currentFrame()->OperandStack.pop();
    theArray= md_arrVal->unboxMultiDimensionalArray();



    assert(theArray->dimensions.count()== boxedIndexes->count, MD_IndexingWrongNumberOfDimensions);

    for(int i=0; i<boxedIndexes->count; i++)
    {
        assert(boxedIndexes->Elements[i]->tag == Int, TypeError2, BuiltInTypes::IntType, boxedIndexes->Elements[i]->type);
        int n = boxedIndexes->Elements[i]->unboxInt();
        assert(n>=1 && n<=theArray->dimensions[i], SubscriptOutOfRange3, str(i+1), str(n), str(theArray->dimensions[i]));
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
    assert(__top()->tag == MultiDimensionalArrayVal, TypeError2, BuiltInTypes::ArrayType, __top()->type);
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
    assert(evname->type == BuiltInTypes::StringType, TypeError2, BuiltInTypes::StringType, evname->type);
    assert(constantPool.contains(SymRef), NoSuchProcedure1, SymRef);
    QString *evName = evname->unboxStr();
    registeredEventHandlers[*evName] = SymRef;
}

void VM::DoIsa(QString SymRef)
{
    // ...value => ...bool
    Value *v = currentFrame()->OperandStack.pop();
    ValueClass *cls = NULL;
    Value *classObj = NULL;
    if(constantPool.contains(SymRef))
    {
        classObj = constantPool[SymRef];
        if(classObj->type == BuiltInTypes::ClassType)
        {
            cls = dynamic_cast<ValueClass *>(classObj->unboxObj());
        }
    }
    assert(cls != NULL, NoSuchClass);
    bool b = v->type->subclassOf(cls);
    currentFrame()->OperandStack.push(allocator.newBool(b));
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

void VM::BuiltInAddOp(int (*intFunc)(int,int), double (*doubleFunc)(double,double), QString *(*strFunc)(QString *,QString *))
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

void VM::BuiltInArithmeticOp(QString opName, int (*intFunc)(int,int), double (*doubleFunc)(double,double))
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

    assert((v1->tag == v2->tag) && (v1->tag == Int || v1->tag == Double), NumericOperationOnNonNumber1 , opName);

    Value *v3 = NULL;

    if(v1->tag == Int && v2->tag == Int)
        v3 = allocator.newInt(intFunc(v1->unboxInt(), v2->unboxInt()));
    else if(v1->tag == Double && v2->tag == Double)
        v3 = allocator.newDouble(doubleFunc(v1->unboxDouble(), v2->unboxDouble()));

    currentFrame()->OperandStack.push(v3);
}

void VM::BuiltInComparisonOp(bool (*intFunc)(int,int),
                             bool (*doubleFunc)(double,double),
                             bool (*strFunc)(QString *, QString *))
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
    bool result;

    if(v1->tag == Int)
        result = intFunc(v1->v.intVal, v2->v.intVal);
    else if(v1->tag == Double)
        result = doubleFunc(v1->v.doubleVal, v2->v.doubleVal);
    else if(v1->tag == StringVal)
        result = strFunc(v1->v.strVal, v2->v.strVal);

    v3 = allocator.newBool(result);
    currentFrame()->OperandStack.push(v3);
}

void VM::EqualityRelatedOp(bool(*intFunc)(int, int),
                           bool(*doubleFunc)(double, double),
                           bool(*boolFunc)(bool, bool),
                           bool(*objFunc)(Object *, Object *),
                           bool(*strFunc)(QString *, QString *),
                           bool(*rawFunc)(void *, void *),
                           bool(*differentTypesFunc)(Value *, Value *),
                           bool(*nullFunc)())
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
    Value *v3 = NULL;
    bool result;

    // If both values are of the same type...
    if(v1->tag == v2->tag
           && (v1->tag == Int || v1->tag == Double || v1->tag == Boolean || v1->tag == ObjectVal
               || v1->tag == StringVal || v1->tag == ObjectVal || v1->tag == RawVal || v1->tag == NullVal))

    {
        if(v1->tag == Int)
            result = intFunc(v1->v.intVal, v2->v.intVal);
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
    v3 = allocator.newBool(result);
    currentFrame()->OperandStack.push(v3);
}

void VM::BinaryLogicOp(bool (*boolFunc)(bool, bool))
{
    Value *v2 = currentFrame()->OperandStack.pop();
    Value *v1 = currentFrame()->OperandStack.pop();
    Value *v3 = NULL;

    assert(v1->tag == Boolean && v2->tag == Boolean, LogicOperationOnNonBoolean);
    v3 = allocator.newBool(boolFunc(v1->unboxBool(), v2->unboxBool()));

    currentFrame()->OperandStack.push(v3);
}

void VM::UnaryLogicOp(bool(*boolFunc)(bool))
{
    Value *v1 = currentFrame()->OperandStack.pop();
    Value *v2 = NULL;

    assert(v1->tag == Boolean, LogicOperationOnNonBoolean);
    v2 = allocator.newBool(boolFunc(v1->unboxBool()));

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
    assert((v1->tag == v2->tag) &&( v1->tag == Double || v1->tag == Int ), NumericOperationOnNonNumber1, QSTR(L"قسمة"));

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
