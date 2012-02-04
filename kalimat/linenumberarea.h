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

#ifndef LINENUMBERAREA_H
#define LINENUMBERAREA_H

#include <QWidget>
#include "myedit.h"

class LineNumberArea : public QWidget
{
public:
    int highest_line;
public:
    LineNumberArea(MyEdit *editor);

    void setTextEdit(MyEdit *edit);
    void setFontPointSize(int f);
    void update(const QRect &);
    int getWidth();
   /* QSize sizeHint() const {
        return QSize(edit->lineNumberAreaWidth(), 0);
    }*/

protected:
    void paintEvent(QPaintEvent *event);

private:
    MyEdit *edit;
};

#endif // LINENUMBERAREA_H
