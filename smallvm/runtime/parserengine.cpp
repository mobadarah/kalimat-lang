#include "parserengine.h"
#include "../utils.h"

#define _method(a, b, c) methodIds[_ws(a)] = b; \
    methodArities[_ws(a)] = c + 1;

ParseResultClass *ParseResultClass::type = new ParseResultClass("ParseResult");

void ParserObj::setSlotValue(QString name, Value *val)
{
    if(name == _ws(L"بيان"))
    {
        this->data = val;
    }
    if(name == _ws(L"موقع"))
    {
        //todo:typecheck
        this->pos = val->unboxInt();
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
    if(name == _ws(L"بيان"))
    {
        Object::setSlotValue(name, data);
    }
    if(name == _ws(L"موقع"))
    {
        Object::setSlotValue(name, allocator->newInt(pos));
    }
    if(name == _ws(L"value_stack"))
    {

    }
    return Object::getSlotValue(name);
}

ParseResultClass::ParseResultClass(QString name)
    :EasyForeignClass(name)
{
    fields.insert(_ws(L"موقع"));
    fields.insert(_ws(L"نتيجة"));

}

IObject *ParseResultClass::newValue(Allocator *allocator)
{
    Object *obj = new Object();
    obj->slotNames.append(_ws(L"موقع"));
    obj->slotNames.append(_ws(L"نتيجة"));
    return obj;
}

Value *ParseResultClass::dispatch(int id, QVector<Value *> args)
{
    return NULL;
}

ParserClass::ParserClass(QString name, RunWindow *rw)
    :EasyForeignClass(name),
      rw(rw)
{
    callCount.clear();
    fields.insert("value_stack");
    fields.insert(_ws(L"بيان"));
    fields.insert(_ws(L"موقع"));

    _method(L"ادفع.مسار.بديل",
            0, 1);
    _method(L"اذهب.مسار.بديل",
            1, 1);
    _method(L"تجاهل.آخر.مسار.بديل",
            2, 0);
    _method(L"الجأ.لبديل",
            3, 0);
    _method(L"تقدم",
            4, 0);
    _method(L"تقدم.عديد",
            5, 1);
    _method(L"انظر",
            6, 0);
    _method(L"انظر.عديد",
            7, 1);
    _method(L"يطل.على",
            8, 1);
    _method(L"يطل.على.عديد",
            9, 1);
    _method(L"تفرع",
            10, 2);
    _method(L"عد",
            11, 0);
    _method(L"ادفع.المتغيرات.المحلية",
            12, 1);
    _method(L"ارفع.المتغيرات.المحلية",
            13, 0);

    _method(L"منته",
            14, 0);
    _method(L"اطرش",
            15, 0);

    _method(L"تذكر",
            16, 4);
    _method(L"استرد.ذكرى",
            17, 2);
    _method(L"هل.تذكر",
            18, 2);
    _method(L"كم.نوديت",
            19, 0);
}

IObject *ParserClass::newValue(Allocator *allocator)
{
    //todo: this is a hack
    this->allocator = allocator;
    callCount.clear();
    ParserObj *obj = new ParserObj();
    obj->allocator = allocator;
    obj->slotNames.append("value_stack");
    obj->slotNames.append(_ws(L"بيان"));
    obj->slotNames.append(_ws(L"موقع"));

    Value *arr = allocator->newArray(30);
    obj->setSlotValue("value_stack", arr);
    return obj;
}

Value *ParserClass::dispatch(int id, QVector<Value *>args)
{
    IObject *receiver = args[0]->unboxObj();
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
    switch(id)
    {

    case 0: // ادفع.مسار.بديل
        //rw->typeCheck(args[1], BuiltInTypes::IntType);
        parser->stack.push(ParseFrame(args[1]->unboxInt(), parser->pos));
        return NULL;

    case 1: // اذهب.مسار.بديل
        f = parser->stack.pop();
        if(!f.backTrack)
            this->rw->assert(false, InternalError1, _ws(L"محاولة الذهاب لمسار إعرابي بديل، لكن قمة المكدس هي %1").arg(f.continuationLabel));

        parser->pos = f.parsePos;
        return allocator->newInt(f.continuationLabel);

    case 2: // تجاهل.آخر.مسار.بديل
        f = parser->stack.pop();
        if(!f.backTrack)
            this->rw->assert(false, InternalError1, _ws(L"تجاهل آخر مسار بديل: قمة المكدس (%1) ليست نقطة تراجع").arg(f.continuationLabel));
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
            return allocator->newString(_ws(L"خطأ.في.الإعراب"));
        }
        f = parser->stack.pop();
        parser->pos = f.parsePos;
        return allocator->newInt(f.continuationLabel);

    case 4:     // تقدم
        parser->pos++;
        return NULL;

    case 5:    // تقدم.عديد
        rw->typeCheck(args[1], BuiltInTypes::IntType);
        parser->pos += args[1]->unboxInt();
        return NULL;

    case 6:    // انظر
        str = parser->data->unboxStr();
        return allocator->newString(str.mid(parser->pos, 1));
    case 7:    // انظر.عديد
        rw->typeCheck(parser->data, BuiltInTypes::StringType);
        rw->typeCheck(args[1], BuiltInTypes::IntType);
        return allocator->newString(parser->data->unboxStr().mid(parser->pos, args[1]->unboxInt()));
    case 8:    // يطل.على
        //rw->typeCheck(parser->data, BuiltInTypes::StringType);
        //rw->typeCheck(args[1], BuiltInTypes::StringType);
        str = parser->data->unboxStr();
        if(parser->pos >= str.length())
            return allocator->newBool(false);
        return allocator->newBool(str.at(parser->pos) == args[1]->unboxStr().at(0));

    case 9:     // يطل.على.عديد
        rw->typeCheck(parser->data, BuiltInTypes::StringType);
        rw->typeCheck(args[1], BuiltInTypes::StringType);
        str = parser->data->unboxStr();
        str2 = args[1]->unboxStr();
        if(parser->pos + str2.length() > str.length())
            return allocator->newBool(false);
        return allocator->newBool(str.mid(parser->pos,str2.length()) == str2);

    case 10:     // تفرع
        //rw->typeCheck(args[1], BuiltInTypes::IntType);
        //rw->typeCheck(args[2], BuiltInTypes::IntType);
        parser->stack.push(ParseFrame(args[2]->unboxInt()));
        return args[1];
    case 11:    // عد
        return allocator->newInt(parser->stack.pop().continuationLabel);

    case 12:     // ادفع.المتغيرات.المحلية
        n = parser->valueStack->unboxArray()->count();
        if(parser->valueStackTop == n)
        {
            Value *arr2 = allocator->newArray(n*2);
            for(i=0; i<n; i++)
            {
                arr2->unboxArray()->Elements[i] = parser->valueStack->unboxArray()->Elements[i];
            }
            receiver->setSlotValue("value_stack", arr2);
        }
        parser->valueStack->unboxArray()->Elements[parser->valueStackTop]
                = args[1];
        parser->valueStackTop++;
        return NULL;
    case 13:    // ارفع.المتغيرات.المحلية
        parser->valueStackTop--;
        return parser->valueStack->unboxArray()->Elements[parser->valueStackTop];

    case 14:     // منته
        str = parser->data->unboxStr();
        if(parser->pos >= str.length())
            return allocator->newBool(false);
        else
            return allocator->newBool(true);
    case 15: // اطرش
        return NULL;
    case 16:// تذكر
        rw->typeCheck(args[1], BuiltInTypes::IntType);
        rw->typeCheck(args[2], BuiltInTypes::IntType);
        rw->typeCheck(args[3], BuiltInTypes::IntType);
        label = args[1]->unboxInt();
        pos = args[2]->unboxInt();
        if(!parser->memoize.contains(label))
        {
            parser->memoize[label] = QMap<int, ParseResult>();
        }
        parser->memoize[label][pos] =
                ParseResult(args[3]->unboxInt(), args[4]);
        return NULL;
    case 17: // استرد.ذكرى
        rw->typeCheck(args[1], BuiltInTypes::IntType);
        rw->typeCheck(args[2], BuiltInTypes::IntType);
        label = args[1]->unboxInt();
        pos = args[2]->unboxInt();
        pr = parser->memoize[label][pos];
        res = ParseResultClass::type->newValue(allocator);
        res->setSlotValue(_ws(L"موقع"), allocator->newInt(pr.pos));
        res->setSlotValue(_ws(L"نتيجة"), pr.v);
        return allocator->newObject(res, ParseResultClass::type);
    case 18:  // هل.تذكر
        rw->typeCheck(args[1], BuiltInTypes::IntType);
        rw->typeCheck(args[2], BuiltInTypes::IntType);
        label = args[1]->unboxInt();
        pos = args[2]->unboxInt();
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
        QString s = strList.join("  /  ");
        return allocator->newString(s);
    }
}
