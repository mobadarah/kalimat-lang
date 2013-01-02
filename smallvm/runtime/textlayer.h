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
#include <QColor>
#include <QStringList>
#include <QTextLayout>

struct ColorRun
{
    int startCol;
    int length;
};

const int visibleTextLines = 18;
const int textLineWidth = 54;


class TextLayer
{
    enum State { Normal, Input };
    enum Mode { Insert, Overwrite};
    State state;
    Mode mode, oldMode;
    QString inputBuffer;
    int inputStartLine, inputStartCol;

    QVector<QString> visibleTextBuffer;
    QStringList htmlLines;
    QColor colorBits[visibleTextLines][textLineWidth];
    QList<QTextLayout::FormatRange> lineFormats[visibleTextLines];
    int cursor_col, cursor_line;
    bool dirtyState;
    void printChar(QChar c);
    void cr();
    void lf();
    QColor currentColor;
    bool noColor;
public:
    TextLayer();
    bool dirty();
    void updated() { dirtyState = false; }
    const QVector<QString> &lines() const { return visibleTextBuffer; }
    const QList<QTextLayout::FormatRange> &formatRanges(int i) { return lineFormats[i];}
    void print(QString);
    void println(QString);
    void print(QString str, int width);
    int cursorLine() { return cursor_line; }
    int cursorColumn() { return cursor_col; }
    int inputCursorPos() { return cursor_col - inputStartCol; }
    QString currentLine() { return visibleTextBuffer[cursor_line]; }
    void nl();
    void del();
    void backSpace();
    void clearText();
    int getCursorCol();
    int getCursorRow();
    bool setCursorPos(int row, int col);
    bool cursorFwd();
    bool cursorBack();
    bool cursorDown();
    bool cursorUp();

    void setColor(QColor);
    void resetColor();
    void updateChangedLines(int fromLine, int count);
    void updateHtmlLine(int i);
    QString toHtml();
    QString toText(QList<QTextLayout::FormatRange> &range);
    QString lineToText(int line, QList<QTextLayout::FormatRange> &range);
    void computeLineFormatRange(int i, QString &line, QList<QTextLayout::FormatRange> &range);
    void updateTextLine(int lineIndex);
    int cursorPos();
    void beginInput();
    QString endInput();
    void typeIn(QString s);
    bool inputState();
    QString formatStringUsingWidth(QString str, int width);
};

#endif // TEXTLAYER_H
