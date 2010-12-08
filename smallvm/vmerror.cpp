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

{
    this->type = NoError;
}

VMError::VMError(VMErrorType type, QStack<Frame>callStack)
{
    this->type = type;
    this->callStack = callStack;
}

VMError &VMError::arg(QString s)
{
    this->args.append(s);
    return *this;
}
