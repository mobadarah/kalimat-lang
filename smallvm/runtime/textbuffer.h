#ifndef TEXTBUFFER_H
#define TEXTBUFFER_H

#include <QString>
#include <QVector>
#include <QList>
#include <QColor>
#include <QTextLayout>

#include "cursor.h"

struct ColorRun
{
    int startCol;
    int length;
};

const int visibleTextLines = 18;
const int textLineWidth = 54;

struct TextBuffer
{
    TextLayer *owner;
    Cursor cursor;
    QString visibleTextBuffer;
    QVector<QColor> colorBits;
    QList<QTextLayout::FormatRange> lineFormats[visibleTextLines];

    QColor currentColor;

    bool dirtyState;
    TextBuffer(TextLayer *owner);

    void clearText();
    void overWriteChar(int pos, QChar c);
    void insertChar(int pos, QChar c);
    void removeChar(int pos);
    void printChar(QChar c);
    void nl();

    void scrollUp();
    void updated() { dirtyState = false; }
    QString substring(int line1, int col1, int line2, int col2);

    const QString line(int i) const
    {
        return visibleTextBuffer.mid(i*textLineWidth, textLineWidth);
    }
    QString currentLine() { return line(cursor.line()); }
    const QList<QTextLayout::FormatRange> &formatRanges(int i) { return lineFormats[i];}
    void computeLineFormatRange(int i);
    void computeLineFormatRange(int i, QString line, QList<QTextLayout::FormatRange> &range);
};

#endif // TEXTBUFFER_H
