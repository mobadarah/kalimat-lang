/**************************************************************************
**   The Kalimat programming language
**   Copyright 2010 Mohamed Samy Ali - samy2004@gmail.com
**   This project is released under the Apache License version 2.0
**   as described in the included license.txt file
**************************************************************************/

#ifndef BUILTINMETHODS_H
#define BUILTINMETHODS_H

class RunWindow;
#ifndef CLASSES_H
    #include "../smallvm/classes.h"
#endif

#ifndef EASYFOREIGNCLASS_H
    #include "../smallvm/easyforeignclass.h"
#endif

typedef void (*VM_PROC)(QStack<Value *> &, RunWindow *, VM *);

void PrintProc(QStack<Value *> &, RunWindow *, VM *);
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
void CheckAsleepProc(QStack<Value *> &stack, RunWindow *w, VM *vm);

void ClsProc(QStack<Value *> &stack, RunWindow *, VM *w);
void ClearTextProc(QStack<Value *> &stack, RunWindow *w, VM *vm);
void SetTextColorProc(QStack<Value *> &stack, RunWindow *, VM *w);
void BuiltInConstantProc(QStack<Value *> &stack, RunWindow *, VM *vm);
void StringIsNumericProc(QStack<Value *> &stack, RunWindow *w, VM *vm);
void StringIsAlphabeticProc(QStack<Value *> &stack, RunWindow *w, VM *vm);
void TypeOfProc(QStack<Value *> &stack, RunWindow *w, VM *vm);

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

void StrBeginsWithProc(QStack<Value *> &stack, RunWindow *w, VM *vm);
void StrEndsWithProc(QStack<Value *> &stack, RunWindow *w, VM *vm);
void StrContainsProc(QStack<Value *> &stack, RunWindow *w, VM *vm);

void StrSplitProc(QStack<Value *> &stack, RunWindow *, VM *);
void StrTrimProc(QStack<Value *> &stack, RunWindow *w, VM *vm);

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

void EditProc(QStack<Value *> &stack, RunWindow *w, VM *vm);
void GetMainWindowProc(QStack<Value *> &stack, RunWindow *w, VM *vm);
void NewChannelProc(QStack<Value *> &stack, RunWindow *w, VM *vm);

double verifyNumeric(Value *v, RunWindow *w); // TODO: make this a method of RunWindow
int popIntOrCoercable(QStack<Value *> &stack, RunWindow *w, VM *vm);
double popDoubleOrCoercable(QStack<Value *> &stack, RunWindow *w, VM *vm);
int popInt(QStack<Value *> &stack, RunWindow *w, VM *vm);
bool popBool(QStack<Value *> &stack, RunWindow *w, VM *vm);
QString *popString(QStack<Value *> &stack, RunWindow *w, VM *vm);
Value *popValue(QStack<Value *> &stack, RunWindow *w, VM *vm);
void verifyStackNotEmpty(QStack<Value *> &stack, VM *vm);

Value *ConvertStringToNumber(QString str, VM *vm);

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

class WindowForeignClass : public EasyForeignClass
{
public:
    WindowForeignClass(QString name);
    IObject *newValue(Allocator *allocator);
    void dispatch(int id, QVector<Value *>args);
};

class ButtonForeignClass : public EasyForeignClass
{
    RunWindow *rw; // for TX
public:
    ButtonForeignClass(QString name, RunWindow *rw);
    IObject *newValue(Allocator *allocator);
    void dispatch(int id, QVector<Value *>args);

};

class WindowForeignObject : public IObject
{
public:
    Value *handle;
public:
    virtual bool hasSlot(QString name);
    virtual QList<QString> getSlotNames();
    virtual Value *getSlotValue(QString name);
    virtual void setSlotValue(QString name, Value *val);
    QString toString();
};

#endif // BUILTINMETHODS_H
