/**************************************************************************
** The Kalimat programming language
**   Copyright (C) 2010 Mohamed Samy Ali - samy2004@gmail.com
**   This project is released under the Apache License version 2.0
**   as described in the included license.txt file
**************************************************************************/

#ifndef ACTION_H
#define ACTION_H

class Action
{
public:
    Action();
    virtual void operator()(Buffer &buffer)=0;
};

class Retract : public Action
{
public:
    Retract();
    void operator()(Buffer &buffer);
};
#endif // ACTION_H
