#include "parserengine.h"
#include "../utils.h"
#include "../runtime_identifiers.h"

void ParserObj::setSlotValue(QString name, Value *val)
{
    if(name == VMId::get(RId::Data))
    {
        this->data = val;
    }
    if(name == VMId::get(RId::InputPos))
    {
        //todo:typecheck
        this->pos = unboxInt(val);
    }
    if(name == _ws(L"value_stack"))
    {
        this->valueStack = val;
    }
    Object::setSlotValue(name,val);
}

Value *ParserObj::getSlotValue(QString name)
{
    // first got to synchonize slots
    // with (possibly internally changed) fields
    if(name == VMId::get(RId::Data))
    {
        Object::setSlotValue(name, data);
    }
    if(name == VMId::get(RId::InputPos))
    {
        Object::setSlotValue(name, allocator->newInt(pos));
    }
    if(name == _ws(L"value_stack"))
    {

    }
    return Object::getSlotValue(name);
}

bool ParserObj::hasInfiniteRecursion(int label)
{
    // We traverse the stack, top to bottom,
    // if we meet a frame that is the same label (i.e same rule)
    // while the input pos hasn't moved
    // then it's infinite recursion!!
    // todo: imp: we haven't dealt yet with backtrack points!
    return false;
    int n = stack.count() -1;
    while(n>=0)
    {
        const ParseFrame &f = stack.at(n--);
        if(f.backTrack)
            continue;
        if(f.parsePos != this->pos)
            return false;
        if(f.continuationLabel == label)
            return true;
    }
    return false;
}

ParseResultClass::ParseResultClass(QString name)
    :EasyForeignClass(name)
{
    fields.insert(VMId::get(RId::InputPos));
    fields.insert(VMId::get(RId::ParseResultOf));

}

IObject *ParseResultClass::newValue(Allocator *allocator)
{
    Object *obj = new Object();
    obj->slotNames.append(VMId::get(RId::InputPos));
    obj->slotNames.append(VMId::get(RId::ParseResultOf));
    return obj;
}

Value *ParseResultClass::dispatch(Process *, int, QVector<Value *>)
{
    return NULL;
}

ParserClass::ParserClass(QString name, RunWindow *rw, ParseResultClass *parseResultClass)
    :EasyForeignClass(name),
      rw(rw),
      parseResultClass(parseResultClass)
{
    callCount.clear();
    fields.insert("value_stack");
    fields.insert(VMId::get(RId::Data));
    fields.insert(VMId::get(RId::InputPos));

    _method(VMId::get(RId::PushBacktrackPoint),
            0, 1);
    _method(VMId::get(RId::GotoBacktrackPoint),
            1, 1);
    _method(VMId::get(RId::IgnoreLastBacktrackPoint),
            2, 0);
    _method(VMId::get(RId::FailAndBackTrack),
            3, 0);
    _method(VMId::get(RId::MoveNext),
            4, 0);
    _method(VMId::get(RId::MoveNextMany),
            5, 1);
    _method(VMId::get(RId::Peek),
            6, 0);
    _method(VMId::get(RId::PeekMany),
            7, 1);
    _method(VMId::get(RId::LookAhead),
            8, 1);
    _method(VMId::get(RId::LookAheadMany),
            9, 1);
    _method(VMId::get(RId::RuleCall),
            10, 2);
    _method(VMId::get(RId::RuleReturn),
            11, 0);
    _method(VMId::get(RId::PushLocals),
            12, 1);
    _method(VMId::get(RId::PopLocals),
            13, 0);

    _method(VMId::get(RId::EndOfInput),
            14, 0);
    _method(VMId::get(RId::Dump),
            15, 0);

    _method(VMId::get(RId::Memoize),
            16, 4);
    _method(VMId::get(RId::GetMemoized),
            17, 2);
    _method(VMId::get(RId::IsMemoized),
            18, 2);
    _method(VMId::get(RId::CallCount),
            19, 0);
    _method(VMId::get(RId::LookAheadRange),
            20, 2);
}

IObject *ParserClass::newValue(Allocator *allocator)
{
    //todo: this is a hack
    this->allocator = allocator;
    callCount.clear();
    ParserObj *obj = new ParserObj();
    obj->allocator = allocator;
    obj->slotNames.append("value_stack");
    obj->slotNames.append(VMId::get(RId::Data));
    obj->slotNames.append(VMId::get(RId::InputPos));

    Value *arr = allocator->newArray(30);
    obj->setSlotValue("value_stack", arr);
    return obj;
}

Value *ParserClass::dispatch(Process *proc, int id, QVector<Value *> args)
{
    IObject *receiver = unboxObj(args[0]);
    ParserObj *parser = dynamic_cast<ParserObj *>(receiver);
    ParseFrame f;
    QString str, str2;
    int n;
    int i;
    IObject *res;
    ParseResult pr;
    int label,pos;
    if(!callCount.contains(id))
        callCount[id] = 0;
    callCount[id]++;
    QStringList strList;
    QChar c, c1, c2;
    bool bv;
    switch(id)
    {

    case 0: // ادفع.مسار.بديل
        //rw->typeCheck(proc, args[1], BuiltInTypes::IntType);
        parser->stack.push(ParseFrame(unboxInt(args[1]), parser->pos, true));
        return NULL;

    case 1: // اذهب.مسار.بديل
        f = parser->stack.pop();
        if(!f.backTrack)
            this->rw->assert(proc, false, InternalError1, VM::argumentErrors[ArgErr::StackTopNotBacktrackPointToBackTrack1].arg(f.continuationLabel));

        parser->pos = f.parsePos;
        return allocator->newInt(f.continuationLabel);

    case 2: // تجاهل.آخر.مسار.بديل
        f = parser->stack.pop();
        if(!f.backTrack)
            this->rw->assert(proc, false, InternalError1, VM::argumentErrors[ArgErr::StackTopNotBacktrackPointToIgnore1].arg(f.continuationLabel));
        return NULL;

    case 3:// الجأ.لبديل
        while(!parser->stack.empty() && !parser->stack.top().backTrack)
        {
            parser->stack.pop();
            parser->valueStackTop--;
        }
        if(parser->stack.empty())
        {
            //rw->assert(false, InternalError1, _ws(L"الجأ لبديل: قمة المكدس فارغة بعد حذف اي علامات عادية"));

            return allocator->newString(VMId::get(RId::ParseLblParseError));
        }
        f = parser->stack.pop();
        parser->pos = f.parsePos;
        return allocator->newInt(f.continuationLabel);

    case 4:     // تقدم
        parser->pos++;
        return NULL;

    case 5:    // تقدم.عديد
        rw->typeCheck(proc, args[1], BuiltInTypes::IntType);
        parser->pos += unboxInt(args[1]);
        return NULL;

    case 6:    // انظر
        str = unboxStr(parser->data);
        return allocator->newString(str.mid(parser->pos, 1));
    case 7:    // انظر.عديد
        rw->typeCheck(proc, parser->data, BuiltInTypes::StringType);
        rw->typeCheck(proc, args[1], BuiltInTypes::IntType);
        return allocator->newString(unboxStr(parser->data).mid(parser->pos, unboxInt(args[1])));
    case 8:    // يطل.على
        //rw->typeCheck(proc, parser->data, BuiltInTypes::StringType);
        //rw->typeCheck(proc, args[1], BuiltInTypes::StringType);
        str = unboxStr(parser->data);
        if(parser->pos >= str.length())
            return allocator->newBool(false);
        return allocator->newBool(str.at(parser->pos) == unboxStr(args[1]).at(0));

    case 9:     // يطل.على.عديد
        rw->typeCheck(proc, parser->data, BuiltInTypes::StringType);
        rw->typeCheck(proc, args[1], BuiltInTypes::StringType);
        str = unboxStr(parser->data);
        str2 = unboxStr(args[1]);
        if(parser->pos + str2.length() > str.length())
            return allocator->newBool(false);
        return allocator->newBool(str.mid(parser->pos,str2.length()) == str2);

    case 10:     // تفرع
        //rw->typeCheck(proc, args[1], BuiltInTypes::IntType);
        //rw->typeCheck(proc, args[2], BuiltInTypes::IntType);
        if(parser->hasInfiniteRecursion(unboxInt(args[2])))
        {
            {
                Value *arg0 = args[0];
                args.clear();
                args.append(arg0);
            }
            // backtrack
            return dispatch(proc, 3, args);
        }
        else
        {
            parser->stack.push(ParseFrame(unboxInt(args[2]),
                                          parser->pos,
                                          false));
            return args[1];
        }
    case 11:    // عد
        return allocator->newInt(parser->stack.pop().continuationLabel);

    case 12:     // ادفع.المتغيرات.المحلية
        n = unboxArray(parser->valueStack)->count();
        if(parser->valueStackTop == n)
        {
            Value *arr2 = allocator->newArray(n*2);
            for(i=0; i<n; i++)
            {
                unboxArray(arr2)->Elements[i] = unboxArray(parser->valueStack)->Elements[i];
            }
            receiver->setSlotValue("value_stack", arr2);
        }
        unboxArray(parser->valueStack)->Elements[parser->valueStackTop]
                = args[1];
        parser->valueStackTop++;
        return NULL;
    case 13:    // ارفع.المتغيرات.المحلية
        parser->valueStackTop--;
        return unboxArray(parser->valueStack)->Elements[parser->valueStackTop];

    case 14:     // منته
        str = unboxStr(parser->data);
        if(parser->pos >= str.length())
            return allocator->newBool(false);
        else
            return allocator->newBool(true);
    case 15: // اطرش
        return NULL;
    case 16:// تذكر
        rw->typeCheck(proc, args[1], BuiltInTypes::IntType);
        rw->typeCheck(proc, args[2], BuiltInTypes::IntType);
        rw->typeCheck(proc, args[3], BuiltInTypes::IntType);
        label = unboxInt(args[1]);
        pos = unboxInt(args[2]);
        if(!parser->memoize.contains(label))
        {
            parser->memoize[label] = QMap<int, ParseResult>();
        }
        parser->memoize[label][pos] =
                ParseResult(unboxInt(args[3]), args[4]);
        return NULL;
    case 17: // استرد.ذكرى
        rw->typeCheck(proc, args[1], BuiltInTypes::IntType);
        rw->typeCheck(proc, args[2], BuiltInTypes::IntType);
        label = unboxInt(args[1]);
        pos = unboxInt(args[2]);
        pr = parser->memoize[label][pos];
        res =  parseResultClass->newValue(allocator);
        res->setSlotValue(VMId::get(RId::InputPos), allocator->newInt(pr.pos));
        res->setSlotValue(VMId::get(RId::ParseResultOf), pr.v);
        return allocator->newObject(res, parseResultClass);
    case 18:  // هل.تذكر
        rw->typeCheck(proc, args[1], BuiltInTypes::IntType);
        rw->typeCheck(proc, args[2], BuiltInTypes::IntType);
        label = unboxInt(args[1]);
        pos = unboxInt(args[2]);
        if(!parser->memoize.contains(label))
            return allocator->newBool(false);
        return allocator->newBool(parser->memoize[label].contains(pos));
    case 19: // كم.نوديت
        n = 0;
        for(i=0; i<19; i++)
        {
            strList.append(QString("%1->%2").arg(i).arg(callCount[i]));
            n+= callCount[i];
        }
        strList.append(QString("Total=%1").arg(n));
        str = strList.join("  /  ");
        return allocator->newString(str);
    case 20: // يطل.على.نطاق
        //rw->typeCheck(proc, parser->data, BuiltInTypes::StringType);
        //rw->typeCheck(proc, args[1], BuiltInTypes::StringType);
        str = unboxStr(parser->data);
        if(parser->pos >= str.length())
            return allocator->newBool(false);

        c = str.at(parser->pos);
        c1 = unboxStr(args[1]).at(0);
        c2 = unboxStr(args[2]).at(0);
        bv = c >=c1 && c <=c2;
        return allocator->newBool(bv);

    default:
        //todo: better error reporting
        //todo: handle default in all internal dispatch functions
        throw VMError(NoSuchMethod2).arg(
                    QString("%1").arg(id).arg("ParserEngine"));
    }
}
