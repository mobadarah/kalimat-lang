/**************************************************************************
**   The Kalimat programming language
**   Copyright 2010 Mohamed Samy Ali - samy2004@gmail.com
**   This project is released under the Apache License version 2.0
**   as described in the included license.txt file
**************************************************************************/

#include "linetracker.h"

LineTracker::LineTracker()
{
}
void LineTracker::setText(QString text)
{
    lineStartPositions.clear();
    this->text = text;
    bool startOfLine = true;
    int count = 0, curLine = 0;
    for(int i=0; i<text.length();i++,count++)
    {
        if(startOfLine)
        {
            lineStartPositions.append(LineInfo(count));
        }
        if(text[i] == '\n')
        {
            startOfLine = true;
        }
        else
            startOfLine = false;
    }
    for(int i=0; i< lineStartPositions.count()-1; i++)
    {
        lineStartPositions[i].length = lineStartPositions[i+1].start - lineStartPositions[i].start;
    }
    if(lineStartPositions.count()>0)
    {
        int last = lineStartPositions.count() -1;
        lineStartPositions[last].length = text.count() - lineStartPositions[last].start;
    }
}

void LineTracker::lineColumnOfPos(int pos, int &line, int &col)
{
    // todo: replace linear search with binary search
    for(int i=0; i<lineStartPositions.count() -1; i++)
    {
        if(pos >= lineStartPositions[i].start && pos < lineStartPositions[i+1].start)
        {
            line = i;
            col = pos - lineStartPositions[i].start;
            return;
        }
    }
    if(pos <= text.count() && lineStartPositions.count()>0)
    {
        int last = lineStartPositions.count()-1;
        line = last;
        col = pos - lineStartPositions[last].start;
    }
    return;
}
int LineTracker::posFromLineColumn(int line, int column)
{
    return lineStartPositions[line].start + column;
}

QVector<LineInfo> LineTracker::linesFromTo(int pos1, int pos2)
{
    int line1, line2, col;
    lineColumnOfPos(pos1, line1, col);
    lineColumnOfPos(pos2, line2, col);
    QVector<LineInfo> ret;
    for(int i=line1; i<=line2; i++)
        ret.append(lineStartPositions[i]);
    return ret;
}
