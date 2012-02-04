/*
  This file is based on code by John Schember.
  The code is located at john.nachtimwald.com/2009/08/15/qtextedit-with-line-numbers/
  and released under an MIT License. License text for his code follows:

The MIT License

Copyright (c) 2009 John Schember <john@nachtimwald.com>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE

*/

#include "linenumberarea.h"
#include <QScrollBar>
#include <QAbstractTextDocumentLayout>
#include <QTextEdit>
#include <QTextBlock>
#include <QPainter>
#include <math.h>

LineNumberArea::LineNumberArea(MyEdit *editor)
    :QWidget(editor)
{
    setTextEdit(editor);
    // This is used to update the width of the control.
    // It is the highest line that is currently visibile.
    highest_line = 0;
}

void LineNumberArea::setTextEdit(MyEdit *edit)
{
    this->edit = edit;
}

void LineNumberArea::setFontPointSize(int f)
{
    QFont ft = edit->font();
    ft.setFamily("Arial");
    ft.setPointSize(f);
    setFont(ft);
}

void LineNumberArea::update(const QRect &)
{
    /*
    Updates the number bar to display the current set of numbers.
    Also, adjusts the width of the number bar if necessary.

    The + 4 is used to compensate for the current line being bold.
    */
    int width = getWidth();
    if(this->width() != width)
        setFixedWidth(width);
    QWidget::update();
}

int LineNumberArea::getWidth()
{
    int ln = highest_line>9? highest_line : 99;
    QString str = QString("%1").arg(ln);
    int width = fontMetrics().width(str) + 5;
    return width;
}

void LineNumberArea::paintEvent(QPaintEvent *event)
{
    int contents_y = edit->verticalScrollBar()->value();
    int page_bottom = contents_y + edit->viewport()->height();
    QFontMetrics font_metrics = fontMetrics();
    QTextBlock current_block = edit->document()->findBlock(
                edit->textCursor().position());

    QPainter painter(this);
    painter.fillRect(event->rect(), Qt::lightGray);
    // Iterate over all text blocks in the document.
    int firstBlockLine = edit->verticalScrollBar()->value()
            / edit->fontMetrics().height() - 2;
    if(firstBlockLine < 0)
        firstBlockLine = 0;
    QTextBlock block = edit->document()->findBlockByLineNumber(firstBlockLine);
    painter.setPen(QPen(Qt::darkGray));
    int line_count = firstBlockLine +1;
    while (block.isValid())
    {

        // The top left position of the block in the document
        QPointF position = edit->document()->documentLayout()->blockBoundingRect(block).topLeft();

        // Check if the position of the block is out side of the visible
        // area.
        if (position.y() > page_bottom)
            break;
        if (!block.isVisible())
            break;
        // We want the line number for the selected line to be bold.
        bool bold = false;
        if (block == current_block)
        {
            bold = true;
            QFont font = painter.font();
            font.setBold(true);
            painter.setFont(font);
        }
        // Draw the line number right justified at the y position of the
        // line. 3 is a magic padding number. drawText(x, y, text).

        QString str= QString("%1").arg(line_count);
        line_count++;
        painter.drawText(width() - (font_metrics.width(str) + 3),
                         round(position.y()) - contents_y + font_metrics.ascent()+2,
                         str);

        // Remove the bold style if it was set previously.
        if(bold)
        {
            QFont font = painter.font();
            font.setBold(false);
            painter.setFont(font);
        }
        block = block.next();
    }
    painter.end();
}
