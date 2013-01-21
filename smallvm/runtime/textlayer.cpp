/**************************************************************************
**   The Kalimat programming language
**   Copyright 2010 Mohamed Samy Ali - samy2004@gmail.com
**   This project is released under the Apache License version 2.0
**   as described in the included license.txt file
**************************************************************************/

#include "textlayer.h"
#include <QPainter>

TextLayer::TextLayer()
    : buffer(this)
{
    state = TextLayerState::Normal;
    buffer.dirtyState = false;
    clearText();
}

void TextLayer::Init(int width, int height, QFont font)
{
    imgWidth = width;
    imgHeight = height;
    textFont = font;
    adjustFontForNumberOfLines(visibleTextLines);
    _stripHeight = imgHeight / visibleTextLines;
    for(int i=0; i< visibleTextLines; ++i)
    {
        strips[i] = QPixmap(imgWidth, _stripHeight);
        strips[i].fill(Qt::transparent);
    }
}

int TextLayer::stripHeight()
{
    return _stripHeight;
}

void TextLayer::TX(int &x)
{
    x = (imgWidth-1)-x;
}

void TextLayer::adjustFontForNumberOfLines(int n)
{
    // Start with a first approximation, and keep decreasing font
    // size until all text lines fit into the display image
    qreal h = imgHeight / n;
    textFont.setPixelSize(h);
    QFontMetrics metrics(textFont);
    while(true)
    {
        qreal line_h = metrics.lineSpacing();
        if(n * (line_h) < imgHeight)
            break;
        h-=0.5;
        textFont.setPixelSize(h);
        metrics = QFontMetrics(textFont);
    }
}

bool TextLayer::dirty()
{
    if(inputState())
        return true; // we need to keep refreshing to make the cursor blink

    return buffer.dirtyState;
}

void TextLayer::updated()
{
    buffer.updated();
}

void TextLayer::clearText()
{
    buffer.clearText();
    for(int i=0; i<visibleTextLines; i++)
    {
        strips[i].fill(Qt::transparent);
    }
}

// 'print' always overwrites
void TextLayer::print(QString str)
{
    if(str == "")
        return;
    int curLine = cursorLine();
    buffer.scrollCount = 0;
    for(int i=0; i<str.length(); i++)
    {
        buffer.printChar(str[i]);
    }
    int curLine2 = cursorLine();
    if(str != "\n")
    {
        updateChangedLines(curLine, 1 + curLine2 - curLine);
    }
    if(buffer.scrollCount > 0)
    {
        updateChangedLines(curLine2-buffer.scrollCount, buffer.scrollCount);
    }
    buffer.dirtyState = true;
}

void TextLayer::print(QString str, int width)
{
    if(str == "\n")
    {
        // todo: How does string formatting with a certain width affect printing of a newline, anyway?
        buffer.nl();
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
    buffer.nl();
}

void TextLayer::setColor(QColor c)
{
    buffer.currentColor = c;
}

void TextLayer::resetColor()
{
    buffer.currentColor = Qt::black;
}

void TextLayer::updateChangedLines(int fromLine, int count)
{
    for(int i=0; i<count; ++i)
    {
        updateTextLine(i + fromLine);
        updateStrip(i + fromLine);
    }
}

void TextLayer::updateStrip(int i, bool drawCursor)
{
    strips[i].fill(Qt::transparent);
    QPainter imgPainter(&strips[i]);
    QTextLayout layout("", textFont, &strips[i]);
    QTextOption option;
    option.setTextDirection(Qt::RightToLeft);
    option.setAlignment(Qt::AlignVCenter | Qt::AlignRight);
    layout.setTextOption(option);

    layout.setFont(textFont);
    imgPainter.setFont(textFont);

    qreal height = imgPainter.fontMetrics().lineSpacing();
    qreal subtractSomeHeigh = 0;

    const QString &text = buffer.lineAt(i);
    const QList<QTextLayout::FormatRange> range = buffer.formatRanges(i);

    layout.setText(text);
    layout.setAdditionalFormats(range);
    layout.beginLayout();
    int temph;
    while(true)
    {
        QTextLine line = layout.createLine();
        if(!line.isValid())
            break;
        line.setLineWidth(imgWidth);
        line.setPosition(QPointF(-5, 0));
        temph = line.height()-1;
        if(temph > height)
        {
            subtractSomeHeigh = ((temph - height) / 2) - 2;
        }
        else
        {
            subtractSomeHeigh = 0;
        }
    }
    layout.endLayout();
    layout.draw(&imgPainter, QPointF(0, -subtractSomeHeigh));
    if(drawCursor)
        layout.drawCursor(&imgPainter, QPointF(0, - subtractSomeHeigh), cursorColumn(), 2);
}

void TextLayer::fastUpdateStrip(int i, bool drawCursor)
{
    // Faster, but cannot give each individual letters a different color
    strips[i].fill(Qt::transparent);
    QPainter imgPainter(&strips[i]);

    imgPainter.setLayoutDirection(Qt::RightToLeft);
    imgPainter.setFont(textFont);

    QPen oldPen = imgPainter.pen();
    //imgPainter.setPen(textColor);

    const QString &text = buffer.lineAt(i);
    QRect rct(0, 0, imgWidth-1, _stripHeight);

    imgPainter.drawText(rct, Qt::AlignVCenter, text);

    if(drawCursor)
    {
        QString line = buffer.currentLine().left(cursorColumn());
        int w = imgPainter.fontMetrics().tightBoundingRect(line).width();
        w+=2;
        int cursorY = rct.top();
        TX(w);
        QRectF cur(w, cursorY, 2, _stripHeight);
        //cur.adjust(-2, 0, -2, 0);
        imgPainter.drawLine(cur.topRight(), cur.bottomRight());
    }

    imgPainter.setPen(oldPen);
}

void TextLayer::updateTextLine(int lineIndex)
{
    buffer.computeLineFormatRange(lineIndex);
}

void TextLayer::beginInput()
{
    state = TextLayerState::Input;
    buffer.cursor.beginInput();
}

QString TextLayer::endInput()
{
    state = TextLayerState::Normal;
    return buffer.cursor.endInput();
}

void TextLayer::typeIn(QString s)
{
    buffer.cursor.typeIn(s);
}

bool TextLayer::inputState()
{
    return state == TextLayerState::Input;
}

bool TextLayer::setCursorPos(int row, int col)
{
    return buffer.cursor.setCursorPos(row, col);
}

int TextLayer::getCursorRow()
{
    return buffer.cursor.cursorLine();
}

int TextLayer::getCursorCol()
{
    return buffer.cursor.cursorColumn();
}

void TextLayer::scrollUp()
{
    for(int i=0; i<visibleTextLines -1; ++i)
    {
        strips[i] = strips[i+1];
    }
    strips[visibleTextLines-1] = QPixmap(imgWidth, imgHeight/visibleTextLines);
    strips[visibleTextLines-1].fill(Qt::transparent);
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
