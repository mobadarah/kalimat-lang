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
    dirtyState = false;
    mode = Overwrite;
    clearText();
    noColor = true;
}

bool TextLayer::dirty()
{
    if(inputState())
        return true; // we need to keep refreshing to make the cursor blink

    return dirtyState;
}

void TextLayer::clearText()
{
    cursor_col = 0;
    cursor_line = 0;
    visibleTextBuffer.clear();
    visibleTextBuffer.resize(visibleTextLines);
    htmlLines.clear();
    dirtyState = true;
    for(int i=0; i<visibleTextLines; i++)
    {
        for(int j=0; j<textLineWidth; j++)
        {
            colorBits[i][j] = Qt::black;
        }
        htmlLines.append("");
    }
    currentColor = Qt::black;
}

void TextLayer::print(QString str)
{
    int curLine = cursor_line;
    for(int i=0; i<str.length(); i++)
    {
        printChar(str[i]);
    }
    int curLine2 = cursor_line;
    if(str != "\n")
        updateChangedLines(curLine, 1 + curLine2 - curLine);
    dirtyState = true;
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

        colorBits[cursor_line][cursor_col] = currentColor;
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

void TextLayer::setColor(QColor c)
{
    noColor = false;
    currentColor = c;
}

void TextLayer::resetColor()
{
    noColor = true;
}

void TextLayer::updateChangedLines(int fromLine, int count)
{
    for(int i=0; i<count; ++i)
    {
        //updateHtmlLine(i + fromLine);
        updateTextLine(i + fromLine);
    }
}

void TextLayer::updateHtmlLine(int i)
{
    //*
    QColor current = colorBits[i][0];
    QString line = visibleTextBuffer[i];
    QString output = QString("<span style=\"color:rgb(%1,%2,%3);\">").arg(current.red()).arg(current.green()).arg(current.blue());

    for(int j=0; j<line.length(); ++j)
    {
        if(colorBits[i][j] != current)
        {
            current = colorBits[i][j];
            output += QString("</span><span style=\"color:rgb(%1,%2,%3);\">").arg(current.red()).arg(current.green()).arg(current.blue());
        }

        output+= line.mid(j, 1);
    }
    output += "</span>";
    htmlLines[i] = output;
   // */
}

QString TextLayer::toHtml()
{
     return htmlLines.join("<br/>");
}

void TextLayer::updateTextLine(int lineIndex)
{
    computeLineFormatRange(lineIndex, visibleTextBuffer[lineIndex], lineFormats[lineIndex]);
}

QString TextLayer::lineToText(int i, QList<QTextLayout::FormatRange> &range)
{
    QString &line = visibleTextBuffer[i];
    computeLineFormatRange(i, line, range);
    return line;
}

void TextLayer::computeLineFormatRange(int i, QString &line, QList<QTextLayout::FormatRange> &range)
{
    int pos =0;
    int runPos = 0;
    int runLen = 0;
    QColor currentColor = colorBits[i][0];
    range.clear();
    for(int j=0; j<line.length(); ++j)
    {
        if(colorBits[i][j] != currentColor)
        {
            runLen = pos - runPos;
            QTextLayout::FormatRange r;
            r.start = runPos;
            r.length = runLen;
            r.format.setForeground(QBrush(currentColor));
            currentColor = colorBits[i][j];
            range.append(r);
            runPos = pos;
        }
        pos++;
    }

    // Final iteration
    runLen = pos - runPos;
    QTextLayout::FormatRange r;
    r.start = runPos;
    r.length = runLen;
    r.format.setForeground(QBrush(currentColor));
    range.append(r);
}

QString TextLayer::toText(QList<QTextLayout::FormatRange> &range)
{
    QStringList result;
    int pos =0;
    int runPos = 0;
    int runLen = 0;
    QColor currentColor = colorBits[0][0];
    for(int i=0; i<visibleTextLines; ++i)
    {
        QString &line = visibleTextBuffer[i];
        result.append(line);
        for(int j=0; j<line.length(); ++j)
        {
            if(colorBits[i][j] != currentColor)
            {
                runLen = pos - runPos;
                QTextLayout::FormatRange r;
                r.start = runPos;
                r.length = runLen;
                r.format.setForeground(QBrush(currentColor));
                currentColor = colorBits[i][j];
                range.append(r);
                runPos = pos;
            }
            pos++;
        }
    }
    // Final iteration
    runLen = pos - runPos;
    QTextLayout::FormatRange r;
    r.start = runPos;
    r.length = runLen;
    r.format.setForeground(QBrush(currentColor));
    range.append(r);
    //runPos = pos;

    return result.join("\n");
}

int TextLayer::cursorPos()
{
    int pos = 0;
    for(int i=0; i< cursor_line; ++i)
    {
        pos += visibleTextBuffer[i].length();
    }
    pos += cursor_col;
    return pos;
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
    dirtyState = true;
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
    dirtyState = true;
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
    updateHtmlLine(cursor_line);
    cursor_col --;
    dirtyState = true;
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
