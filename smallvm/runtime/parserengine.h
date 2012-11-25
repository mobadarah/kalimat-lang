#ifndef PARSERENGINE_H
#define PARSERENGINE_H

#ifndef CLASSES_H
    #include "../smallvm/classes.h"
#endif

#ifndef EASYFOREIGNCLASS_H
    #include "../smallvm/easyforeignclass.h"
#endif

#ifndef VALUE_H
    #include "../smallvm/value.h"
#endif

#ifndef ALLOCATOR_H
    #include "../smallvm/allocator.h"
#endif
#ifndef RUNWINDOW_H
    #include "runwindow.h"
#endif

struct ParseFrame
{
    bool backTrack;
    int continuationLabel;
    int parsePos;
    ParseFrame() { continuationLabel = -1; parsePos = -1; }
    ParseFrame(int label, int parsePos, bool backTrack)
    {
        this->backTrack = backTrack;
        this->parsePos = parsePos;
        continuationLabel = label;
    }
};

class ParseResultClass : public EasyForeignClass
{
public:
    ParseResultClass(QString name);
    IObject *newValue(Allocator *allocator);
    Value *dispatch(Process *, int , QVector<Value *>);
    static ParseResultClass *type;
};

struct ParseResult
{
    int pos;
    Value *v;
    ParseResult(int pos, Value *v)
        : pos(pos), v(v)
    {

    }
    ParseResult() {}
};

struct ParserObj : public Object
{
    Allocator *allocator;
    Value *data;
    QStack<ParseFrame> stack;
    // label -> pos -> result
    QMap<int, QMap<int, ParseResult> > memoize;
    Value *valueStack;
    int valueStackTop;
    int pos;
    ParserObj() { valueStackTop = 0;}
    virtual Value *getSlotValue(QString name);
    virtual void setSlotValue(QString name, Value *val);
    bool hasInfiniteRecursion(int label);
};

class ParserClass : public EasyForeignClass
{
    QMap<int,int> callCount; // for profiling
    RunWindow *rw;
    Allocator *allocator;
public:
    ParserClass(QString name, RunWindow *rw);
    IObject *newValue(Allocator *allocator);
    Value *dispatch(Process *proc, int id, QVector<Value *>args);
};

#endif // PARSER_H
