/**************************************************************************
**   The Kalimat programming language
**   Copyright (C) 2010 Mohamed Samy Ali - samy2004@gmail.com
**   This project is released under the Apache License version 2.0
**   as described in the included license.txt file
**************************************************************************/

#ifndef EXTERNALMETHOD_H
#define EXTERNALMETHOD_H

#include "operandstack.h"

class ExternalMethod : public Object
{
    int arity;
public:
    bool mustRunInGui;
public:
    explicit ExternalMethod();
    explicit ExternalMethod(int arity);
    int Arity() { return arity;}
    virtual void operator ()(VOperandStack &operandStack, Process *) =0;
};


#endif // EXTERNALMETHOD_H
