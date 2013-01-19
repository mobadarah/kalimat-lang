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
#include <QImage>

#include "textbuffer.h"

namespace TextLayerState
{
    enum State { Normal, Input };
}

class TextLayer
{
    TextLayerState::State state;

    TextBuffer buffer;

    int imgWidth, imgHeight, _stripHeight;
    QFont textFont;

    QPixmap strips[visibleTextLines];

    void scrollUp();

    void adjustFontForNumberOfLines(int n);
    void updateChangedLines(int fromLine, int count);

    void fastUpdateStrip(int i, bool drawCursor= false);
    void TX(int &x);

    void updateTextLine(int lineIndex);
    friend class Cursor;
    friend struct TextBuffer;
public:

    TextLayer();
    void Init(int width, int height, QFont font);
    bool dirty();
    void updated();

    void print(QString);
    void println(QString);
    void print(QString str, int width);
    int cursorLine() { return buffer.cursor.line(); }
    int cursorColumn() { return buffer.cursor.column(); }

    void nl() { buffer.nl(); }
    void del() { buffer.cursor.del(); }
    void backSpace() { buffer.cursor.backSpace(); }
    void cursorFwd() { buffer.cursor.fwd(); }
    void cursorBack() { buffer.cursor.back(); }

    void updateStrip(int i, bool drawCursor= false);

    void clearText();
    int getCursorCol();
    int getCursorRow();
    bool setCursorPos(int row, int col);
    int cursorPos();
    void beginInput();
    QString endInput();
    void typeIn(QString s);
    bool inputState();

    void setColor(QColor);
    void resetColor();

    const QPixmap &strip(int i) { return strips[i]; }
    int stripHeight();

    QString formatStringUsingWidth(QString str, int width);
};

#endif // TEXTLAYER_H
