/**************************************************************************
** The Kalimat programming language
**   Copyright (C) 2010 Mohamed Samy Ali - samy2004@gmail.com
**   This project is released under the Apache License version 2.0
**   as described in the included license.txt file
**************************************************************************/

#ifndef AST_H
#define AST_H

struct PrettyPrintable
{
    virtual void prettyPrint(CodeFormatter *formatter) = 0;
};

class AST : public PrettyPrintable
{
public:
    virtual ~AST() {}
    virtual QString toString()=0;
    virtual Token getPos()=0;
};

#endif // AST_H
