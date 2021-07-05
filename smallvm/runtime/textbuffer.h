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
const int textLineWidth = 50;

struct TextBuffer
{
    TextLayer *owner;
    Cursor cursor;
    QVector<QString> visibleTextBuffer;
    QVector<QVector<QColor> > colorBits;
    QList<QTextLayout::FormatRange> lineFormats[visibleTextLines];

    int scrollCount;
    QColor currentColor;

    bool dirtyState;
    TextBuffer(TextLayer *owner);

    void clearText();
    void printChar(QChar c);
    void nl();

    void scrollUp();
    void updated() { dirtyState = false; }
    QString substring(int line1, int col1, int line2, int col2);

    QString &lineAt(int i)
    {
        return visibleTextBuffer[i];
    }
    QString currentLine() { return lineAt(cursor.cursorLine()); }
    const QList<QTextLayout::FormatRange> &formatRanges(int i) { return lineFormats[i];}
    void computeLineFormatRange(int i);
    void computeLineFormatRange(int i, const QString &line, QList<QTextLayout::FormatRange> &range);
};

#endif // TEXTBUFFER_H
