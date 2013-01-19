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

#ifndef WINDOWPROXYMETHOD_H
#include "windowproxymethod.h"
#endif

Sprite *GetSpriteFromValue(Value * v);
Value *MakeSpriteValue(Sprite *sprite, Allocator *alloc);

void PrintProc(VOperandStack &, Process *proc, RunWindow *, VM *);
void PushReadChanProc(VOperandStack &stack, Process *, RunWindow *w, VM *);
void SetCursorPosProc(VOperandStack &, Process *proc, RunWindow *, VM *);
void GetCursorRowProc(VOperandStack &, Process *proc, RunWindow *, VM *);
void GetCursorColProc(VOperandStack &, Process *proc, RunWindow *, VM *);

void PrintUsingWidthProc(VOperandStack &, Process *, RunWindow *, VM *);
void DrawPixelProc(VOperandStack &, Process *proc, RunWindow *, VM *);
void DrawLineProc(VOperandStack &, Process *proc, RunWindow *, VM *);
void DrawRectProc(VOperandStack &, Process *proc, RunWindow *, VM *);
void DrawCircleProc(VOperandStack &, Process *proc, RunWindow *, VM *);

void LoadImageProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm);
void LoadSpriteProc(VOperandStack &, Process *proc, RunWindow *, VM *);
void SpriteFromImageProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm);
void DrawImageProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm);
void DrawSpriteProc(VOperandStack &, Process *proc, RunWindow *, VM *);
void ZoomProc(VOperandStack &, Process *proc, RunWindow *, VM *);
void HideSpriteProc(VOperandStack &, Process *proc, RunWindow *, VM *);
void ShowSpriteProc(VOperandStack &, Process *proc, RunWindow *, VM *);

void GetSpriteLeftProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm);
void GetSpriteRightProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm);
void GetSpriteTopProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm);
void GetSpriteBottomProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm);
void GetSpriteWidthProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm);
void GetSpriteHeightProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm);
void GetSpriteImageProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm);
void SetSpriteImageProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm);

void WaitProc(VOperandStack &, Process *proc, RunWindow *, VM *);
void MouseEventChanProc(VOperandStack &stack, Process *, RunWindow *w, VM *);
void MouseDownEventChanProc(VOperandStack &stack, Process *, RunWindow *w, VM *);
void MouseUpEventChanProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm);
void MouseMoveEventChanProc(VOperandStack &stack, Process *, RunWindow *w, VM *);
void KbEventChanProc(VOperandStack &stack, Process *, RunWindow *w, VM *);

void ClsProc(VOperandStack &stack, Process *proc, RunWindow *, VM *w);
void ClearTextProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm);
void SetTextColorProc(VOperandStack &stack, Process *proc, RunWindow *, VM *w);
void PointAtProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm);
void PointRgbAtProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm);
void BuiltInConstantProc(VOperandStack &stack, Process *proc, RunWindow *, VM *vm);
void StringIsNumericProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm);
void StringIsAlphabeticProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm);
void TypeOfProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm);
void TypeFromIdProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm);
void NewMapProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm);
void HasKeyProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm);
void KeysOfProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm);

void RandomProc(VOperandStack &stack, Process *proc, RunWindow *, VM *w);
void ToNumProc(VOperandStack &stack, Process *proc, RunWindow *, VM *w);
void ToStringProc(VOperandStack &stack, Process *proc, RunWindow *, VM *);
void RoundProc(VOperandStack &stack, Process *proc, RunWindow *, VM *);
void RemainderProc(VOperandStack &stack, Process *proc, RunWindow *, VM *);
void ConcatProc(VOperandStack &stack, Process *proc, RunWindow *, VM *);
void StrFirstProc(VOperandStack &stack, Process *proc, RunWindow *, VM *);
void StrLastProc(VOperandStack &stack, Process *proc, RunWindow *, VM *);
void StrLenProc(VOperandStack &stack, Process *proc, RunWindow *, VM *);
void StrMidProc(VOperandStack &stack, Process *proc, RunWindow *, VM *);

void StrBeginsWithProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm);
void StrEndsWithProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm);
void StrContainsProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm);

void StrSplitProc(VOperandStack &stack, Process *proc, RunWindow *, VM *);
void StrTrimProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm);
void StrReplaceProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm);

void SinProc(VOperandStack &stack, Process *proc, RunWindow *, VM *);
void CosProc(VOperandStack &stack, Process *proc, RunWindow *, VM *);
void TanProc(VOperandStack &stack, Process *proc, RunWindow *, VM *);
void ASinProc(VOperandStack &stack, Process *proc, RunWindow *, VM *);
void ACosProc(VOperandStack &stack, Process *proc, RunWindow *, VM *);
void ATanProc(VOperandStack &stack, Process *proc, RunWindow *, VM *);

void SqrtProc(VOperandStack &stack, Process *proc, RunWindow *, VM *);
void PowProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm);
void Log10Proc(VOperandStack &stack, Process *proc, RunWindow *, VM *);
void LnProc(VOperandStack &stack, Process *proc, RunWindow *, VM *);

void FileWriteProc(VOperandStack &stack, Process *proc, RunWindow *, VM *);
void FileWriteUsingWidthProc(VOperandStack &stack, Process *proc, RunWindow *, VM *);
void FileWriteLineProc(VOperandStack &stack, Process *proc, RunWindow *, VM *);
void FileReadLineProc(VOperandStack &stack, Process *proc, RunWindow *, VM *);
void FileEofProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm);
void FileOpenProc(VOperandStack &stack, Process *proc, RunWindow *, VM *);
void FileCreateProc(VOperandStack &stack, Process *proc, RunWindow *, VM *);
void FileAppendProc(VOperandStack &stack, Process *proc, RunWindow *, VM *);
void FileCloseProc(VOperandStack &stack, Process *proc, RunWindow *, VM *);

void EditProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm);
void GetMainWindowProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm);
void NewChannelProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm);

void LoadLibraryProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm);
void GetProcAddressProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm);
void AddressOfProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm);
void InvokeForeignProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm);
void TestMakeCArrayProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm);

void CurrentParseTreeProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm);
void MakeParserProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm);
void PushParserBacktrackPointProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm);
void IgnoreParserBacktrackPointProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm);
void ActivationFrameProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm);

void MigrateToGuiThreadProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm);
void MigrateBackFromGuiThreadProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm);

void ImageRotatedProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm);
void ImageScaledProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm);
void ImageDrawLineProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm);
void ImageFlippedProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm);
void ImageCopiedProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm);
void ImageDrawTextProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm);
void ImageSetPixelColorProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm);
void ImagePixelColorProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm);
void ImageWidthProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm);
void ImageHeightProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm);

void ForeignWindowMaximizeProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm);
void ForeignWindowMoveToProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm);
void ForeignWindowAddProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm);
void ForeignWindowSetSizeProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm);
void ForeignWindowSetTitleProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm);
void ForeignWindowSetupProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm);

void ControlSetTextProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm);
void ControlSetSizeProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm);
void ControlSetLocationProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm);
void ControlTextProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm);
void ControlShowProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm);
void ControlHideProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm);
void ControlSetVisibleProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm);
void ControlCloseProc(VOperandStack &stack, Process *proc, RunWindow *rw, VM *vm);

void ButtonSetTextProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm);
void ButtonTextProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm);

void TextboxSetTextProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm);
void TextboxTextProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm);
void TextboxAppendTextProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm);

void LineEditSetTextProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm);
void LineEditTextProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm);
void LineEditAppendTextProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm);

void ListboxAddProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm);
void ListboxInsertItemProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm);
void ListboxGetItemProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm);

void ComboBoxSetTextProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm);
void ComboBoxTextProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm);
void ComboBoxAddProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm);
void ComboBoxInsertItemProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm);
void ComboBoxGetItemProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm);
void ComboBoxSetEditableProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm);

void LabelSetTextProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm);
void LabelTextProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm);

void CheckboxSetTextProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm);
void CheckboxTextProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm);
void CheckboxSetValueProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm);
void CheckboxValueProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm);

void RadioButtonSetTextProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm);
void RadioButtonTextProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm);
void RadioButtonSetValueProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm);
void RadioButtonValueProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm);

void ButtonGroupAddProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm);
void ButtonGroupGetButtonProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm);

void ClassNewObjectProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm);

double verifyNumeric(Value *v, RunWindow *w); // TODO: make this a method of RunWindow
int popIntOrCoercable(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm);
double popDoubleOrCoercable(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm);
int popInt(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm);
bool popBool(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm);
void *popRaw(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm, IClass *type);
QString popString(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm);
Value *popValue(VOperandStack &stack, Process *proc,  RunWindow *w, VM *vm);
void verifyStackNotEmpty(VOperandStack &stack, Process *proc, VM *vm);

template<class T> T popGuiReceiver(VOperandStack &stack, IClass *&type, Process *proc, RunWindow *w, VM *vm)
{
    Value *rv = popValue(stack, proc, w, vm);
    IObject *receiver = unboxObj(rv);
    T handle = reinterpret_cast<T>
            (unboxRaw(receiver->getSlotValue("handle")));
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
    void operator()(VOperandStack &operandStack, Process *);
};

#endif // BUILTINMETHODS_H
