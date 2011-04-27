/**************************************************************************
**   The Kalimat programming language
**   Copyright 2010 Mohamed Samy Ali - samy2004@gmail.com
**   This project is released under the Apache License version 2.0
**   as described in the included license.txt file
**************************************************************************/


#include "runwindow.h"
#include "builtinmethods.h"
#include "guieditwidgethandler.h"

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

using namespace std;

void PrintProc(QStack<Value *> &stack, RunWindow *w, VM *vm)
{
    Value *v = stack.pop();
    QString str = v->toString();
    w->textLayer.print(str);
}

void PushReadChanProc(QStack<Value *> &stack, RunWindow *w, VM *vm)
{
    stack.push(w->readChannel);
}

WindowReadMethod::WindowReadMethod(RunWindow *parent, VM *vm)
{
    this->parent = parent;
    this->vm = vm;
    this->readNum = false;
}

void WindowReadMethod::operator ()(QStack<Value *> &operandStack)
{
    readNum = popInt(operandStack, parent, vm);
    parent->beginInput();
    parent->update(); // We must do this, because normal updating is done
                      // by calling redrawWindow() in the instruction loop, and
                      // here we suspend the instruction loop...
    this->operandStack = &operandStack;
}

void WindowReadMethod::SetReadValue(Value *v)
{
    operandStack->push(v);
}

WindowProxyMethod::WindowProxyMethod(RunWindow *parent, VM *vm, VM_PROC proc)
{
    this->vm = vm;
    this->parent = parent;
    this->proc = proc;
}

void WindowProxyMethod::operator ()(QStack<Value *> &operandStack)
{
    proc(operandStack, parent, vm);
}

void SetCursorPosProc(QStack<Value *> &stack, RunWindow *w, VM *vm)
{
    int line = popInt(stack, w, vm);
    int col = popInt(stack, w, vm);
    bool result = w->textLayer.setCursorPos(line, col);
    w->assert(result, ArgumentError, QString::fromStdWString(L"قيم غير صحيحة لتحديد موقع المؤشر"));
}
void GetCursorRowProc(QStack<Value *> &stack, RunWindow *w, VM *vm)
{
    int r = w->textLayer.getCursorRow();
    stack.push(vm->GetAllocator().newInt(r));
}
void GetCursorColProc(QStack<Value *> &stack, RunWindow *w, VM *vm)
{
    int c = w->textLayer.getCursorCol();
    stack.push(vm->GetAllocator().newInt(c));
}


void PrintUsingWidthProc(QStack<Value *> &stack, RunWindow *w, VM *vm)
{
    Value *v = stack.pop();
    w->typeCheck(stack.top(), BuiltInTypes::IntType);
    int wid = stack.pop()->unboxInt();
    QString str = v->toString();
    w->textLayer.print(str, wid);
}

void DrawPixelProc(QStack<Value *> &stack, RunWindow *w, VM *vm)
{
    int x = popIntOrCoercable(stack, w, vm);
    int y = popIntOrCoercable(stack, w, vm);
    int color = popInt(stack, w, vm);
    if(color == -1)
        color = 0;
    QColor clr = w->paintSurface->GetColor(color);
    w->paintSurface->TX(x);
    QPainter p(w->paintSurface->GetImage());
    p.fillRect(x, y, 1, 1, clr);
    w->redrawWindow();
}

void DrawLineProc(QStack<Value *> &stack, RunWindow *w, VM *vm)
{
    int x1 = popIntOrCoercable(stack, w, vm);
    int y1 = popIntOrCoercable(stack, w, vm);
    int x2 = popIntOrCoercable(stack, w, vm);
    int y2 = popIntOrCoercable(stack, w, vm);

    w->paintSurface->TX(x1);
    w->paintSurface->TX(x2);
    int color = popInt(stack, w, vm);
    if(color ==-1)
        color = 0;

    QColor clr = w->paintSurface->GetColor(color);
    QPainter p(w->paintSurface->GetImage());

    QColor oldcolor = p.pen().color();
    QPen pen = p.pen();
    pen.setColor(clr);
    p.setPen(pen);
    p.drawLine(x1, y1, x2, y2);
    pen.setColor(oldcolor);
    p.setPen(pen);

    w->redrawWindow();
}

void DrawRectProc(QStack<Value *> &stack, RunWindow *w, VM *vm)
{
    int x1 = popIntOrCoercable(stack, w, vm);
    int y1 = popIntOrCoercable(stack, w, vm);
    int x2 = popIntOrCoercable(stack, w, vm);
    int y2 = popIntOrCoercable(stack, w, vm);

    w->paintSurface->TX(x1);
    w->paintSurface->TX(x2);
    int color = popInt(stack, w, vm);
    bool filled = popBool(stack, w, vm);

    if(color ==-1)
        color = 0;

    QPainter p(w->paintSurface->GetImage());

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

void DrawCircleProc(QStack<Value *> &stack, RunWindow *w, VM *vm)
{
    int cx = popIntOrCoercable(stack, w, vm);
    int cy = popIntOrCoercable(stack, w, vm);

    int radius = popIntOrCoercable(stack, w, vm);

    int color = popInt(stack, w, vm);

    bool filled = popBool(stack, w, vm);
    w->paintSurface->TX(cx);
    QPainter p(w->paintSurface->GetImage());
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

void RandomProc(QStack<Value *> &stack, RunWindow *w, VM *vm)
{
    int max = popIntOrCoercable(stack, w, vm);
    int ret = rand()%max;
    stack.push(vm->GetAllocator().newInt(ret));
}

Value *ConvertStringToNumber(QString str, VM *vm)
{
    bool ok;
    QLocale loc(QLocale::Arabic, QLocale::Egypt);
    int i = loc.toLongLong(str, &ok,10);
    if(ok)
    {
       return vm->GetAllocator().newInt(i);
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

void ToNumProc(QStack<Value *> &stack, RunWindow *w, VM *vm)
{

    QString *str = popString(stack, w, vm);
    Value * v = ConvertStringToNumber(*str, vm);

    if(v != NULL)
        stack.push(v);
    else
    {
      vm->signal(TypeError2, QString::fromStdWString(L"عدد"), v->type->getName());
    }

}

void ConcatProc(QStack<Value *> &stack, RunWindow *w, VM *vm)
{
    QString *str1 = popString(stack, w, vm);
    QString *str2 = popString(stack, w, vm);


    QString *ret = new QString((*str1)+(*str2));
    stack.push(vm->GetAllocator().newString(ret));

}

void StrLenProc(QStack<Value *> &stack, RunWindow *w, VM *vm)
{
    QString *str = popString(stack, w, vm);

    int ret = str->length();
    stack.push(vm->GetAllocator().newInt(ret));
}

void StrFirstProc(QStack<Value *> &stack, RunWindow *w, VM *vm)
{
    int n = popInt(stack, w, vm);
    QString *str = popString(stack, w, vm);

    QString *ret = new QString(str->left(n));
    stack.push(vm->GetAllocator().newString(ret));
}

void StrLastProc(QStack<Value *> &stack, RunWindow *w,VM *vm)
{
    int n = popInt(stack, w, vm);

    QString *str = popString(stack, w, vm);

    QString *ret = new QString(str->right(n));
    stack.push(vm->GetAllocator().newString(ret));
}

void StrMidProc(QStack<Value *> &stack, RunWindow *w, VM *vm)
{
    QString *str = popString(stack, w, vm);


    int i = popInt(stack, w, vm);

    int n = popInt(stack, w, vm);

    // We make indexing one-based instead of QT's zero-based
    // todo: range checking in StrMidProc()
    QString *ret = new QString(str->mid(i -1 ,n));
    stack.push(vm->GetAllocator().newString(ret));
}

void StrBeginsWithProc(QStack<Value *> &stack, RunWindow *w, VM *vm)
{
    QString *strMain = popString(stack, w, vm);
    QString *strSub = popString(stack, w, vm);

    bool ret = strMain->startsWith(*strSub, Qt::CaseSensitive);
    stack.push(vm->GetAllocator().newBool(ret));
}

void StrEndsWithProc(QStack<Value *> &stack, RunWindow *w, VM *vm)
{
    QString *strMain = popString(stack, w, vm);
    QString *strSub = popString(stack, w, vm);

    bool ret = strMain->endsWith(*strSub, Qt::CaseSensitive);
    stack.push(vm->GetAllocator().newBool(ret));
}

void StrContainsProc(QStack<Value *> &stack, RunWindow *w, VM *vm)
{
    QString *strMain = popString(stack, w, vm);
    QString *strSub = popString(stack, w, vm);

    bool ret = strMain->contains(*strSub, Qt::CaseSensitive);
    stack.push(vm->GetAllocator().newBool(ret));
}

void StrSplitProc(QStack<Value *> &stack, RunWindow *w, VM *vm)
{
    QString *str = popString(stack, w, vm);
    QString *separator = popString(stack, w, vm);

    QStringList result = str->split(*separator, QString::KeepEmptyParts);
    Value *ret = vm->GetAllocator().newArray(result.count());
    for(int i=0; i<result.count(); i++)
    {
        ret->v.arrayVal->Elements[i] = vm->GetAllocator().newString(new QString(result[i]));
    }
    stack.push(ret);
}

void StrTrimProc(QStack<Value *> &stack, RunWindow *w, VM *vm)
{
    QString *str = popString(stack, w, vm);
    QString *str2 = new QString(str->trimmed());
    Value *ret = vm->GetAllocator().newString(str2);
    stack.push(ret);
}

void ToStringProc(QStack<Value *> &stack, RunWindow *, VM *vm)
{
    verifyStackNotEmpty(stack, vm);
    Value *v = stack.pop();
    QString ret;
    switch(v->tag)
    {
    case Int:
        ret = QString("%1").arg(v->unboxInt());
        break;
    case Double:
        ret = QString("%1").arg(v->unboxDouble());
        break;
    case StringVal:
        ret = QString(*v->unboxStr());
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
    stack.push(vm->GetAllocator().newString(new QString(ret)));
}

void RoundProc(QStack<Value *> &stack, RunWindow *w, VM *vm)
{
    double d = popDoubleOrCoercable(stack, w, vm);
    int i = (int) d;
    stack.push(vm->GetAllocator().newInt(i));

}

void RemainderProc(QStack<Value *> &stack, RunWindow *w, VM *vm)
{
    int n1 = popInt(stack, w, vm);
    int n2 = popInt(stack, w, vm);

    if(n2 == 0)
        vm->signal(DivisionByZero);
    int i = n1 % n2;
    stack.push(vm->GetAllocator().newInt(i));
}

int popIntOrCoercable(QStack<Value *> &stack, RunWindow *w, VM *vm)
{
    if(stack.empty())
    {
        vm->signal(InternalError);
    }
    Value *v = stack.pop();
    if(v->tag != Int && v->tag != Double)
    {
        w->typeError(BuiltInTypes::NumericType, v->type);
    }
    if(v->tag == Double)
        v = vm->GetAllocator().newInt((int) v->unboxDouble());
    return v->unboxInt();
}

double popDoubleOrCoercable(QStack<Value *> &stack, RunWindow *w, VM *vm)
{
    if(stack.empty())
    {
        vm->signal(InternalError);
    }
    Value *v = stack.pop();
    if(v->tag != Int && v->tag != Double)
    {
        w->typeError(BuiltInTypes::NumericType, v->type);
    }
    if(v->tag == Int)
        v = vm->GetAllocator().newDouble(v->unboxInt());
    return v->unboxDouble();
}

void verifyStackNotEmpty(QStack<Value *> &stack, VM *vm)
{
    if(stack.empty())
    {
        vm->signal(InternalError);
    }
}

void SinProc(QStack<Value *> &stack, RunWindow *w, VM *vm)
{

    double theta = popDoubleOrCoercable(stack, w, vm);

    double result = sin(theta);
    stack.push(vm->GetAllocator().newDouble(result));

}
void CosProc(QStack<Value *> &stack, RunWindow *w, VM *vm)
{
    double theta = popDoubleOrCoercable(stack, w, vm);

    double result = cos(theta);
    stack.push(vm->GetAllocator().newDouble(result));

}
void TanProc(QStack<Value *> &stack, RunWindow *w, VM *vm)
{
    double theta = popDoubleOrCoercable(stack, w, vm);

    double result = tan(theta);
    stack.push(vm->GetAllocator().newDouble(result));

}
void ASinProc(QStack<Value *> &stack, RunWindow *w, VM *vm)
{

    double theta = popDoubleOrCoercable(stack, w, vm);

    double result = asin(theta);
    stack.push(vm->GetAllocator().newDouble(result));

}
void ACosProc(QStack<Value *> &stack, RunWindow *w, VM *vm)
{
    double theta = popDoubleOrCoercable(stack, w, vm);

    double result = acos(theta);
    stack.push(vm->GetAllocator().newDouble(result));

}
void ATanProc(QStack<Value *> &stack, RunWindow *w, VM *vm)
{
    double theta = popDoubleOrCoercable(stack, w, vm);

    double result = atan(theta);
    stack.push(vm->GetAllocator().newDouble(result));

}
void SqrtProc(QStack<Value *> &stack, RunWindow *w, VM *vm)
{
    double theta = popDoubleOrCoercable(stack, w, vm);
    double result = sqrt(theta);
    stack.push(vm->GetAllocator().newDouble(result));

}
void Log10Proc(QStack<Value *> &stack, RunWindow *w, VM *vm)
{
    double theta = popDoubleOrCoercable(stack, w, vm);
    double result = log10(theta);
    stack.push(vm->GetAllocator().newDouble(result));

}
void LnProc(QStack<Value *> &stack, RunWindow *w, VM *vm)
{
    double theta = popDoubleOrCoercable(stack, w, vm);
    double result = log(theta);
    stack.push(vm->GetAllocator().newDouble(result));

}

void LoadSpriteProc(QStack<Value *> &stack, RunWindow *w, VM *vm)
{
    verifyStackNotEmpty(stack, vm);
    w->typeCheck(stack.top(), BuiltInTypes::StringType);
    QString *fname = stack.pop()->unboxStr();
    *fname = w->ensureCompletePath(*fname);

    if(!QFile::exists(*fname))
    {
        w->assert(false, ArgumentError, QString::fromStdWString(L"تحميل طيف من ملف غير موجود"));
    }
    Sprite *sprite = new Sprite(*fname);
    w->spriteLayer.AddSprite(sprite);
    stack.push(vm->GetAllocator().newRaw(sprite, BuiltInTypes::SpriteType));
}
void DrawSpriteProc(QStack<Value *> &stack, RunWindow *w, VM *vm)
{

    w->typeCheck(stack.top(), BuiltInTypes::SpriteType);
    void *rawVal = stack.pop()->unboxRaw();
    Sprite  *sprite = (Sprite *) rawVal;

    int x = popIntOrCoercable(stack, w , vm);

    int y = popIntOrCoercable(stack, w , vm);

    w->paintSurface->TX(x);
    x-= sprite->image.width();
    sprite->location = QPoint(x,y);
    sprite->visible = true;
    w->spriteLayer.showSprite(sprite);
    w->checkCollision(sprite);
    w->redrawWindow();
}
void HideSpriteProc(QStack<Value *> &stack, RunWindow *w, VM *vm)
{
    w->typeCheck(stack.top(), BuiltInTypes::SpriteType);
    void *rawVal = stack.pop()->unboxRaw();
    Sprite  *sprite = (Sprite *) rawVal;

    sprite->visible = false;
    w->spriteLayer.hideSprite(sprite);
    w->redrawWindow();
}
void GetSpriteLeftProc(QStack<Value *> &stack, RunWindow *w, VM *vm)
{
    w->typeCheck(stack.top(), BuiltInTypes::SpriteType);
    void *rawVal = stack.pop()->unboxRaw();
    Sprite  *sprite = (Sprite *) rawVal;

    int ret = sprite->boundingRect().left();
    w->paintSurface->TX(ret);
    stack.push(vm->GetAllocator().newInt(ret));
}

void GetSpriteRightProc(QStack<Value *> &stack, RunWindow *w, VM *vm)
{
    w->typeCheck(stack.top(), BuiltInTypes::SpriteType);
    void *rawVal = stack.pop()->unboxRaw();
    Sprite  *sprite = (Sprite *) rawVal;

    int ret = sprite->boundingRect().right();
    w->paintSurface->TX(ret);
    stack.push(vm->GetAllocator().newInt(ret));
}

void GetSpriteTopProc(QStack<Value *> &stack, RunWindow *w, VM *vm)
{
    w->typeCheck(stack.top(), BuiltInTypes::SpriteType);
    void *rawVal = stack.pop()->unboxRaw();
    Sprite  *sprite = (Sprite *) rawVal;

    int ret = sprite->boundingRect().top();
    stack.push(vm->GetAllocator().newInt(ret));
}

void GetSpriteBottomProc(QStack<Value *> &stack, RunWindow *w, VM *vm)
{
    w->typeCheck(stack.top(), BuiltInTypes::SpriteType);
    void *rawVal = stack.pop()->unboxRaw();
    Sprite  *sprite = (Sprite *) rawVal;

    int ret = sprite->boundingRect().bottom();
    stack.push(vm->GetAllocator().newInt(ret));
}

void GetSpriteWidthProc(QStack<Value *> &stack, RunWindow *w, VM *vm)
{
    w->typeCheck(stack.top(), BuiltInTypes::SpriteType);
    void *rawVal = stack.pop()->unboxRaw();
    Sprite  *sprite = (Sprite *) rawVal;

    int ret = sprite->boundingRect().width();
    stack.push(vm->GetAllocator().newInt(ret));
}

void GetSpriteHeightProc(QStack<Value *> &stack, RunWindow *w, VM *vm)
{
    w->typeCheck(stack.top(), BuiltInTypes::SpriteType);
    void *rawVal = stack.pop()->unboxRaw();
    Sprite  *sprite = (Sprite *) rawVal;

    int ret = sprite->boundingRect().height();
    stack.push(vm->GetAllocator().newInt(ret));
}

void ShowSpriteProc(QStack<Value *> &stack, RunWindow *w, VM *vm)
{
    w->typeCheck(stack.top(), BuiltInTypes::SpriteType);
    void *rawVal = stack.pop()->unboxRaw();
    Sprite  *sprite = (Sprite *) rawVal;

    sprite->visible = true;
    w->spriteLayer.showSprite(sprite);
    w->checkCollision(sprite);
    w->redrawWindow();
}
void WaitProc(QStack<Value *> &stack, RunWindow *w, VM *vm)
{
    int ms = stack.pop()->unboxInt();
    w->suspend();
    int cookie = w->startTimer(ms);
    w->setAsleep(cookie);
    stack.push(vm->GetAllocator().newInt(cookie));
}

void CheckAsleepProc(QStack<Value *> &stack, RunWindow *w, VM *vm)
{
    qApp->processEvents();
    int cookie = popInt(stack, w, vm);
    bool res = w->isAsleep(cookie);
    stack.push(vm->GetAllocator().newBool(res));
}

void ZoomProc(QStack<Value *> &stack, RunWindow *w, VM *vm)
{
    int x1 = popIntOrCoercable(stack, w , vm);
    int y1 = popIntOrCoercable(stack, w , vm);
    int x2 = popIntOrCoercable(stack, w , vm);
    int y2 = popIntOrCoercable(stack, w , vm);
    //w->TX(x1);
    //w->TX(x2);
    w->paintSurface->zoom(x1, y1, x2, y2);
}

void ClsProc(QStack<Value *> &stack, RunWindow *w, VM *vm)
{
    w->cls();
}

void ClearTextProc(QStack<Value *> &stack, RunWindow *w, VM *vm)
{
    w->clearAllText();
}

void SetTextColorProc(QStack<Value *> &stack, RunWindow *w, VM *vm)
{
    int color = popInt(stack, w, vm);
    w->assert(color>=0 && color <=15, ArgumentError, "Color value must be from 0 to 15");
    w->paintSurface->setTextColor(w->paintSurface->GetColor(color));
    w->redrawWindow();
}


void BuiltInConstantProc(QStack<Value *> &stack, RunWindow *w, VM *vm)
{
    QString *constName = popString(stack, w, vm);
    if(constName->operator ==(QString::fromStdWString(L"سطر.جديد")))
    {
        stack.push(vm->GetAllocator().newString(new QString("\n")));
        return;
    }
    w->assert(false, ArgumentError, QString::fromStdWString(L"لا يوجد ثابت بهذا الاسم"));
}
void StringIsNumericProc(QStack<Value *> &stack, RunWindow *w, VM *vm)
{
    QString *s = popString(stack, w, vm);
    bool yep = true;
    for(int i=0; i<s->length(); i++)
    {
        QChar c = s->operator [](i);
        if(!c.isDigit())
            yep = false;
    }
    stack.push(vm->GetAllocator().newBool(yep));
}

void StringIsAlphabeticProc(QStack<Value *> &stack, RunWindow *w, VM *vm)
{
    QString *s = popString(stack, w, vm);
    bool yep = true;
    for(int i=0; i<s->length(); i++)
    {
        QChar c = s->operator [](i);
        if(!c.isLetter())
            yep = false;
    }
    stack.push(vm->GetAllocator().newBool(yep));
}

void TypeOfProc(QStack<Value *> &stack, RunWindow *w, VM *vm)
{
    Value *v = popValue(stack, w, vm);

    // We use a gcMonitor value of false since we don't want the GC
    // to collect class objects
    stack.push(vm->GetAllocator().newObject(v->type, BuiltInTypes::ClassType, false));
}

struct FileBlob
{
    QFile *file;
    QTextStream *stream;
    ~FileBlob() { file->close(); delete file; delete stream;}
};

// TODO: use the helpers popXXX(...) functions instead of manually calling
// typecheck() and pop() in all external methods.
FileBlob *popFileBlob(QStack<Value *> &stack, RunWindow *w, VM *vm)
{
    verifyStackNotEmpty(stack, vm);
    w->typeCheck(stack.top(), BuiltInTypes::FileType);
    IObject *ob = stack.pop()->unboxObj();
    Value *rawFile = ob->getSlotValue("file_handle");
    w->typeCheck(rawFile, BuiltInTypes::RawFileType);
    void *fileObj = rawFile->unboxRaw();
    FileBlob *f = (FileBlob *) fileObj;
    return f;
}

Value *popValue(QStack<Value *> &stack, RunWindow *w, VM *vm)
{
    verifyStackNotEmpty(stack, vm);
    Value *v = stack.pop();
    return v;
}

QString *popString(QStack<Value *> &stack, RunWindow *w, VM *vm)
{
    verifyStackNotEmpty(stack, vm);
    w->typeCheck(stack.top(), BuiltInTypes::StringType);
    QString *s = stack.pop()->unboxStr();
    return s;
}
int popInt(QStack<Value *> &stack, RunWindow *w, VM *vm)
{
    verifyStackNotEmpty(stack, vm);
    w->typeCheck(stack.top(), BuiltInTypes::IntType);
    int i = stack.pop()->unboxInt();
    return i;
}

bool popBool(QStack<Value *> &stack, RunWindow *w, VM *vm)
{
    verifyStackNotEmpty(stack, vm);
    w->typeCheck(stack.top(), BuiltInTypes::BoolType);
    bool b = stack.pop()->unboxBool();
    return b;
}

void DoFileWrite(QStack<Value *> &stack, RunWindow *w, VM *vm, bool newLine)
{
    FileBlob *f = popFileBlob(stack, w, vm);
    QString *s = popString(stack, w, vm);

    if(f->file == NULL)
        w->assert(false, ArgumentError, QString::fromStdWString(L"لا يمكن الكتابة في ملف مغلق"));
    if(newLine)
        *(f->stream) << *s << endl;
    else
        *(f->stream) << *s;
}

void FileWriteProc(QStack<Value *> &stack, RunWindow *w, VM *vm)
{
    DoFileWrite(stack, w, vm, false);
}

void FileWriteUsingWidthProc(QStack<Value *> &stack, RunWindow *w, VM *vm)
{
    FileBlob *f = popFileBlob(stack, w, vm);
    if(f->file == NULL)
        w->assert(false, ArgumentError, QString::fromStdWString(L"لا يمكن الكتابة في ملف مغلق"));
    QString *s = popString(stack, w, vm);
    int width = popInt(stack, w, vm);

    QString s2 = w->textLayer.formatStringUsingWidth(*s, width);
    *(f->stream) << s2;
}

void FileWriteLineProc(QStack<Value *> &stack, RunWindow *w, VM *vm)
{
    DoFileWrite(stack, w, vm, true);
}

void FileReadLineProc(QStack<Value *> &stack, RunWindow *w, VM *vm)
{

    FileBlob *f = popFileBlob(stack, w, vm);
    if(f->file == NULL)
        w->assert(false, ArgumentError, QString::fromStdWString(L"لا يمكن القراءة من ملف مغلق"));
    QString *s = new QString(f->stream->readLine());
    Value *v = vm->GetAllocator().newString(s);
    stack.push(v);
}

void FileEofProc(QStack<Value *> &stack, RunWindow *w, VM *vm)
{
    FileBlob *f = popFileBlob(stack, w, vm);
    if(f->file == NULL)
        w->assert(false, ArgumentError, QString::fromStdWString(L"لا يمكن التعامل مع ملف مغلق"));
    bool ret = f->stream->atEnd();
    Value *v = vm->GetAllocator().newBool(ret);
    stack.push(v);
}

void FileOpenProc(QStack<Value *> &stack, RunWindow *w, VM *vm)
{
    QString *fname = popString(stack, w, vm);
    *fname = w->ensureCompletePath(*fname);
    w->assert(QFile::exists(*fname), ArgumentError, QString::fromStdWString(L"محاولة فتح ملف غير موجود"));
    QFile *f = new QFile(*fname);
    bool ret = f->open(QIODevice::ReadOnly | QIODevice::Text);
    w->assert(ret, RuntimeError, QString::fromStdWString(L"لم ينجح فتح الملف"));
    QTextStream *stream = new QTextStream(f);
    FileBlob *blob = new FileBlob();
    blob->file = f;
    blob->stream = stream;
    Value *v = vm->GetAllocator().newObject(BuiltInTypes::FileType);
    v->v.objVal->setSlotValue("file_handle", vm->GetAllocator().newRaw(blob, BuiltInTypes::RawFileType));
    stack.push(v);
}

void FileCreateProc(QStack<Value *> &stack, RunWindow *w, VM *vm)
{
    QString *fname = popString(stack, w, vm);
    *fname = w->ensureCompletePath(*fname);
    QFile *f = new QFile(*fname);
    bool ret = f->open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate);
    w->assert(ret, RuntimeError, QString::fromStdWString(L"لم ينجح فتح الملف"));
    QTextStream *stream = new QTextStream(f);
    FileBlob *blob = new FileBlob();
    blob->file = f;
    blob->stream = stream;
    Value *v = vm->GetAllocator().newObject(BuiltInTypes::FileType);
    v->v.objVal->setSlotValue("file_handle", vm->GetAllocator().newRaw(blob, BuiltInTypes::RawFileType));
    stack.push(v);
}

void FileAppendProc(QStack<Value *> &stack, RunWindow *w, VM *vm)
{
    QString *fname = popString(stack, w, vm);
    *fname = w->ensureCompletePath(*fname);
    QFile *f = new QFile(*fname);
    bool ret = f->open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append);
    w->assert(ret, ArgumentError, QString::fromStdWString(L"لم ينجح فتح الملف"));
    Value *v = vm->GetAllocator().newObject(BuiltInTypes::FileType);
    QTextStream *stream = new QTextStream(f);
    FileBlob *blob = new FileBlob();
    blob->file = f;
    blob->stream = stream;
    v->v.objVal->setSlotValue("file_handle", vm->GetAllocator().newRaw(blob, BuiltInTypes::RawFileType));
    stack.push(v);
}

void FileCloseProc(QStack<Value *> &stack, RunWindow *w, VM *vm)
{
    // TODO: use popFileblob
    w->typeCheck(stack.top(), BuiltInTypes::FileType);
    IObject *ob = stack.pop()->unboxObj();
    Value *rawFile = ob->getSlotValue("file_handle");
    w->typeCheck(rawFile, BuiltInTypes::RawFileType);
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
void EditProc(QStack<Value *> &stack, RunWindow *w, VM *vm)
{
    Value *v = stack.pop();
    w->assert(v->tag == ObjectVal, ArgumentError, QString::fromStdWString(L"القيمة المرسلة لابد أن تكون كائناً"));
    v = editAndReturn(v, w, vm);
    stack.push(v);
}

void GetMainWindowProc(QStack<Value *> &stack, RunWindow *w, VM *vm)
{
    stack.push(vm->GetAllocator().newQObject(w));
}

void NewChannelProc(QStack<Value *> &stack, RunWindow *w, VM *vm)
{
    stack.push(vm->GetAllocator().newChannel());
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
        for(int i=0; i<arr->count; i++)
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
        break;
    }
}

Value *editAndReturn(Value *v, RunWindow *w, VM *vm)
{
    QDialog *dlg = new QDialog(w);
    dlg->setWindowTitle(QString::fromStdWString(L"تحرير %1").arg(v->type->getName()));
    QVBoxLayout *ly = new QVBoxLayout(dlg);

    QFrame *frame = new QFrame();
    QGridLayout *gl = new QGridLayout();
    setupChildren(gl, v, NULL, "", 0, vm);
    frame->setLayout(gl);
    ly->addWidget(frame);

    QPushButton *ok = new QPushButton(QString::fromStdWString(L"حسناً"));
    QPushButton *cancel = new QPushButton(QString::fromStdWString(L"الغاء"));
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
