/**************************************************************************
**   The Kalimat programming language
**   Copyright 2010 Mohamed Samy Ali - samy2004@gmail.com
**   This project is released under the Apache License version 2.0
**   as described in the included license.txt file
**************************************************************************/

#include "textlayer.h"

TextLayer::TextLayer()
{
    clearText();
}

void TextLayer::clearText()
{
    cursor_col = 0;
    cursor_line = 0;
    visibleTextLines = 25;


    textLineWidth = 54;
    visibleTextBuffer.clear();
    visibleTextBuffer.resize(25);
}

void TextLayer::print(QString str)
{
    for(int i=0; i<str.length(); i++)
    {
        printChar(str[i]);
    }

}
void TextLayer::printChar(QChar c)
{
    if(c == '\n')
        nl();
    else
    {
        if(cursor_col>= textLineWidth)
            nl();

        if(cursor_line >= visibleTextBuffer.count())
            visibleTextBuffer.resize(cursor_line+1);

        QString s = visibleTextBuffer[cursor_line];
        if(s.length() <= cursor_col)
        {
            int n = cursor_col - s.length() +1;
                s.append(QString(n, ' '));
        }
        s[cursor_col] = c;
        visibleTextBuffer[cursor_line] = s;
        cursor_col++;
    }
}


void TextLayer::print(QString str, int width)
{
    if(str == "\n")
    {
        // todo: How does string formatting with a certain width affect printing of a newline, anyway?
        nl();
    }
    else
    {
        str = formatStringUsingWidth(str, width);
        print(str);
    }
}
void TextLayer::println(QString str)
{
    print(str);
    nl();
}

bool TextLayer::setCursorPos(int row, int col)
{
    if(row < 0 || row >= visibleTextLines || col < 0 || col>=textLineWidth)
    {
        return false;
    }
    cursor_col = col;
    cursor_line = row;
    return true;
}

int TextLayer::getCursorRow()
{
    return cursor_line;
}
int TextLayer::getCursorCol()
{
    return cursor_col;
}
void TextLayer::cr()
{
    cursor_col = 0;
}
void TextLayer::lf()
{
    cursor_line++;
    if(cursor_line ==visibleTextLines)
    {
        visibleTextBuffer.pop_front();
        visibleTextBuffer.append("");
        cursor_line--;
    }
}
void TextLayer::nl()
{
    cr();
    lf();
}

void TextLayer::backSpace()
{
    QString s = visibleTextBuffer[cursor_line];

    s = s.left(cursor_col-1) + s.mid(cursor_col, s.length() - cursor_col);
    visibleTextBuffer[cursor_line] = s;
    cursor_col --;
}

QString TextLayer::formatStringUsingWidth(QString str, int width)
{
    if(str.length()<width)
    {
        int n = width - str.length();
        QString sp = "";
        for(int i=0; i<n; i++)
            sp += " ";
        str+=sp;
    }
    else if(str.length() > width)
    {
        str = "";
        for(int i=0; i<width; i++)
        {
            str += "#";
        }
    }
    return str;
}
