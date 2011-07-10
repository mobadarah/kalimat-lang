/**************************************************************************
**   The Kalimat programming language
**   Copyright 2010 Mohamed Samy Ali - samy2004@gmail.com
**   This project is released under the Apache License version 2.0
**   as described in the included license.txt file
**************************************************************************/

#include "textlayer.h"
#include <algorithm>
using namespace std;

TextLayer::TextLayer()
{
    state = Normal;
    mode = Overwrite;
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
            int n = cursor_col - s.length() + 1; // TODO: is this the source of
                                                 // the 'delete key slightly shifts text display'
                                                 // bug?
                s.append(QString(n, ' '));
        }
        if(mode == Overwrite)
            s[cursor_col] = c;
        else
            s.insert(cursor_col, c);

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

void TextLayer::beginInput()
{
    state = Input;
    inputBuffer = "";
    inputStartLine = cursor_line;
    inputStartCol = cursor_col;
    oldMode = mode;
    mode = Insert;
}

QString TextLayer::endInput()
{
    state = Normal;
    mode = oldMode;
    return inputBuffer;
}

void TextLayer::typeIn(QString s)
{
    if(mode == Insert)
        inputBuffer.insert(inputCursorPos(), s);
    else
    {
        inputBuffer.replace(inputCursorPos(), s.length(), s);
    }
    print(s);
}

bool TextLayer::inputState()
{
    return state == Input;
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

bool TextLayer::cursorFwd()
{
    if(cursor_col + 1 < currentLine().length())
    {
        cursor_col++;
        return true;
    }
    return false;
}

bool TextLayer::cursorBack()
{
    if(state == Input && cursor_col == inputStartCol)
        return false;

    if(cursor_col > 0)
    {
        cursor_col--;
        return true;
    }
    return false;
}

bool TextLayer::cursorDown()
{
    if(cursor_line + 1 < lines().count())
    {
        cursor_line++;
        cursor_col = min(cursor_col, currentLine().length());
        return true;
    }
    return false;
}

bool TextLayer::cursorUp()
{
    if(cursor_line > 0)
    {
        cursor_line--;
        cursor_col = min(cursor_col, currentLine().length());
        return true;
    }
    return false;
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
    if(state == Input && mode == Insert)
    {
        QString s = currentLine();
        QString s1 = s.left(cursor_col);
        QString s2 = s.right(s.length() - cursor_col);
        visibleTextBuffer[cursor_line] = s1;
        visibleTextBuffer.insert(cursor_line+1, s2);
    }
    cr();
    lf();
}

void TextLayer::del()
{
    QString s = visibleTextBuffer[cursor_line];
    if(cursor_col >= s.length())
        return;
    if(state == Input)
        inputBuffer = inputBuffer.remove(inputCursorPos(), 1);

    s = s.remove(cursor_col, 1);
    visibleTextBuffer[cursor_line] = s;
}

void TextLayer::backSpace()
{
    if(cursor_col == 0)
        return;
    if(cursor_col <= inputStartCol)
        return;

    if(state == Input)
        inputBuffer = inputBuffer.remove(inputCursorPos()-1, 1);
    QString s = visibleTextBuffer[cursor_line];
    s = s.remove(cursor_col-1, 1);
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
