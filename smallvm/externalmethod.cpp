/**************************************************************************
**   The Kalimat programming language
**   Copyright (C) 2010 Mohamed Samy Ali - samy2004@gmail.com
**   This project is released under the Apache License version 2.0
**   as described in the included license.txt file
**************************************************************************/

#include <QStack>
#include <QString>
#include <QMap>
#include <QSet>

#include "value.h"
#include "externalmethod.h"
using namespace std;
ExternalMethod::ExternalMethod() :
    Object()
{
    arity = -1;
    mustRunInGui = false;
}
ExternalMethod::ExternalMethod(int arity) :
    Object()
{
    this->arity = arity;
    mustRunInGui = false;
}
