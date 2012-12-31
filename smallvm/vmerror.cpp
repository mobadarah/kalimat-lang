/**************************************************************************
**  The Kalimat programming language
**  Copyright (C) 2010 Mohamed Samy Ali - samy2004@gmail.com
**  This project is released under the Apache License version 2.0
**  as described in the included license.txt file
**************************************************************************/

#include <QStack>
#include <QVector>
#include <QString>
#include "method.h"
#include "frame.h"
#include "vmerror.h"

VMError::VMError()
: QObject()
{
    this->type = NoError;
    this->callStack = NULL;
}

VMError::VMError(VMErrorType type, Process *process, Scheduler *scheduler, Frame *callStack)
    : QObject()
{
    this->type = type;
    this->callStack = callStack;
    this->process = process;
    this->scheduler = scheduler;
}

VMError::VMError(const VMError &other)
    :QObject()
{
    this->type = other.type;
    this->callStack = other.callStack;
    this->process = other.process;
    this->scheduler = other.scheduler;
    this->args = other.args;
}

VMError &VMError::operator =(const VMError &other)
{
    this->type = other.type;
    this->callStack = other.callStack;
    this->process = other.process;
    this->scheduler = other.scheduler;
    this->args = other.args;
    return *this;
}

VMError::VMError(VMErrorType type)
    : QObject()
{
    this->type = type;
    this->callStack = NULL;
}

VMError &VMError::arg(QString s)
{
    this->args.append(s);
    return *this;
}
