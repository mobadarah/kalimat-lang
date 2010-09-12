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

namespace Ui {
    class RunWindow;
}

enum RunWindowState
{
    rwNormal, rwTextInput, rwSuspended, rwWaiting
};

class RunWindow;

typedef void (*VM_PROC)(QStack<Value *> &, RunWindow *, VM *);

void SetCursorPosProc(QStack<Value *> &, RunWindow *, VM *);
void GetCursorRowProc(QStack<Value *> &, RunWindow *, VM *);
void GetCursorColProc(QStack<Value *> &, RunWindow *, VM *);

void PrintUsingWidthProc(QStack<Value *> &, RunWindow *, VM *);
void DrawPixelProc(QStack<Value *> &, RunWindow *, VM *);
void DrawLineProc(QStack<Value *> &, RunWindow *, VM *);
void DrawRectProc(QStack<Value *> &, RunWindow *, VM *);
void DrawCircleProc(QStack<Value *> &, RunWindow *, VM *);
void LoadSpriteProc(QStack<Value *> &, RunWindow *, VM *);
void DrawSpriteProc(QStack<Value *> &, RunWindow *, VM *);
void ZoomProc(QStack<Value *> &, RunWindow *, VM *);
void HideSpriteProc(QStack<Value *> &, RunWindow *, VM *);
void ShowSpriteProc(QStack<Value *> &, RunWindow *, VM *);

void GetSpriteLeftProc(QStack<Value *> &stack, RunWindow *w, VM *vm);
void GetSpriteRightProc(QStack<Value *> &stack, RunWindow *w, VM *vm);
void GetSpriteTopProc(QStack<Value *> &stack, RunWindow *w, VM *vm);
void GetSpriteBottomProc(QStack<Value *> &stack, RunWindow *w, VM *vm);
void GetSpriteWidthProc(QStack<Value *> &stack, RunWindow *w, VM *vm);
void GetSpriteHeightProc(QStack<Value *> &stack, RunWindow *w, VM *vm);

void WaitProc(QStack<Value *> &, RunWindow *, VM *);
void ClsProc(QStack<Value *> &stack, RunWindow *, VM *w);
void ClearTextProc(QStack<Value *> &stack, RunWindow *w, VM *vm);
void SetTextColorProc(QStack<Value *> &stack, RunWindow *, VM *w);

void RandomProc(QStack<Value *> &stack, RunWindow *, VM *w);
void ToNumProc(QStack<Value *> &stack, RunWindow *, VM *w);
void ToStringProc(QStack<Value *> &stack, RunWindow *, VM *);
void RoundProc(QStack<Value *> &stack, RunWindow *, VM *);
void RemainderProc(QStack<Value *> &stack, RunWindow *, VM *);
void ConcatProc(QStack<Value *> &stack, RunWindow *, VM *);
void StrFirstProc(QStack<Value *> &stack, RunWindow *, VM *);
void StrLastProc(QStack<Value *> &stack, RunWindow *, VM *);
void StrLenProc(QStack<Value *> &stack, RunWindow *, VM *);
void StrMidProc(QStack<Value *> &stack, RunWindow *, VM *);
void StrSplitProc(QStack<Value *> &stack, RunWindow *, VM *);

void SinProc(QStack<Value *> &stack, RunWindow *, VM *);
void CosProc(QStack<Value *> &stack, RunWindow *, VM *);
void TanProc(QStack<Value *> &stack, RunWindow *, VM *);
void ASinProc(QStack<Value *> &stack, RunWindow *, VM *);
void ACosProc(QStack<Value *> &stack, RunWindow *, VM *);
void ATanProc(QStack<Value *> &stack, RunWindow *, VM *);

void SqrtProc(QStack<Value *> &stack, RunWindow *, VM *);
void Log10Proc(QStack<Value *> &stack, RunWindow *, VM *);
void LnProc(QStack<Value *> &stack, RunWindow *, VM *);

void FileWriteProc(QStack<Value *> &stack, RunWindow *, VM *);
void FileWriteUsingWidthProc(QStack<Value *> &stack, RunWindow *, VM *);
void FileWriteLineProc(QStack<Value *> &stack, RunWindow *, VM *);
void FileReadLineProc(QStack<Value *> &stack, RunWindow *, VM *);
void FileEofProc(QStack<Value *> &stack, RunWindow *w, VM *vm);
void FileOpenProc(QStack<Value *> &stack, RunWindow *, VM *);
void FileCreateProc(QStack<Value *> &stack, RunWindow *, VM *);
void FileAppendProc(QStack<Value *> &stack, RunWindow *, VM *);
void FileCloseProc(QStack<Value *> &stack, RunWindow *, VM *);

double verifyNumeric(Value *v, RunWindow *w); // TODO: make this a method of RunWindow
int popIntOrCoercable(QStack<Value *> &stack, RunWindow *w, VM *vm);
double popDoubleOrCoercable(QStack<Value *> &stack, RunWindow *w, VM *vm);
int popInt(QStack<Value *> &stack, RunWindow *w, VM *vm);
QString *popString(QStack<Value *> &stack, RunWindow *w, VM *vm);
void verifyStackNotEmpty(QStack<Value *> &stack, VM *vm);

Value *ConvertStringToNumber(QString str, VM *vm);

class WindowPrintMethod : public ExternalMethod
{
    RunWindow *parent;
public:
    WindowPrintMethod(RunWindow *parent);
    void operator()(QStack<Value *> &operandStack);
};
class WindowReadMethod : public ExternalMethod
{
    RunWindow *parent;
    QStack<Value *> *operandStack;
    VM *vm;
public:
    int readNum;
public:
    WindowReadMethod(RunWindow *parent, VM *vm);
    void operator()(QStack<Value *> &operandStack);
    void SetReadValue(Value *v);
};

class WindowProxyMethod : public ExternalMethod
{
    RunWindow *parent;
    VM_PROC proc;
    VM *vm;
public:
    WindowProxyMethod(RunWindow *parent, VM *vm, VM_PROC proc);
    void operator()(QStack<Value *> &operandStack);
};

class RunWindow : public QMainWindow {
    Q_OBJECT
public:
    RunWindow(QWidget *parent, QString pathOfProgramsFile);
    ~RunWindow();
    void Init(QString program, QMap<QString, QString> stringConstants);
    void InitVMPrelude(VM *vm);
    QImage *GetImage();
    QColor GetColor(int color);
    void assert(bool condition,  VMErrorType errorType, QString errorMsg);
    void setTextColor(QColor);
    QString pathOfRunningProgram();
    QString ensureCompletePath(QString fileName);
private:
    QString pathOfProgramsFile;
    VM *vm;
    WindowPrintMethod *printMethod;
    WindowReadMethod *readMethod;
    QMap<VMErrorType, QString> ErrorMap;
    QString translate_error(VMError err);
    void reportError(QString msg, VMError );

protected:
    void changeEvent(QEvent *e);
private:
    QImage image;
    RunWindowState state;
    QString inputBuffer;

public:
    TextLayer textLayer;
    SpriteLayer spriteLayer;
    void checkCollision(Sprite *s);
    void onCollision(Sprite *s1, Sprite *s2);
private:
    QFont textFont;
    QColor textColor;

    int timerID;
public:
    PaintTimer updateTimer;
private:
    Ui::RunWindow *ui;
    void resizeImage(QImage *image, const QSize &newSize);

    void activateMouseEvent(QMouseEvent *ev, QString evName);
    void activateKeyEvent(QKeyEvent *ev, QString evName);


public:
    void redrawWindow();
    QRect cursor();
    void TX(int &);
    void resetTimer(int interval);
    void suspend();
    void resume();
    void typeCheck(Value *val, ValueClass *type);
    void typeError(ValueClass *givenType);
    void zoom(int x1, int y1, int x2, int y2);
    void cls();
    void clearAllText();
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
    friend class WindowPrintMethod;
    friend class WindowReadMethod;

    void timerEvent(QTimerEvent *);
    void Run();
    void on_actionGC_triggered();
};


#endif // RUNWINDOW_H
