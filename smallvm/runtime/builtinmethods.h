/**************************************************************************
**   The Kalimat programming language
**   Copyright 2010 Mohamed Samy Ali - samy2004@gmail.com
**   This project is released under the Apache License version 2.0
**   as described in the included license.txt file
**************************************************************************/

#ifndef BUILTINMETHODS_H
#define BUILTINMETHODS_H


class RunWindow;

#ifndef VM_H
#include "../smallvm/vm.h"
#endif

Sprite *GetSpriteFromValue(Value * v);
Value *MakeSpriteValue(Sprite *sprite, Allocator *alloc);

typedef void (*VM_PROC)(QStack<Value *> &, Process *proc, RunWindow *, VM *);

void PrintProc(QStack<Value *> &, Process *proc, RunWindow *, VM *);
void PushReadChanProc(QStack<Value *> &stack, Process *, RunWindow *w, VM *);
void SetCursorPosProc(QStack<Value *> &, Process *proc, RunWindow *, VM *);
void GetCursorRowProc(QStack<Value *> &, Process *proc, RunWindow *, VM *);
void GetCursorColProc(QStack<Value *> &, Process *proc, RunWindow *, VM *);

void PrintUsingWidthProc(QStack<Value *> &, Process *, RunWindow *, VM *);
void DrawPixelProc(QStack<Value *> &, Process *proc, RunWindow *, VM *);
void DrawLineProc(QStack<Value *> &, Process *proc, RunWindow *, VM *);
void DrawRectProc(QStack<Value *> &, Process *proc, RunWindow *, VM *);
void DrawCircleProc(QStack<Value *> &, Process *proc, RunWindow *, VM *);

void LoadImageProc(QStack<Value *> &stack, Process *proc, RunWindow *w, VM *vm);
void LoadSpriteProc(QStack<Value *> &, Process *proc, RunWindow *, VM *);
void SpriteFromImageProc(QStack<Value *> &stack, Process *proc, RunWindow *w, VM *vm);
void DrawImageProc(QStack<Value *> &stack, Process *proc, RunWindow *w, VM *vm);
void DrawSpriteProc(QStack<Value *> &, Process *proc, RunWindow *, VM *);
void ZoomProc(QStack<Value *> &, Process *proc, RunWindow *, VM *);
void HideSpriteProc(QStack<Value *> &, Process *proc, RunWindow *, VM *);
void ShowSpriteProc(QStack<Value *> &, Process *proc, RunWindow *, VM *);

void GetSpriteLeftProc(QStack<Value *> &stack, Process *proc, RunWindow *w, VM *vm);
void GetSpriteRightProc(QStack<Value *> &stack, Process *proc, RunWindow *w, VM *vm);
void GetSpriteTopProc(QStack<Value *> &stack, Process *proc, RunWindow *w, VM *vm);
void GetSpriteBottomProc(QStack<Value *> &stack, Process *proc, RunWindow *w, VM *vm);
void GetSpriteWidthProc(QStack<Value *> &stack, Process *proc, RunWindow *w, VM *vm);
void GetSpriteHeightProc(QStack<Value *> &stack, Process *proc, RunWindow *w, VM *vm);
void GetSpriteImageProc(QStack<Value *> &stack, Process *proc, RunWindow *w, VM *vm);
void SetSpriteImageProc(QStack<Value *> &stack, Process *proc, RunWindow *w, VM *vm);

void WaitProc(QStack<Value *> &, Process *proc, RunWindow *, VM *);
void MouseEventChanProc(QStack<Value *> &stack, Process *, RunWindow *w, VM *);
void MouseDownEventChanProc(QStack<Value *> &stack, Process *, RunWindow *w, VM *);
void MouseUpEventChanProc(QStack<Value *> &stack, Process *proc, RunWindow *w, VM *vm);
void MouseMoveEventChanProc(QStack<Value *> &stack, Process *, RunWindow *w, VM *);
void KbEventChanProc(QStack<Value *> &stack, Process *, RunWindow *w, VM *);

void ClsProc(QStack<Value *> &stack, Process *proc, RunWindow *, VM *w);
void ClearTextProc(QStack<Value *> &stack, Process *proc, RunWindow *w, VM *vm);
void SetTextColorProc(QStack<Value *> &stack, Process *proc, RunWindow *, VM *w);
void PointAtProc(QStack<Value *> &stack, Process *proc, RunWindow *w, VM *vm);
void PointRgbAtProc(QStack<Value *> &stack, Process *proc, RunWindow *w, VM *vm);
void BuiltInConstantProc(QStack<Value *> &stack, Process *proc, RunWindow *, VM *vm);
void StringIsNumericProc(QStack<Value *> &stack, Process *proc, RunWindow *w, VM *vm);
void StringIsAlphabeticProc(QStack<Value *> &stack, Process *proc, RunWindow *w, VM *vm);
void TypeOfProc(QStack<Value *> &stack, Process *proc, RunWindow *w, VM *vm);
void TypeFromIdProc(QStack<Value *> &stack, Process *proc, RunWindow *w, VM *vm);
void NewMapProc(QStack<Value *> &stack, Process *proc, RunWindow *w, VM *vm);
void HasKeyProc(QStack<Value *> &stack, Process *proc, RunWindow *w, VM *vm);
void KeysOfProc(QStack<Value *> &stack, Process *proc, RunWindow *w, VM *vm);

void RandomProc(QStack<Value *> &stack, Process *proc, RunWindow *, VM *w);
void ToNumProc(QStack<Value *> &stack, Process *proc, RunWindow *, VM *w);
void ToStringProc(QStack<Value *> &stack, Process *proc, RunWindow *, VM *);
void RoundProc(QStack<Value *> &stack, Process *proc, RunWindow *, VM *);
void RemainderProc(QStack<Value *> &stack, Process *proc, RunWindow *, VM *);
void ConcatProc(QStack<Value *> &stack, Process *proc, RunWindow *, VM *);
void StrFirstProc(QStack<Value *> &stack, Process *proc, RunWindow *, VM *);
void StrLastProc(QStack<Value *> &stack, Process *proc, RunWindow *, VM *);
void StrLenProc(QStack<Value *> &stack, Process *proc, RunWindow *, VM *);
void StrMidProc(QStack<Value *> &stack, Process *proc, RunWindow *, VM *);

void StrBeginsWithProc(QStack<Value *> &stack, Process *proc, RunWindow *w, VM *vm);
void StrEndsWithProc(QStack<Value *> &stack, Process *proc, RunWindow *w, VM *vm);
void StrContainsProc(QStack<Value *> &stack, Process *proc, RunWindow *w, VM *vm);

void StrSplitProc(QStack<Value *> &stack, Process *proc, RunWindow *, VM *);
void StrTrimProc(QStack<Value *> &stack, Process *proc, RunWindow *w, VM *vm);
void StrReplaceProc(QStack<Value *> &stack, Process *proc, RunWindow *w, VM *vm);

void SinProc(QStack<Value *> &stack, Process *proc, RunWindow *, VM *);
void CosProc(QStack<Value *> &stack, Process *proc, RunWindow *, VM *);
void TanProc(QStack<Value *> &stack, Process *proc, RunWindow *, VM *);
void ASinProc(QStack<Value *> &stack, Process *proc, RunWindow *, VM *);
void ACosProc(QStack<Value *> &stack, Process *proc, RunWindow *, VM *);
void ATanProc(QStack<Value *> &stack, Process *proc, RunWindow *, VM *);

void SqrtProc(QStack<Value *> &stack, Process *proc, RunWindow *, VM *);
void PowProc(QStack<Value *> &stack, Process *proc, RunWindow *w, VM *vm);
void Log10Proc(QStack<Value *> &stack, Process *proc, RunWindow *, VM *);
void LnProc(QStack<Value *> &stack, Process *proc, RunWindow *, VM *);

void FileWriteProc(QStack<Value *> &stack, Process *proc, RunWindow *, VM *);
void FileWriteUsingWidthProc(QStack<Value *> &stack, Process *proc, RunWindow *, VM *);
void FileWriteLineProc(QStack<Value *> &stack, Process *proc, RunWindow *, VM *);
void FileReadLineProc(QStack<Value *> &stack, Process *proc, RunWindow *, VM *);
void FileEofProc(QStack<Value *> &stack, Process *proc, RunWindow *w, VM *vm);
void FileOpenProc(QStack<Value *> &stack, Process *proc, RunWindow *, VM *);
void FileCreateProc(QStack<Value *> &stack, Process *proc, RunWindow *, VM *);
void FileAppendProc(QStack<Value *> &stack, Process *proc, RunWindow *, VM *);
void FileCloseProc(QStack<Value *> &stack, Process *proc, RunWindow *, VM *);

void EditProc(QStack<Value *> &stack, Process *proc, RunWindow *w, VM *vm);
void GetMainWindowProc(QStack<Value *> &stack, Process *proc, RunWindow *w, VM *vm);
void NewChannelProc(QStack<Value *> &stack, Process *proc, RunWindow *w, VM *vm);

void LoadLibraryProc(QStack<Value *> &stack, Process *proc, RunWindow *w, VM *vm);
void GetProcAddressProc(QStack<Value *> &stack, Process *proc, RunWindow *w, VM *vm);
void AddressOfProc(QStack<Value *> &stack, Process *proc, RunWindow *w, VM *vm);
void InvokeForeignProc(QStack<Value *> &stack, Process *proc, RunWindow *w, VM *vm);

void CurrentParseTreeProc(QStack<Value *> &stack, Process *proc, RunWindow *w, VM *vm);
void MakeParserProc(QStack<Value *> &stack, Process *proc, RunWindow *w, VM *vm);
void PushParserBacktrackPointProc(QStack<Value *> &stack, Process *proc, RunWindow *w, VM *vm);
void IgnoreParserBacktrackPointProc(QStack<Value *> &stack, Process *proc, RunWindow *w, VM *vm);
void ActivationFrameProc(QStack<Value *> &stack, Process *proc, RunWindow *w, VM *vm);

double verifyNumeric(Value *v, RunWindow *w); // TODO: make this a method of RunWindow
int popIntOrCoercable(QStack<Value *> &stack, Process *proc, RunWindow *w, VM *vm);
double popDoubleOrCoercable(QStack<Value *> &stack, Process *proc, RunWindow *w, VM *vm);
int popInt(QStack<Value *> &stack, Process *proc, RunWindow *w, VM *vm);
bool popBool(QStack<Value *> &stack, Process *proc, RunWindow *w, VM *vm);
void *popRaw(QStack<Value *> &stack, Process *proc, RunWindow *w, VM *vm, IClass *type);
QString popString(QStack<Value *> &stack, Process *proc, RunWindow *w, VM *vm);
Value *popValue(QStack<Value *> &stack, Process *proc,  RunWindow *w, VM *vm);
void verifyStackNotEmpty(QStack<Value *> &stack, Process *proc, VM *vm);

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
    void operator()(QStack<Value *> &operandStack, Process *);
};

class WindowProxyMethod : public ExternalMethod
{
    RunWindow *parent;
    VM_PROC proc;
    VM *vm;
public:
    WindowProxyMethod(RunWindow *parent, VM *vm, VM_PROC proc);
    void operator()(QStack<Value *> &operandStack, Process *process);
};

#endif // BUILTINMETHODS_H
