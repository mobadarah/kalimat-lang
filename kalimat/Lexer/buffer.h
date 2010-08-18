/**************************************************************************
** The Kalimat programming language
**   Copyright (C) 2010 Mohamed Samy Ali - samy2004@gmail.com
**   This project is released under the Apache License version 2.0
**   as described in the included license.txt file
**************************************************************************/

#ifndef BUFFER_H
#define BUFFER_H

#include<QtCore>

class BufferState
{
public:
    BufferState(QString buffer, int p1, int p2, bool startOfLine, int col, int line);
    QString buffer;
    int p1,p2;
    int col, line;
    bool startOfLine;
};
class Buffer {
    QString buffer;
    int p1,p2; // p2 is the "lookahead" pointer, p1 is the "reset" pointer. both are indexes in the 'buffer' variable.
    bool startOfLine; // when we read a '\n' and this is not the end of file, we set this to true
                    // since the next QChar is at the start of the line
    int line, col;
public:
    Buffer();
    // init this.buffer and p1, p2
    void init(QString buffer);

    // have we read all characters?
    bool eof();
    bool eof(int pos);
    //return true if the next QChar is c, does not advance the read index
    bool readAhead(QChar c);
    bool readAhead(QChar c, int offset);
    // return true if the next set of QChars to read is s.
    bool readAhead(QString s);

    // return true if we are at the first Character of a line
    bool isStartOfLine();

    // reads a QChar, advances p2. (all readAhead functions don't advance p2)
    QChar read();
    void read(QString string);
    // returns the QString between p1 and p2, inclusive. and sets p1 = p2
    QString accept(int &line, int &col, int &pos);
    // returns the QString that would've been returned by accept(), but doesn't change buffer state
    QString acceptable();

    // cancels all read operations from the last start state, i.e sets p2 = p1
    void reset();

    // decrements p2 by numChars
    void retract(int numChars);

    int GetLine();
    int GetColumn();

    BufferState saveState();
    void restoreState(BufferState s);
private:
    void updateStartOfLine();
    void updateStartOfLine(int p);

};

#endif // BUFFER_H
