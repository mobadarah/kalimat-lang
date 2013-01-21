#include "textbuffer.h"
#include "textlayer.h"

TextBuffer::TextBuffer(TextLayer *owner)
    :owner(owner), cursor(owner)
{
    visibleTextBuffer.reserve(visibleTextLines);
}

void TextBuffer::clearText()
{
    cursor.reset();
    dirtyState = true;
    visibleTextBuffer.clear();
    visibleTextBuffer.resize(visibleTextLines);
    colorBits.resize(visibleTextLines);

    for(int i=0; i<visibleTextLines; ++i)
    {
        colorBits[i].resize(textLineWidth);
        for(int j=0; j<textLineWidth; ++j)
            colorBits[i][j] = Qt::black;
    }
    currentColor = Qt::black;
}

void TextBuffer::printChar(QChar c)
{
    if(c == '\n')
        nl();
    else
    {
        QString &s= lineAt(cursor.line);
        if(s.length() < (cursor.col+1))
        {
            int delta = cursor.col + 1 - s.length();
            s+= QString(delta, ' ');
            for(int i=0; i<delta; ++i)
                colorBits[cursor.line].append(currentColor);
        }
        s[cursor.col] = c;
        colorBits[cursor.line][cursor.col] = currentColor;
        cursor.fwd();
    }
}

void TextBuffer::nl()
{
    if(owner->state == TextLayerState::Input && cursor.mode == Cursor::Insert)
    {
        // todo: Split current line if state is 'input' and cursor at insert mode
    }

    cursor.lf();
    cursor.cr();
    dirtyState = true;
}

void TextBuffer::scrollUp()
{
    scrollCount++;
    visibleTextBuffer.remove(0);
    visibleTextBuffer.append("");
}

void TextBuffer::computeLineFormatRange(int i)
{
    computeLineFormatRange(i, lineAt(i), lineFormats[i]);
}

void TextBuffer::computeLineFormatRange(int i, const QString &line, QList<QTextLayout::FormatRange> &range)
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
