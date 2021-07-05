/**************************************************************************
** The Kalimat programming language
**   Copyright (C) 2010 Mohamed Samy Ali - samy2004@gmail.com
**   This project is released under the Apache License version 2.0
**   as described in the included license.txt file
**************************************************************************/

#ifndef CHARPREDICATE_H
#define CHARPREDICATE_H

class Buffer;

class Predicate;
class CharPredicate;

Predicate *loop(Predicate *p);
Predicate *choice(Predicate *p1, Predicate *p2);
Predicate *seq(Predicate *p1, Predicate *p2);
Predicate *seq(Predicate *p1, Predicate *p2, Predicate *p3);
Predicate *seq(Predicate *p1, Predicate *p2, Predicate *p3, Predicate *p4);
Predicate *seq(Predicate *p1, Predicate *p2, Predicate *p3, Predicate *p4, Predicate *p5);
Predicate *la(CharPredicate *p);
Predicate *la(QChar p);

class Predicate
{
public:
    virtual bool operator()(Buffer &b)=0;
};

class CharPredicate
{
public:
        virtual bool operator()(QChar c)=0;
};
class LA : public Predicate
{
    CharPredicate *predicate;
public:
    LA(CharPredicate *pred);
    virtual bool operator()(Buffer &b);
};
class LAStr : public Predicate
{
    QString str;
public:
    LAStr(QString s);
    LAStr(QStdWString s);
    virtual bool operator()(Buffer &b);
};

class CharEquals : public CharPredicate
{
        QChar _c;
public:
        CharEquals(QChar c);
        bool operator()(QChar c);
};
class CharNotEqual : public CharPredicate
{
        QChar _c;
public:
        CharNotEqual(QChar c);
        bool operator()(QChar c);
};

class CharIsLetter : public CharPredicate
{
public:
    bool operator()(QChar c);
};

class CharRange: public CharPredicate
{
        QChar _c1, _c2;
public:
        CharRange(QChar c1, QChar c2);
        bool operator()(QChar c);
};
class CharOr: public CharPredicate
{
        CharPredicate *_a;
        CharPredicate *_b;
public:
        CharOr(CharPredicate *a, CharPredicate *b);
        bool operator()(QChar c);
};
class AnyChar : public CharPredicate
{
public:
    AnyChar();
    bool operator()(QChar c);
};
class Else: public Predicate
{
public:
        Else();
        bool operator()(Buffer &b);
};
class LineStart : public Predicate
{
public:

    bool operator()(Buffer &b);
};

class Eof: public Predicate
{
public:
        Eof();
        bool operator()(Buffer &b);
};

class ReSeq : public Predicate
{
    int predicateCount;
    Predicate ** predicates;
public:
    ReSeq(Predicate *p1, Predicate *p2);
    bool operator()(Buffer &b);
};
class ReChoice : public Predicate
{
    int predicateCount;
    Predicate ** predicates;
public:
    ReChoice(Predicate *p1, Predicate *p2);
    bool operator()(Buffer &b);
};

class ReLoop : public Predicate
{
    Predicate * predicate;
public:
    ReLoop(Predicate *p);
    bool operator()(Buffer &b);
};

#endif // CHARPREDICATE_H
