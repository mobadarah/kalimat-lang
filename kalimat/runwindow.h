/**************************************************************************
** The Kalimat programming language
**   Copyright (C) 2010 Mohamed Samy Ali - samy2004@gmail.com
**   This project is released under the Apache License version 2.0
**   as described in the included license.txt file
**************************************************************************/

#ifndef RUNWINDOW_H
#define RUNWINDOW_H

#include <QMainWindow>
#include <QTimer>

#include "../smallvm/vm_incl.h"
#include "../smallvm/vm.h"
#include "../smallvm/externalmethod.h"
#include "sprite.h"
#include "painttimer.h"
#include "textlayer.h"
#include "spritelayer.h"
#include "paintsurface.h"

#include "builtinmethods.h"

namespace Ui {
    class RunWindow;
}

enum RunWindowState
{
    rwNormal, rwTextInput, rwSuspended, rwWaiting
};

class RunWindow : public QMainWindow {
    Q_OBJECT
public:
    RunWindow(QWidget *parent, QString pathOfProgramsFile);
    ~RunWindow();
    void Init(QString program, QMap<QString, QString> stringConstants);
    void InitVMPrelude(VM *vm);
    void assert(bool condition,  VMErrorType errorType, QString errorMsg);
    QString pathOfRunningProgram();
    QString ensureCompletePath(QString fileName);
private:
    QString pathOfProgramsFile;
    VM *vm;
    WindowReadMethod *readMethod;
    QMap<VMErrorType, QString> ErrorMap;
    QString translate_error(VMError err);
    void reportError(VMError );

protected:
    void changeEvent(QEvent *e);
private:
    RunWindowState state;
    bool asleep;
public:
    TextLayer textLayer;
    SpriteLayer spriteLayer;
    PaintSurface *paintSurface;
    void checkCollision(Sprite *s);
    void onCollision(Sprite *s1, Sprite *s2);
    void cls();
    void clearAllText();
private:
    int timerID;
public:
    PaintTimer updateTimer;
private:
    Ui::RunWindow *ui;
    void activateMouseEvent(QMouseEvent *ev, QString evName);
    void activateKeyEvent(QKeyEvent *ev, QString evName);
public:
    void redrawWindow();
    void resetTimer(int interval);
    void suspend();
    void resume();

    void setAsleep();
    bool isAsleep();

    void typeCheck(Value *val, ValueClass *type);
    void typeError(ValueClass *givenType);
    void beginInput();
    void Run();
    friend class WindowPrintMethod;
    friend class WindowReadMethod;

private slots:
    void mousePressEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);
    void mouseMoveEvent(QMouseEvent *);

    void keyReleaseEvent(QKeyEvent *);
    void keyPressEvent(QKeyEvent *);

    void paintEvent(QPaintEvent *);
    void resizeEvent(QResizeEvent *event);
    void closeEvent(QCloseEvent *);
    void parentDestroyed(QObject *);
    void timerEvent(QTimerEvent *);
    void on_actionGC_triggered();
};


#endif // RUNWINDOW_H
