/**************************************************************************
**   The Kalimat programming language
**   Copyright 2010 Mohamed Samy Ali - samy2004@gmail.com
**   This project is released under the Apache License version 2.0
**   as described in the included license.txt file
**************************************************************************/


#include "runwindow.h"
#include "builtinmethods.h"
#include "guieditwidgethandler.h"
#include "parserengine.h"
#include "../runtime_identifiers.h"

#include "guicontrols.h" // for ButtonGroupForeignClass

#include <QApplication>
#include <QDialog>
#include <QPushButton>
#include <QLineEdit>
#include <QCheckBox>
#include <QLabel>
#include <QFrame>
#include <QScrollArea>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QListWidget>
#include <QComboBox>
#include <QRadioButton>
#include <QButtonGroup>
#include <QString>
#include <QMap>
#include <QPainter>
#include <QFile>
#include <QTextStream>

#include <QLibrary>
#include <math.h>
#include <algorithm>
#include <QPushButton>
#include <QVariant>
#include <QRgb>
#include <QtDebug>
//#include <QtConcurrentRun>
#include <QMessageBox>
#include <iostream>
#include "../smallvm/vm_ffi.h"
using namespace std;

#define _ws(s) QString::fromStdWString(s)

void PrintProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *)
{
    if(stack.empty())
        proc->signal(InternalError1, "Empty operand stack when reading value to in 'print'");
    Value *v = stack.pop();
    QString str = v->toString();
    w->textLayer.print(str);
    w->redrawWindow();
}

void PushReadChanProc(VOperandStack &stack, Process *, RunWindow *w, VM *)
{
    stack.push(w->readChannel);
}

void MouseEventChanProc(VOperandStack &stack, Process *, RunWindow *w, VM *vm)
{
    stack.push(w->realmouseEventChannel);
}

void MouseDownEventChanProc(VOperandStack &stack, Process *, RunWindow *w, VM *vm)
{
    stack.push(w->realmouseDownEventChannel);
}

void MouseUpEventChanProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm)
{
    stack.push(w->realmouseUpEventChannel);
}

void MouseMoveEventChanProc(VOperandStack &stack, Process *, RunWindow *w, VM *)
{
    stack.push(w->realmouseMoveEventChannel);
}

void KbEventChanProc(VOperandStack &stack, Process *, RunWindow *w, VM *)
{
    stack.push(w->realkbEventChannel);
}

void EnableMouseEventChanProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm)
{
    bool enable = popBool(stack, proc, w, vm);
    if(enable)
    {
        w->mouseEventChannel = w->realmouseEventChannel;
    }
    else
    {
        w->mouseEventChannel = NULL;
    }
}

void EnableMouseDownEventChanProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm)
{
    bool enable = popBool(stack, proc, w, vm);
    if(enable)
    {
        w->mouseDownEventChannel = w->realmouseDownEventChannel;
    }
    else
    {
        w->mouseDownEventChannel = NULL;
    }
}

void EnableMouseUpEventChanProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm)
{
    bool enable = popBool(stack, proc, w, vm);
    if(enable)
    {
        w->mouseUpEventChannel = w->realmouseUpEventChannel;
    }
    else
    {
        w->mouseUpEventChannel = NULL;
    }
}

void EnableMouseMoveEventChanProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm)
{
    bool enable = popBool(stack, proc, w, vm);
    if(enable)
    {
        w->mouseMoveEventChannel = w->realmouseMoveEventChannel;
    }
    else
    {
        w->mouseMoveEventChannel = NULL;
    }
}

void EnableKbEventChanProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm)
{
    bool enable = popBool(stack, proc, w, vm);
    if(enable)
    {
        w->kbEventChannel = w->realkbEventChannel;
    }
    else
    {
        w->kbEventChannel = NULL;
    }
}

WindowReadMethod::WindowReadMethod(RunWindow *parent, VM *vm)
{
    this->parent = parent;
    this->vm = vm;
    this->readNum = false;
}

void WindowReadMethod::operator ()(VOperandStack &operandStack, Process *proc)
{
    readNum = popInt(operandStack, proc, parent, vm);
    parent->beginInput();
    parent->update(); // We must do this, because normal updating is done
                      // by calling redrawWindow() in the instruction loop, and
                      // here we suspend the instruction loop...
}

void SetCursorPosProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm)
{
    int line = popInt(stack, proc, w, vm);
    int col = popInt(stack, proc, w, vm);
    bool result = w->textLayer.setCursorPos(line, col);
    proc->assert(result, ArgumentError, VM::argumentErrors[ArgErr::InvalidCursorPosition]);
}

void GetCursorRowProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm)
{
    int r = w->textLayer.getCursorRow();
    stack.push(vm->GetAllocator().newInt(r));
}

void GetCursorColProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm)
{
    int c = w->textLayer.getCursorCol();
    stack.push(vm->GetAllocator().newInt(c));
}


void PrintUsingWidthProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *)
{
    Value *v = stack.pop();
    w->typeCheck(proc, stack.top(), BuiltInTypes::IntType);
    int wid = unboxInt(stack.pop());
    QString str = v->toString();
    w->textLayer.print(str, wid);
}

void DrawPixelProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm)
{
    int x = popIntOrCoercable(stack, proc, w, vm);
    int y = popIntOrCoercable(stack, proc, w, vm);
    int color = popInt(stack, proc, w, vm);
    if(color == -1)
        color = 0;
    QColor clr = w->paintSurface->GetColor(color);
    w->paintSurface->TX(x);
    QPainter p(w->paintSurface->GetImageForWriting());
    p.fillRect(x, y, 1, 1, clr);
    w->redrawWindow();
}

void DrawLineProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm)
{
    int x1 = popIntOrCoercable(stack, proc, w, vm);
    int y1 = popIntOrCoercable(stack, proc, w, vm);
    int x2 = popIntOrCoercable(stack, proc, w, vm);
    int y2 = popIntOrCoercable(stack, proc, w, vm);

    w->paintSurface->TX(x1);
    w->paintSurface->TX(x2);
    int color = popInt(stack, proc, w, vm);
    if(color ==-1)
        color = 0;

    QColor clr = w->paintSurface->GetColor(color);
    QPainter p(w->paintSurface->GetImageForWriting());

    QColor oldcolor = p.pen().color();
    QPen pen = p.pen();
    pen.setColor(clr);
    p.setPen(pen);
    p.drawLine(x1, y1, x2, y2);
    pen.setColor(oldcolor);
    p.setPen(pen);

    w->redrawWindow();
}

void DrawRectProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm)
{
    int x1 = popIntOrCoercable(stack, proc, w, vm);
    int y1 = popIntOrCoercable(stack, proc, w, vm);
    int x2 = popIntOrCoercable(stack, proc, w, vm);
    int y2 = popIntOrCoercable(stack, proc, w, vm);

    w->paintSurface->TX(x1);
    w->paintSurface->TX(x2);
    int color = popInt(stack, proc, w, vm);
    bool filled = popBool(stack, proc, w, vm);

    if(color ==-1)
        color = 0;

    QPainter p(w->paintSurface->GetImageForWriting());

    int top = min(y1, y2);
    int left = min(x1, x2);
    int wid = abs(x2-x1);

    int hei = abs(y2-y1);
    QColor clr = w->paintSurface->GetColor(color);
    QColor oldcolor = p.pen().color();
    QPen pen = p.pen();

    pen.setColor(clr);
    p.setPen(pen);

    if(filled)
    {
        QBrush oldBrush = p.brush();
        p.setBrush(QBrush(clr,Qt::SolidPattern));
        p.drawRect(left, top, wid, hei);
        p.setBrush(oldBrush);
    }
    else
    {
        p.setBrush(Qt::NoBrush);
        p.drawRect(left, top, wid, hei);
    }

    pen.setColor(oldcolor);
    p.setPen(pen);

    w->redrawWindow();
}

void DrawCircleProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm)
{
    int cx = popIntOrCoercable(stack, proc, w, vm);
    int cy = popIntOrCoercable(stack, proc, w, vm);

    int radius = popIntOrCoercable(stack, proc, w, vm);

    int color = popInt(stack, proc, w, vm);

    bool filled = popBool(stack, proc, w, vm);
    w->paintSurface->TX(cx);
    QPainter p(w->paintSurface->GetImageForWriting());
    if(color ==-1)
        color = 0;
    QColor clr = w->paintSurface->GetColor(color);
    QColor oldcolor = p.pen().color();
    QPen pen = p.pen();


    pen.setColor(clr);
    p.setPen(pen);

    if(filled)
    {
        QBrush oldBrush = p.brush();
        p.setBrush(QBrush(clr,Qt::SolidPattern));
        p.drawEllipse(cx-radius, cy-radius, radius*2, radius*2);
        p.setBrush(oldBrush);
    }
    else
    {
        p.setBrush(Qt::NoBrush);
        p.drawEllipse(cx-radius, cy-radius, radius*2, radius*2);
    }
    pen.setColor(oldcolor);
    p.setPen(pen);

    w->redrawWindow();
}

void RandomProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm)
{
    int max = popIntOrCoercable(stack, proc, w, vm);
    vm->assert(proc, max >0, ArgumentError, VM::argumentErrors[ArgErr::RandTakesPositiveValues]);
    int ret = rand()%max;
    stack.push(vm->GetAllocator().newInt(ret));
}

Value *ConvertStringToNumber(QString str, VM *vm)
{
    bool ok;
    QLocale loc(QLocale::Arabic, QLocale::Egypt);
    int i = loc.toInt(str, &ok,10);
    if(ok)
    {
       return vm->GetAllocator().newInt(i);
    }

    long lng = loc.toLongLong(str, &ok, 10);
    if(ok)
    {
        return vm->GetAllocator().newLong(lng);
    }

    i = str.toInt(&ok, 10);

    if(ok)
    {
        return vm->GetAllocator().newInt(i);
    }
    double d = str.toDouble(&ok);
    if(ok)
    {
        return vm->GetAllocator().newDouble(d);
    }
    return NULL;
}

void ToNumProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm)
{

    QString str = popString(stack, proc, w, vm);
    Value * v = ConvertStringToNumber(str, vm);

    if(v != NULL)
        stack.push(v);
    else
    {
        proc->signal(ArgumentError, VM::argumentErrors.get(ArgErr::CannotConvertStrToInt1, str));
    }
}

void ConcatProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm)
{
    QString str1 = popString(stack, proc, w, vm);
    QString str2 = popString(stack, proc, w, vm);


    QString ret = str1 + str2;
    stack.push(vm->GetAllocator().newString(ret));
}

void StrLenProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm)
{
    QString str = popString(stack, proc, w, vm);

    int ret = str.length();
    stack.push(vm->GetAllocator().newInt(ret));
}

void StrFirstProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm)
{
    QString str = popString(stack, proc, w, vm);
    int n = popInt(stack, proc, w, vm);

    QString ret = str.left(n);
    stack.push(vm->GetAllocator().newString(ret));
}

void StrLastProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm)
{
    QString str = popString(stack, proc, w, vm);
    int n = popInt(stack, proc, w, vm);

    QString ret = str.right(n);
    stack.push(vm->GetAllocator().newString(ret));
}

void StrMidProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm)
{
    QString str = popString(stack, proc, w, vm);


    int i = popInt(stack, proc, w, vm);

    int n = popInt(stack, proc, w, vm);

    // We make indexing one-based instead of QT's zero-based
    // todo: range checking in StrMidProc()
    QString ret = str.mid(i -1 ,n);
    stack.push(vm->GetAllocator().newString(ret));
}

void StrBeginsWithProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm)
{
    QString strMain = popString(stack, proc, w, vm);
    QString strSub = popString(stack, proc, w, vm);

    bool ret = strMain.startsWith(strSub, Qt::CaseSensitive);
    stack.push(vm->GetAllocator().newBool(ret));
}

void StrEndsWithProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm)
{
    QString strMain = popString(stack, proc, w, vm);
    QString strSub = popString(stack, proc, w, vm);

    bool ret = strMain.endsWith(strSub, Qt::CaseSensitive);
    stack.push(vm->GetAllocator().newBool(ret));
}

void StrContainsProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm)
{
    QString strMain = popString(stack, proc, w, vm);
    QString strSub = popString(stack, proc, w, vm);

    bool ret = strMain.contains(strSub, Qt::CaseSensitive);
    stack.push(vm->GetAllocator().newBool(ret));
}

void StrSplitProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm)
{
    QString str = popString(stack, proc, w, vm);
    QString separator = popString(stack, proc, w, vm);

    QStringList result = str.split(separator, QString::KeepEmptyParts);
    Value *ret = vm->GetAllocator().newArray(result.count());
    for(int i=0; i<result.count(); i++)
    {
        unboxArray(ret)->Elements[i] = vm->GetAllocator().newString(result[i]);
    }
    stack.push(ret);
}

void StrTrimProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm)
{
    QString str = popString(stack, proc, w, vm);
    QString str2 = str.trimmed();
    Value *ret = vm->GetAllocator().newString(str2);
    stack.push(ret);
}

void StrReplaceProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm)
{
    QString str = popString(stack, proc, w, vm);
    QString str2 = popString(stack, proc, w, vm);
    QString str3 = popString(stack, proc, w, vm);
    str = str.replace(str2, str3);
    Value *ret = vm->GetAllocator().newString(str);
    stack.push(ret);
}

void ToStringProc(VOperandStack &stack, Process *proc, RunWindow *, VM *vm)
{
    verifyStackNotEmpty(stack, proc, vm);
    Value *v = stack.pop();
    QString ret = v->toString();
    stack.push(vm->GetAllocator().newString(ret));
}

void RoundProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm)
{
    double d = popDoubleOrCoercable(stack, proc, w, vm);
    int i = (int) d;
    stack.push(vm->GetAllocator().newInt(i));

}

void RemainderProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm)
{
    int n1 = popInt(stack, proc, w, vm);
    int n2 = popInt(stack, proc, w, vm);

    if(n2 == 0)
        proc->signal(DivisionByZero);
    int i = n1 % n2;
    stack.push(vm->GetAllocator().newInt(i));
}

int popIntOrCoercable(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm)
{
    if(stack.empty())
    {
        proc->signal(InternalError1, "Empty operand stack when reading value");
    }
    Value *v = stack.pop();
    if(v->type != BuiltInTypes::IntType &&
       v->type != BuiltInTypes::DoubleType &&
       v->type != BuiltInTypes::LongType)
    {
        w->typeError(proc, BuiltInTypes::NumericType, v->type);
    }

    if(v->type == BuiltInTypes::DoubleType)
        v = vm->GetAllocator().newInt((int) unboxDouble(v));
    if(v->type == BuiltInTypes::LongType)
        v = vm->GetAllocator().newInt((int) unboxLong(v));

    return unboxInt(v);
}

double popDoubleOrCoercable(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm)
{
    if(stack.empty())
    {
        proc->signal(InternalError1, "Empty operand stack when reading double or double-coercible value");
    }
    Value *v = stack.pop();
    if(v->type != BuiltInTypes::IntType &&
       v->type != BuiltInTypes::DoubleType &&
       v->type != BuiltInTypes::LongType)
    {
        w->typeError(proc, BuiltInTypes::NumericType, v->type);
    }
    if(v->type == BuiltInTypes::IntType)
        v = vm->GetAllocator().newDouble(unboxInt(v));
    if(v->type == BuiltInTypes::LongType)
        v = vm->GetAllocator().newDouble(unboxLong(v));
    return unboxDouble(v);
}

void verifyStackNotEmpty(VOperandStack &stack, Process *proc, VM *)
{
    if(stack.empty())
    {
        proc->signal(InternalError1, "Empty operand stack");
    }
}

void SinProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm)
{

    double theta = popDoubleOrCoercable(stack, proc, w, vm);

    double result = sin(theta);
    stack.push(vm->GetAllocator().newDouble(result));

}
void CosProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm)
{
    double theta = popDoubleOrCoercable(stack, proc, w, vm);

    double result = cos(theta);
    stack.push(vm->GetAllocator().newDouble(result));

}
void TanProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm)
{
    double theta = popDoubleOrCoercable(stack, proc, w, vm);

    double result = tan(theta);
    stack.push(vm->GetAllocator().newDouble(result));

}
void ASinProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm)
{

    double theta = popDoubleOrCoercable(stack, proc, w, vm);

    double result = asin(theta);
    stack.push(vm->GetAllocator().newDouble(result));

}
void ACosProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm)
{
    double theta = popDoubleOrCoercable(stack, proc, w, vm);

    double result = acos(theta);
    stack.push(vm->GetAllocator().newDouble(result));

}
void ATanProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm)
{
    double theta = popDoubleOrCoercable(stack, proc, w, vm);

    double result = atan(theta);
    stack.push(vm->GetAllocator().newDouble(result));

}
void SqrtProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm)
{
    double theta = popDoubleOrCoercable(stack, proc, w, vm);
    double result = sqrt(theta);
    stack.push(vm->GetAllocator().newDouble(result));

}

void PowProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm)
{
    double base = popDoubleOrCoercable(stack, proc, w, vm);
    double power = popDoubleOrCoercable(stack, proc, w, vm);
    double result = pow(base, power);
    stack.push(vm->GetAllocator().newDouble(result));

}

void Log10Proc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm)
{
    double theta = popDoubleOrCoercable(stack, proc, w, vm);
    double result = log10(theta);
    stack.push(vm->GetAllocator().newDouble(result));

}
void LnProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm)
{
    double theta = popDoubleOrCoercable(stack, proc, w, vm);
    double result = log(theta);
    stack.push(vm->GetAllocator().newDouble(result));

}

void LoadImageProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm)
{
    verifyStackNotEmpty(stack, proc, vm);
    QString fname = popString(stack, proc, w, vm);
    fname = w->ensureCompletePath(proc, fname);

    if(!QFile::exists(fname))
    {
        w->assert(proc, false, ArgumentError, VM::argumentErrors.get(ArgErr::NonExistingImageFile1,fname));
    }
    IClass *imgClass = dynamic_cast<IClass *>(unboxObj(vm->GetType(VMId::get(RId::Image))));
    QImage *img = new QImage(fname);
    IObject *obj = imgClass->newValue(&vm->GetAllocator());
    obj->setSlotValue("handle", vm->GetAllocator().newRaw(img, BuiltInTypes::RawType));
    stack.push(vm->GetAllocator().newObject(obj, imgClass));
}

void LoadSpriteProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm)
{
    verifyStackNotEmpty(stack, proc, vm);
    QString fname = popString(stack, proc, w, vm);
    fname = w->ensureCompletePath(proc, fname);

    if(!QFile::exists(fname))
    {
        w->assert(proc, false, ArgumentError, VM::argumentErrors.get(ArgErr::NonExistingSpriteFile1, fname));
    }
    Sprite *sprite = new Sprite(fname);
    w->spriteLayer.AddSprite(sprite);

    stack.push(MakeSpriteValue(sprite, &vm->GetAllocator()));
}

void SpriteFromImageProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm)
{
    IClass *imgClass = dynamic_cast<IClass *>(unboxObj(vm->GetType(VMId::get(RId::Image))));
    verifyStackNotEmpty(stack, proc, vm);
    w->typeCheck(proc, stack.top(), imgClass);
    IObject *obj = unboxObj(stack.pop());
    QImage *handle = reinterpret_cast<QImage*>
            (unboxRaw((obj->getSlotValue("handle"))));

    Sprite *sprite = new Sprite(QPixmap::fromImage(*handle));
    w->spriteLayer.AddSprite(sprite);

    stack.push(MakeSpriteValue(sprite, &vm->GetAllocator()));
}

Sprite *GetSpriteFromValue(Value * v)
{
    IObject *obj = unboxObj(v);
    Value *rawSpr = obj->getSlotValue("_handle");
    Sprite *spr = (Sprite *) unboxRaw(rawSpr);
    return spr;
}

Value *MakeSpriteValue(Sprite *sprite, Allocator *alloc)
{
    Value *spriteHandle = alloc->newRaw(sprite, BuiltInTypes::RawType);
    alloc->stopGcMonitoring(spriteHandle);

    //todo: we stopGcMonitoring each thing we allocate since
    // possible GC in the next allocation could erase it
    // the return GcMonitoring after each allocation.
    // but the problem is we don't have access to whatever objects
    // allocated internally by SpriteType.newValue (luckily
    // nothing is actually allocated there now), so we can't do this
    // we really need atomic allocation of multiple values

    IObject *spriteObj = BuiltInTypes::SpriteType->newValue(alloc);

    spriteObj->setSlotValue("_handle", spriteHandle);
    Value *spriteVal = alloc->newObject(
                spriteObj,
                BuiltInTypes::SpriteType);
    alloc->makeGcMonitored(spriteHandle);
    sprite->extraValue = spriteVal;
    return spriteVal;
}

void DrawImageProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm)
{
    IClass *imgClass = dynamic_cast<IClass *>(unboxObj(vm->GetType(VMId::get(RId::Image))));

    w->typeCheck(proc, stack.top(), imgClass);
    IObject *obj = unboxObj(stack.pop());
    QImage *handle = reinterpret_cast<QImage*>
            (unboxRaw((obj->getSlotValue("handle"))));

    int x = popIntOrCoercable(stack, proc, w , vm);
    int y = popIntOrCoercable(stack, proc, w , vm);

    w->paintSurface->TX(x, handle->width());
    x-= handle->width();

    QPainter p(w->paintSurface->GetImageForWriting());

    p.drawImage(x, y, *handle);
    w->redrawWindow();

}

void DrawSpriteProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm)
{
    w->typeCheck(proc, stack.top(), BuiltInTypes::SpriteType);
    Value *spriteVal =stack.pop();
    Sprite  *sprite = GetSpriteFromValue(spriteVal);

    int x = popIntOrCoercable(stack, proc, w , vm);
    int y = popIntOrCoercable(stack, proc, w , vm);

    sprite->location = QPoint(x,y);
    sprite->visible = true;

    vm->GetAllocator().stopGcMonitoring(spriteVal);
    w->spriteLayer.showSprite(sprite);

    // If a sprite is visible on the screen, we don't want to GC
    // it, since a collision function that might use it
    // could be called. A sprite can be GC'd only if it is
    // both unreachable and invisible

    w->checkCollision(sprite);
    w->redrawWindow();
}

void ShowSpriteProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm)
{
    w->typeCheck(proc, stack.top(), BuiltInTypes::SpriteType);
    Value *spriteVal = stack.pop();
    Sprite  *sprite = GetSpriteFromValue(spriteVal);

    sprite->visible = true;
    vm->GetAllocator().stopGcMonitoring(spriteVal);
    w->spriteLayer.showSprite(sprite);
    // If a sprite is visible on the screen, we don't want to GC
    // it, since a collision function that might use it
    // could be called. A sprite can be GC'd only if it is
    // both unreachable and invisible

    w->checkCollision(sprite);
    w->redrawWindow();
}

void HideSpriteProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm)
{
    w->typeCheck(proc, stack.top(), BuiltInTypes::SpriteType);
    Value *spriteVal = stack.pop();
    Sprite  *sprite = GetSpriteFromValue(spriteVal);

    sprite->visible = false;
    w->spriteLayer.hideSprite(sprite);
    // See ShowSpriteProc and DrawSpriteProc
    // for why the sprite was not GC monitored when visible
    //vm->GetAllocator().makeGcMonitored(spriteVal);
    w->redrawWindow();
}

void GetSpriteLeftProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm)
{
    w->typeCheck(proc, stack.top(), BuiltInTypes::SpriteType);
    Sprite  *sprite = GetSpriteFromValue(stack.pop());
#ifdef ENGLISH_PL
    int ret = sprite->location.x();
#else
    int ret = sprite->location.x() + sprite->image.width();
#endif
    stack.push(vm->GetAllocator().newInt(ret));
}

void GetSpriteRightProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm)
{
    w->typeCheck(proc, stack.top(), BuiltInTypes::SpriteType);
    Sprite  *sprite = GetSpriteFromValue(stack.pop());
#ifdef ENGLISH_PL
    int ret = sprite->location.x() + sprite->image.width();
#else
    int ret = sprite->location.x();
#endif
    stack.push(vm->GetAllocator().newInt(ret));
}

void GetSpriteTopProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm)
{
    w->typeCheck(proc, stack.top(), BuiltInTypes::SpriteType);
    Sprite  *sprite = GetSpriteFromValue(stack.pop());

    int ret = sprite->boundingRect().top();
    stack.push(vm->GetAllocator().newInt(ret));
}

void GetSpriteBottomProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm)
{
    w->typeCheck(proc, stack.top(), BuiltInTypes::SpriteType);
    Sprite  *sprite = GetSpriteFromValue(stack.pop());

    int ret = sprite->boundingRect().bottom();
    stack.push(vm->GetAllocator().newInt(ret));
}

void GetSpriteWidthProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm)
{
    w->typeCheck(proc, stack.top(), BuiltInTypes::SpriteType);
    Value *spriteVal = stack.pop();
    Sprite  *sprite = GetSpriteFromValue(spriteVal);

    int ret = sprite->boundingRect().width();
    stack.push(vm->GetAllocator().newInt(ret));
}

void GetSpriteHeightProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm)
{
    w->typeCheck(proc, stack.top(), BuiltInTypes::SpriteType);
    Sprite  *sprite = GetSpriteFromValue(stack.pop());

    int ret = sprite->boundingRect().height();
    stack.push(vm->GetAllocator().newInt(ret));
}

void GetSpriteImageProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm)
{
    w->typeCheck(proc, stack.top(), BuiltInTypes::SpriteType);
    Sprite  *sprite = GetSpriteFromValue(stack.pop());

    QString clsName = VMId::get(RId::Image);
    QImage *img = new QImage(sprite->image.toImage());
    IClass *imgClass = dynamic_cast<IClass *>
            (unboxObj(vm->GetType(clsName)));
    IObject *imgObj = imgClass->newValue(&vm->GetAllocator());
    imgObj->setSlotValue("handle", vm->GetAllocator().newRaw(img, BuiltInTypes::RawType));

    stack.push(vm->GetAllocator().newObject(imgObj, imgClass));
}

void SetSpriteImageProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm)
{
    QString clsName = VMId::get(RId::Image);
    IClass *imgClass = dynamic_cast<IClass *>
            (unboxObj(vm->GetType(clsName)));

    w->typeCheck(proc, stack.top(), BuiltInTypes::SpriteType);
    Sprite  *sprite = GetSpriteFromValue(stack.pop());

    w->typeCheck(proc, stack.top(), imgClass);
    IObject *imgObj = unboxObj(stack.pop());
    QImage *img = reinterpret_cast<QImage *>
            (unboxRaw((imgObj->getSlotValue("handle"))));

    sprite->setImage(QPixmap::fromImage(*img));
    w->spriteLayer.changing();
    w->redrawWindow();
}

void WaitProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm)
{
    /*
    int ms = stack.pop()->unboxNumeric();
    // The GC could collect the channel
    // even when the RunWindow is still to send it a message
    // therefore in the wait() builtin
    // we shall store a reference to it
    // in a local variable

    Value *channel = vm->GetAllocator().newChannel(false);
    //w->suspend();
    int cookie = w->startTimer(ms);
    w->setAsleep(cookie, channel, ms);
    stack.push(channel);
    */
    int ms = unboxNumeric(stack.pop());
    proc->owner->makeItWaitTimer(proc, ms);
}

void ZoomProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm)
{
    int x1 = popIntOrCoercable(stack, proc, w , vm);
    int y1 = popIntOrCoercable(stack, proc, w , vm);
    int x2 = popIntOrCoercable(stack, proc, w , vm);
    int y2 = popIntOrCoercable(stack, proc, w , vm);
    //w->TX(x1);
    //w->TX(x2);
    w->paintSurface->zoom(x1, y1, x2, y2);
}

void ClsProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm)
{
    w->cls();
}

void ClearTextProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm)
{
    w->clearAllText();
}

void SetTextColorProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm)
{
    int color = popInt(stack, proc, w, vm);
    w->assert(proc, color>=0 && color <=15, ArgumentError, "Color value must be from 0 to 15");

    //w->paintSurface->setTextColor(w->paintSurface->GetColor(color));
    w->textLayer.setColor(w->paintSurface->GetColor(color));

    w->redrawWindow();
}

void PointAtProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm)
{
    int x = popIntOrCoercable(stack, proc, w, vm);
    int y = popIntOrCoercable(stack, proc, w, vm);
    w->paintSurface->TX(x);
    int color = 15;
    /*
    w->assert(x>=0 && x <w->paintSurface->GetImage()->width(), ArgumentError,
              VM::argumentErrors.get(ArgErr::InvalidArgRange3, VM::argumentErrors.get(ArgErr::X, "0",
              str(w->paintSurface->GetImage()->width())))
              );

    w->assert(y>=0 && y <w->paintSurface->GetImage()->height(), ArgumentError,
              VM::argumentErrors.get(ArgErr::InvalidArgRange3, VM::argumentErrors.get(ArgErr::Y, "0",
                                     str(w->paintSurface->GetImage()->height())))
              );
    //*/
    if(x>=0 && y >=0 &&
            x<w->paintSurface->GetImage()->width() &&
            y < w->paintSurface->GetImage()->height())
    {
        // todo: implement colorConstant that takes
        // QRgb to save time converting QRgb->QColor
        color = w->paintSurface->colorConstant(QColor(w->paintSurface->GetImage()->pixel(x, y)));
    }


    stack.push(vm->GetAllocator().newInt(color));
}

void PointRgbAtProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm)
{
    int x = popIntOrCoercable(stack, proc, w, vm);
    int y = popIntOrCoercable(stack, proc, w, vm);
    w->paintSurface->TX(x);
    QRgb color = qRgb(255,255,255);

    /*
    w->assert(x>=0 && x <w->paintSurface->GetImage()->width(), ArgumentError,
              VM::argumentErrors.get(ArgErr::InvalidArgRange3, VM::argumentErrors.get(ArgErr::X, "0",
              str(w->paintSurface->GetImage()->width())))
              );

    w->assert(y>=0 && y <w->paintSurface->GetImage()->height(), ArgumentError,
              VM::argumentErrors.get(ArgErr::InvalidArgRange3, VM::argumentErrors.get(ArgErr::Y, "0",
              str(w->paintSurface->GetImage()->height())))
              );
    */
    if(x>=0 && y >=0 &&
            x<w->paintSurface->GetImage()->width() &&
            y < w->paintSurface->GetImage()->height())
    {
        color = w->paintSurface->GetImage()->pixel(x, y);
    }


    Allocator &a = vm->GetAllocator();
    Value *arr = a.newArray(3);
    unboxArray(arr)->set(a.newInt(1), a.newInt(qRed(color)));
    unboxArray(arr)->set(a.newInt(2), a.newInt(qGreen(color)));
    unboxArray(arr)->set(a.newInt(3), a.newInt(qBlue(color)));

    stack.push(arr);
}

void BuiltInConstantProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm)
{
    QString constName = popString(stack, proc, w, vm);
    if(constName == VMId::get(RId::NewLine))
    {
        stack.push(vm->GetAllocator().newString("\n"));
        return;
    }
    if(constName ==QString::fromStdWString(L"c_int"))
    {
        stack.push(vm->GetAllocator().newObject(BuiltInTypes::c_int, BuiltInTypes::ClassType, false));
        return;
    }
    if(constName == QString::fromStdWString(L"c_long"))
    {
        stack.push(vm->GetAllocator().newObject(BuiltInTypes::c_long, BuiltInTypes::ClassType, false));
        return;
    }
    if(constName == QString::fromStdWString(L"c_float"))
    {
        stack.push(vm->GetAllocator().newObject(BuiltInTypes::c_float, BuiltInTypes::ClassType, false));
        return;
    }
    if(constName == QString::fromStdWString(L"c_double"))
    {
        stack.push(vm->GetAllocator().newObject(BuiltInTypes::c_double, BuiltInTypes::ClassType, false));
        return;
    }
    if(constName== QString::fromStdWString(L"c_char"))
    {
        stack.push(vm->GetAllocator().newObject(BuiltInTypes::c_char, BuiltInTypes::ClassType, false));
        return;
    }
    if(constName == QString::fromStdWString(L"c_asciiz"))
    {
        stack.push(vm->GetAllocator().newObject(BuiltInTypes::c_asciiz, BuiltInTypes::ClassType, false));
        return;
    }
    if(constName == QString::fromStdWString(L"c_wstr"))
    {
        stack.push(vm->GetAllocator().newObject(BuiltInTypes::c_wstr, BuiltInTypes::ClassType, false));
        return;
    }
    if(constName == QString::fromStdWString(L"c_ptr"))
    {
        stack.push(vm->GetAllocator().newObject(BuiltInTypes::c_ptr, BuiltInTypes::ClassType, false));
        return;
    }

    w->assert(proc, false, ArgumentError, VM::argumentErrors.get(ArgErr::InvalidConstantName1, constName));
}

void StringIsNumericProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm)
{
    QString s = popString(stack, proc, w, vm);
    bool yep = true;
    for(int i=0; i<s.length(); i++)
    {
        QChar c = s[i];
        if(!c.isDigit())
            yep = false;
    }
    stack.push(vm->GetAllocator().newBool(yep));
}

void StringIsAlphabeticProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm)
{
    QString s = popString(stack, proc, w, vm);
    bool yep = true;
    for(int i=0; i<s.length(); i++)
    {
        QChar c = s[i];
        if(!c.isLetter())
            yep = false;
    }
    stack.push(vm->GetAllocator().newBool(yep));
}

void TypeOfProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm)
{
    Value *v = popValue(stack, proc, w, vm);

    // We use a gcMonitor value of false since we don't want the GC
    // to collect class objects
    stack.push(vm->GetAllocator().newObject(v->type, BuiltInTypes::ClassType, false));
}

void TypeFromIdProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm)
{
    QString typeId = popString(stack, proc, w, vm);
    Value *type = vm->GetType(typeId);
    stack.push(type);
}

void AddressOfProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm)
{
    Value *v = popValue(stack, proc, w, vm);
    ffi_type *type;
    IClass *guessedType;
    default_C_Type_Of(v->type, guessedType);

    kalimat_to_ffi_type(guessedType, type, vm);

    if(type == NULL)
    {
        proc->signal(InternalError1, QString("Cannot take address of value of type: '%1'").arg(v->type->toString()));
    }

    void *ptr = malloc(type->size);
    kalimat_to_ffi_value(v->type, v, type, ptr, proc, vm);
    //todo:
    stack.push(vm->GetAllocator().newRaw(ptr, new PointerClass(v->type)));
}

void NewMapProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm)
{
    stack.push(vm->GetAllocator().newMap());
}

void HasKeyProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm)
{
    w->typeCheck(proc, stack.top(), BuiltInTypes::MapType);
    Value *v = popValue(stack, proc, w, vm);

    Value *key = popValue(stack, proc, w, vm);

    VMap *m = unboxMap(v);
    VMError err;
    if(!m->keyCheck(key, err))
        throw err;
    bool result = (m->get(key) != NULL);
    stack.push(vm->GetAllocator().newBool(result));
}

void KeysOfProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm)
{
    w->typeCheck(proc, stack.top(), BuiltInTypes::MapType);
    Value *v = popValue(stack, proc, w, vm);

    VMap *m = unboxMap(v);
    const int keyCount = m->Elements.keys().count();
    Value *k = vm->GetAllocator().newArray(keyCount);
    for(int i=0; i<keyCount; i++)
        unboxArray(k)->Elements[i] = m->Elements.keys()[i].v;
    stack.push(k);
}

void MapKeyProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm)
{
    w->typeCheck(proc, stack.top(), BuiltInTypes::MapType);
    Value *v = popValue(stack, proc, w, vm);
    VMap *m = unboxMap(v);

    int keyIndex = popInt(stack, proc, w, vm);
    const int keyCount = m->Elements.keys().count();
    if(keyIndex < 1 || keyIndex > keyCount)
        throw VMError(SubscriptOutOfRange2, proc,
                      proc->owner, proc->currentFrame()).arg(str(keyIndex)).arg(str(keyCount));

    Value *ret = m->Elements.keys()[keyIndex-1].v;
    stack.push(ret);
}

struct FileBlob
{
    QFile *file;
    QTextStream *stream;
    ~FileBlob() { file->close(); delete file; delete stream;}
};

// TODO: use the helpers popXXX(...) functions instead of manually calling
// typecheck() and pop() in all external methods.
FileBlob *popFileBlob(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm)
{
    verifyStackNotEmpty(stack, proc, vm);
    w->typeCheck(proc, stack.top(), BuiltInTypes::FileType);
    IObject *ob = unboxObj(stack.pop());
    Value *rawFile = ob->getSlotValue("file_handle");
    w->typeCheck(proc, rawFile, BuiltInTypes::RawFileType);
    void *fileObj = unboxRaw(rawFile);
    FileBlob *f = (FileBlob *) fileObj;
    return f;
}

Value *popValue(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm)
{
    verifyStackNotEmpty(stack, proc, vm);
    Value *v = stack.pop();
    return v;
}

Value *newGuiObject(void *ptr, IClass *type, VM *vm)
{
    IObject *obj = type->newValue(&vm->GetAllocator());
    obj->setSlotValue("handle", vm->GetAllocator().newRaw(ptr, BuiltInTypes::RawType));
    return vm->GetAllocator().newObject(obj, type);
}

QString popString(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm)
{
    verifyStackNotEmpty(stack, proc, vm);
    w->typeCheck(proc, stack.top(), BuiltInTypes::StringType);
    QString s = unboxStr(stack.pop());
    return s;
}

int popInt(VOperandStack &stack, Process *proc,  RunWindow *w, VM *vm)
{
    verifyStackNotEmpty(stack, proc, vm);
    w->typeCheck(proc, stack.top(), BuiltInTypes::IntType);
    int i = unboxInt(stack.pop());
    return i;
}

void *popRaw(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm, IClass *type)
{
    verifyStackNotEmpty(stack, proc, vm);
    w->typeCheck(proc, stack.top(), type);
    void *ret = unboxRaw(stack.pop());
    return ret;
}

Channel *popChannel(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm)
{
    verifyStackNotEmpty(stack, proc, vm);
    w->typeCheck(proc, stack.top(), BuiltInTypes::ChannelType);
    Channel *ret = unboxChan(stack.pop());
    return ret;
}

bool popBool(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm)
{
    verifyStackNotEmpty(stack, proc, vm);
    w->typeCheck(proc, stack.top(), BuiltInTypes::BoolType);
    bool b = unboxBool(stack.pop());
    return b;
}

VArray *popArray(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm)
{
    verifyStackNotEmpty(stack, proc, vm);
    w->typeCheck(proc, stack.top(), BuiltInTypes::ArrayType);
    VArray *arr = unboxArray(stack.pop());
    return arr;
}

void DoFileWrite(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm, bool newLine)
{
    FileBlob *f = popFileBlob(stack, proc, w, vm);
    QString s = popValue(stack, proc, w, vm)->toString();

    if(f->file == NULL)
        w->assert(proc, false, ArgumentError, VM::argumentErrors[ArgErr::CannotWriteToClosedFile]);
    if(newLine)
        *(f->stream) << s << endl;
    else
        *(f->stream) << s;
}

void FileWriteProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm)
{
    DoFileWrite(stack, proc, w, vm, false);
}

void FileWriteUsingWidthProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm)
{
    FileBlob *f = popFileBlob(stack, proc, w, vm);
    if(f->file == NULL)
        w->assert(proc, false, ArgumentError,  VM::argumentErrors[ArgErr::CannotWriteToClosedFile]);
    QString s = popValue(stack, proc, w, vm)->toString();
    int width = popInt(stack, proc, w, vm);

    QString s2 = w->textLayer.formatStringUsingWidth(s, width);
    *(f->stream) << s2;
}

void FileWriteLineProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm)
{
    DoFileWrite(stack, proc, w, vm, true);
}

void FileReadLineProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm)
{

    FileBlob *f = popFileBlob(stack, proc, w, vm);
    if(f->file == NULL)
        w->assert(proc, false, ArgumentError,  VM::argumentErrors[ArgErr::CannotReadFromClosedFile]);
    QString s = f->stream->readLine();
    Value *v = vm->GetAllocator().newString(s);
    stack.push(v);
}

void FileEofProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm)
{
    FileBlob *f = popFileBlob(stack, proc, w, vm);
    if(f->file == NULL)
        w->assert(proc, false, ArgumentError, VM::argumentErrors[ArgErr::CannotReadFromClosedFile]);
    bool ret = f->stream->atEnd();
    Value *v = vm->GetAllocator().newBool(ret);
    stack.push(v);
}

void FileOpenProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm)
{
    QString fname = popString(stack, proc, w, vm);
    fname = w->ensureCompletePath(proc, fname);
    w->assert(proc, QFile::exists(fname), ArgumentError, VM::argumentErrors.get(ArgErr::TryingToOpenMissingFile1, fname));
    QFile *f = new QFile(fname);
    bool ret = f->open(QIODevice::ReadOnly | QIODevice::Text);
    w->assert(proc, ret, RuntimeError, VM::argumentErrors.get(ArgErr::FailedToOpenFile1,fname));
    QTextStream *stream = new QTextStream(f);
    FileBlob *blob = new FileBlob();
    blob->file = f;
    blob->stream = stream;
    Value *v = vm->GetAllocator().newObject(BuiltInTypes::FileType);
    unboxObj(v)->setSlotValue("file_handle", vm->GetAllocator().newRaw(blob, BuiltInTypes::RawFileType));
    stack.push(v);
}

void FileCreateProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm)
{
    QString fname = popString(stack, proc, w, vm);
    fname = w->ensureCompletePath(proc, fname);
    QFile *f = new QFile(fname);
    bool ret = f->open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate);
    w->assert(proc, ret, RuntimeError, VM::argumentErrors.get(ArgErr::FailedToOpenFile1,fname));
    QTextStream *stream = new QTextStream(f);
    FileBlob *blob = new FileBlob();
    blob->file = f;
    blob->stream = stream;
    Value *v = vm->GetAllocator().newObject(BuiltInTypes::FileType);
    unboxObj(v)->setSlotValue("file_handle", vm->GetAllocator().newRaw(blob, BuiltInTypes::RawFileType));
    stack.push(v);
}

void FileAppendProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm)
{
    QString fname = popString(stack, proc, w, vm);
    fname = w->ensureCompletePath(proc, fname);
    QFile *f = new QFile(fname);
    bool ret = f->open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append);
    w->assert(proc, ret, ArgumentError, VM::argumentErrors.get(ArgErr::FailedToOpenFile1,fname));
    Value *v = vm->GetAllocator().newObject(BuiltInTypes::FileType);
    QTextStream *stream = new QTextStream(f);
    FileBlob *blob = new FileBlob();
    blob->file = f;
    blob->stream = stream;
    unboxObj(v)->setSlotValue("file_handle", vm->GetAllocator().newRaw(blob, BuiltInTypes::RawFileType));
    stack.push(v);
}

void FileCloseProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm)
{
    // TODO: use popFileblob
    w->typeCheck(proc, stack.top(), BuiltInTypes::FileType);
    IObject *ob = unboxObj(stack.pop());
    Value *rawFile = ob->getSlotValue("file_handle");
    w->typeCheck(proc, rawFile, BuiltInTypes::RawFileType);
    void *fileObj = unboxRaw(rawFile);
    FileBlob *f = (FileBlob *) fileObj;
    f->file->close();
    // TODO: memory leak if we comment the following line
    // but a segfault if we delete 'f' and the the kalimat code
    // tries to do some operation on the file :(
    delete f->file;
    delete f->stream;
    f->file = NULL;
}

Value *editAndReturn(Value *v, RunWindow *w, VM *vm);
void EditProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm)
{
    Value *v = stack.pop();
    w->assert(proc, v->isObject(), ArgumentError, VM::argumentErrors.get(ArgErr::SentValueHasToBeAnObject1, v->toString()));
    v = editAndReturn(v, w, vm);
    stack.push(v);
}

void GetMainWindowProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm)
{
    stack.push(vm->GetAllocator().newQObject(w));
}

void NewChannelProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm)
{
    stack.push(vm->GetAllocator().newChannel());
}

void LoadLibraryProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm)
{
    QString libName = popString(stack, proc, w, vm);
    // todo: will this leak?
    QLibrary *lib = new QLibrary(libName);
    if(!lib->load())
        proc->signal(InternalError1, QString("Failed to load library '%1'").arg(libName));

    Value *ret = vm->GetAllocator().newRaw(lib, BuiltInTypes::ExternalLibrary);
    stack.push(ret);
}

void GetProcAddressProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm)
{
    void *libRaw = popRaw(stack, proc, w, vm, BuiltInTypes::ExternalLibrary);
    QString funcName = popString(stack, proc, w, vm);
    // todo: invalid casts here will crash the VM
    QLibrary *lib = (QLibrary *) libRaw;
    // todo: all those conversion might be slow
    void * func = lib->resolve(funcName.toStdString().c_str());
    if(func == NULL)
    {
        proc->signal(InternalError1, QString("Cannot find function called '%1' in external library %2")
                   .arg(funcName).arg(lib->fileName()));
    }
    Value *ret = vm->GetAllocator().newRaw(func, BuiltInTypes::ExternalMethodType);
    stack.push(ret);
}

void TestMakeCArrayProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm)
{
    int *a = new int[5];
    for(int i=0; i<5; i++)
        a[i] = 10 - i;
    stack.push(vm->GetAllocator().newRaw(a, BuiltInTypes::c_ptr));
}

void InvokeForeignProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm)
{
    void *funcPtr = popRaw(stack, proc, w, vm, BuiltInTypes::ExternalMethodType);
    VArray *args = popArray(stack, proc, w, vm);
    VArray *argTypes = NULL;
    bool guessArgTypes = false;
    if(stack.top()->type == BuiltInTypes::NullType)
    {
        guessArgTypes = true;
        stack.pop();
    }
    else
    {
        argTypes = popArray(stack, proc, w, vm);
    }

    IClass *retType = unboxClass(stack.pop());
    QVector<Value *> argz;
    QVector<IClass *> kargTypes;
    for(int i=0; i<args->count(); i++)
    {
        argz.append(args->Elements[i]);
        if(!guessArgTypes)
        {
            IClass *type = unboxClass(argTypes->Elements[i]);
            if(!type)
            {
                proc->signal(TypeError2, BuiltInTypes::ClassType->toString(), argTypes->Elements[i]->type->toString());
            }
            kargTypes.append(type);
        }
    }

    Value *ret = CallForeign(funcPtr, argz, retType, kargTypes, guessArgTypes, proc, vm);
    if(ret)
        stack.push(ret);
    else
        stack.push(vm->GetAllocator().null());
}

void CurrentParseTreeProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm)
{
    Value *v = vm->GetGlobal("%parseTree");
    stack.push(v);
}

void MakeParserProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm)
{
    QString datum = popString(stack, proc, w, vm);
    IClass *parserClass = unboxClass(vm->GetType(VMId::get(RId::Parser)));
    IObject *parser = parserClass->newValue(&vm->GetAllocator());
    parser->setSlotValue(VMId::get(RId::InputPos), vm->GetAllocator().newInt(0));
    parser->setSlotValue(VMId::get(RId::Data), vm->GetAllocator().newString(
                             datum));
    stack.push(vm->GetAllocator().newObject(parser, parserClass));
}

void PushParserBacktrackPointProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm)
{
    IObject *receiver = unboxObj(popValue(stack, proc, w, vm));
    int arg1 = popInt(stack, proc, w, vm);
    ParserObj *parser = dynamic_cast<ParserObj *>(receiver);
    parser->stack.push(ParseFrame(arg1, parser->pos, true));
}

void IgnoreParserBacktrackPointProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm)
{

    IObject *receiver = unboxObj(popValue(stack, proc, w, vm));
    ParserObj *parser = dynamic_cast<ParserObj *>(receiver);
    ParseFrame f = parser->stack.pop();
    if(!f.backTrack)
        w->assert(proc, false, InternalError1,
                  VM::argumentErrors.get(ArgErr::StackTopNotBacktrackPointToIgnore1, str(f.continuationLabel)));
}

void ActivationFrameProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm)
{

    int n = popIntOrCoercable(stack, proc, w, vm);
    Frame *s = proc->stack;
    n++; // skip the frame of this very function, 'activation_frame'

    // we used n > 0 instead of n!=0 to avoid problems with negative arguments
    // todo: this slows down things since we still search the entire frame list before
    // discovering the problem
    while(n > 0 && s != NULL)
    {
        s = s->next;
        n--;
    }

    if(n == 0)
    {
        // todo: not garbage collected?
        // we need not only a GCMonitor flag
        // but "heap objec owns real object" flag
        // for deletion upon destruction
        stack.push(vm->GetAllocator().newObject(s, BuiltInTypes::ActivationFrameType, false));
    }
    else
    {
        throw VMError(InternalError1).arg(VM::argumentErrors.get(ArgErr::BadFrameNumber1, str(n)));
    }
}

void MigrateToGuiThreadProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm)
{
    //proc->migrateTo(&vm->guiScheduler);
    //emit w->EmitGuiSchedule();
    proc->wannaMigrateTo = &vm->guiScheduler;
    proc->exitTimeSlice();
}

void MigrateBackFromGuiThreadProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm)
{
    //proc->migrateTo(&vm->mainScheduler);
    proc->wannaMigrateTo = &vm->mainScheduler;
    proc->exitTimeSlice();
}

void CloseChannelProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm)
{
    Channel *chan = popChannel(stack, proc, w, vm);
    chan->close();
}

void ChannelClosedProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm)
{
    Channel *chan = popChannel(stack, proc, w, vm);
    stack.push(vm->GetAllocator().newBool(chan->closed()));
}

void ImageRotatedProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm)
{
    IClass *type;
    QImage *handle = popGuiReceiver<QImage *>(stack, type, proc, w, vm);
    double degrees = popDoubleOrCoercable(stack, proc, w, vm);

    QTransform trans;
    double width, height;
    QImage *img2;

    width = handle->width()/2;
    height = handle->height() /2 ;
#ifndef ENGLISH_PL
    // negative because of 'Arabic' coordinate system
    trans = trans.translate(width,height).rotate(-degrees).translate(-width,-height);
#else
    trans = trans.translate(width,height).rotate(degrees).translate(-width,-height);
#endif
    img2 = new QImage(handle->width(),handle->height(), handle->format());

    QPainter p(img2);
    QBrush brsh(handle->pixel(0,0));
    p.setBrush(brsh);
    p.fillRect(0,0,handle->width(), handle->height(), handle->pixel(0,0));
    p.translate(width, height);
    p.rotate(-degrees);
    p.translate(-width, -height);
    p.drawImage(0,0, *handle);

    Value *ret = newGuiObject(img2, type, vm);
    stack.push(ret);
}

void ImageScaledProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm)
{
    IClass *type;
    QImage *handle = popGuiReceiver<QImage *>(stack, type, proc, w, vm);
    double s1= popDoubleOrCoercable(stack, proc, w, vm);
    double s2= popDoubleOrCoercable(stack, proc, w, vm);

    QTransform trans;
    trans = trans.scale(s1, s2);
    QImage *img2 = new QImage(handle->transformed(trans));

    Value *ret = newGuiObject(img2, type, vm);
    stack.push(ret);
}

void ImageDrawLineProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm)
{
    IClass *type;
    QImage *handle = popGuiReceiver<QImage *>(stack, type, proc, w, vm);

    int x1 = popIntOrCoercable(stack, proc, w, vm);
    int y1 = popIntOrCoercable(stack, proc, w, vm);
    int x2 = popIntOrCoercable(stack, proc, w, vm);
    int y2 = popIntOrCoercable(stack, proc, w, vm);

    {
        QPainter p(handle);
        p.drawLine(x1,y1,x2,y2);
    }
}

void ImageFlippedProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm)
{
    IClass *type;
    QImage *handle = popGuiReceiver<QImage *>(stack, type, proc, w, vm);
    double s1 = popDoubleOrCoercable(stack, proc, w, vm);
    double s2 = popDoubleOrCoercable(stack, proc, w, vm);

    if(s1>0)
        s1 = 1;
    else if(s1<0)
        s1 = -1;
    if(s2>0)
        s2 = 1;
    else if(s2<0)
        s2 = -1;

    QTransform trans;
    trans = trans.scale(s1, s2);
    QImage *img2 = new QImage(handle->transformed(trans));

    Value *ret = newGuiObject(img2, type, vm);
    stack.push(ret);
}

void ImageCopiedProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm)
{
    IClass *type;
    QImage *handle = popGuiReceiver<QImage *>(stack, type, proc, w, vm);

    QImage *img2 = new QImage(handle->copy());

    Value *ret = newGuiObject(img2, type, vm);
    stack.push(ret);
}

void ImageSetPixelColorProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm)
{
    IClass *type;
    QImage *handle = popGuiReceiver<QImage *>(stack, type, proc, w, vm);
    int x = popIntOrCoercable(stack, proc, w, vm);
    int y = popIntOrCoercable(stack, proc, w, vm);
    int clr = popIntOrCoercable(stack, proc, w, vm);

    handle->setPixel(x,y, clr);
}

void ImagePixelColorProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm)
{
    IClass *type;
    QImage *handle = popGuiReceiver<QImage *>(stack, type, proc, w, vm);
    int x = popIntOrCoercable(stack, proc, w, vm);
    int y = popIntOrCoercable(stack, proc, w, vm);
    int clr = handle->pixel(x, y);

    stack.push(vm->GetAllocator().newInt(clr));
}

void ImageWidthProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm)
{
    IClass *type;
    QImage *handle = popGuiReceiver<QImage *>(stack, type, proc, w, vm);

    stack.push(vm->GetAllocator().newInt(handle->width()));
}

void ImageHeightProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm)
{
    IClass *type;
    QImage *handle = popGuiReceiver<QImage *>(stack, type, proc, w, vm);

    stack.push(vm->GetAllocator().newInt(handle->height()));
}

void ImageDrawTextProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm)
{
    IClass *type;
    QImage *handle = popGuiReceiver<QImage *>(stack, type, proc, w, vm);
    QString text = popString(stack, proc, w, vm);
    int x = popIntOrCoercable(stack, proc, w, vm);
    int y = popIntOrCoercable(stack, proc, w, vm);

    QPainter p(handle);
    p.setFont(QFont("Arial", 12));
    p.drawText(x, y, text);
}

void ForeignWindowMaximizeProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm)
{
    IClass *type;
    QWidget *win = popGuiReceiver<QWidget *>(stack, type, proc, w, vm);

    win->setWindowState(Qt::WindowMaximized);
}

void ForeignWindowMoveToProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm)
{
    IClass *type;
    QWidget *win = popGuiReceiver<QWidget *>(stack, type, proc, w, vm);
    int x = popIntOrCoercable(stack, proc, w, vm);
    int y = popIntOrCoercable(stack, proc, w, vm);

    win->move(x, y);
}

void ForeignWindowAddProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm)
{
    IClass *type;
    QWidget *widget = popGuiReceiver<QWidget *>(stack, type, proc, w, vm);
    QWidget *control = popGuiReceiver<QWidget *>(stack, type, proc, w, vm);
#ifndef ENGLISH_PL
    control->move(widget->width() - (control->x() + control->width()),
                  control->y());
#endif
    control->setParent(widget);
    QFont f = control->font();
    control->setFont(QFont(f.family(), f.pointSize()+3));
    control->show();
}

void ForeignWindowSetSizeProc(VOperandStack &stack, Process *proc, RunWindow *rw, VM *vm)
{
    IClass *type;
    QWidget *widget = popGuiReceiver<QWidget *>(stack, type, proc, rw, vm);
    int w = popIntOrCoercable(stack, proc, rw, vm);
    int h = popIntOrCoercable(stack, proc, rw, vm);

#ifndef ENGLISH_PL
    //int wdiff = widget->width() - w;
    //*
    for(int i=0; i<widget->children().count(); i++)
    {
        QWidget *c = dynamic_cast<QWidget *>(widget->children().at(i));
        if(c)
        {
            int cw = c->width();
            int cx = c->x();
            int cright = cx + cw;
            int right_delta = widget->width() - cright;
            int newx = w - (right_delta + cw);
            c->move(newx, c->y());
            //qDebug() << "Moving " << c->windowTitle() << " to (" << c->x() <<", " << c->y() <<")";
        }
    }
    //*/
    // make it so resizing makes the righ side fixed, not
    // left side
    int left = widget->pos().x() + widget->width() - w;
    int top = widget->pos().y();
#endif
    widget->setFixedSize(w, h);
#ifndef ENGLISH_PL
    widget->move(left, top);
#endif
}

void ForeignWindowSetTitleProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm)
{
    IClass *type;
    QWidget *widget = popGuiReceiver<QWidget *>(stack, type, proc, w, vm);
    QString t = popString(stack, proc, w, vm);

    widget->setWindowTitle(t);
}

void ForeignWindowSetupProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm)
{
    IObject *window = unboxObj(popValue(stack, proc, w, vm));

    QMainWindow *win = new QMainWindow();
    Value *handle = vm->GetAllocator().newQObject(win);
    window->setSlotValue("handle", handle);
}

void ControlSetTextProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm)
{
    IClass *type;
    QWidget *widget = popGuiReceiver<QWidget *>(stack, type, proc, w, vm);
    QString str = popString(stack, proc, w, vm);
    widget->setWindowTitle(str);
}

void ControlSetSizeProc(VOperandStack &stack, Process *proc, RunWindow *rw, VM *vm)
{
    IClass *type;
    QWidget *widget = popGuiReceiver<QWidget *>(stack, type, proc, rw, vm);
    int w = popIntOrCoercable(stack, proc, rw, vm);
    int h = popIntOrCoercable(stack, proc, rw, vm);

#ifndef ENGLISH_PL
    int originalx = widget->x() + widget->width();
#endif
    widget->resize(w, h);
#ifndef ENGLISH_PL
    if(widget->parentWidget() != NULL)
    {
        originalx -= widget->width();
        widget->move(originalx, widget->y());
    }
#endif
}

void ControlSetLocationProc(VOperandStack &stack, Process *proc, RunWindow *rw, VM *vm)
{
    IClass *type;
    QWidget *widget = popGuiReceiver<QWidget *>(stack, type, proc, rw, vm);
    int x = popIntOrCoercable(stack, proc, rw, vm);
    int y = popIntOrCoercable(stack, proc, rw, vm);
#ifndef ENGLISH_PL
    if(widget->parentWidget())
    {
        int pw = widget->parentWidget()->width();
        x = (pw-1)-x;
        x-= widget->width();
    }
#endif
    widget->move(x, y);
}

void ControlTextProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm)
{
    IClass *type;
    QWidget *widget = popGuiReceiver<QWidget *>(stack, type, proc, w, vm);
    stack.push(vm->GetAllocator().newString(widget->windowTitle()));
}

void ControlShowProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm)
{
    IClass *type;
    QWidget *widget = popGuiReceiver<QWidget *>(stack, type, proc, w, vm);
    widget->show();
}

void ControlHideProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm)
{
    IClass *type;
    QWidget *widget = popGuiReceiver<QWidget *>(stack, type, proc, w, vm);
    widget->hide();
}

void ControlSetVisibleProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm)
{
    IClass *type;
    QWidget *widget = popGuiReceiver<QWidget *>(stack, type, proc, w, vm);
    bool show = popBool(stack, proc, w, vm);
    widget->setVisible(show);
}

void ControlCloseProc(VOperandStack &stack, Process *proc, RunWindow *rw, VM *vm)
{
    IClass *type;
    QWidget *widget = popGuiReceiver<QWidget *>(stack, type, proc, rw, vm);
    widget->close();
}

void ButtonSetTextProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm)
{
    IClass *type;
    QPushButton *handle = popGuiReceiver<QPushButton *>(stack, type, proc, w, vm);
    QString s = popString(stack, proc, w, vm);
    handle->setText(s);
}

void ButtonTextProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm)
{
    IClass *type;
    QPushButton *handle = popGuiReceiver<QPushButton *>(stack, type, proc, w, vm);

    stack.push(vm->GetAllocator().newString(handle->text()));
}

void TextboxSetTextProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm)
{
    IClass *type;
    QTextEdit *handle = popGuiReceiver<QTextEdit *>(stack, type, proc, w, vm);
    QString text = popString(stack, proc, w, vm);
    handle->document()->setPlainText(text);
}

void TextboxTextProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm)
{
    IClass *type;
    QTextEdit *handle = popGuiReceiver<QTextEdit *>(stack, type, proc, w, vm);
    stack.push(vm->GetAllocator().newString(handle->document()->toPlainText()));
}

void TextboxAppendTextProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm)
{
    IClass *type;
    QTextEdit *handle = popGuiReceiver<QTextEdit *>(stack, type, proc, w, vm);
    QString text = popString(stack, proc, w, vm);
    handle->append(text);
}

void LineEditSetTextProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm)
{
    IClass *type;
    QLineEdit *handle = popGuiReceiver<QLineEdit *>(stack, type, proc, w, vm);
    QString text = popString(stack, proc, w, vm);
    handle->setText(text);
}

void LineEditTextProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm)
{
    IClass *type;
    QLineEdit *handle = popGuiReceiver<QLineEdit *>(stack, type, proc, w, vm);
    stack.push(vm->GetAllocator().newString(handle->text()));
}

void LineEditAppendTextProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm)
{
    IClass *type;
    QLineEdit *handle = popGuiReceiver<QLineEdit *>(stack, type, proc, w, vm);
    QString text = popString(stack, proc, w, vm);
    handle->setText(handle->text() + text);
}

void ListboxAddProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm)
{
    IClass *type;
    QListWidget *handle = popGuiReceiver<QListWidget *>(stack, type, proc, w, vm);
    QString text = popString(stack, proc, w, vm);
    handle->addItem(text);
}

void ListboxInsertItemProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm)
{
    IClass *type;
    QListWidget *handle = popGuiReceiver<QListWidget *>(stack, type, proc, w, vm);
    QString text = popString(stack, proc, w, vm);
    int index = popInt(stack, proc, w, vm);

    handle->insertItem(index, text);
}

void ListboxGetItemProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm)
{
    IClass *type;
    QListWidget *handle = popGuiReceiver<QListWidget *>(stack, type, proc, w, vm);
    int index = popInt(stack, proc, w, vm);

    stack.push(vm->GetAllocator().newString(handle->item(index)->text()));
}

void ComboBoxSetTextProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm)
{
    IClass *type;
    QComboBox *handle = popGuiReceiver<QComboBox *>(stack, type, proc, w, vm);
    QString text = popString(stack, proc, w, vm);
    handle->setEditText(text);
}

void ComboBoxTextProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm)
{
    IClass *type;
    QComboBox *handle = popGuiReceiver<QComboBox *>(stack, type, proc, w, vm);
    stack.push(vm->GetAllocator().newString(handle->currentText()));
}

void ComboBoxAddProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm)
{
    IClass *type;
    QComboBox *handle = popGuiReceiver<QComboBox *>(stack, type, proc, w, vm);
    Value *v = popValue(stack, proc, w, vm);
    handle->addItem(v->toString());
}

void ComboBoxInsertItemProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm)
{
    IClass *type;
    QComboBox *handle = popGuiReceiver<QComboBox *>(stack, type, proc, w, vm);
    Value *v = popValue(stack, proc, w, vm);
    int index = popInt(stack, proc, w, vm);

    handle->insertItem(index, v->toString());
}

void ComboBoxGetItemProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm)
{
    IClass *type;
    QComboBox *handle = popGuiReceiver<QComboBox *>(stack, type, proc, w, vm);
    int index = popInt(stack, proc, w, vm);
    stack.push(vm->GetAllocator().newString(handle->itemText(index)));
}

void ComboBoxSetEditableProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm)
{
    IClass *type;
    QComboBox *handle = popGuiReceiver<QComboBox *>(stack, type, proc, w, vm);
    bool editable = popBool(stack, proc, w, vm);
    handle->setEditable(editable);
}

void LabelSetTextProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm)
{
    IClass *type;
    QLabel *handle = popGuiReceiver<QLabel *>(stack, type, proc, w, vm);
    QString text = popString(stack, proc, w, vm);
    handle->setText(text);
}

void LabelTextProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm)
{
    IClass *type;
    QLabel *handle = popGuiReceiver<QLabel *>(stack, type, proc, w, vm);
    stack.push(vm->GetAllocator().newString(handle->text()));
}

void CheckboxSetTextProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm)
{
    IClass *type;
    QCheckBox *handle = popGuiReceiver<QCheckBox *>(stack, type, proc, w, vm);
    QString text = popString(stack, proc, w, vm);
    handle->setText(text);
}

void CheckboxTextProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm)
{
    IClass *type;
    QCheckBox *handle = popGuiReceiver<QCheckBox *>(stack, type, proc, w, vm);
    stack.push(vm->GetAllocator().newString(handle->text()));
}

void CheckboxSetValueProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm)
{
    IClass *type;
    QCheckBox *handle = popGuiReceiver<QCheckBox *>(stack, type, proc, w, vm);
    int value = popInt(stack, proc, w, vm);
    handle->setCheckState((Qt::CheckState)value);
}

void CheckboxValueProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm)
{
    IClass *type;
    QCheckBox *handle = popGuiReceiver<QCheckBox *>(stack, type, proc, w, vm);
    stack.push(vm->GetAllocator().newInt(handle->checkState()));
}

void RadioButtonSetTextProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm)
{
    IClass *type;
    QRadioButton *handle = popGuiReceiver<QRadioButton *>(stack, type, proc, w, vm);
    QString text = popString(stack, proc, w, vm);
    handle->setText(text);
}

void RadioButtonTextProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm)
{
    IClass *type;
    QRadioButton *handle = popGuiReceiver<QRadioButton *>(stack, type, proc, w, vm);
    stack.push(vm->GetAllocator().newString(handle->text()));
}

void RadioButtonSetValueProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm)
{
    IClass *type;
    QRadioButton *handle = popGuiReceiver<QRadioButton *>(stack, type, proc, w, vm);
    bool value = popBool(stack, proc, w, vm);
    handle->setChecked(value);
}

void RadioButtonValueProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm)
{
    IClass *type;
    QRadioButton *handle = popGuiReceiver<QRadioButton *>(stack, type, proc, w, vm);
    stack.push(vm->GetAllocator().newBool(handle->isChecked()));
}

void ButtonGroupAddProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm)
{
    IClass *type, *type2;
    QButtonGroup *handle = popGuiReceiver<QButtonGroup *>(stack, type, proc, w, vm);

    verifyStackNotEmpty(stack, proc, vm);
    Value *btn = stack.top();

    QAbstractButton *button = popGuiReceiver<QAbstractButton *>(stack, type2, proc, w, vm);

    if(!button)
        throw VMError(InternalError);

    button->setProperty("valueptr", QVariant::fromValue<void *>(btn));
    int theId = ((ButtonGroupForeignClass *) type)->runningIdCount++;

    handle->addButton(button, theId);
    stack.push(vm->GetAllocator().newInt(theId));
}

void ButtonGroupGetButtonProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm)
{
    IClass *type;
    QButtonGroup *handle = popGuiReceiver<QButtonGroup *>(stack, type, proc, w, vm);
    int theId = popInt(stack, proc, w, vm);

    QAbstractButton *button = handle->button(theId);

    Value *btnObj = (Value *) button->property("valueptr").value<void *>();
    stack.push(btnObj);
}

void ClassNewObjectProc(VOperandStack &stack, Process *proc, RunWindow *w, VM *vm)
{
    Value *v = popValue(stack, proc, w, vm);
    IObject *theClassObj = unboxObj(v);
    IClass *theClass = dynamic_cast<IClass *>(theClassObj);
    if(theClass)
    {
        stack.push(vm->GetAllocator().newObject(theClass->newValue(&vm->GetAllocator()), theClass));
    }
    else
    {
        throw VMError(InternalError);
    }
}

void setupChildren(QGridLayout *layout,Value *v, Reference *ref, QString label, int row, VM *vm)
{
    QCheckBox *cb;
    QGroupBox *qf;
    QScrollArea *sa;
    Object *obj;
    QGridLayout *vb;
    QLineEdit *le;

    int subRow;
    VArray *arr;
    if(v->type == BuiltInTypes::IntType ||
       v->type == BuiltInTypes::DoubleType ||
       v->type == BuiltInTypes::LongType ||
       v->type == BuiltInTypes::StringType)
    {
        layout->addWidget(new QLabel(label), row, 0);
        le = new QLineEdit(v->toString());
        if(ref != NULL)
        {

            QObject::connect(le,
                       SIGNAL(textChanged(QString)),
                       new GUIEditWidgetHandler(
                           ref,
                           le,
                           vm),
                       SLOT(lineEditChanged()));
        }

        layout->addWidget(le, row, 1);
    }
    else if(v->type == BuiltInTypes::BoolType)
    {
        cb = new QCheckBox();
        cb->setChecked(unboxBool(v));
        if(ref != NULL)
        {
            QObject::connect(cb,
                       SIGNAL(stateChanged(int)),
                       new GUIEditWidgetHandler(
                           ref,
                           cb,
                           vm),
                       SLOT(checkboxChanged(int)));
        }
        layout->addWidget(new QLabel(label), row, 0);
        layout->addWidget(cb, row, 1);
    }
    else if(v->isObject())
    {
        qf = new QGroupBox(label);
        layout->addWidget(qf, row, 0, 1, 2);
        obj = dynamic_cast<Object *>(unboxObj(v));
        if(obj != NULL)
        {
            vb = new QGridLayout();
            subRow = 0;
            for(QVector<QString>::iterator i = obj->slotNames.begin(); i!= obj->slotNames.end(); ++i)
            {
                setupChildren(vb, obj->_slots[*i], new FieldReference(obj, *i), *i, subRow++,vm);
            }
            qf->setLayout(vb);
        }
     }
    else if(v->type == BuiltInTypes::ArrayType)
    {
        sa = new QScrollArea();
        vb = new QGridLayout();
        layout->addWidget(sa, row, 0, 1, 2);
        arr = unboxArray(v);
        vb->addWidget(new QLabel(label), 0, 0, 1, 2);
        for(int i=0; i<arr->count(); i++)
        {
            setupChildren(vb, arr->Elements[i], new ArrayReference(arr, i),QString("%1").arg(i+1), i+1, vm);
        }
        sa->setLayout(vb);
        sa->adjustSize();
     }
}

Value *editAndReturn(Value *v, RunWindow *w, VM *vm)
{
    QDialog *dlg = new QDialog(w);
    dlg->setWindowTitle(VM::argumentErrors.get(ArgErr::Editing1, v->type->getName()));
    QVBoxLayout *ly = new QVBoxLayout(dlg);

    QFrame *frame = new QFrame();
    QGridLayout *gl = new QGridLayout();
    setupChildren(gl, v, NULL, "", 0, vm);
    frame->setLayout(gl);
    ly->addWidget(frame);

    QPushButton *ok = new QPushButton(VM::argumentErrors[ArgErr::Ok]);
    QPushButton *cancel = new QPushButton(VM::argumentErrors[ArgErr::Cancel]);
    ly->addWidget(ok);
    ly->addWidget(cancel);
    dlg->setLayout(ly);
#ifndef ENGLISH_PL
    dlg->setLayoutDirection(Qt::RightToLeft);
#endif
    dlg->connect(ok, SIGNAL(clicked()), dlg, SLOT(accept()));
    dlg->connect(cancel, SIGNAL(clicked()), dlg, SLOT(reject()));
    bool ret = dlg->exec()== QDialog::Accepted;
    v = vm->GetAllocator().newBool(ret);

    return v;
}
