/**************************************************************************
**   The Kalimat programming language
**   Copyright (C) 2010 Mohamed Samy Ali - samy2004@gmail.com
**   This project is released under the Apache License version 2.0
**   as described in the included license.txt file
**************************************************************************/

#include "runwindow.h"
#include "ui_runwindow.h"

#include "../utils.h"
#include "guicontrols.h"

#include <QPainter>
#include <QKeyEvent>
#include <QMessageBox>
#include <QWaitCondition>
#include <QClipboard>
#include <QTime>
//#include <iostream>
#include <QFileInfo>
#include <QDir>

using namespace std;

RunWindow::RunWindow(QWidget *parent, QString pathOfProgramsFile, VMClient *client) :
    QMainWindow(parent),
    updateTimer(30),
    ui(new Ui::RunWindow)
{
    setup(pathOfProgramsFile, client);
}

RunWindow::RunWindow(QString pathOfProgramsFile, VMClient *client) :
    updateTimer(30),
    ui(new Ui::RunWindow)
{
    setup(pathOfProgramsFile, client);
}


void RunWindow::setup(QString pathOfProgramsFile, VMClient *client)
{
    ui->setupUi(this);
    this->pathOfProgramsFile = pathOfProgramsFile;

    this->setFixedSize(800, 600);
    paintSurface = new PaintSurface(size(), font());

    state = rwNormal;
    readMethod = new WindowReadMethod(this, vm);
    vm = NULL;
    textLayer.clearText();
    this->client = client;
}

void RunWindow::closeEvent(QCloseEvent *ev)
{
    suspend();
    if(vm != NULL)
        vm->gc();
    client->programStopped(this);
    if(parentWidget() == NULL)
    {
        // Workaround because .exe programs don't terminate currently
        QApplication::instance()->exit(0);
        exit(0);
    }
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

    delete mouseEventChannel;
    delete mouseDownEventChannel;
    delete mouseUpEventChannel;
    delete kbEventChannel;
    delete readChannel;
}

void RunWindow::Init(QString program, QMap<QString, QString> stringConstants, QSet<Breakpoint> breakPoints, DebugInfo debugInfo)
{
    try
    {
        vm = new VM();
        readChannel =  vm->GetAllocator().newChannel(false);
        mouseEventChannel = vm->GetAllocator().newChannel(false);
        mouseDownEventChannel = vm->GetAllocator().newChannel(false);
        mouseUpEventChannel = vm->GetAllocator().newChannel(false);
        kbEventChannel = vm->GetAllocator().newChannel(false);

        vm->DefineStringConstant("new_line", "\n");
        vm->Register("print", new WindowProxyMethod(this, vm, PrintProc));
        vm->Register("input", readMethod);
        vm->Register("pushreadchan", new WindowProxyMethod(this, vm, PushReadChanProc));

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
        vm->Register("builtinconstant", new WindowProxyMethod(this, vm, BuiltInConstantProc));
        vm->Register("stringisnumeric", new WindowProxyMethod(this, vm, StringIsNumericProc));
        vm->Register("stringisalphabetic", new WindowProxyMethod(this, vm, StringIsAlphabeticProc));
        vm->Register("typeof", new WindowProxyMethod(this, vm, TypeOfProc));
        vm->Register("typefromid", new WindowProxyMethod(this, vm, TypeFromIdProc));
        vm->Register("newmap", new WindowProxyMethod(this, vm, NewMapProc));
        vm->Register("haskey", new WindowProxyMethod(this, vm, HasKeyProc));

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
        vm->Register("showsprite", new WindowProxyMethod(this, vm, ShowSpriteProc));
        vm->Register("hidesprite", new WindowProxyMethod(this, vm, HideSpriteProc));

        vm->Register("getspriteleft", new WindowProxyMethod(this, vm, GetSpriteLeftProc));
        vm->Register("getspriteright", new WindowProxyMethod(this, vm, GetSpriteRightProc));
        vm->Register("getspritetop", new WindowProxyMethod(this, vm, GetSpriteTopProc));
        vm->Register("getspritebottom", new WindowProxyMethod(this, vm, GetSpriteBottomProc));
        vm->Register("getspritewidth", new WindowProxyMethod(this, vm, GetSpriteWidthProc));
        vm->Register("getspriteheight", new WindowProxyMethod(this, vm, GetSpriteHeightProc));

        vm->Register("wait", new WindowProxyMethod(this, vm, WaitProc));
        vm->Register("mouse_event_channel", new WindowProxyMethod(this, vm, MouseEventChanProc));
        vm->Register("mouseDown_event_channel", new WindowProxyMethod(this, vm, MouseUpEventChanProc));
        vm->Register("mouseUp_event_channel", new WindowProxyMethod(this, vm, MouseUpEventChanProc));
        vm->Register("kb_event_channel", new WindowProxyMethod(this, vm, KbEventChanProc));

        vm->Register("file_write", new WindowProxyMethod(this, vm, FileWriteProc));
        vm->Register("file_write_using_width", new WindowProxyMethod(this, vm, FileWriteUsingWidthProc));
        vm->Register("file_write_line", new WindowProxyMethod(this, vm, FileWriteLineProc));
        vm->Register("file_read_line", new WindowProxyMethod(this, vm, FileReadLineProc));
        vm->Register("file_eof", new WindowProxyMethod(this, vm, FileEofProc));
        vm->Register("file_open", new WindowProxyMethod(this, vm, FileOpenProc));
        vm->Register("file_create", new WindowProxyMethod(this, vm, FileCreateProc));
        vm->Register("file_append", new WindowProxyMethod(this, vm, FileAppendProc));
        vm->Register("file_close", new WindowProxyMethod(this, vm, FileCloseProc));

        vm->Register("edit", new WindowProxyMethod(this, vm, EditProc));
        vm->Register("getmainwindow", new WindowProxyMethod(this, vm, GetMainWindowProc));
        vm->Register("newchannel", new WindowProxyMethod(this, vm, NewChannelProc));

        vm->Register("loadlibrary", new WindowProxyMethod(this, vm, LoadLibraryProc));
        vm->Register("getprocaddress", new WindowProxyMethod(this, vm, GetProcAddressProc));
        vm->Register("addressof", new WindowProxyMethod(this, vm, AddressOfProc));
        vm->Register("callforeign", new WindowProxyMethod(this, vm, InvokeForeignProc));


        for(int i=0; i<stringConstants.keys().count(); i++)
        {

            QString strValue = stringConstants.keys()[i];
            QString strSymRef = stringConstants[strValue];
            vm->DefineStringConstant(strSymRef, strValue);
        }

        IClass *builtIns[] = {
            BuiltInTypes::ObjectType,
            BuiltInTypes::NumericType,
            BuiltInTypes::IntType,
            BuiltInTypes::DoubleType,

            BuiltInTypes::BoolType,
            BuiltInTypes::MethodType,
            BuiltInTypes::ExternalMethodType,
            BuiltInTypes::ClassType,

            BuiltInTypes::IndexableType,
            BuiltInTypes::ArrayType,
            BuiltInTypes::MapType,
            BuiltInTypes::StringType,

            BuiltInTypes::SpriteType,
            BuiltInTypes::WindowType,
            BuiltInTypes::NullType,
            BuiltInTypes::c_int,

            BuiltInTypes::c_long,
            BuiltInTypes::c_float,
            BuiltInTypes::c_double,
            BuiltInTypes::c_char,

            BuiltInTypes::c_asciiz,
            BuiltInTypes::c_wstr,
            BuiltInTypes::c_ptr

                             };
        // todo: handle built-in file type
        const int numBuiltIns = 23;
        for(int i=0; i<numBuiltIns; i++)
        {
            vm->RegisterType(builtIns[i]->getName(), builtIns[i]);
        }
        vm->RegisterType("ForeignWindow", new WindowForeignClass(_ws(L"نافذة"), this));
        vm->RegisterType(_ws(L"زر"), new ButtonForeignClass (_ws(L"زر"), this));
        vm->RegisterType(_ws(L"صندوق.نصي"), new TextboxForeignClass(_ws(L"صندوق.نصي"), this));
        vm->RegisterType(_ws(L"سطر.نصي"), new LineEditForeignClass(_ws(L"سطر.نصي"), this));
        vm->RegisterType(_ws(L"صندوق.سرد"), new ListboxForeignClass(_ws(L"صندوق.سرد"), this));
        vm->RegisterType(_ws(L"علامة.نصية"), new LabelForeignClass(_ws(L"علامة.نصية"), this));
        vm->RegisterType(_ws(L"صندوق.استبيان"), new CheckboxForeignClass(_ws(L"صندوق.استبيان"), this));
        vm->RegisterType(_ws(L"صندوق.اختيار"), new RadioButtonForeignClass(_ws(L"صندوق.اختيار"), this));
        vm->RegisterType(_ws(L"مجموعة.اختيارات"), new ButtonGroupForeignClass(_ws(L"مجموعة.اختيارات"), this));
        vm->RegisterType(_ws(L"صندوق.مركب"), new ComboboxForeignClass(_ws(L"صندوق.مركب"), this));


        InitVMPrelude(vm);
        vm->Load(program);
        for(QSet<Breakpoint>::const_iterator i=breakPoints.begin(); i!=breakPoints.end(); ++i)
        {
            setBreakpoint(*i, debugInfo);
        }
        vm->Init();

        vm->setDebugger(client);

        Run();
    }
    catch(VMError err)
    {
        reportError(err);
        this->close();
    }
}

void RunWindow::setBreakpoint(Breakpoint b, const DebugInfo &debugInfo)
{
    CodeDocument *doc = b.doc;
    int line =  b.line;

    QString methodName;
    int offset;

    debugInfo.instructionFromLine(doc, line, methodName, offset);
    vm->setBreakPoint(methodName, offset);

    //cout << "Setting breakpoint at method= " << methodName.toStdString() << " and offset= " << offset
    //     << " and line= " << line << endl;
}

void RunWindow::InitVMPrelude(VM *vm)
{
    LineIterator in = Utils::readResourceTextFile(":/vm_prelude.txt");
    QString prelude = in.readAll();
    in.close();
    vm->Load(prelude);
}

void RunWindow::singleStep(Process *proc)
{
    int pos,  len, oldPos = -1, oldLen = -1;
    try
    {
        if((state == rwNormal || state ==rwTextInput)&& vm->isRunning() && !vm->processIsFinished(proc))
        {
            bool visualize = client->isWonderfulMonitorEnabled();

            if(visualize)
                client->markCurrentInstruction(vm, pos, len);

            vm->RunSingleInstruction(proc);
            redrawWindow();

            if(visualize
                &&  (oldPos != pos ) && (oldLen != len)
                )
            {
                QDateTime dieTime = QDateTime::currentDateTime().addMSecs(client->wonderfulMonitorDelay());
                while( QDateTime::currentDateTime() < dieTime )
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

void RunWindow::Run()
{
    int pos,  len, oldPos = -1, oldLen = -1;
    try
    {
        while((state == rwNormal || state ==rwTextInput)&& vm->isRunning())
        {
            bool visualize = client->isWonderfulMonitorEnabled();

            if(visualize)
                client->markCurrentInstruction(vm, pos, len);

            if(visualize)
                vm->RunStep(true);
            else
                vm->RunStep();

            redrawWindow();

            if(visualize
                    // commenting out the following condition makes the debugger work,
                    // but not the wonderful monitor :(    --todo: fix this
                &&  (oldPos != pos ) && (oldLen != len)
                )
            {
                QTime dieTime = QTime::currentTime().addMSecs(client->wonderfulMonitorDelay());
                while( QTime::currentTime() < dieTime )
                    QCoreApplication::processEvents(QEventLoop::AllEvents, 30);
            }
            oldPos = pos;
            oldLen = len;

            qApp->processEvents();
        }
        if(vm->isDone())
            client->programStopped(this);
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
        //cout << "runwindow::reportError message before translation is:" << msg.toStdString() << endl;
        //cout.flush();
        for(int i=0; i<err.args.count(); i++)
        {
            msg = msg.arg(err.args[i]);
        }

        QMessageBox box;
        box.setTextFormat(Qt::RichText);
        box.setWindowTitle(QString::fromWCharArray(L"كلمات"));

        box.setText(msg);
        box.exec();

        client->handleVMError(err);
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

}

void RunWindow::resetTimer(int interval)
{
    killTimer(timerID);
    timerID = startTimer(interval);
}

void RunWindow::typeCheck(Value *val, IClass *type)
{
    vm->assert(val->type->subclassOf(type), TypeError2, type, val->type);
}

void RunWindow::assert(bool condition, VMErrorType errorType, QString errorMsg)
{
    vm->assert(condition, errorType, errorMsg);
}

void RunWindow::typeError(IClass *expected, IClass *given)
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

void RunWindow::reactivateVM()
{
    vm->reactivate();
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

    Value *xval = vm->GetAllocator().newInt(x);
    Value *yval = vm->GetAllocator().newInt(y);
    args.append(xval);
    args.append(yval);

    bool normalRunning = vm->isRunning() && (state == rwNormal || state == rwTextInput);
    try
    {
        // Send to mouse event channel
        Value *mouseDataV = vm->GetAllocator().newObject((IClass *)vm->GetType(QString::fromStdWString(L"معلومات.حادثة.فارة"))->unboxObj());
        IObject *mouseData = mouseDataV->unboxObj();
        mouseData->setSlotValue(QString::fromStdWString(L"س"), xval);
        mouseData->setSlotValue(QString::fromStdWString(L"ص"), yval);

        bool leftBtn = ev->buttons() & Qt::LeftButton;
        bool rightBtn = ev->buttons() & Qt::RightButton;

        mouseData->setSlotValue(QString::fromStdWString(L"الزر.الأيسر"), vm->GetAllocator().newBool(leftBtn));
        mouseData->setSlotValue(QString::fromStdWString(L"الزر.الأيمن"), vm->GetAllocator().newBool(rightBtn));
        mouseEventChannel->unboxChan()->send(mouseDataV, NULL);

        if(evName == "mousedown")
            mouseDownEventChannel->unboxChan()->send(mouseDataV, NULL);
        else if(evName == "mouseup")
            mouseUpEventChannel->unboxChan()->send(mouseDataV, NULL);


        vm->ActivateEvent(evName, args);
        //resume();
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
                readChannel->unboxChan()->send(v, NULL);
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

        Value *key = vm->GetAllocator().newInt(k);
        Value *kchar = vm->GetAllocator().newString(txt);
        args.append(key);
        args.append(kchar);

        bool normalRunning = vm->isRunning() && state == rwNormal;

        // Send to KB channel
        Value *kbInfoV = vm->GetAllocator().newObject((IClass *)vm->GetType(QString::fromStdWString(L"معلومات.حادثة.لوحة.مفاتيح"))->unboxObj());
        IObject *obj = kbInfoV->unboxObj();
        obj->setSlotValue(QString::fromStdWString(L"زر"), key);
        obj->setSlotValue(QString::fromStdWString(L"حرف"), kchar);
        kbEventChannel->unboxChan()->send(kbInfoV, NULL);

        vm->ActivateEvent(evName, args);
        if(state != rwTextInput)
        {
            resume(); // If the RunWindow was not in a running state (say from a wait() call) then a key
                      // event should wake it up.
                      // TODO: This means a key event in the middle of a long wait will resume execution
                      // prematurely after the event is handled. We should resolve this.
        }

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
