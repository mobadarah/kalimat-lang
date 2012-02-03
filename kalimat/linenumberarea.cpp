#include "linenumberarea.h"
#include <QScrollBar>
#include <QAbstractTextDocumentLayout>
#include <QTextEdit>
#include <QTextBlock>
#include <QPainter>
#include <math.h>

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
    QString str = QString("%1").arg(highest_line);
    int width = edit->fontMetrics().width(str) + 4;
    return width;
}

void LineNumberArea::paintEvent(QPaintEvent *event)
{
    int contents_y = edit->verticalScrollBar()->value();
    int page_bottom = contents_y + edit->viewport()->height();
    QFontMetrics font_metrics = edit->fontMetrics();
    QTextBlock current_block = edit->document()->findBlock(
                edit->textCursor().position());

    QPainter painter(this);
    painter.fillRect(event->rect(), Qt::lightGray);
    int line_count = 0;
    // Iterate over all text blocks in the document.
    QTextBlock block = edit->document()->begin();
    painter.setPen(QPen(Qt::darkGray));
    while (block.isValid())
    {
        line_count += 1;

        // The top left position of the block in the document
        QPointF position = edit->document()->documentLayout()->blockBoundingRect(block).topLeft();

        // Check if the position of the block is out side of the visible
        // area.
        if (position.y() > page_bottom)
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

        painter.drawText(width() - font_metrics.width(str) - 3,
                         round(position.y()) - contents_y + font_metrics.ascent(),
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
    this->highest_line = line_count;
    painter.end();
}
