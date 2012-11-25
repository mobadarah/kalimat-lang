/**************************************************************************
** The Kalimat programming language
**   Copyright (C) 2010 Mohamed Samy Ali - samy2004@gmail.com
**   This project is released under the Apache License version 2.0
**   as described in the included license.txt file
**************************************************************************/

#ifndef KALIMATAST_H
#define KALIMATAST_H
#include <QObject>
#include <QString>
#include <QLocale>
#include <QStringList>
#include <QVector>
#include <QMap>
#include <memory>

#ifndef TOKEN_H
#include "../../Lexer/token.h"
#endif

#ifndef CODEFORMATTER_H
#include "../codeformatter.h"
#endif

#ifndef AST_H
#include "../ast.h"
#endif

using namespace std;

QString strLiteralRepr(QString value);
/*
class KalimatAst : public AST
{
public:
    Token _pos;
public:
    KalimatAst(Token _pos):_pos(_pos) {}
    virtual ~KalimatAst(){}
    virtual QString toString() { return "";}
    Token getPos() { return _pos;}
};
*/
class Identifier;
class IScopeIntroducer
{
    virtual QVector<shared_ptr<Identifier> > getIntroducedVariables()=0;
};

class Traverser
{
public:
    virtual void visit(shared_ptr<PrettyPrintable> tree)=0;
    virtual void exit(shared_ptr<PrettyPrintable> tree)=0;
};

#endif // KALIMATAST_H
