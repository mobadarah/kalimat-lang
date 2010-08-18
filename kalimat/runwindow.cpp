/**************************************************************************
**   The Kalimat programming language
**   Copyright (C) 2010 Mohamed Samy Ali - samy2004@gmail.com
**   This project is released under the Apache License version 2.0
**   as described in the included license.txt file
**************************************************************************/

#include "../smallvm/vm_incl.h"
#include "../smallvm/vm.h"
#include "../smallvm/externalmethod.h"

#include "sprite.h"
#include "runwindow.h"
#include "mainwindow.h"
#include "ui_runwindow.h"

#include "utils.h"

#include <QPainter>
#include <QKeyEvent>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <math.h>

#include <QWaitCondition>

RunWindow::RunWindow(QWidget *parent, QString pathOfProgramsFile) :
    QMainWindow(parent),
    updateTimer(20),
    ui(new Ui::RunWindow)
{
    ui->setupUi(this);
    this->pathOfProgramsFile = pathOfProgramsFile;
    QImage im(size(), QImage::Format_RGB32);
    im.fill(qRgb(255,255,255));
    image = im;
    state = rwNormal;
    textColor = Qt::black;
    clearText();
    printMethod = new WindowPrintMethod(this);
    readMethod = new WindowReadMethod(this);
    vm = NULL;
    this->setFixedSize(800, 600);
}

void RunWindow::closeEvent(QCloseEvent *ev)
{
   //*
    this->state = rwSuspended;
    if(vm != NULL)
        vm->gc();

   //*/
}
void RunWindow::parentDestroyed(QObject *)
{
    this->close();
}

RunWindow::~RunWindow()
{
    delete ui;
    if(vm != NULL)
    {
        vm->gc();
        delete vm;
    }
    delete printMethod;
    delete readMethod;
}
void RunWindow::Init(QString program, QMap<QString, QString>stringConstants)
{
    try
    {
        vm = new VM();
        vm->DefineStringConstant("new_line", "\n");
        vm->Register("print", printMethod);
        vm->Register("input", readMethod);

        vm->Register("setcursorpos",  new WindowProxyMethod(this, vm, SetCursorPosProc));
        vm->Register("getcursorrow",  new WindowProxyMethod(this, vm, GetCursorRowProc));
        vm->Register("getcursorcol",  new WindowProxyMethod(this, vm, GetCursorColProc));

        vm->Register("printw",  new WindowProxyMethod(this, vm, PrintUsingWidthProc));
        vm->Register("drawpixel", new WindowProxyMethod(this, vm, DrawPixelProc));
        vm->Register("drawline", new WindowProxyMethod(this, vm, DrawLineProc));
        vm->Register("drawrect", new WindowProxyMethod(this, vm, DrawRectProc));
        vm->Register("drawcircle", new WindowProxyMethod(this, vm, DrawCircleProc));
        vm->Register("drawsprite", new WindowProxyMethod(this, vm, DrawSpriteProc));
        vm->Register("zoom", new WindowProxyMethod(this, vm, ZoomProc));
        vm->Register("cls", new WindowProxyMethod(this, vm, ClsProc));
        vm->Register("cleartext", new WindowProxyMethod(this, vm, ClearTextProc));
        vm->Register("settextcolor", new WindowProxyMethod(this, vm, SetTextColorProc));

        vm->Register("random", new WindowProxyMethod(this, vm, RandomProc));
        vm->Register("sin", new WindowProxyMethod(this, vm, SinProc));
        vm->Register("cos", new WindowProxyMethod(this, vm, CosProc));
        vm->Register("tan", new WindowProxyMethod(this, vm, TanProc));
        vm->Register("sqrt", new WindowProxyMethod(this, vm, SqrtProc));
        vm->Register("log10", new WindowProxyMethod(this, vm, Log10Proc));
        vm->Register("ln", new WindowProxyMethod(this, vm, LnProc));
        vm->Register("to_num", new WindowProxyMethod(this, vm, ToNumProc));
        vm->Register("to_string", new WindowProxyMethod(this, vm, ToStringProc));
        vm->Register("round", new WindowProxyMethod(this, vm, RoundProc));
        vm->Register("concat", new WindowProxyMethod(this, vm, ConcatProc));
        vm->Register("str_first", new WindowProxyMethod(this, vm, StrFirstProc));
        vm->Register("str_last", new WindowProxyMethod(this, vm, StrLastProc));
        vm->Register("str_mid", new WindowProxyMethod(this, vm, StrMidProc));
        vm->Register("str_len", new WindowProxyMethod(this, vm, StrLenProc));

        vm->Register("load_sprite", new WindowProxyMethod(this, vm, LoadSpriteProc));
        vm->Register("hidesprite", new WindowProxyMethod(this, vm, HideSpriteProc));

        vm->Register("getspriteleft", new WindowProxyMethod(this, vm, GetSpriteLeftProc));
        vm->Register("getspriteright", new WindowProxyMethod(this, vm, GetSpriteRightProc));
        vm->Register("getspritetop", new WindowProxyMethod(this, vm, GetSpriteTopProc));
        vm->Register("getspritebottom", new WindowProxyMethod(this, vm, GetSpriteBottomProc));
        vm->Register("getspritewidth", new WindowProxyMethod(this, vm, GetSpriteWidthProc));
        vm->Register("getspriteheight", new WindowProxyMethod(this, vm, GetSpriteHeightProc));

        vm->Register("wait", new WindowProxyMethod(this, vm, WaitProc));

        vm->Register("file_write", new WindowProxyMethod(this, vm, FileWriteProc));
        vm->Register("file_write_using_width", new WindowProxyMethod(this, vm, FileWriteUsingWidthProc));
        vm->Register("file_write_line", new WindowProxyMethod(this, vm, FileWriteLineProc));
        vm->Register("file_read_line", new WindowProxyMethod(this, vm, FileReadLineProc));
        vm->Register("file_eof", new WindowProxyMethod(this, vm, FileEofProc));
        vm->Register("file_open", new WindowProxyMethod(this, vm, FileOpenProc));
        vm->Register("file_create", new WindowProxyMethod(this, vm, FileCreateProc));
        vm->Register("file_append", new WindowProxyMethod(this, vm, FileAppendProc));
        vm->Register("file_close", new WindowProxyMethod(this, vm, FileCloseProc));

        for(int i=0; i<stringConstants.keys().count(); i++)
        {
            QString strValue = stringConstants.keys()[i];
            QString strSymRef = stringConstants[strValue];
            vm->DefineStringConstant(strSymRef, strValue);
        }
        InitVMPrelude(vm);
        vm->Load(program);
        vm->Init();
        Run();
        update();// final update, in case the last instruction didn't update things in time.
        //timerID = startTimer(0);

    }
    catch(VMError *err)
    {
        QString str = translate_error(err);
        QMessageBox box;
        box.setWindowTitle(QString::fromWCharArray(L"كلمات"));
        box.setText(str);
        box.exec();
        this->close();
    }
}

void RunWindow::InitVMPrelude(VM *vm)
{
    LineIterator in = Utils::readResourceTextFile(":/vm_prelude.txt");
    QString prelude = in.readAll();
    in.close();
    vm->Load(prelude);
}

void RunWindow::Run()
{
    MainWindow *mw = dynamic_cast<MainWindow *>(this->parent());
    int pos,  len, oldPos = -1, oldLen = -1;
    try
    {
        while(state == rwNormal && vm->isRunning())
        //if(state == rwNormal && vm->isRunning())
        {
            bool visualize = mw != NULL && mw->isWonderfulMonitorEnabled();

            if(visualize)
                mw->markCurrentInstruction(vm, pos, len);

            vm->RunStep();
            redrawWindow();

            if(visualize
                //&&  (oldPos != pos ) && (oldLen != len)
                )
            {
                QTime dieTime = QTime::currentTime().addMSecs(500);
                while( QTime::currentTime() < dieTime )
                    QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
            }
            oldPos = pos;
            oldLen = len;

            qApp->processEvents();
        }

        //else
        {
         //   killTimer(timerID);
        }
    }
    catch(VMError *err)
    {
        QString msg = translate_error(err);
        if(err->args.count()==1)
            msg = "<u>" + msg +"</u>" + ":<p>"+ err->args[0]+ "</p";

        reportError(msg, err);
    }
}

void RunWindow::reportError(QString msg, VMError *err)
{
        this->suspend();
        QMessageBox box;
        box.setTextFormat(Qt::RichText);
        box.setWindowTitle(QString::fromWCharArray(L"كلمات"));

        box.setText(msg);
        box.exec();


        MainWindow *mw = dynamic_cast<MainWindow *>(this->parent());
        if(mw!= NULL)
        {
            mw->handleVMError(err);
        }
}

QString RunWindow::pathOfRunningProgram()
{
    return this->pathOfProgramsFile;
}

QString RunWindow::ensureCompletePath(QString fileName)
{

    QFileInfo fi(fileName);
    if(!fi.isAbsolute())
    {
        if(pathOfRunningProgram() == "")
            assert(false, ArgumentError, QString::fromStdWString(L"لا يمكن استخدام ملف بإسم ناقص إلا عند تخزين البرنامج أولا"));
        else
        {
            QFileInfo i2(pathOfRunningProgram());
            QFileInfo i3(i2.absoluteDir(), fileName);
            fileName = i3.absoluteFilePath();
        }
    }
    return fileName;
}



void RunWindow::timerEvent(QTimerEvent *ev)
{
    if(state == rwWaiting)
    {
       killTimer(ev->timerId());
       resume();
       //timerID = startTimer(0);
       Run();
    }
    if(state == rwNormal)
    {
//        Run();
    }
}

void RunWindow::resetTimer(int interval)
{
    killTimer(timerID);
    timerID = startTimer(interval);
}

void RunWindow::typeCheck(Value *val, ValueClass *type)
{
    vm->assert(val->type->subclassOf(type), TypeError);
}
void RunWindow::assert(bool condition, VMErrorType errorType, QString errorMsg)
{
    vm->assert(condition, errorType, errorMsg);
}

void RunWindow::typeError(ValueClass *)
{
    vm->assert(false, TypeError);
}
void RunWindow::clearText()
{
    cursor_col = 0;
    cursor_line = 0;
    visibleTextLines = 25;


    textLineWidth = 54;
    visibleTextBuffer.clear();
    visibleTextBuffer.resize(25);
}

void RunWindow::print(QString str)
{
    for(int i=0; i<str.length(); i++)
    {
        printChar(str[i]);
    }
    redrawWindow();
}
void RunWindow::printChar(QChar c)
{
    if(c == '\n')
        nl();
    else
    {
        if(cursor_col>= textLineWidth)
            nl();

        if(cursor_line >= visibleTextBuffer.count())
            visibleTextBuffer.resize(cursor_line+1);

        QString s = visibleTextBuffer[cursor_line];
        if(s.length() <= cursor_col)
        {
            int n = cursor_col - s.length() +1;
                s.append(QString(n, ' '));
        }
        s[cursor_col] = c;
        visibleTextBuffer[cursor_line] = s;
        cursor_col++;
    }
}

QString RunWindow::formatStringUsingWidth(QString str, int width)
{
    if(str.length()<width)
    {
        int n = width - str.length();
        QString sp = "";
        for(int i=0; i<n; i++)
            sp += " ";
        str+=sp;
    }
    else if(str.length() > width)
    {
        str = "";
        for(int i=0; i<width; i++)
        {
            str += "#";
        }
    }
    return str;
}

void RunWindow::print(QString str, int width)
{
    if(str == "\n")
    {
        // todo: How does string formatting with a certain width affect printing of a newline, anyway?
        nl();
    }
    else
    {
        str = formatStringUsingWidth(str, width);
        print(str);
    }
}
void RunWindow::println(QString str)
{
    print(str);
    nl();
}

bool RunWindow::setCursorPos(int row, int col)
{
    if(row < 0 || row >= visibleTextLines || col < 0 || col>=textLineWidth)
    {
        return false;
    }
    cursor_col = col;
    cursor_line = row;
    return true;
}

int RunWindow::getCursorRow()
{
    return cursor_line;
}
int RunWindow::getCursorCol()
{
    return cursor_col;
}
void RunWindow::cr()
{
    cursor_col = 0;
}
void RunWindow::lf()
{
    cursor_line++;
    if(cursor_line ==visibleTextLines)
    {
        visibleTextBuffer.pop_front();
        visibleTextBuffer.append("");
        cursor_line--;
    }
}
void RunWindow::nl()
{
    cr();
    lf();
}
void RunWindow::suspend()
{
    state = rwWaiting;
}

void RunWindow::resume()
{
    state = rwNormal;
}

QRect RunWindow::cursor()
{
    QPainter p(GetImage());

    p.setFont(textFont);
    QString line = visibleTextBuffer[cursor_line].left(cursor_col);
    int w = p.fontMetrics().tightBoundingRect(line).width();
    int h = textFont.pixelSize();

    int x = width() - w;


    int y = cursor_line * h;

    return QRect(x-2, y, 2, h);

}
void RunWindow::backSpace()
{
    QString s = visibleTextBuffer[cursor_line];

    s = s.left(cursor_col-1) + s.mid(cursor_col, s.length() - cursor_col);
    visibleTextBuffer[cursor_line] = s;
    cursor_col --;
}
void RunWindow::redrawWindow()
{
    if(updateTimer.canUpdateNow())
        update();
}

void RunWindow::paintEvent(QPaintEvent *)
{


   QPainter painter(this);
   QImage finalImg(image.size(), image.format());
   QPainter imgPainter(&finalImg);

   QTextOption options;
   options.setAlignment(Qt::AlignLeft);
   options.setTextDirection(Qt::RightToLeft);



   imgPainter.drawImage(QPoint(0,0), image);
   imgPainter.setLayoutDirection(Qt::RightToLeft);

   textFont = this->font();
   textFont.setPixelSize(image.height()/25);

   textFont.setRawName("Simplified Arabic Fixed");
   imgPainter.setFont(textFont);
   imgPainter.setBackgroundMode((Qt::TransparentMode));
   for(int i=0; i<sprites.count(); i++)
   {
       Sprite *s = sprites[i];
       if(s->visible)
       {
           imgPainter.drawPixmap(s->location, s->image);
       }
   }
   QPen oldPen = imgPainter.pen();
   imgPainter.setPen(textColor);
   for(int i=0; i<visibleTextBuffer.count(); i++)
   {
       QRect rct(0, i* textFont.pixelSize(), width()-1, height());
       imgPainter.drawText(rct, visibleTextBuffer[i], options);
   }
   imgPainter.setPen(oldPen);
   if(state == rwTextInput)
   {
       QRect cur = cursor();
       imgPainter.drawLine(cur.topRight(), cur.bottomRight());
   }
   painter.drawImage(QPoint(0, 0), finalImg);
}
void RunWindow::resizeEvent(QResizeEvent *event)
{

    //visibleTextLines = height() / p.fontMetrics().height();

    if (width() > image.width() || height() > image.height()) {
        int newWidth = qMax(width() + 128, image.width());
        int newHeight = qMax(height() + 128, image.height());
        resizeImage(&image, QSize(newWidth, newHeight));
        redrawWindow();
    }
    QWidget::resizeEvent(event);

}
void RunWindow::resizeImage(QImage *image, const QSize &newSize)
 {
     if (image->size() == newSize)
         return;
     QImage newImage(newSize, QImage::Format_RGB32);
     newImage.fill(qRgb(255, 255, 255));
     QPainter painter(&newImage);
     painter.drawImage(QPoint(0, 0), *image);
     *image = newImage;

 }
void RunWindow::changeEvent(QEvent *e)
{
    QMainWindow::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void RunWindow::mousePressEvent(QMouseEvent *ev)
{
    /*
    //wchar_t *s = L"مرحبا";
    QStdWString ss = L"مرحبا";

    QString str = QString::fromStdWString(ss);
    print(str);
    //*/
    //state = rwTextInput;
    QVector<Value *> args;
    int x = ev->x();
    int y = ev->y();

    TX(x);
    args.append(vm->GetAllocator().newInt(x));
    args.append(vm->GetAllocator().newInt(y));

    bool wasRunning = vm->isRunning();
    try
    {
        vm->ActivateEvent("mouse", args);
        if(!wasRunning)
            Run();
    }
    catch(VMError *err)
    {
        suspend();
        QString msg = translate_error(err);
        if(err->args.count()==1)
            msg = "<u>" + msg +"</u>" + ":<p>"+ err->args[0]+ "</p";
        this->close();
        reportError(msg, err);
    }



}
void RunWindow::keyPressEvent(QKeyEvent *ev)
{
    if(ev->key() == Qt::Key_Escape)
    {
        state = rwSuspended;
        this->close();
        return;
    }
    else if(state == rwTextInput)
    {

        if(ev->key() == Qt::Key_Return)
        {
            //print(inputBuffer);
            state = rwNormal;
            readMethod->SetReadValue(vm->GetAllocator().newString(new QString(inputBuffer)));
            inputBuffer = "";
            nl();
            update();
            Run();
        }
        else if(ev->key() == Qt::Key_Backspace)
        {
            inputBuffer = inputBuffer.left(inputBuffer.length()-1);
            backSpace();
            update();
        }
        else if(ev->text() != "")
        {
            QString s = ev->text();
            inputBuffer += s;
            print(s);
            update();
        }
    }
}
QImage *RunWindow::GetImage()
{
    return &this->image;
}
void RunWindow::AddSprite(Sprite *s)
{
    sprites.append(s);
}

QColor RunWindow::GetColor(int color)
{
/*
    switch(color)
    {
    case 0:
        return Qt::black;
    case 1:
        return Qt::darkRed;
    case 2:
        return Qt::red;
    case 3:
        return Qt::darkYellow;
    case 4:
        return Qt::yellow;
    case 5:
    return Qt::darkGreen;
    case 6:
        return Qt::green;
    case 7:
        return Qt::darkGray;
    case 8:
        return Qt::gray;
    case 9:
        return Qt::darkCyan;
    case 10:
        return Qt::cyan;
    case 11:
        return Qt::darkBlue;
    case 12:
        return Qt::blue;
    case 13:
        return Qt::darkMagenta;
    case 14:
        return Qt::magenta;
    case 15:
        return Qt::white;
    default:
        return Qt::white;
    }
    */
    switch(color)
    {
    case 0:
        return QColor::fromRgb(0,0,0);
    case 1:
        return QColor::fromRgb(255,145,72);
    case 2:
        return QColor::fromRgb(36,218,36);
    case 3:
        return QColor::fromRgb(109,255,109);
    case 4:
        return QColor::fromRgb(36,35,255);
    case 5:
        return QColor::fromRgb(72,109,255);
    case 6:
        return QColor::fromRgb(182,36,36);
    case 7:
        return QColor::fromRgb(72,182,255);
    case 8:
        return QColor::fromRgb(255,36,36);
    case 9:
        return QColor::fromRgb(255,109,109);
    case 10:
        return QColor::fromRgb(218,218,36);
    case 11:
        return QColor::fromRgb(218,218,145);
    case 12:
        return QColor::fromRgb(36,145,36);
    case 13:
        return QColor::fromRgb(218,72,182);
    case 14:
        return QColor::fromRgb(182,182,182);
    case 15:
        return QColor::fromRgb(255,255,255);
    default:
        return Qt::white;
    }
}
void RunWindow::showSprite(Sprite *s)
{
    visibleSprites.insert(s);
}
void RunWindow::hideSprite(Sprite *s)
{
    visibleSprites.remove(s);
}

void RunWindow::checkCollision(Sprite *s)
{
    QMap<Sprite *, Sprite *> collides;
    for(QSet<Sprite *>::iterator i = visibleSprites.begin(); i!=visibleSprites.end(); ++i)
    {
        Sprite *s2 = *i;
        if(s != s2)
        {
            if(s->boundingRect().intersects(s2->boundingRect()))
            {
                collides[s] = s2;
            }
        }
    }
    for(QMap<Sprite *, Sprite *>::iterator i = collides.begin(); i!= collides.end(); ++i)
    {
        Sprite *s1 = i.key();
        Sprite *s2 = i.value();
        QVector<Value *> args;

        args.append(vm->GetAllocator().newRaw(s1, Value::SpriteType));
        args.append(vm->GetAllocator().newRaw(s2, Value::SpriteType));
        vm->ActivateEvent("collision", args);
    }
}

void RunWindow::zoom(int x1, int y1, int x2, int y2)
{

}
void RunWindow::cls()
{
    QPainter p(GetImage());
    p.fillRect(0,0,GetImage()->width(), GetImage()->height(), Qt::white);
    clearText();
    redrawWindow();
}
void RunWindow::clearAllText()
{
    clearText();
    redrawWindow();
}

void RunWindow::setTextColor(QColor color)
{
    textColor = color;
}

void RunWindow::TX(int &x)
{
    x = (this->width()-1)-x;
}
QString RunWindow::translate_error(VMError *err)
{
    if(ErrorMap.empty())
    {
        LineIterator in = Utils::readResourceTextFile(":/error_map.txt");
        int i=0;
        while(!in.atEnd())
        {
            QString val = in.readLine().trimmed();
            ErrorMap[(VMErrorType) i++] = val;
        }
        in.close();
    }
    return ErrorMap[err->type];
}

WindowPrintMethod::WindowPrintMethod(RunWindow *parent)
{
    this->parent = parent;
}

void WindowPrintMethod::operator ()(QStack<Value *> &operandStack)
{
    Value *v = operandStack.pop();
    QString str = v->toString();
    bool isnum = true;
    for(int i=0; i<str.length(); i++)
    {
        if(!str[i].isNumber())
        {
            isnum = false;
            break;
        }
    }
    // If a char is a number, insert an
    // 'invisible space' character (zero width space, U+200B) after it
    // so that printing several numbers in succession
    // doesn't combine them into a large number
    // and print the combination left-to-right
    if(isnum)
        str +=QString("%1").arg(QChar((uint) 0x200b));
    parent->print(str);
}
WindowReadMethod::WindowReadMethod(RunWindow *parent)
{
    this->parent = parent;
}

void WindowReadMethod::operator ()(QStack<Value *> &operandStack)
{
    parent->state = rwTextInput;
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
    bool result = w->setCursorPos(line, col);
    w->assert(result, ArgumentError, QString::fromStdWString(L"قيم غير صحيحة لتحديد موقع المؤشر"));
}
void GetCursorRowProc(QStack<Value *> &stack, RunWindow *w, VM *vm)
{
    int r = w->getCursorRow();
    stack.push(vm->GetAllocator().newInt(r));
}
void GetCursorColProc(QStack<Value *> &stack, RunWindow *w, VM *vm)
{
    int c = w->getCursorCol();
    stack.push(vm->GetAllocator().newInt(c));
}


void PrintUsingWidthProc(QStack<Value *> &stack, RunWindow *w, VM *vm)
{
    Value *v = stack.pop();
    w->typeCheck(stack.top(), Value::IntType);
    int wid = stack.pop()->unboxInt();
    QString str = v->toString();
    w->print(str, wid);
}

void DrawPixelProc(QStack<Value *> &stack, RunWindow *w, VM *vm)
{
    int x = popIntOrCoercable(stack, w, vm);
    int y = popIntOrCoercable(stack, w, vm);
    int color = popInt(stack, w, vm);
    if(color == -1)
        color = 0;
    QColor clr = w->GetColor(color);
    w->TX(x);
    QPainter p(w->GetImage());
    p.fillRect(x, y, 1, 1, color);
    w->redrawWindow();
}

void DrawLineProc(QStack<Value *> &stack, RunWindow *w, VM *vm)
{
    int x1 = popIntOrCoercable(stack, w, vm);
    int x2 = popIntOrCoercable(stack, w, vm);
    int y1 = popIntOrCoercable(stack, w, vm);
    int y2 = popIntOrCoercable(stack, w, vm);

    w->TX(x1);
    w->TX(x2);
    int color = popInt(stack, w, vm);
    if(color ==-1)
        color = 0;

    QColor clr = w->GetColor(color);
    QPainter p(w->GetImage());

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
    int x2 = popIntOrCoercable(stack, w, vm);
    int y1 = popIntOrCoercable(stack, w, vm);
    int y2 = popIntOrCoercable(stack, w, vm);

    w->TX(x1);
    w->TX(x2);
    int color = popInt(stack, w, vm);
    int filled = popInt(stack, w, vm);

    if(color ==-1)
        color = 0;

    QPainter p(w->GetImage());

    int top = min(y1, y2);
    int left = min(x1, x2);
    int wid = abs(x2-x1);
    int hei = abs(y2-y1);
    QColor clr = w->GetColor(color);
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

    int filled = popInt(stack, w, vm);
    w->TX(cx);
    QPainter p(w->GetImage());
    if(color ==-1)
        color = 0;
    QColor clr = w->GetColor(color);
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

void ToNumProc(QStack<Value *> &stack, RunWindow *w, VM *vm)
{

    QString *str = popString(stack, w, vm);
    bool ok;
    QLocale loc(QLocale::Arabic, QLocale::Egypt);

    int i= loc.toLongLong(*str, &ok,10);
    if(ok)
    {
       stack.push(vm->GetAllocator().newInt(i));
       return;
    }

    i = str->toInt(&ok, 10);

    if(ok)
    {
        stack.push(vm->GetAllocator().newInt(i));
        return;
    }
    double d = str->toDouble(&ok);
    if(ok)
    {
        stack.push(vm->GetAllocator().newDouble(d));
        return;
    }
    //todo: We should do something like Basic's "Redo from start" when reading incorrectly-formatted input.
    vm->signal(TypeError, QString::fromStdWString(L"القيمة المدخلة لم تكن عددية"));

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

    QString *ret = new QString(str->mid(i,n));
    stack.push(vm->GetAllocator().newString(ret));
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


int popIntOrCoercable(QStack<Value *> &stack, RunWindow *w, VM *vm)
{
    if(stack.empty())
    {
        vm->signal(InternalError);
    }
    Value *v = stack.pop();
    if(v->tag != Int && v->tag != Double)
    {
        w->typeError(v->type);
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
        w->typeError(v->type);
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
    w->typeCheck(stack.top(), Value::StringType);
    QString *fname = stack.pop()->unboxStr();
    *fname = w->ensureCompletePath(*fname);

    if(!QFile::exists(*fname))
    {
        w->assert(false, ArgumentError, QString::fromStdWString(L"تحميل طيف من ملف غير موجود"));
    }
    Sprite *sprite = new Sprite(*fname);
    w->AddSprite(sprite);
    stack.push(vm->GetAllocator().newRaw(sprite, Value::SpriteType));
}
void DrawSpriteProc(QStack<Value *> &stack, RunWindow *w, VM *vm)
{

    w->typeCheck(stack.top(), Value::SpriteType);
    void *rawVal = stack.pop()->unboxRaw();
    Sprite  *sprite = (Sprite *) rawVal;

    int x = popIntOrCoercable(stack, w , vm);

    int y = popIntOrCoercable(stack, w , vm);

    w->TX(x);
    x-= sprite->image.width();
    sprite->location = QPoint(x,y);
    sprite->visible = true;
    w->showSprite(sprite);
    w->checkCollision(sprite);
    w->redrawWindow();
}
void HideSpriteProc(QStack<Value *> &stack, RunWindow *w, VM *vm)
{
    w->typeCheck(stack.top(), Value::SpriteType);
    void *rawVal = stack.pop()->unboxRaw();
    Sprite  *sprite = (Sprite *) rawVal;
    
    sprite->visible = false;
    w->hideSprite(sprite);
    w->redrawWindow();
}
void GetSpriteLeftProc(QStack<Value *> &stack, RunWindow *w, VM *vm)
{
    w->typeCheck(stack.top(), Value::SpriteType);
    void *rawVal = stack.pop()->unboxRaw();
    Sprite  *sprite = (Sprite *) rawVal;

    int ret = sprite->boundingRect().left();
    w->TX(ret);
    stack.push(vm->GetAllocator().newInt(ret));
}

void GetSpriteRightProc(QStack<Value *> &stack, RunWindow *w, VM *vm)
{
    w->typeCheck(stack.top(), Value::SpriteType);
    void *rawVal = stack.pop()->unboxRaw();
    Sprite  *sprite = (Sprite *) rawVal;

    int ret = sprite->boundingRect().right();
    w->TX(ret);
    stack.push(vm->GetAllocator().newInt(ret));
}

void GetSpriteTopProc(QStack<Value *> &stack, RunWindow *w, VM *vm)
{
    w->typeCheck(stack.top(), Value::SpriteType);
    void *rawVal = stack.pop()->unboxRaw();
    Sprite  *sprite = (Sprite *) rawVal;

    int ret = sprite->boundingRect().top();
    stack.push(vm->GetAllocator().newInt(ret));
}

void GetSpriteBottomProc(QStack<Value *> &stack, RunWindow *w, VM *vm)
{
    w->typeCheck(stack.top(), Value::SpriteType);
    void *rawVal = stack.pop()->unboxRaw();
    Sprite  *sprite = (Sprite *) rawVal;

    int ret = sprite->boundingRect().bottom();
    stack.push(vm->GetAllocator().newInt(ret));
}

void GetSpriteWidthProc(QStack<Value *> &stack, RunWindow *w, VM *vm)
{
    w->typeCheck(stack.top(), Value::SpriteType);
    void *rawVal = stack.pop()->unboxRaw();
    Sprite  *sprite = (Sprite *) rawVal;

    int ret = sprite->boundingRect().width();
    stack.push(vm->GetAllocator().newInt(ret));
}

void GetSpriteHeightProc(QStack<Value *> &stack, RunWindow *w, VM *vm)
{
    w->typeCheck(stack.top(), Value::SpriteType);
    void *rawVal = stack.pop()->unboxRaw();
    Sprite  *sprite = (Sprite *) rawVal;

    int ret = sprite->boundingRect().height();
    stack.push(vm->GetAllocator().newInt(ret));
}

void ShowSpriteProc(QStack<Value *> &stack, RunWindow *w, VM *vm)
{
    w->typeCheck(stack.top(), Value::SpriteType);
    void *rawVal = stack.pop()->unboxRaw();
    Sprite  *sprite = (Sprite *) rawVal;

    sprite->visible = true;
    w->showSprite(sprite);
    w->checkCollision(sprite);
    w->redrawWindow();
}
void WaitProc(QStack<Value *> &stack, RunWindow *w, VM *vm)
{
    int ms = stack.pop()->unboxInt();
    w->suspend();
    //w->resetTimer(ms);
    w->startTimer(ms);
}
void ZoomProc(QStack<Value *> &stack, RunWindow *w, VM *vm)
{
    int x1 = popIntOrCoercable(stack, w , vm);
    int y1 = popIntOrCoercable(stack, w , vm);
    int x2 = popIntOrCoercable(stack, w , vm);
    int y2 = popIntOrCoercable(stack, w , vm);
    //w->TX(x1);
    //w->TX(x2);
    w->zoom(x1, y1, x2, y2);
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
    w->setTextColor(w->GetColor(color));
    w->redrawWindow();
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
    w->typeCheck(stack.top(), Value::FileType);
    Object *ob = stack.pop()->unboxObj();
    Value *rawFile = ob->getSlotValue("file_handle");
    w->typeCheck(rawFile, Value::RawFileType);
    void *fileObj = rawFile->unboxRaw();
    FileBlob *f = (FileBlob *) fileObj;
    return f;
}

QString *popString(QStack<Value *> &stack, RunWindow *w, VM *vm)
{
    verifyStackNotEmpty(stack, vm);
    w->typeCheck(stack.top(), Value::StringType);
    QString *s = stack.pop()->unboxStr();
    return s;
}
int popInt(QStack<Value *> &stack, RunWindow *w, VM *vm)
{
    verifyStackNotEmpty(stack, vm);
    w->typeCheck(stack.top(), Value::IntType);
    int i = stack.pop()->unboxInt();
    return i;
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

    QString s2 = w->formatStringUsingWidth(*s, width);
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
    int r = ret? 1:0;
    Value *v = vm->GetAllocator().newInt(r);
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
    Value *v = vm->GetAllocator().newObject(Value::FileType);
    v->v.objVal->_slots["file_handle"] = vm->GetAllocator().newRaw(blob, Value::RawFileType);
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
    Value *v = vm->GetAllocator().newObject(Value::FileType);
    v->v.objVal->_slots["file_handle"] = vm->GetAllocator().newRaw(blob, Value::RawFileType);
    stack.push(v);
}

void FileAppendProc(QStack<Value *> &stack, RunWindow *w, VM *vm)
{
    QString *fname = popString(stack, w, vm);
    *fname = w->ensureCompletePath(*fname);
    QFile *f = new QFile(*fname);
    bool ret = f->open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append);
    w->assert(ret, ArgumentError, QString::fromStdWString(L"لم ينجح فتح الملف"));
    Value *v = vm->GetAllocator().newObject(Value::FileType);
    QTextStream *stream = new QTextStream(f);
    FileBlob *blob = new FileBlob();
    blob->file = f;
    blob->stream = stream;
    v->v.objVal->_slots["file_handle"] = vm->GetAllocator().newRaw(blob, Value::RawFileType);
    stack.push(v);
}

void FileCloseProc(QStack<Value *> &stack, RunWindow *w, VM *vm)
{
    // TODO: use popFileblob
    w->typeCheck(stack.top(), Value::FileType);
    Object *ob = stack.pop()->unboxObj();
    Value *rawFile = ob->getSlotValue("file_handle");
    w->typeCheck(rawFile, Value::RawFileType);
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

void RunWindow::on_actionGC_triggered()
{
    vm->gc();
}
