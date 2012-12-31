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
#include <memory>
#include <QEvent>

using namespace std;

#ifndef VM_H
    #include "../smallvm/vm_incl.h"
    #include "../smallvm/vm.h"
    #include "../smallvm/vm_ffi.h"
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

#ifndef DEBUGINFO_H
    #include "../debuginfo.h"
#endif

#ifndef BREAKPOINT_H
    #include "../breakpoint.h"
#endif

#ifndef VMCLIENT_H
    #include "vmclient.h"
#endif

#ifndef VMRUNNERTHREAD_H
    #include "vmrunnerthread.h"
#endif

#include "../smallvm/utils.h"

namespace Ui {
    class RunWindow;
}

enum RunWindowState
{
    rwNormal, rwTextInput, rwSuspended, rwWaiting
};

class GUISchedulerEvent : public QEvent
{
public:
    GUISchedulerEvent() :QEvent((QEvent::Type) 1200)
    {

    }
};

class RunWindow : public QMainWindow {
    Q_OBJECT
public:
    RunWindow(QWidget *parent, QString pathOfProgramsFile, VMClient *client = NULL);
    RunWindow(QString pathOfProgramsFile, VMClient *client = NULL);
    void setup(QString pathOfProgramsFile, VMClient *client);
    ~RunWindow();
    void Init(QString program, QMap<QString, QString> stringConstants, QSet<Breakpoint> breakPoints, DebugInfo debugInfo);
    void InitVMPrelude(VM *vm);
    void RegisterGuiControls(VM *vm);
    void RegisterQtTypes(VM *vm);
    void assert(Process *proc,  bool condition, VMErrorType errorType, QString errorMsg);
    QString pathOfRunningProgram();
    QString ensureCompletePath(Process *proc, QString fileName);
    VM *getVM() { return vm;}
private:
    QString pathOfProgramsFile;
    VM *vm;
    bool alreadyRunningScheduler;
    VMClient *client;
    WindowReadMethod *readMethod;
    QMap<VMErrorType, QString> ErrorMap;
    QString translate_error(VMError err);
    void reportError(VMError );
protected:
    void changeEvent(QEvent *e);
private:
    RunWindowState state;
    friend class VMRunthread;
    static void vmRunner(VM *vm, RunWindow *rw);
public:
    Value *readChannel;
    TextLayer textLayer;
    SpriteLayer spriteLayer;
    PaintSurface *paintSurface;

    Value *mouseEventChannel;
    Value *mouseDownEventChannel;
    Value *mouseUpEventChannel;
    Value *mouseMoveEventChannel;
    Value *kbEventChannel;
    void checkCollision(Sprite *s);
    void onCollision(Sprite *s1, Sprite *s2);
    void cls();
    void clearAllText();
    void setMouseDemoMode(bool enable);
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
    void suspend();
    void resume();
    void reactivateVM();

    void setAsleep(int cookie, Value *channel, int ms);
    bool isAsleep(int cookie);

    void typeCheck(Process *proc, Value *val, IClass *type);
    void typeError(Process *proc, IClass *expected, IClass *given);
    void beginInput();
    void Run();
    void RunGUIScheduler();
    //void singleStep(Process *proc);
    friend class WindowPrintMethod;
    friend class WindowReadMethod;
    void EmitGuiSchedule();
    void emitErrorEvent(VMError error);
signals:
    void guiSchedule(GUISchedulerEvent *);
    void errorSignal(VMError err);
private slots:
    void errorEvent(VMError err);
    void timerEvent(QTimerEvent *);
    void do_gui_schedule(GUISchedulerEvent *);
    void mousePressEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);
    void mouseMoveEvent(QMouseEvent *);

    void keyReleaseEvent(QKeyEvent *);
    void keyPressEvent(QKeyEvent *);

    void paintEvent(QPaintEvent *);
    void resizeEvent(QResizeEvent *event);
    void closeEvent(QCloseEvent *);
    void parentDestroyed(QObject *);
    void on_actionGC_triggered();
};

#endif // RUNWINDOW_H
