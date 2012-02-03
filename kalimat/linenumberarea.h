#ifndef LINENUMBERAREA_H
#define LINENUMBERAREA_H

#include <QWidget>
#include "myedit.h"

class LineNumberArea : public QWidget
{
    int highest_line;
public:
    LineNumberArea(MyEdit *editor) : QWidget(editor)
    {
        edit = editor;
        // This is used to update the width of the control.
        // It is the highest line that is currently visibile.
        highest_line = 0;
    }

    void setTextEdit(MyEdit *edit)
    {
        this->edit = edit;
    }
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
