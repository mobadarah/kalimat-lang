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

typedef void (*VM_PROC)(Stack<Value *> &, Process *proc, RunWindow *, VM *);

void PrintProc(Stack<Value *> &, Process *proc, RunWindow *, VM *);
void PushReadChanProc(Stack<Value *> &stack, Process *, RunWindow *w, VM *);
void SetCursorPosProc(Stack<Value *> &, Process *proc, RunWindow *, VM *);
void GetCursorRowProc(Stack<Value *> &, Process *proc, RunWindow *, VM *);
void GetCursorColProc(Stack<Value *> &, Process *proc, RunWindow *, VM *);

void PrintUsingWidthProc(Stack<Value *> &, Process *, RunWindow *, VM *);
void DrawPixelProc(Stack<Value *> &, Process *proc, RunWindow *, VM *);
void DrawLineProc(Stack<Value *> &, Process *proc, RunWindow *, VM *);
void DrawRectProc(Stack<Value *> &, Process *proc, RunWindow *, VM *);
void DrawCircleProc(Stack<Value *> &, Process *proc, RunWindow *, VM *);

void LoadImageProc(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm);
void LoadSpriteProc(Stack<Value *> &, Process *proc, RunWindow *, VM *);
void SpriteFromImageProc(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm);
void DrawImageProc(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm);
void DrawSpriteProc(Stack<Value *> &, Process *proc, RunWindow *, VM *);
void ZoomProc(Stack<Value *> &, Process *proc, RunWindow *, VM *);
void HideSpriteProc(Stack<Value *> &, Process *proc, RunWindow *, VM *);
void ShowSpriteProc(Stack<Value *> &, Process *proc, RunWindow *, VM *);

void GetSpriteLeftProc(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm);
void GetSpriteRightProc(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm);
void GetSpriteTopProc(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm);
void GetSpriteBottomProc(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm);
void GetSpriteWidthProc(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm);
void GetSpriteHeightProc(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm);
void GetSpriteImageProc(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm);
void SetSpriteImageProc(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm);

void WaitProc(Stack<Value *> &, Process *proc, RunWindow *, VM *);
void MouseEventChanProc(Stack<Value *> &stack, Process *, RunWindow *w, VM *);
void MouseDownEventChanProc(Stack<Value *> &stack, Process *, RunWindow *w, VM *);
void MouseUpEventChanProc(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm);
void MouseMoveEventChanProc(Stack<Value *> &stack, Process *, RunWindow *w, VM *);
void KbEventChanProc(Stack<Value *> &stack, Process *, RunWindow *w, VM *);

void ClsProc(Stack<Value *> &stack, Process *proc, RunWindow *, VM *w);
void ClearTextProc(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm);
void SetTextColorProc(Stack<Value *> &stack, Process *proc, RunWindow *, VM *w);
void PointAtProc(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm);
void PointRgbAtProc(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm);
void BuiltInConstantProc(Stack<Value *> &stack, Process *proc, RunWindow *, VM *vm);
void StringIsNumericProc(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm);
void StringIsAlphabeticProc(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm);
void TypeOfProc(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm);
void TypeFromIdProc(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm);
void NewMapProc(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm);
void HasKeyProc(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm);
void KeysOfProc(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm);

void RandomProc(Stack<Value *> &stack, Process *proc, RunWindow *, VM *w);
void ToNumProc(Stack<Value *> &stack, Process *proc, RunWindow *, VM *w);
void ToStringProc(Stack<Value *> &stack, Process *proc, RunWindow *, VM *);
void RoundProc(Stack<Value *> &stack, Process *proc, RunWindow *, VM *);
void RemainderProc(Stack<Value *> &stack, Process *proc, RunWindow *, VM *);
void ConcatProc(Stack<Value *> &stack, Process *proc, RunWindow *, VM *);
void StrFirstProc(Stack<Value *> &stack, Process *proc, RunWindow *, VM *);
void StrLastProc(Stack<Value *> &stack, Process *proc, RunWindow *, VM *);
void StrLenProc(Stack<Value *> &stack, Process *proc, RunWindow *, VM *);
void StrMidProc(Stack<Value *> &stack, Process *proc, RunWindow *, VM *);

void StrBeginsWithProc(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm);
void StrEndsWithProc(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm);
void StrContainsProc(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm);

void StrSplitProc(Stack<Value *> &stack, Process *proc, RunWindow *, VM *);
void StrTrimProc(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm);
void StrReplaceProc(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm);

void SinProc(Stack<Value *> &stack, Process *proc, RunWindow *, VM *);
void CosProc(Stack<Value *> &stack, Process *proc, RunWindow *, VM *);
void TanProc(Stack<Value *> &stack, Process *proc, RunWindow *, VM *);
void ASinProc(Stack<Value *> &stack, Process *proc, RunWindow *, VM *);
void ACosProc(Stack<Value *> &stack, Process *proc, RunWindow *, VM *);
void ATanProc(Stack<Value *> &stack, Process *proc, RunWindow *, VM *);

void SqrtProc(Stack<Value *> &stack, Process *proc, RunWindow *, VM *);
void PowProc(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm);
void Log10Proc(Stack<Value *> &stack, Process *proc, RunWindow *, VM *);
void LnProc(Stack<Value *> &stack, Process *proc, RunWindow *, VM *);

void FileWriteProc(Stack<Value *> &stack, Process *proc, RunWindow *, VM *);
void FileWriteUsingWidthProc(Stack<Value *> &stack, Process *proc, RunWindow *, VM *);
void FileWriteLineProc(Stack<Value *> &stack, Process *proc, RunWindow *, VM *);
void FileReadLineProc(Stack<Value *> &stack, Process *proc, RunWindow *, VM *);
void FileEofProc(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm);
void FileOpenProc(Stack<Value *> &stack, Process *proc, RunWindow *, VM *);
void FileCreateProc(Stack<Value *> &stack, Process *proc, RunWindow *, VM *);
void FileAppendProc(Stack<Value *> &stack, Process *proc, RunWindow *, VM *);
void FileCloseProc(Stack<Value *> &stack, Process *proc, RunWindow *, VM *);

void EditProc(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm);
void GetMainWindowProc(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm);
void NewChannelProc(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm);

void LoadLibraryProc(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm);
void GetProcAddressProc(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm);
void AddressOfProc(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm);
void InvokeForeignProc(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm);
void TestMakeCArrayProc(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm);

void CurrentParseTreeProc(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm);
void MakeParserProc(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm);
void PushParserBacktrackPointProc(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm);
void IgnoreParserBacktrackPointProc(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm);
void ActivationFrameProc(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm);

void MigrateToGuiThreadProc(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm);
void MigrateBackFromGuiThreadProc(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm);

void ImageRotatedProc(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm);
void ImageScaledProc(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm);
void ImageDrawLineProc(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm);
void ImageFlippedProc(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm);
void ImageCopiedProc(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm);
void ImageDrawTextProc(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm);
void ImageSetPixelColorProc(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm);
void ImagePixelColorProc(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm);
void ImageWidthProc(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm);
void ImageHeightProc(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm);

void ForeignWindowMaximizeProc(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm);
void ForeignWindowMoveToProc(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm);
void ForeignWindowAddProc(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm);
void ForeignWindowSetSizeProc(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm);
void ForeignWindowSetTitleProc(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm);

void ControlSetTextProc(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm);
void ControlSetSizeProc(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm);
void ControlSetLocationProc(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm);
void ControlTextProc(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm);

void ButtonSetTextProc(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm);
void ButtonTextProc(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm);

void TextboxSetTextProc(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm);
void TextboxTextProc(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm);
void TextboxAppendTextProc(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm);

void LineEditSetTextProc(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm);
void LineEditTextProc(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm);
void LineEditAppendTextProc(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm);

void ListboxAddProc(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm);
void ListboxInsertItemProc(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm);
void ListboxGetItemProc(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm);

void ComboBoxSetTextProc(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm);
void ComboBoxTextProc(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm);
void ComboBoxAddProc(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm);
void ComboBoxInsertItemProc(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm);
void ComboBoxGetItemProc(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm);
void ComboBoxSetEditableProc(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm);

void LabelSetTextProc(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm);
void LabelTextProc(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm);

void CheckboxSetTextProc(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm);
void CheckboxTextProc(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm);
void CheckboxSetValueProc(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm);
void CheckboxValueProc(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm);

void RadioButtonSetTextProc(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm);
void RadioButtonTextProc(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm);
void RadioButtonSetValueProc(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm);
void RadioButtonValueProc(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm);

void ButtonGroupAddProc(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm);
void ButtonGroupGetButtonProc(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm);

double verifyNumeric(Value *v, RunWindow *w); // TODO: make this a method of RunWindow
int popIntOrCoercable(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm);
double popDoubleOrCoercable(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm);
int popInt(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm);
bool popBool(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm);
void *popRaw(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm, IClass *type);
QString popString(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm);
Value *popValue(Stack<Value *> &stack, Process *proc,  RunWindow *w, VM *vm);
void verifyStackNotEmpty(Stack<Value *> &stack, Process *proc, VM *vm);

template<class T> T popGuiReceiver(Stack<Value *> &stack, IClass *&type, Process *proc, RunWindow *w, VM *vm)
{
    Value *rv = popValue(stack, proc, w, vm);
    IObject *receiver = rv->unboxObj();
    T handle = reinterpret_cast<T>
            (receiver->getSlotValue("handle")->unboxRaw());
    type = rv->type;
    return handle;
}

Value *ConvertStringToNumber(QString str, VM *vm);

class WindowReadMethod : public ExternalMethod
{
    RunWindow *parent;
    //QStack<Value *> *operandStack;
    VM *vm;
public:
    int readNum;
public:
    WindowReadMethod(RunWindow *parent, VM *vm);
    void operator()(Stack<Value *> &operandStack, Process *);
};

class WindowProxyMethod : public ExternalMethod
{
    RunWindow *parent;
    VM_PROC proc;
    VM *vm;
public:
    WindowProxyMethod(RunWindow *parent, VM *vm, VM_PROC proc, bool mustRunInGui=true);
    void operator()(Stack<Value *> &operandStack, Process *process);
};

#endif // BUILTINMETHODS_H
