/**************************************************************************
**   The Kalimat programming language
**   Copyright 2010 Mohamed Samy Ali - samy2004@gmail.com
**   This project is released under the Apache License version 2.0
**   as described in the included license.txt file
**************************************************************************/

#ifndef MYEDIT_H
#define MYEDIT_H

#include "linetracker.h"

#include <QTextEdit>
#include <QKeyEvent>
class MainWindow;
class MyEdit : public QTextEdit
{
    Q_OBJECT
    LineTracker lineTracker;
    MainWindow *owner;
    int _line, _column;
public:
    MyEdit(MainWindow *owner);
    int line();
    int column();
    void tabBehavior();
    void shiftTabBehavior();
    void enterKeyBehavior(QKeyEvent *ev);
    void colonBehavior(QKeyEvent *ev);
    LineInfo currentLine();
    int indentOfLine(LineInfo li);
    QString textOfLine(LineInfo line);
    void indentAndTerminate(LineInfo prevLine, QString termination);
    int calculateDeindent(int by, QString lineText);
    void eraseFromBeginOfLine(LineInfo li, int toErase);
    void deindentLine(int line, int by);
private slots:
    void keyPressEvent(QKeyEvent *);
    void textChangedEvent();
    void selectionChangedEvent();
};

#endif // MYEDIT_H
