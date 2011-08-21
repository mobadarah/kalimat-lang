/**************************************************************************
** The Kalimat programming language
**   Copyright (C) 2010 Mohamed Samy Ali - samy2004@gmail.com
**   This project is released under the Apache License version 2.0
**   as described in the included license.txt file
**************************************************************************/

#ifndef PAINTTIMER_H
#define PAINTTIMER_H

#include <time.h>
class PaintTimer
{
    clock_t lastTimeStamp;
    long elapsedTime;
    long minTime;
    bool first;
public:
    PaintTimer(int frameRate);
    bool canUpdateNow();
};

#endif // PAINTTIMER_H
