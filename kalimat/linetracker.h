/**************************************************************************
**   The Kalimat programming language
**   Copyright 2010 Mohamed Samy Ali - samy2004@gmail.com
**   This project is released under the Apache License version 2.0
**   as described in the included license.txt file
**************************************************************************/

#ifndef LINETRACKER_H
#define LINETRACKER_H

#include <QString>
#include <QVector>

struct LineInfo
{
    int start;
    int length;
    LineInfo(){}
    LineInfo(int s, int l) { start = s; length = l;}
    LineInfo(int s) { start = s;}
};

class LineTracker
{
    QString text;
    QVector<LineInfo> lineStartPositions;
public:
    LineTracker();
    void setText(QString);
    void lineColumnOfPos(int pos, int &line, int &col);
    int lineFromPos(int pos);
    int posFromLineColumn(int line, int column);
    void linesFromTo(int pos1, int pos2, int &lineFrom, int &lineTo);
    LineInfo line(int i);
};

#endif // LINETRACKER_H
