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

void PrintProc(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *)
{
    if(stack.empty())
        proc->signal(InternalError1, "Empty operand stack when reading value to in 'print'");
    Value *v = stack.pop();
    QString str = v->toString();
    w->textLayer.print(str);
    w->redrawWindow();
}

void PushReadChanProc(Stack<Value *> &stack, Process *, RunWindow *w, VM *)
{
    stack.push(w->readChannel);
}

void MouseEventChanProc(Stack<Value *> &stack, Process *, RunWindow *w, VM *)
{
    stack.push(w->mouseEventChannel);
}

void MouseDownEventChanProc(Stack<Value *> &stack, Process *, RunWindow *w, VM *)
{
    stack.push(w->mouseDownEventChannel);
}

void MouseUpEventChanProc(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm)
{
    stack.push(w->mouseUpEventChannel);
}

void MouseMoveEventChanProc(Stack<Value *> &stack, Process *, RunWindow *w, VM *)
{
    stack.push(w->mouseMoveEventChannel);
}


void KbEventChanProc(Stack<Value *> &stack, Process *, RunWindow *w, VM *)
{
    stack.push(w->kbEventChannel);
}

WindowReadMethod::WindowReadMethod(RunWindow *parent, VM *vm)
{
    this->parent = parent;
    this->vm = vm;
    this->readNum = false;
}

void WindowReadMethod::operator ()(Stack<Value *> &operandStack, Process *proc)
{
    readNum = popInt(operandStack, proc, parent, vm);
    parent->beginInput();
    parent->update(); // We must do this, because normal updating is done
                      // by calling redrawWindow() in the instruction loop, and
                      // here we suspend the instruction loop...
}

WindowProxyMethod::WindowProxyMethod(RunWindow *parent, VM *vm, VM_PROC proc, bool mustRunInGui)
{
    this->vm = vm;
    this->parent = parent;
    this->proc = proc;
    this->mustRunInGui = mustRunInGui;
}

void WindowProxyMethod::operator ()(Stack<Value *> &operandStack, Process *process)
{
    proc(operandStack, process, parent, vm);
}

void SetCursorPosProc(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm)
{
    int line = popInt(stack, proc, w, vm);
    int col = popInt(stack, proc, w, vm);
    bool result = w->textLayer.setCursorPos(line, col);
    proc->assert(result, ArgumentError, VM::argumentErrors[ArgErr::InvalidCursorPosition]);
}

void GetCursorRowProc(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm)
{
    int r = w->textLayer.getCursorRow();
    stack.push(vm->GetAllocator().newInt(r));
}

void GetCursorColProc(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm)
{
    int c = w->textLayer.getCursorCol();
    stack.push(vm->GetAllocator().newInt(c));
}


void PrintUsingWidthProc(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *)
{
    Value *v = stack.pop();
    w->typeCheck(proc, stack.top(), BuiltInTypes::IntType);
    int wid = stack.pop()->unboxInt();
    QString str = v->toString();
    w->textLayer.print(str, wid);
}

void DrawPixelProc(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm)
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

void DrawLineProc(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm)
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

void DrawRectProc(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm)
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

void DrawCircleProc(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm)
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

void RandomProc(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm)
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

void ToNumProc(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm)
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

void ConcatProc(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm)
{
    QString str1 = popString(stack, proc, w, vm);
    QString str2 = popString(stack, proc, w, vm);


    QString ret = str1 + str2;
    stack.push(vm->GetAllocator().newString(ret));
}

void StrLenProc(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm)
{
    QString str = popString(stack, proc, w, vm);

    int ret = str.length();
    stack.push(vm->GetAllocator().newInt(ret));
}

void StrFirstProc(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm)
{
    int n = popInt(stack, proc, w, vm);
    QString str = popString(stack, proc, w, vm);

    QString ret = str.left(n);
    stack.push(vm->GetAllocator().newString(ret));
}

void StrLastProc(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm)
{
    int n = popInt(stack, proc, w, vm);

    QString str = popString(stack, proc, w, vm);

    QString ret = str.right(n);
    stack.push(vm->GetAllocator().newString(ret));
}

void StrMidProc(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm)
{
    QString str = popString(stack, proc, w, vm);


    int i = popInt(stack, proc, w, vm);

    int n = popInt(stack, proc, w, vm);

    // We make indexing one-based instead of QT's zero-based
    // todo: range checking in StrMidProc()
    QString ret = str.mid(i -1 ,n);
    stack.push(vm->GetAllocator().newString(ret));
}

void StrBeginsWithProc(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm)
{
    QString strMain = popString(stack, proc, w, vm);
    QString strSub = popString(stack, proc, w, vm);

    bool ret = strMain.startsWith(strSub, Qt::CaseSensitive);
    stack.push(vm->GetAllocator().newBool(ret));
}

void StrEndsWithProc(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm)
{
    QString strMain = popString(stack, proc, w, vm);
    QString strSub = popString(stack, proc, w, vm);

    bool ret = strMain.endsWith(strSub, Qt::CaseSensitive);
    stack.push(vm->GetAllocator().newBool(ret));
}

void StrContainsProc(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm)
{
    QString strMain = popString(stack, proc, w, vm);
    QString strSub = popString(stack, proc, w, vm);

    bool ret = strMain.contains(strSub, Qt::CaseSensitive);
    stack.push(vm->GetAllocator().newBool(ret));
}

void StrSplitProc(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm)
{
    QString str = popString(stack, proc, w, vm);
    QString separator = popString(stack, proc, w, vm);

    QStringList result = str.split(separator, QString::KeepEmptyParts);
    Value *ret = vm->GetAllocator().newArray(result.count());
    for(int i=0; i<result.count(); i++)
    {
        ret->v.arrayVal->Elements[i] = vm->GetAllocator().newString(result[i]);
    }
    stack.push(ret);
}

void StrTrimProc(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm)
{
    QString str = popString(stack, proc, w, vm);
    QString str2 = str.trimmed();
    Value *ret = vm->GetAllocator().newString(str2);
    stack.push(ret);
}

void StrReplaceProc(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm)
{
    QString str = popString(stack, proc, w, vm);
    QString str2 = popString(stack, proc, w, vm);
    QString str3 = popString(stack, proc, w, vm);
    str = str.replace(str2, str3);
    Value *ret = vm->GetAllocator().newString(str);
    stack.push(ret);
}

void ToStringProc(Stack<Value *> &stack, Process *proc, RunWindow *, VM *vm)
{
    verifyStackNotEmpty(stack, proc, vm);
    Value *v = stack.pop();
    QString ret;
    switch(v->tag)
    {
    case Int:
        ret = QString("%1").arg(v->unboxInt());
        break;
    case Long:
        ret = QString("%1").arg(v->unboxLong());
        break;
    case Double:
        ret = QString("%1").arg(v->unboxDouble());
        break;
    case StringVal:
        ret = v->unboxStr();
        break;
    case RawVal:
        ret = QString("%1").arg((long)v->unboxRaw());
        break;
    case ObjectVal:
        ret = QString("%1").arg(v->unboxObj()->toString());
        break;
    default:
        break;
    }
    stack.push(vm->GetAllocator().newString(ret));
}

void RoundProc(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm)
{
    double d = popDoubleOrCoercable(stack, proc, w, vm);
    int i = (int) d;
    stack.push(vm->GetAllocator().newInt(i));

}

void RemainderProc(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm)
{
    int n1 = popInt(stack, proc, w, vm);
    int n2 = popInt(stack, proc, w, vm);

    if(n2 == 0)
        proc->signal(DivisionByZero);
    int i = n1 % n2;
    stack.push(vm->GetAllocator().newInt(i));
}

int popIntOrCoercable(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm)
{
    if(stack.empty())
    {
        proc->signal(InternalError1, "Empty operand stack when reading value");
    }
    Value *v = stack.pop();
    if(v->tag != Int && v->tag != Double && v->tag != Long)
    {
        w->typeError(proc, BuiltInTypes::NumericType, v->type);
    }
    if(v->tag == Double)
        v = vm->GetAllocator().newInt((int) v->unboxDouble());
    if(v->tag == Long)
        v = vm->GetAllocator().newInt((int) v->unboxLong());

    return v->unboxInt();
}

double popDoubleOrCoercable(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm)
{
    if(stack.empty())
    {
        proc->signal(InternalError1, "Empty operand stack when reading double or double-coercible value");
    }
    Value *v = stack.pop();
    if(v->tag != Int && v->tag != Double && v->tag != Long)
    {
        w->typeError(proc, BuiltInTypes::NumericType, v->type);
    }
    if(v->tag == Int)
        v = vm->GetAllocator().newDouble(v->unboxInt());
    if(v->tag == Long)
        v = vm->GetAllocator().newDouble(v->unboxLong());
    return v->unboxDouble();
}

void verifyStackNotEmpty(Stack<Value *> &stack, Process *proc, VM *)
{
    if(stack.empty())
    {
        proc->signal(InternalError1, "Empty operand stack");
    }
}

void SinProc(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm)
{

    double theta = popDoubleOrCoercable(stack, proc, w, vm);

    double result = sin(theta);
    stack.push(vm->GetAllocator().newDouble(result));

}
void CosProc(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm)
{
    double theta = popDoubleOrCoercable(stack, proc, w, vm);

    double result = cos(theta);
    stack.push(vm->GetAllocator().newDouble(result));

}
void TanProc(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm)
{
    double theta = popDoubleOrCoercable(stack, proc, w, vm);

    double result = tan(theta);
    stack.push(vm->GetAllocator().newDouble(result));

}
void ASinProc(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm)
{

    double theta = popDoubleOrCoercable(stack, proc, w, vm);

    double result = asin(theta);
    stack.push(vm->GetAllocator().newDouble(result));

}
void ACosProc(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm)
{
    double theta = popDoubleOrCoercable(stack, proc, w, vm);

    double result = acos(theta);
    stack.push(vm->GetAllocator().newDouble(result));

}
void ATanProc(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm)
{
    double theta = popDoubleOrCoercable(stack, proc, w, vm);

    double result = atan(theta);
    stack.push(vm->GetAllocator().newDouble(result));

}
void SqrtProc(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm)
{
    double theta = popDoubleOrCoercable(stack, proc, w, vm);
    double result = sqrt(theta);
    stack.push(vm->GetAllocator().newDouble(result));

}

void PowProc(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm)
{
    double base = popDoubleOrCoercable(stack, proc, w, vm);
    double power = popDoubleOrCoercable(stack, proc, w, vm);
    double result = pow(base, power);
    stack.push(vm->GetAllocator().newDouble(result));

}

void Log10Proc(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm)
{
    double theta = popDoubleOrCoercable(stack, proc, w, vm);
    double result = log10(theta);
    stack.push(vm->GetAllocator().newDouble(result));

}
void LnProc(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm)
{
    double theta = popDoubleOrCoercable(stack, proc, w, vm);
    double result = log(theta);
    stack.push(vm->GetAllocator().newDouble(result));

}

void LoadImageProc(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm)
{
    verifyStackNotEmpty(stack, proc, vm);
    QString fname = popString(stack, proc, w, vm);
    fname = w->ensureCompletePath(proc, fname);

    if(!QFile::exists(fname))
    {
        w->assert(proc, false, ArgumentError, VM::argumentErrors.get(ArgErr::NonExistingImageFile1,fname));
    }
    IClass *imgClass = dynamic_cast<IClass *>(vm->GetType(VMId::get(RId::Image))->unboxObj());
    QImage *img = new QImage(fname);
    IObject *obj = imgClass->newValue(&vm->GetAllocator());
    obj->setSlotValue("handle", vm->GetAllocator().newRaw(img, BuiltInTypes::ObjectType));
    stack.push(vm->GetAllocator().newObject(obj, imgClass));
}

void LoadSpriteProc(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm)
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

void SpriteFromImageProc(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm)
{
    IClass *imgClass = dynamic_cast<IClass *>(vm->GetType(VMId::get(RId::Image))->unboxObj());
    verifyStackNotEmpty(stack, proc, vm);
    w->typeCheck(proc, stack.top(), imgClass);
    IObject *obj = stack.pop()->unboxObj();
    QImage *handle = reinterpret_cast<QImage*>
            (obj->getSlotValue("handle")->unboxRaw());

    Sprite *sprite = new Sprite(QPixmap::fromImage(*handle));
    w->spriteLayer.AddSprite(sprite);

    stack.push(MakeSpriteValue(sprite, &vm->GetAllocator()));
}

Sprite *GetSpriteFromValue(Value * v)
{
    IObject *obj = v->unboxObj();
    Value *rawSpr = obj->getSlotValue("_handle");
    Sprite *spr = (Sprite *) rawSpr->unboxRaw();
    return spr;
}

Value *MakeSpriteValue(Sprite *sprite, Allocator *alloc)
{
    Value *spriteHandle = alloc->newRaw(sprite, BuiltInTypes::ObjectType);
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

void DrawImageProc(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm)
{
    IClass *imgClass = dynamic_cast<IClass *>(vm->GetType(VMId::get(RId::Image))->unboxObj());

    w->typeCheck(proc, stack.top(), imgClass);
    IObject *obj = stack.pop()->unboxObj();
    QImage *handle = reinterpret_cast<QImage*>
            (obj->getSlotValue("handle")->unboxRaw());

    int x = popIntOrCoercable(stack, proc, w , vm);
    int y = popIntOrCoercable(stack, proc, w , vm);

    w->paintSurface->TX(x, handle->width());
    x-= handle->width();

    QPainter p(w->paintSurface->GetImageForWriting());

    p.drawImage(x, y, *handle);
    w->redrawWindow();

}

void DrawSpriteProc(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm)
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

void ShowSpriteProc(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm)
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

void HideSpriteProc(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm)
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

void GetSpriteLeftProc(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm)
{
    w->typeCheck(proc, stack.top(), BuiltInTypes::SpriteType);
    Sprite  *sprite = GetSpriteFromValue(stack.pop());

    int ret = sprite->location.x() + sprite->image.width();
    stack.push(vm->GetAllocator().newInt(ret));
}

void GetSpriteRightProc(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm)
{
    w->typeCheck(proc, stack.top(), BuiltInTypes::SpriteType);
    Sprite  *sprite = GetSpriteFromValue(stack.pop());

    int ret = sprite->location.x();
    stack.push(vm->GetAllocator().newInt(ret));
}

void GetSpriteTopProc(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm)
{
    w->typeCheck(proc, stack.top(), BuiltInTypes::SpriteType);
    Sprite  *sprite = GetSpriteFromValue(stack.pop());

    int ret = sprite->boundingRect().top();
    stack.push(vm->GetAllocator().newInt(ret));
}

void GetSpriteBottomProc(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm)
{
    w->typeCheck(proc, stack.top(), BuiltInTypes::SpriteType);
    Sprite  *sprite = GetSpriteFromValue(stack.pop());

    int ret = sprite->boundingRect().bottom();
    stack.push(vm->GetAllocator().newInt(ret));
}

void GetSpriteWidthProc(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm)
{
    w->typeCheck(proc, stack.top(), BuiltInTypes::SpriteType);
    Value *spriteVal = stack.pop();
    Sprite  *sprite = GetSpriteFromValue(spriteVal);

    int ret = sprite->boundingRect().width();
    stack.push(vm->GetAllocator().newInt(ret));
}

void GetSpriteHeightProc(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm)
{
    w->typeCheck(proc, stack.top(), BuiltInTypes::SpriteType);
    Sprite  *sprite = GetSpriteFromValue(stack.pop());

    int ret = sprite->boundingRect().height();
    stack.push(vm->GetAllocator().newInt(ret));
}

void GetSpriteImageProc(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm)
{
    w->typeCheck(proc, stack.top(), BuiltInTypes::SpriteType);
    Sprite  *sprite = GetSpriteFromValue(stack.pop());

    QString clsName = VMId::get(RId::Image);
    QImage *img = new QImage(sprite->image.toImage());
    IClass *imgClass = dynamic_cast<IClass *>
            (vm->GetType(clsName)->unboxObj());
    IObject *imgObj = imgClass->newValue(&vm->GetAllocator());
    imgObj->setSlotValue("handle", vm->GetAllocator().newRaw(img, BuiltInTypes::ObjectType));

    stack.push(vm->GetAllocator().newObject(imgObj, imgClass));
}

void SetSpriteImageProc(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm)
{
    QString clsName = VMId::get(RId::Image);
    IClass *imgClass = dynamic_cast<IClass *>
            (vm->GetType(clsName)->unboxObj());

    w->typeCheck(proc, stack.top(), BuiltInTypes::SpriteType);
    Sprite  *sprite = GetSpriteFromValue(stack.pop());

    w->typeCheck(proc, stack.top(), imgClass);
    IObject *imgObj = stack.pop()->unboxObj();
    QImage *img = reinterpret_cast<QImage *>
            (imgObj->getSlotValue("handle")->unboxRaw());

    sprite->setImage(QPixmap::fromImage(*img));
    w->spriteLayer.changing();
    w->redrawWindow();
}

void WaitProc(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm)
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
    int ms = stack.pop()->unboxNumeric();
    proc->owner->makeItWaitTimer(proc, ms);
}

void ZoomProc(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm)
{
    int x1 = popIntOrCoercable(stack, proc, w , vm);
    int y1 = popIntOrCoercable(stack, proc, w , vm);
    int x2 = popIntOrCoercable(stack, proc, w , vm);
    int y2 = popIntOrCoercable(stack, proc, w , vm);
    //w->TX(x1);
    //w->TX(x2);
    w->paintSurface->zoom(x1, y1, x2, y2);
}

void ClsProc(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm)
{
    w->cls();
}

void ClearTextProc(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm)
{
    w->clearAllText();
}

void SetTextColorProc(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm)
{
    int color = popInt(stack, proc, w, vm);
    w->assert(proc, color>=0 && color <=15, ArgumentError, "Color value must be from 0 to 15");
    w->paintSurface->setTextColor(w->paintSurface->GetColor(color));
    w->redrawWindow();
}

void PointAtProc(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm)
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

void PointRgbAtProc(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm)
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
    arr->v.arrayVal->set(a.newInt(1), a.newInt(qRed(color)));
    arr->v.arrayVal->set(a.newInt(2), a.newInt(qGreen(color)));
    arr->v.arrayVal->set(a.newInt(3), a.newInt(qBlue(color)));

    stack.push(arr);
}

void BuiltInConstantProc(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm)
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

void StringIsNumericProc(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm)
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

void StringIsAlphabeticProc(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm)
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

void TypeOfProc(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm)
{
    Value *v = popValue(stack, proc, w, vm);

    // We use a gcMonitor value of false since we don't want the GC
    // to collect class objects
    stack.push(vm->GetAllocator().newObject(v->type, BuiltInTypes::ClassType, false));
}

void TypeFromIdProc(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm)
{
    QString typeId = popString(stack, proc, w, vm);
    Value *type = vm->GetType(typeId);
    stack.push(type);
}

void AddressOfProc(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm)
{
    Value *v = popValue(stack, proc, w, vm);
    ffi_type *type;
    kalimat_to_ffi_type(v->type, type, vm);
    void *ptr = malloc(type->size);
    kalimat_to_ffi_value(v->type, v, type, ptr, vm);
    //todo:
    stack.push(vm->GetAllocator().newRaw(ptr, new PointerClass(v->type)));
}

void NewMapProc(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm)
{
    stack.push(vm->GetAllocator().newMap());
}

void HasKeyProc(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm)
{
    w->typeCheck(proc, stack.top(), BuiltInTypes::MapType);
    Value *v = popValue(stack, proc, w, vm);

    Value *key = popValue(stack, proc, w, vm);

    VMap *m = v->unboxMap();
    VMError err;
    if(!m->keyCheck(key, err))
        throw err;
    bool result = (m->get(key) != NULL);
    stack.push(vm->GetAllocator().newBool(result));
}

void KeysOfProc(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm)
{
    w->typeCheck(proc, stack.top(), BuiltInTypes::MapType);
    Value *v = popValue(stack, proc, w, vm);

    VMap *m = v->unboxMap();
    Value *k = vm->GetAllocator().newArray(m->allKeys.count());
    for(int i=0; i<m->allKeys.count(); i++)
        k->unboxArray()->Elements[i] = m->allKeys.at(i);
    stack.push(k);
}

struct FileBlob
{
    QFile *file;
    QTextStream *stream;
    ~FileBlob() { file->close(); delete file; delete stream;}
};

// TODO: use the helpers popXXX(...) functions instead of manually calling
// typecheck() and pop() in all external methods.
FileBlob *popFileBlob(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm)
{
    verifyStackNotEmpty(stack, proc, vm);
    w->typeCheck(proc, stack.top(), BuiltInTypes::FileType);
    IObject *ob = stack.pop()->unboxObj();
    Value *rawFile = ob->getSlotValue("file_handle");
    w->typeCheck(proc, rawFile, BuiltInTypes::RawFileType);
    void *fileObj = rawFile->unboxRaw();
    FileBlob *f = (FileBlob *) fileObj;
    return f;
}

Value *popValue(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm)
{
    verifyStackNotEmpty(stack, proc, vm);
    Value *v = stack.pop();
    return v;
}

QString popString(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm)
{
    verifyStackNotEmpty(stack, proc, vm);
    w->typeCheck(proc, stack.top(), BuiltInTypes::StringType);
    QString s = stack.pop()->unboxStr();
    return s;
}

int popInt(Stack<Value *> &stack, Process *proc,  RunWindow *w, VM *vm)
{
    verifyStackNotEmpty(stack, proc, vm);
    w->typeCheck(proc, stack.top(), BuiltInTypes::IntType);
    int i = stack.pop()->unboxInt();
    return i;
}

void *popRaw(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm, IClass *type)
{
    verifyStackNotEmpty(stack, proc, vm);
    w->typeCheck(proc, stack.top(), type);
    void *ret = stack.pop()->unboxRaw();
    return ret;
}

bool popBool(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm)
{
    verifyStackNotEmpty(stack, proc, vm);
    w->typeCheck(proc, stack.top(), BuiltInTypes::BoolType);
    bool b = stack.pop()->unboxBool();
    return b;
}

VArray *popArray(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm)
{
    verifyStackNotEmpty(stack, proc, vm);
    w->typeCheck(proc, stack.top(), BuiltInTypes::ArrayType);
    VArray *arr = stack.pop()->unboxArray();
    return arr;
}

void DoFileWrite(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm, bool newLine)
{
    FileBlob *f = popFileBlob(stack, proc, w, vm);
    QString s = popString(stack, proc, w, vm);

    if(f->file == NULL)
        w->assert(proc, false, ArgumentError, VM::argumentErrors[ArgErr::CannotWriteToClosedFile]);
    if(newLine)
        *(f->stream) << s << endl;
    else
        *(f->stream) << s;
}

void FileWriteProc(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm)
{
    DoFileWrite(stack, proc, w, vm, false);
}

void FileWriteUsingWidthProc(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm)
{
    FileBlob *f = popFileBlob(stack, proc, w, vm);
    if(f->file == NULL)
        w->assert(proc, false, ArgumentError,  VM::argumentErrors[ArgErr::CannotWriteToClosedFile]);
    QString s = popString(stack, proc, w, vm);
    int width = popInt(stack, proc, w, vm);

    QString s2 = w->textLayer.formatStringUsingWidth(s, width);
    *(f->stream) << s2;
}

void FileWriteLineProc(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm)
{
    DoFileWrite(stack, proc, w, vm, true);
}

void FileReadLineProc(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm)
{

    FileBlob *f = popFileBlob(stack, proc, w, vm);
    if(f->file == NULL)
        w->assert(proc, false, ArgumentError,  VM::argumentErrors[ArgErr::CannotReadFromClosedFile]);
    QString s = f->stream->readLine();
    Value *v = vm->GetAllocator().newString(s);
    stack.push(v);
}

void FileEofProc(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm)
{
    FileBlob *f = popFileBlob(stack, proc, w, vm);
    if(f->file == NULL)
        w->assert(proc, false, ArgumentError, VM::argumentErrors[ArgErr::CannotReadFromClosedFile]);
    bool ret = f->stream->atEnd();
    Value *v = vm->GetAllocator().newBool(ret);
    stack.push(v);
}

void FileOpenProc(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm)
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
    v->v.objVal->setSlotValue("file_handle", vm->GetAllocator().newRaw(blob, BuiltInTypes::RawFileType));
    stack.push(v);
}

void FileCreateProc(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm)
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
    v->v.objVal->setSlotValue("file_handle", vm->GetAllocator().newRaw(blob, BuiltInTypes::RawFileType));
    stack.push(v);
}

void FileAppendProc(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm)
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
    v->v.objVal->setSlotValue("file_handle", vm->GetAllocator().newRaw(blob, BuiltInTypes::RawFileType));
    stack.push(v);
}

void FileCloseProc(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm)
{
    // TODO: use popFileblob
    w->typeCheck(proc, stack.top(), BuiltInTypes::FileType);
    IObject *ob = stack.pop()->unboxObj();
    Value *rawFile = ob->getSlotValue("file_handle");
    w->typeCheck(proc, rawFile, BuiltInTypes::RawFileType);
    void *fileObj = rawFile->unboxRaw();
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
void EditProc(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm)
{
    Value *v = stack.pop();
    w->assert(proc, v->tag == ObjectVal, ArgumentError, VM::argumentErrors.get(ArgErr::SentValueHasToBeAnObject1, v->toString()));
    v = editAndReturn(v, w, vm);
    stack.push(v);
}

void GetMainWindowProc(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm)
{
    stack.push(vm->GetAllocator().newQObject(w));
}

void NewChannelProc(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm)
{
    stack.push(vm->GetAllocator().newChannel());
}

void LoadLibraryProc(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm)
{
    QString libName = popString(stack, proc, w, vm);
    // todo: will this leak?
    QLibrary *lib = new QLibrary(libName);
    if(!lib->load())
        proc->signal(InternalError1, QString("Failed to load library '%1'").arg(libName));

    Value *ret = vm->GetAllocator().newRaw(lib, BuiltInTypes::ExternalLibrary);
    stack.push(ret);
}

void GetProcAddressProc(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm)
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

void InvokeForeignProc(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm)
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

    IClass *retType = (IClass *) stack.pop()->unboxObj();
    QVector<Value *> argz;
    QVector<IClass *> kargTypes;
    for(int i=0; i<args->count(); i++)
    {
        argz.append(args->Elements[i]);
        if(!guessArgTypes)
        {
            IClass *type = dynamic_cast<IClass *>(argTypes->Elements[i]->unboxObj());
            if(!type)
            {
                proc->signal(TypeError2, BuiltInTypes::ClassType->toString(), argTypes->Elements[i]->type->toString());
            }
            kargTypes.append(type);
        }
    }

    Value *ret = CallForeign(funcPtr, argz, retType, kargTypes, guessArgTypes, vm);
    if(ret)
        stack.push(ret);
    else
        stack.push(vm->GetAllocator().null());
}

void CurrentParseTreeProc(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm)
{
    Value *v = vm->GetGlobal("%parseTree");
    stack.push(v);
}

void MakeParserProc(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm)
{
    QString datum = popString(stack, proc, w, vm);
    IClass *parserClass = dynamic_cast<IClass *>(vm->GetType(VMId::get(RId::Parser))->unboxObj());
    IObject *parser = parserClass->newValue(&vm->GetAllocator());
    parser->setSlotValue(VMId::get(RId::InputPos), vm->GetAllocator().newInt(0));
    parser->setSlotValue(VMId::get(RId::Data), vm->GetAllocator().newString(
                             datum));
    stack.push(vm->GetAllocator().newObject(parser, parserClass));
}

void PushParserBacktrackPointProc(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm)
{
    IObject *receiver = popValue(stack, proc, w, vm)->unboxObj();
    int arg1 = popInt(stack, proc, w, vm);
    ParserObj *parser = dynamic_cast<ParserObj *>(receiver);
    parser->stack.push(ParseFrame(arg1, parser->pos, true));
}

void IgnoreParserBacktrackPointProc(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm)
{

    IObject *receiver = popValue(stack, proc, w, vm)->unboxObj();
    ParserObj *parser = dynamic_cast<ParserObj *>(receiver);
    ParseFrame f = parser->stack.pop();
    if(!f.backTrack)
        w->assert(proc, false, InternalError1,
                  VM::argumentErrors.get(ArgErr::StackTopNotBacktrackPointToIgnore1, str(f.continuationLabel)));
}

void ActivationFrameProc(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm)
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

void MigrateToGuiThreadProc(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm)
{
    //proc->migrateTo(&vm->guiScheduler);
    //emit w->EmitGuiSchedule();
    proc->wannaMigrateTo = &vm->guiScheduler;
    proc->timeSlice = 0;
}

void MigrateBackFromGuiThreadProc(Stack<Value *> &stack, Process *proc, RunWindow *w, VM *vm)
{
    //proc->migrateTo(&vm->mainScheduler);
    proc->wannaMigrateTo = &vm->mainScheduler;
    proc->timeSlice = 0;
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
    switch(v->tag)
    {
    case Int:
    case Double:
    case Long:
    case StringVal:
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
        break;
    case Boolean:
        cb = new QCheckBox();
        cb->setChecked(v->unboxBool());
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
        break;
    case ObjectVal:
        qf = new QGroupBox(label);
        layout->addWidget(qf, row, 0, 1, 2);
        obj = dynamic_cast<Object *>(v->unboxObj());
        if(obj == NULL)
            break;
        vb = new QGridLayout();
        subRow = 0;
        for(QVector<QString>::iterator i = obj->slotNames.begin(); i!= obj->slotNames.end(); ++i)
        {
            setupChildren(vb, obj->_slots[*i], new FieldReference(obj, *i), *i, subRow++,vm);
        }
        qf->setLayout(vb);

        break;
    case ArrayVal:
        sa = new QScrollArea();
        vb = new QGridLayout();
        layout->addWidget(sa, row, 0, 1, 2);
        arr = v->unboxArray();
        vb->addWidget(new QLabel(label), 0, 0, 1, 2);
        for(int i=0; i<arr->count(); i++)
        {
            setupChildren(vb, arr->Elements[i], new ArrayReference(arr, i),QString("%1").arg(i+1), i+1, vm);
        }
        sa->setLayout(vb);
        sa->adjustSize();
        break;
    case NullVal:
    case RawVal:
    case RefVal:
    case MultiDimensionalArrayVal:
    case ChannelVal:
    case QObjectVal:
    case MapVal:
        break;
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
    dlg->setLayoutDirection(Qt::RightToLeft);
    dlg->connect(ok, SIGNAL(clicked()), dlg, SLOT(accept()));
    dlg->connect(cancel, SIGNAL(clicked()), dlg, SLOT(reject()));
    bool ret = dlg->exec()== QDialog::Accepted;
    v = vm->GetAllocator().newBool(ret);

    return v;
}
