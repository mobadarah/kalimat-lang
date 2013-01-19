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
    visibleTextBuffer = QString(visibleTextLines * textLineWidth, ' ');
    colorBits.resize(visibleTextLines * textLineWidth);

    for(int i=0; i<visibleTextLines *textLineWidth; i++)
    {
        colorBits[i] = Qt::black;
    }
    currentColor = Qt::black;
}

void TextBuffer::overWriteChar(int pos, QChar c)
{
    visibleTextBuffer[pos] = c;
    colorBits[pos] = currentColor;
}

void TextBuffer::insertChar(int pos, QChar c)
{
    visibleTextBuffer.insert(pos, c);
    colorBits.insert(pos, currentColor);
}

void TextBuffer::removeChar(int pos)
{
    visibleTextBuffer.remove(pos,1);
    colorBits.remove(pos);
}

void TextBuffer::printChar(QChar c)
{
    if(c == '\n')
        nl();
    else
    {
        overWriteChar(cursor.pos, c);
        cursor.fwd();
    }
}

void TextBuffer::nl()
{
    if(owner->state == TextLayerState::Input && cursor.mode == Cursor::Insert)
    {
        // todo: Split current line if state is 'input' and cursor at insert mode
    }

    cursor.cr();
    cursor.lf();
    dirtyState = true;
}

void TextBuffer::scrollUp()
{
    visibleTextBuffer.remove(0, textLineWidth);
    visibleTextBuffer.append(QString(textLineWidth, ' '));
}

void TextBuffer::computeLineFormatRange(int i)
{
    computeLineFormatRange(i, line(i).trimmed(), lineFormats[i]);
}

void TextBuffer::computeLineFormatRange(int i, QString line, QList<QTextLayout::FormatRange> &range)
{
    int pos =0;
    int runPos = 0;
    int runLen = 0;
    QColor currentColor = colorBits[i * textLineWidth];
    range.clear();
    for(int j=0; j<line.length(); ++j)
    {
        if(colorBits[i * textLineWidth + j] != currentColor)
        {
            runLen = pos - runPos;
            QTextLayout::FormatRange r;
            r.start = runPos;
            r.length = runLen;
            r.format.setForeground(QBrush(currentColor));
            currentColor = colorBits[i * textLineWidth + j];
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
