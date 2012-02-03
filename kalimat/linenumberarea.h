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
