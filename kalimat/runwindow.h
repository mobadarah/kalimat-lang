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

#ifndef VM_H
    #include "../smallvm/vm_incl.h"
    #include "../smallvm/vm.h"
#endif

#ifndef EXTERNALMETHOD_H
    #include "../smallvm/externalmethod.h"
#endif

#ifndef SPRITE_H
    #include "sprite.h"
#endif

#ifndef PAINTTIMER_H
    #include "painttimer.h"
#endif

#ifndef TEXTLAYER_H
    #include "textlayer.h"
#endif

#ifndef SPRITELAYER_H
    #include "spritelayer.h"
#endif

#ifndef PAINTSURFACE_H
    #include "paintsurface.h"
#endif

#ifndef BUILTINMETHODS_H
    #include "builtinmethods.h"
#endif

#ifndef CODEDOCUMENT_H
    #include "codedocument.h"
#endif

#ifndef DEBUGINFO_H
    #include "Compiler/debuginfo.h"
#endif

#ifndef BREAKPOINT_H
    #include "breakpoint.h"
#endif

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
    void Init(QString program, QMap<QString, QString> stringConstants, QSet<Breakpoint> breakPoints, DebugInfo debugInfo);
    void InitVMPrelude(VM *vm);
    void assert(bool condition,  VMErrorType errorType, QString errorMsg);
    QString pathOfRunningProgram();
    QString ensureCompletePath(QString fileName);
    VM *getVM() { return vm;}
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
    QMap<int, Value *> asleep;
public:
    Value *readChannel;
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
    void setBreakpoint(Breakpoint, const DebugInfo &);

    void redrawWindow();
    void resetTimer(int interval);
    void suspend();
    void resume();
    void reactivateVM();

    void setAsleep(int cookie, Value *channel);
    bool isAsleep(int cookie);

    void typeCheck(Value *val, ValueClass *type);
    void typeError(IClass *expected, IClass *given);
    void beginInput();
    void Run();
    void singleStep(Process *proc);
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
