/**************************************************************************
**   The Kalimat programming language
**   Copyright (C) 2010 Mohamed Samy Ali - samy2004@gmail.com
**   This project is released under the Apache License version 2.0
**   as described in the included license.txt file
**************************************************************************/

#include "frame.h"

Frame::Frame()
{

}

Frame::Frame(Method *method)
{
    currentMethod = method;
    ip = 0;
    returnReferenceIfRefMethod = true;
}
Frame::Frame(Method *method, int ip)
{
    currentMethod = method;
    this->ip = ip;
    returnReferenceIfRefMethod = true;
}

Instruction Frame::getPreviousRunningInstruction()
{
    int n = currentMethod->InstructionCount();
    if(ip>=1 && ip-1 < n)
        return currentMethod->Get(this->ip-1);
    else
        return currentMethod->Get(n-1);
}
