/**************************************************************************
**   The Kalimat programming language
**   Copyright (C) 2010 Mohamed Samy Ali - samy2004@gmail.com
**   This project is released under the Apache License version 2.0
**   as described in the included license.txt file
**************************************************************************/

#ifndef EXTERNALMETHOD_H
#define EXTERNALMETHOD_H

class ExternalMethod : public Object
{
    int arity;
public:
    explicit ExternalMethod();
    explicit ExternalMethod(int arity);
    int Arity() { return arity;}
    virtual void operator ()(QStack<Value *> &operandStack) =0;
};

class Print : public ExternalMethod
{
public:
    Print();
    void operator ()(QStack<Value *> &operandStack);
};

#endif // EXTERNALMETHOD_H
