/**************************************************************************
**   The Kalimat programming language
**   Copyright (C) 2010 Mohamed Samy Ali - samy2004@gmail.com
**   This project is released under the Apache License version 2.0
**   as described in the included license.txt file
**************************************************************************/

#include "runwindow.h"
#include "mainwindow.h"
#include "ui_runwindow.h"

#include "utils.h"

#include <QPainter>
#include <QKeyEvent>
#include <QMessageBox>
#include <QWaitCondition>
#include <QClipboard>

RunWindow::RunWindow(QWidget *parent, QString pathOfProgramsFile) :
    QMainWindow(parent),
    updateTimer(20),
    ui(new Ui::RunWindow)
{
    ui->setupUi(this);
    this->pathOfProgramsFile = pathOfProgramsFile;

    this->setFixedSize(800, 600);
    paintSurface = new PaintSurface(size(), font());

    state = rwNormal;

    readMethod = new WindowReadMethod(this, vm);
    vm = NULL;
    textLayer.clearText();
}

void RunWindow::closeEvent(QCloseEvent *ev)
{
    suspend();
    if(vm != NULL)
        vm->gc();
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
    delete readMethod;
    delete paintSurface;
}

void RunWindow::Init(QString program, QMap<QString, QString>stringConstants)
{
    try
    {
        vm = new VM();
        vm->DefineStringConstant("new_line", "\n");
        vm->Register("print", new WindowProxyMethod(this, vm, PrintProc));
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
        vm->Register("asin", new WindowProxyMethod(this, vm, ASinProc));
        vm->Register("acos", new WindowProxyMethod(this, vm, ACosProc));
        vm->Register("atan", new WindowProxyMethod(this, vm, ATanProc));
        vm->Register("sqrt", new WindowProxyMethod(this, vm, SqrtProc));
        vm->Register("log10", new WindowProxyMethod(this, vm, Log10Proc));
        vm->Register("ln", new WindowProxyMethod(this, vm, LnProc));
        vm->Register("to_num", new WindowProxyMethod(this, vm, ToNumProc));
        vm->Register("to_string", new WindowProxyMethod(this, vm, ToStringProc));
        vm->Register("round", new WindowProxyMethod(this, vm, RoundProc));
        vm->Register("remainder", new WindowProxyMethod(this, vm, RemainderProc));
        vm->Register("concat", new WindowProxyMethod(this, vm, ConcatProc));
        vm->Register("str_first", new WindowProxyMethod(this, vm, StrFirstProc));
        vm->Register("str_last", new WindowProxyMethod(this, vm, StrLastProc));
        vm->Register("str_mid", new WindowProxyMethod(this, vm, StrMidProc));
        vm->Register("str_begins_with", new WindowProxyMethod(this, vm, StrBeginsWithProc));
        vm->Register("str_ends_with", new WindowProxyMethod(this, vm, StrEndsWithProc));
        vm->Register("str_contains", new WindowProxyMethod(this, vm, StrContainsProc));
        vm->Register("str_split", new WindowProxyMethod(this, vm, StrSplitProc));
        vm->Register("str_trim", new WindowProxyMethod(this, vm, StrTrimProc));
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
        vm->Register("check_asleep", new WindowProxyMethod(this, vm, CheckAsleepProc));

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
    }
    catch(VMError err)
    {
        reportError(err);
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
        {
            bool visualize = mw != NULL && mw->isWonderfulMonitorEnabled();

            if(visualize)
                mw->markCurrentInstruction(vm, pos, len);

            vm->RunStep();
            redrawWindow();

            if(visualize
                &&  (oldPos != pos ) && (oldLen != len)
                )
            {
                QTime dieTime = QTime::currentTime().addMSecs(mw->wonderfulMonitorDelay());
                while( QTime::currentTime() < dieTime )
                    QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
            }
            oldPos = pos;
            oldLen = len;

            qApp->processEvents();
        }
        update();// Final update, in case the last instruction didn't update things in time.
    }
    catch(VMError err)
    {
        reportError(err);
        this->close();
    }
}

void RunWindow::reportError(VMError err)
{
        this->suspend();
        QString msg = translate_error(err);
        /*
            if(err.args.count()==1)
                msg = "<u>" + msg +"</u>" + ":<p>"+ err.args[0]+ "</p";
        */
        for(int i=0; i<err.args.count(); i++)
            msg = msg.arg(err.args[i]);

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
    asleep[ev->timerId()] = false;
    killTimer(ev->timerId());
    if(state == rwWaiting)
    {
       resume();
       Run();
    }
}

void RunWindow::resetTimer(int interval)
{
    killTimer(timerID);
    timerID = startTimer(interval);
}

void RunWindow::typeCheck(Value *val, ValueClass *type)
{
    vm->assert(val->type->subclassOf(type), TypeError2, type, val->type);
}

void RunWindow::assert(bool condition, VMErrorType errorType, QString errorMsg)
{
    vm->assert(condition, errorType, errorMsg);
}

void RunWindow::typeError(ValueClass *expected, ValueClass *given)
{
    vm->assert(false, TypeError2, expected, given);
}

void RunWindow::suspend()
{
    state = rwWaiting;
}

void RunWindow::resume()
{
    state = rwNormal;
}

void RunWindow::setAsleep(int cookie)
{
    asleep[cookie] = true;
}

bool RunWindow::isAsleep(int cookie)
{
    return asleep[cookie];
}

void RunWindow::redrawWindow()
{
    if(updateTimer.canUpdateNow())
        update();
}

void RunWindow::paintEvent(QPaintEvent *)
{
   QPainter painter(this);
   paintSurface->paint(painter, textLayer, spriteLayer);
}

void RunWindow::checkCollision(Sprite *s)
{
    class MyListener : public CollisionListener
    {
        RunWindow *owner;
    public:
        MyListener(RunWindow *owner) { this->owner = owner; }
        void operator() (Sprite *s1, Sprite *s2) { owner->onCollision(s1, s2); }
    } callBack(this);

    spriteLayer.checkCollision(s, &callBack);
}

void RunWindow::resizeEvent(QResizeEvent *event)
{
    paintSurface->resize(width(), height());
    redrawWindow();
    QWidget::resizeEvent(event);
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
   activateMouseEvent(ev, "mousedown");
}

void RunWindow::mouseReleaseEvent(QMouseEvent *ev)
{
    activateMouseEvent(ev, "mouseup");
}

void RunWindow::mouseMoveEvent(QMouseEvent *ev)
{
    activateMouseEvent(ev, "mousemove");
}

void RunWindow::activateMouseEvent(QMouseEvent *ev, QString evName)
{
    QVector<Value *> args;
    int x = ev->x();
    int y = ev->y();

    paintSurface->TX(x);
    args.append(vm->GetAllocator().newInt(x));
    args.append(vm->GetAllocator().newInt(y));
    bool normalRunning = vm->isRunning() && state == rwNormal;
    try
    {
        vm->ActivateEvent(evName, args);
        resume();
        if(!normalRunning && vm->isRunning())
        {
            Run();
        }
    }
    catch(VMError err)
    {
        reportError(err);
        this->close();
    }
}

void RunWindow::keyPressEvent(QKeyEvent *ev)
{
    if(ev->key() == Qt::Key_Escape)
    {
        suspend();
        this->close();
        return;
    }
    else if(state == rwTextInput)
    {

        if(ev->key() == Qt::Key_Return)
        {
            QString inputText = textLayer.endInput();
            Value *v = NULL;
            if(readMethod->readNum)
            {
                v = ConvertStringToNumber(inputText, vm);
            }
            else
            {
                v = vm->GetAllocator().newString(new QString(inputText));
            }
            if(v == NULL)
            {
                textLayer.nl();
                textLayer.print(QString::fromStdWString(L"يجب إدخال رقم! حاول مرة اخرى"));
                textLayer.print("\n");
                textLayer.beginInput();
                update();
            }
            else
            {
                state = rwNormal;
                readMethod->SetReadValue(v);
                textLayer.nl();
                update();
                Run();
            }
        }
        else if(ev->key() == Qt::Key_Backspace)
        {
            textLayer.backSpace();
            update();
        }
        else if(ev->key() == Qt::Key_Left)
        {
            textLayer.cursorFwd();
            update();
        }
        else if(ev->key() == Qt::Key_Delete)
        {
            textLayer.del();
            update();
        }
        else if(ev->key() == Qt::Key_Right)
        {
            textLayer.cursorBack();
            update();
        }
        else if(ev->modifiers() & Qt::ControlModifier && ev->key() == Qt::Key_V)
        {
            textLayer.typeIn(qApp->clipboard()->text());
            update();
        }
        else if(ev->text() != "")
        {
            textLayer.typeIn(ev->text());
            update();
        }
    }
    else if(state == rwNormal || state == rwWaiting)
    {
        activateKeyEvent(ev, "keydown");
    }
}

void RunWindow::keyReleaseEvent(QKeyEvent *ev)
{
    if(state == rwNormal || rwWaiting)
    {
        if(!ev->isAutoRepeat())
            activateKeyEvent(ev, "keyup");
    }
}

void RunWindow::activateKeyEvent(QKeyEvent *ev, QString evName)
{
    try
    {
        QVector<Value *> args;
        int k = ev->key();
        QString *txt = new QString(ev->text());

        args.append(vm->GetAllocator().newInt(k));
        args.append(vm->GetAllocator().newString(txt));

        bool normalRunning = vm->isRunning() && state == rwNormal;

        vm->ActivateEvent(evName, args);
        if(state != rwTextInput)
            resume(); // If the RunWindow was not in a running state (say from a wait() call) then a key
                      // event should wake it up.
                      // TODO: This means a key event in the middle of a long wait will resume execution
                      // prematurely after the event is handled. We should resolve this.

        if(!normalRunning && vm->isRunning())
        {
            Run();
        }
    }
    catch(VMError err)
    {
        reportError(err);
    }
}

void RunWindow::beginInput()
{
    state = rwTextInput;
    textLayer.beginInput();
}

void RunWindow::onCollision(Sprite *s1, Sprite *s2)
{
    QVector<Value *> args;
    args.append(vm->GetAllocator().newRaw(s1, BuiltInTypes::SpriteType));
    args.append(vm->GetAllocator().newRaw(s2, BuiltInTypes::SpriteType));
    vm->ActivateEvent("collision", args);
}

void RunWindow::cls()
{
    paintSurface->clearImage();
    textLayer.clearText();
    redrawWindow();
}

void RunWindow::clearAllText()
{
    textLayer.clearText();
    redrawWindow();
}

QString RunWindow::translate_error(VMError err)
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
    return ErrorMap[err.type];
}

void RunWindow::on_actionGC_triggered()
{
    vm->gc();
}
