/**************************************************************************
**   The Kalimat programming language
**   Copyright 2010 Mohamed Samy Ali - samy2004@gmail.com
**   This project is released under the Apache License version 2.0
**   as described in the included license.txt file
**************************************************************************/

#ifndef TEXTLAYER_H
#define TEXTLAYER_H

#include <QString>
#include <QVector>

class TextLayer
{
    QVector<QString> visibleTextBuffer;
    int visibleTextLines;
    int textLineWidth;
    int cursor_col, cursor_line;

    void printChar(QChar c);
    void cr();
    void lf();

public:
    TextLayer();
    QVector<QString> &lines() { return visibleTextBuffer; }
    void print(QString);
    void println(QString);
    void print(QString str, int width);
    int cursorLine() { return cursor_line; }
    int cursorColumn() { return cursor_col; }
    QString currentLine() { return visibleTextBuffer[cursor_line]; }
    void nl();
    void backSpace();
    void clearText();
    int getCursorCol();
    int getCursorRow();
    bool setCursorPos(int row, int col);


    QString formatStringUsingWidth(QString str, int width);
};

#endif // TEXTLAYER_H
