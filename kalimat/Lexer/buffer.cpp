/**************************************************************************
**   The Kalimat programming language
**   Copyright (C) 2010 Mohamed Samy Ali - samy2004@gmail.com
**   This project is released under the Apache License version 2.0
**   as described in the included license.txt file
**************************************************************************/

#include "buffer.h"

BufferState Buffer::saveState()
{
    BufferState ret(buffer,p1,p2,startOfLine, col, line);
    return ret;
}
void Buffer::restoreState(BufferState s)
{
    this->buffer = s.buffer;
    this->p1 = s.p1;
    this->p2 = s.p2;
    this->startOfLine= s.startOfLine ;
    this->col = s.col;
    this->line = s.line;

}
Buffer::Buffer()
{
}
void Buffer::init(QString str)
{
    buffer = str;
    p1 = p2 = 0;
    startOfLine = true;
    line = col = 0;
}
bool Buffer::eof()
{
    return eof(p2);
}
bool Buffer::eof(int pos)
{
    return pos>= buffer.length();
}

bool Buffer::readAhead(QChar c)
{
    return readAhead(c,0);
}
bool Buffer::readAhead(QChar c, int offset)
{
    if(eof(p2+offset))
        return false;
    if(buffer.at(p2+offset)==c)
        return true;
    return false;
}

bool Buffer::readAhead(QString s)
{
    for(int i=0; i<s.length();i++)
    {
        if(!readAhead(s.at(i), i))
        {
            return false;
        }
    }
    return true;
}
bool Buffer::isStartOfLine()
{
    return startOfLine;
}

QChar Buffer::read()
{
    updateStartOfLine(p2);
    QChar c = buffer.at(p2++);
    col ++;
    if(c == '\n')
    {
        col = 0;
        line ++;
    }
    return c;
}

void Buffer::updateStartOfLine(int pos)
{
    if(eof())
    {
        startOfLine = false;
        return;
    }

    QChar c = buffer.at(pos);
    if(c=='\n')
        startOfLine = true;
    else
        startOfLine = false;
}

void Buffer::updateStartOfLine()
{
        updateStartOfLine(p2);
}

QString Buffer::accept(int &line, int &col, int &pos)
{
    QString ret = buffer.mid(p1, p2 - p1);
    pos = p1;
    line = GetLine();
    col = GetColumn();

    p1 = p2;
    return ret;
}

QString Buffer::acceptable()
{
    QString ret = buffer.mid(p1, p2 - p1);
    return ret;
}


void Buffer::read(QString string)
{
// todo: this is unoptimized but at least
// doesn't mess up the startOfLine
// which is handled in char read()
// so we use it here:
    for(int i=0; i<string.length(); i++)
        read();
}
void Buffer::reset()
{
    p2 = p1;
    updateStartOfLine();
}

void Buffer::retract(int numChars)
{
    for(int i=0; i<numChars; i++)
    {
        QChar c = buffer[p2-1];
        col --;
        if(c=='\n')
        {
            // We haven't fixed columns yet! Column info will be unreliable
            // whenever retract() is used.
            line--;
        }
        p2--;
    }
    updateStartOfLine();
}

BufferState::BufferState(QString _buffer, int _p1, int _p2, bool _startOfLine, int _col, int _line)
{
    buffer = _buffer;
    p1 = _p1;
    p2 = _p2;
    col = _col;
    line = _line;
    startOfLine = _startOfLine;
}
int Buffer::GetColumn()
{
    return col;
}

int Buffer::GetLine()
{
    return line;
}

int Buffer::GetPos()
{
    return p2;
}
