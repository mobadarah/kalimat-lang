/**************************************************************************
** The Kalimat programming language
**   Copyright (C) 2010 Mohamed Samy Ali - samy2004@gmail.com
**   This project is released under the Apache License version 2.0
**   as described in the included license.txt file
**************************************************************************/

#ifndef KALIMATAST_H
#define KALIMATAST_H
#define QT_SHAREDPOINTER_TRACK_POINTERS
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

class Expression;
class TypeExpression;
class Identifier;
class AssignableExpression;
class Declaration;
class MethodDecl;
class NumLiteral;
class StrLiteral;
class SimpleLiteral;
class IInvokation;

using namespace std;

QString strLiteralRepr(QString value);

class KalimatAst : public QObject, public AST
{
    Q_OBJECT
};

class IScopeIntroducer
{
    virtual QVector<shared_ptr<Identifier> > getIntroducedVariables()=0;
};

#endif // KALIMATAST_H
