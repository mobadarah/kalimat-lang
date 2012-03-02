/**************************************************************************
**   The Kalimat programming language
**   Copyright (C) 2010 Mohamed Samy Ali - samy2004@gmail.com
**   This project is released under the Apache License version 2.0
**   as described in the included license.txt file
**************************************************************************/

#include <QChar>
#include "state.h"
#include "buffer.h"
#include "charpredicate.h"

LA::LA(CharPredicate *p) :Predicate()
{
    this->predicate = p;
}

bool LA::operator ()(Buffer &buffer)
{
    BufferState s = buffer.saveState();
    if(buffer.eof())
        return false;
    QChar c = buffer.read();
    if((*predicate)(c))
    {
        return true;
    }
    buffer.restoreState(s);
    return false;

}

LAStr::LAStr(QString s) :Predicate()
{
    this->str = s;
}

LAStr::LAStr(QStdWString s) : Predicate()
{
    this->str = QString::fromStdWString(s);
}

bool LAStr::operator ()(Buffer &buffer)
{
    BufferState s = buffer.saveState();
    if(buffer.eof())
        return false;
    if(buffer.readAhead(str))
    {
        buffer.read(str);
        return true;
    }
    buffer.restoreState(s);
    return false;

}

CharEquals::CharEquals(QChar c)
{
        _c = c;
}

bool CharEquals::operator ()(QChar c)
{
        return _c == c;
}

CharNotEqual::CharNotEqual(QChar c)
{
        _c = c;
}

bool CharNotEqual::operator ()(QChar c)
{
        return _c != c;
}

bool CharIsLetter::operator ()(QChar c)
{
    return c.isLetter();
}

CharRange::CharRange(QChar c1, QChar c2)
{
        _c1 = c1, _c2 = c2;;
}

bool CharRange::operator ()(QChar c)
{
        return (_c1<= c) && (_c2 >= c);
}

CharOr::CharOr(CharPredicate *a,CharPredicate *b)
{
        _a = a; _b = b;
}

bool CharOr::operator ()(QChar c)
{
        return (*_a)(c) || (*_b)(c);
}

AnyChar::AnyChar() : CharPredicate()
{
}

bool AnyChar::operator ()(QChar)
{
    return true;
}

Else::Else()
{

}

bool Else::operator ()(Buffer &)
{
        return true;
}

bool LineStart::operator ()(Buffer &b)
{
    return b.isStartOfLine();
}

Eof::Eof() : Predicate()
{
}

bool Eof::operator ()(Buffer &b)
{
    return b.eof();
}

ReSeq::ReSeq(Predicate *p1, Predicate *p2)
{
    predicateCount = 2;
    predicates = new Predicate*[2];
    predicates[0] = p1;
    predicates[1] = p2;
}

bool ReSeq::operator ()(Buffer &b)
{
    BufferState s = b.saveState();
    for(int i=0; i<predicateCount; i++)
    {
        Predicate *p = predicates[i];

        bool result = (*p)(b);
        if(!result)
        {
            b.restoreState(s);
            return false;
        }
    }
    return true;
}

ReChoice::ReChoice(Predicate *p1, Predicate *p2)
{
    predicateCount = 2;
    predicates = new Predicate*[2];
    predicates[0] = p1;
    predicates[1] = p2;
}
bool ReChoice::operator ()(Buffer &b)
{
    for(int i=0; i<predicateCount; i++)
    {
        Predicate *p = predicates[i];
        BufferState s = b.saveState();
        if((*p)(b))
            return true;
        b.restoreState(s);
    }
    return false;
}
ReLoop::ReLoop(Predicate *p)
{
    predicate = p;
}
bool ReLoop::operator ()(Buffer &b)
{
    while(true)
    {
        BufferState s = b.saveState();
        bool result = (*predicate)(b);
        if(!result)
        {
            b.restoreState(s);
            break;
        }
    }
    return true;
}

Predicate *seq(Predicate *p1, Predicate *p2)
{
    return new ReSeq(p1, p2);
}
Predicate *seq(Predicate *p1, Predicate *p2, Predicate *p3)
{
    return new ReSeq(p1, new ReSeq(p2, p3));
}
Predicate *seq(Predicate *p1, Predicate *p2, Predicate *p3, Predicate *p4)
{
    return new ReSeq(p1, new ReSeq(p2, new ReSeq(p3,p4)));
}
Predicate *seq(Predicate *p1, Predicate *p2, Predicate *p3, Predicate *p4, Predicate *p5)
{
    return new ReSeq(p1, new ReSeq(p2, new ReSeq(p3, new ReSeq(p4, p5))));
}
Predicate *choice(Predicate *p1, Predicate *p2)
{
    return new ReChoice(p1, p2);
}
Predicate *loop(Predicate *p)
{
    return new ReLoop(p);
}

Predicate *la(CharPredicate *p)
{
    return new LA(p);
}

Predicate *la(QChar p)
{
    return new LA(new CharEquals(p));
}
