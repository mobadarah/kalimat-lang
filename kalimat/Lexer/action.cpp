/**************************************************************************
**   The Kalimat programming language
**   Copyright (C) 2010 Mohamed Samy Ali - samy2004@gmail.com
**   This project is released under the Apache License version 2.0
**   as described in the included license.txt file
**************************************************************************/

#include "buffer.h"
#include "action.h"

Action::Action()
{
}
Retract::Retract() : Action()
{
}
void Retract::operator()(Buffer &buffer)
{
    buffer.retract(1);
}
