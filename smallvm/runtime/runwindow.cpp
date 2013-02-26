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
#include "parserengine.h"

#include <QPushButton>
#include <QPainter>
#include <QKeyEvent>
#include <QMessageBox>
#include <QWaitCondition>
#include <QClipboard>
#include <QTime>
//#include <iostream>
#include <QFileInfo>
#include <QDir>
#include <QtConcurrentRun>
#include <thread>

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
    textLayer.Init(width(), height(), font());
    paintSurface = new PaintSurface(size(), font());

    state = rwNormal;
    readMethod = new WindowReadMethod(this, vm);
    vm = NULL;
    textLayer.clearText();
    this->client = client;
    centralWidget()->setAttribute(Qt::WA_TransparentForMouseEvents);

    setMouseTracking(true);
    alreadyRunningScheduler = false;
    connect(this, SIGNAL(guiSchedule(GUISchedulerEvent*)), this, SLOT(do_gui_schedule(GUISchedulerEvent*)));
    connect(this, SIGNAL(errorSignal(VMError)), this, SLOT(errorEvent(VMError)));
}

void RunWindow::closeEvent(QCloseEvent *ev)
{
    suspend();
    /*
    if(vm != NULL)
        vm->gc();
    */
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
        // vm->gc();
        delete vm;
    }
    delete readMethod;
    delete paintSurface;

    delete realmouseEventChannel;
    delete realmouseDownEventChannel;
    delete realmouseUpEventChannel;
    delete realmouseMoveEventChannel;
    delete realkbEventChannel;
    delete readChannel;
}

void RunWindow::Init(QString program, QMap<QString, QString> stringConstants, QSet<Breakpoint> breakPoints, DebugInfo debugInfo)
{
    try
    {
        vm = new VM();
        readChannel =  vm->GetAllocator().newChannel(false);

        mouseEventChannel = NULL;
        mouseDownEventChannel = NULL;
        mouseUpEventChannel = NULL;
        mouseMoveEventChannel = NULL;
        kbEventChannel = NULL;

        realmouseEventChannel = vm->GetAllocator().newChannel(false);
        realmouseDownEventChannel = vm->GetAllocator().newChannel(false);
        realmouseUpEventChannel = vm->GetAllocator().newChannel(false);
        realmouseMoveEventChannel = vm->GetAllocator().newChannel(false);
        realkbEventChannel = vm->GetAllocator().newChannel(false);

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
        vm->Register("drawimage", new WindowProxyMethod(this, vm, DrawImageProc));
        vm->Register("drawsprite", new WindowProxyMethod(this, vm, DrawSpriteProc));
        vm->Register("zoom", new WindowProxyMethod(this, vm, ZoomProc));
        vm->Register("cls", new WindowProxyMethod(this, vm, ClsProc));
        vm->Register("cleartext", new WindowProxyMethod(this, vm, ClearTextProc));
        vm->Register("settextcolor", new WindowProxyMethod(this, vm, SetTextColorProc));
        vm->Register("pointat", new WindowProxyMethod(this, vm, PointAtProc));
        vm->Register("pointrgbat", new WindowProxyMethod(this, vm, PointRgbAtProc));
        vm->Register("builtinconstant", new WindowProxyMethod(this, vm, BuiltInConstantProc));
        vm->Register("stringisnumeric", new WindowProxyMethod(this, vm, StringIsNumericProc));
        vm->Register("stringisalphabetic", new WindowProxyMethod(this, vm, StringIsAlphabeticProc));
        vm->Register("typeof", new WindowProxyMethod(this, vm, TypeOfProc));
        vm->Register("typefromid", new WindowProxyMethod(this, vm, TypeFromIdProc));
        vm->Register("newmap", new WindowProxyMethod(this, vm, NewMapProc));
        vm->Register("haskey", new WindowProxyMethod(this, vm, HasKeyProc));
        vm->Register("keysof", new WindowProxyMethod(this, vm, KeysOfProc));
        vm->Register("mapkey", new WindowProxyMethod(this, vm, MapKeyProc));

        vm->Register("random", new WindowProxyMethod(this, vm, RandomProc));
        vm->Register("sin", new WindowProxyMethod(this, vm, SinProc));
        vm->Register("cos", new WindowProxyMethod(this, vm, CosProc));
        vm->Register("tan", new WindowProxyMethod(this, vm, TanProc));
        vm->Register("asin", new WindowProxyMethod(this, vm, ASinProc));
        vm->Register("acos", new WindowProxyMethod(this, vm, ACosProc));
        vm->Register("atan", new WindowProxyMethod(this, vm, ATanProc));
        vm->Register("sqrt", new WindowProxyMethod(this, vm, SqrtProc));
        vm->Register("pow", new WindowProxyMethod(this, vm, PowProc));
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
        vm->Register("str_replace", new WindowProxyMethod(this, vm, StrReplaceProc));
        vm->Register("str_len", new WindowProxyMethod(this, vm, StrLenProc));

        vm->Register("load_image", new WindowProxyMethod(this, vm, LoadImageProc));
        vm->Register("load_sprite", new WindowProxyMethod(this, vm, LoadSpriteProc));
        vm->Register("sprite_from_image", new WindowProxyMethod(this, vm, SpriteFromImageProc));
        vm->Register("showsprite", new WindowProxyMethod(this, vm, ShowSpriteProc));
        vm->Register("hidesprite", new WindowProxyMethod(this, vm, HideSpriteProc));

        vm->Register("getspriteleft", new WindowProxyMethod(this, vm, GetSpriteLeftProc));
        vm->Register("getspriteright", new WindowProxyMethod(this, vm, GetSpriteRightProc));
        vm->Register("getspritetop", new WindowProxyMethod(this, vm, GetSpriteTopProc));
        vm->Register("getspritebottom", new WindowProxyMethod(this, vm, GetSpriteBottomProc));
        vm->Register("getspritewidth", new WindowProxyMethod(this, vm, GetSpriteWidthProc));
        vm->Register("getspriteheight", new WindowProxyMethod(this, vm, GetSpriteHeightProc));
        vm->Register("getspriteimage", new WindowProxyMethod(this, vm, GetSpriteImageProc));
        vm->Register("setspriteimage", new WindowProxyMethod(this, vm, SetSpriteImageProc));


        vm->Register("wait", new WindowProxyMethod(this, vm, WaitProc, false));
        vm->Register("mouse_event_channel", new WindowProxyMethod(this, vm, MouseEventChanProc));
        vm->Register("mouseDown_event_channel", new WindowProxyMethod(this, vm, MouseDownEventChanProc));
        vm->Register("mouseUp_event_channel", new WindowProxyMethod(this, vm, MouseUpEventChanProc));
        vm->Register("mouseMove_event_channel", new WindowProxyMethod(this, vm, MouseMoveEventChanProc));
        vm->Register("kb_event_channel", new WindowProxyMethod(this, vm, KbEventChanProc));
        vm->Register("enable_mouse_event_channel", new WindowProxyMethod(this, vm, EnableMouseEventChanProc));
        vm->Register("enable_mouseDown_event_channel", new WindowProxyMethod(this, vm, EnableMouseDownEventChanProc));
        vm->Register("enable_mouseUp_event_channel", new WindowProxyMethod(this, vm, EnableMouseUpEventChanProc));
        vm->Register("enable_mouseMove_event_channel", new WindowProxyMethod(this, vm, EnableMouseMoveEventChanProc));
        vm->Register("enable_kb_event_channel", new WindowProxyMethod(this, vm, EnableKbEventChanProc));


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

        vm->Register("current_parse_tree", new WindowProxyMethod(this, vm, CurrentParseTreeProc));
        vm->Register("make_parser", new WindowProxyMethod(this, vm, MakeParserProc));
        vm->Register("push_bk_pt", new WindowProxyMethod(this, vm, PushParserBacktrackPointProc));
        vm->Register("ignore_bk_pt", new WindowProxyMethod(this, vm, IgnoreParserBacktrackPointProc));
        vm->Register("activation_frame", new WindowProxyMethod(this, vm, ActivationFrameProc));

        vm->Register("migrate_to_gui_thread", new WindowProxyMethod(this, vm, MigrateToGuiThreadProc));
        vm->Register("migrate_back_from_gui_thread", new WindowProxyMethod(this, vm, MigrateBackFromGuiThreadProc));

        vm->Register("closechannel", new WindowProxyMethod(this, vm, CloseChannelProc));
        vm->Register("channelclosed", new WindowProxyMethod(this, vm, ChannelClosedProc));

        //vm->Register("test_make_c_array", new WindowProxyMethod(this, vm, TestMakeCArrayProc));

        RegisterGuiControls(vm);

        // This qDebug() and the following are for debugging
        // executables that use SmallVM.dll, where we normally
        // don't have a debugger
        //qDebug("Defining string constants");

        for(int i=0; i<stringConstants.keys().count(); i++)
        {

            QString strValue = stringConstants.keys()[i];
            QString strSymRef = stringConstants[strValue];
            vm->DefineStringConstant(strSymRef, strValue);
        }
        //qDebug("Registering built-in types");
        IClass *builtIns[] = {
            BuiltInTypes::ObjectType,
            BuiltInTypes::NumericType,
            BuiltInTypes::IntType,
            BuiltInTypes::DoubleType,

            BuiltInTypes::BoolType,
            BuiltInTypes::RawType,
            BuiltInTypes::IMethodType,
            BuiltInTypes::MethodType,

            BuiltInTypes::ExternalMethodType,
            BuiltInTypes::ClassType,
            BuiltInTypes::IndexableType,
            BuiltInTypes::ArrayType,

            BuiltInTypes::MapType,
            BuiltInTypes::StringType,
            BuiltInTypes::SpriteType,
            BuiltInTypes::NullType,

            BuiltInTypes::LambdaType,
            BuiltInTypes::c_int,
            BuiltInTypes::c_long,
            BuiltInTypes::c_float,

            BuiltInTypes::c_double,
            BuiltInTypes::c_char,
            BuiltInTypes::c_asciiz,
            BuiltInTypes::c_wstr,

            BuiltInTypes::c_void,
            BuiltInTypes::c_ptr

        };
        // todo: handle built-in file type
        const int numBuiltIns = 26;
        for(int i=0; i<numBuiltIns; i++)
        {
            vm->RegisterType(builtIns[i]->getName(), builtIns[i]);
        }

        // This has to be before InitVMPrelude()
        vm->Register("class_newobject", new WindowProxyMethod(this, vm, ClassNewObjectProc));

        //qDebug("Loading prelude");
        InitVMPrelude(vm);

        // ...and this has to be after InitVMPrelude()
        // together they allow a metaclass to create new objects
        BuiltInTypes::ClassType->attachVmMethod(vm, VMId::get(RId::NewObject));

        BuiltInTypes::ArrayType->attachVmMethod(vm, VMId::get(RId::GetEnumerator));
        BuiltInTypes::StringType->attachVmMethod(vm, VMId::get(RId::GetEnumerator));
        BuiltInTypes::MapType->attachVmMethod(vm, VMId::get(RId::GetEnumerator));
        BuiltInTypes::MapType->attachVmMethod(vm, VMId::get(RId::KeyAt));
        BuiltInTypes::MapType->attachVmMethod(vm, VMId::get(RId::Contains));

        BuiltInTypes::ChannelType->attachVmMethod(vm, VMId::get(RId::GetEnumerator));
        BuiltInTypes::ChannelType->attachVmMethod(vm, VMId::get(RId::EnumMoveNext));
        BuiltInTypes::ChannelType->attachVmMethod(vm, VMId::get(RId::Current));
        BuiltInTypes::ChannelType->attachVmMethod(vm, VMId::get(RId::ChanClosed));
        BuiltInTypes::ChannelType->attachVmMethod(vm, VMId::get(RId::CloseChan));

        //qDebug("Registering GUI classes");
        // Meta class

        vm->RegisterType(VMId::get(RId::ForeignWindow), new WindowForeignClass(VMId::get(RId::ForeignWindow), this, vm));
        vm->RegisterType(VMId::get(RId::Button), new ButtonForeignClass (VMId::get(RId::Button), this, vm));
        vm->RegisterType(VMId::get(RId::TextBox), new TextboxForeignClass(VMId::get(RId::TextBox), this, vm));
        vm->RegisterType(VMId::get(RId::TextLine), new LineEditForeignClass(VMId::get(RId::TextLine), this, vm));
        vm->RegisterType(VMId::get(RId::ListBox), new ListboxForeignClass(VMId::get(RId::ListBox), this, vm));
        vm->RegisterType(VMId::get(RId::Label), new LabelForeignClass(VMId::get(RId::Label), this, vm));
        vm->RegisterType(VMId::get(RId::CheckBox), new CheckboxForeignClass(VMId::get(RId::CheckBox), this, vm));
        vm->RegisterType(VMId::get(RId::RadioButton), new RadioButtonForeignClass(VMId::get(RId::RadioButton), this, vm));
        vm->RegisterType(VMId::get(RId::ButtonGroup), new ButtonGroupForeignClass(VMId::get(RId::ButtonGroup), this, vm));
        vm->RegisterType(VMId::get(RId::ComboBox), new ComboboxForeignClass(VMId::get(RId::ComboBox), this, vm));
        vm->RegisterType(VMId::get(RId::Image), new ImageForeignClass(VMId::get(RId::Image), this, vm));
        vm->RegisterType(VMId::get(RId::ParseResultClass), new ParseResultClass(VMId::get(RId::ParseResultClass)));
        vm->RegisterType(VMId::get(RId::Parser), new ParserClass(VMId::get(RId::Parser), this, dynamic_cast<ParseResultClass *>(
                                                                     unboxClass(vm->GetType(VMId::get(RId::ParseResultClass))))));

        BuiltInTypes::ActivationFrameType = new FrameClass(VMId::get(RId::ActivationRecord), vm);
        vm->RegisterType(VMId::get(RId::ActivationRecord), BuiltInTypes::ActivationFrameType);
        ((FrameClass *) BuiltInTypes::ActivationFrameType)->allocator = &vm->GetAllocator();

        //qDebug("Loading user program");
        vm->Load(program);
        for(QSet<Breakpoint>::const_iterator i=breakPoints.begin(); i!=breakPoints.end(); ++i)
        {
            setBreakpoint(*i, debugInfo);
        }
        vm->vmThread = new VMRunthread(vm, this);
        vm->guiScheduler.runWindow = this;
        vm->Init();

        //qDebug("Setting debugger");
        if(breakPoints.empty())
            vm->setDebugger(NullaryDebugger::instance());
        else
            vm->setDebugger(client);

        /*
        if(parseTree)
        {
            // todo: deal with reified parse trees
            vm->SetGlobal("%parseTree", pt);
        }
        //*/
        //qDebug("Running user program");
        Run();
        //FastRun();
    }
    catch(VMError err)
    {
        reportError(err);
        this->close();
    }
}

void RunWindow::RegisterGuiControls(VM *vm)
{
    // Images
    vm->Register("image_rotated", new WindowProxyMethod(this, vm, ImageRotatedProc));
    vm->Register("image_scaled", new WindowProxyMethod(this, vm, ImageScaledProc));
    vm->Register("image_drawline", new WindowProxyMethod(this, vm, ImageDrawLineProc));
    vm->Register("image_flipped", new WindowProxyMethod(this, vm, ImageFlippedProc));
    vm->Register("image_copied", new WindowProxyMethod(this, vm, ImageCopiedProc));
    vm->Register("image_setpixelcolor", new WindowProxyMethod(this, vm, ImageSetPixelColorProc));
    vm->Register("image_pixelcolor", new WindowProxyMethod(this, vm, ImagePixelColorProc));
    vm->Register("image_width", new WindowProxyMethod(this, vm, ImageWidthProc));
    vm->Register("image_height", new WindowProxyMethod(this, vm, ImageHeightProc));
    vm->Register("image_drawtext", new WindowProxyMethod(this, vm, ImageDrawTextProc));

    // Windows
    vm->Register("foreignwindow_maximize", new WindowProxyMethod(this, vm, ForeignWindowMaximizeProc));
    vm->Register("foreignwindow_moveto", new WindowProxyMethod(this, vm, ForeignWindowMoveToProc));
    vm->Register("foreignwindow_add", new WindowProxyMethod(this, vm, ForeignWindowAddProc));
    vm->Register("foreignwindow_setsize", new WindowProxyMethod(this, vm, ForeignWindowSetSizeProc));
    vm->Register("foreignwindow_settitle", new WindowProxyMethod(this, vm, ForeignWindowSetTitleProc));
    vm->Register("foreignwindow_setup", new WindowProxyMethod(this, vm, ForeignWindowSetupProc));

    // Controls
    vm->Register("control_settext", new WindowProxyMethod(this, vm, ControlSetTextProc));
    vm->Register("control_setsize", new WindowProxyMethod(this, vm, ControlSetSizeProc));
    vm->Register("control_setlocation", new WindowProxyMethod(this, vm, ControlSetLocationProc));
    vm->Register("control_text", new WindowProxyMethod(this, vm, ControlTextProc));
    vm->Register("control_show", new WindowProxyMethod(this, vm, ControlShowProc));
    vm->Register("control_hide", new WindowProxyMethod(this, vm, ControlHideProc));
    vm->Register("control_setvisible", new WindowProxyMethod(this, vm, ControlSetVisibleProc));
    vm->Register("control_close", new WindowProxyMethod(this, vm, ControlCloseProc));

    // Buttons
    vm->Register("button_settext", new WindowProxyMethod(this, vm, ButtonSetTextProc));
    vm->Register("button_text", new WindowProxyMethod(this, vm, ButtonTextProc));

    // TextBox
    vm->Register("textbox_settext", new WindowProxyMethod(this, vm, TextboxSetTextProc));
    vm->Register("textbox_text", new WindowProxyMethod(this, vm, TextboxTextProc));
    vm->Register("textbox_appendtext", new WindowProxyMethod(this, vm, TextboxAppendTextProc));

    // LineEdit
    vm->Register("lineedit_settext", new WindowProxyMethod(this, vm, LineEditSetTextProc));
    vm->Register("lineedit_text", new WindowProxyMethod(this, vm, LineEditTextProc));
    vm->Register("lineedit_appendtext", new WindowProxyMethod(this, vm, LineEditAppendTextProc));

    // ListBox
    vm->Register("listbox_add", new WindowProxyMethod(this, vm, ListboxAddProc));
    vm->Register("listbox_insertitem", new WindowProxyMethod(this, vm, ListboxInsertItemProc));
    vm->Register("listbox_getitem", new WindowProxyMethod(this, vm, ListboxGetItemProc));

    // Combo box
    vm->Register("combobox_settext", new WindowProxyMethod(this, vm, ComboBoxSetTextProc));
    vm->Register("combobox_text", new WindowProxyMethod(this, vm, ComboBoxTextProc));
    vm->Register("combobox_add", new WindowProxyMethod(this, vm, ComboBoxAddProc));
    vm->Register("combobox_insertitem", new WindowProxyMethod(this, vm, ComboBoxInsertItemProc));
    vm->Register("combobox_getitem", new WindowProxyMethod(this, vm, ComboBoxGetItemProc));
    vm->Register("combobox_seteditable", new WindowProxyMethod(this, vm, ComboBoxSetEditableProc));

    // Labels
    vm->Register("label_settext", new WindowProxyMethod(this, vm, LabelSetTextProc));
    vm->Register("label_text", new WindowProxyMethod(this, vm, LabelTextProc));

    // Check box
    vm->Register("checkbox_settext", new WindowProxyMethod(this, vm, CheckboxSetTextProc));
    vm->Register("checkbox_text", new WindowProxyMethod(this, vm, CheckboxTextProc));
    vm->Register("checkbox_setvalue", new WindowProxyMethod(this, vm, CheckboxSetValueProc));
    vm->Register("checkbox_value", new WindowProxyMethod(this, vm, CheckboxValueProc));

    // Radio button
    vm->Register("radiobutton_settext", new WindowProxyMethod(this, vm, RadioButtonSetTextProc));
    vm->Register("radiobutton_text", new WindowProxyMethod(this, vm, RadioButtonTextProc));
    vm->Register("radiobutton_setvalue", new WindowProxyMethod(this, vm, RadioButtonSetValueProc));
    vm->Register("radiobutton_value", new WindowProxyMethod(this, vm, RadioButtonValueProc));

    // Button group
    vm->Register("buttongroup_add", new WindowProxyMethod(this, vm, ButtonGroupAddProc));
    vm->Register("buttongroup_getbutton", new WindowProxyMethod(this, vm, ButtonGroupGetButtonProc));
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
#ifdef ENGLISH_PL
    LineIterator in = Utils::readResourceTextFile(":/vm_prelude_en.txt", false);
#else
    LineIterator in = Utils::readResourceTextFile(":/vm_prelude.txt", false);
#endif
    QString prelude = in.readAll();
    in.close();
    vm->Load(prelude);
}

/*
void RunWindow::singleStep(Process *proc)
{
    int pos,  len, oldPos = -1, oldLen = -1;
    try
    {
        if((state == rwNormal || state ==rwTextInput)&& vm->isRunning() && !proc->isFinished())
        {
            bool visualize = client->isWonderfulMonitorEnabled();

            if(visualize)
                client->markCurrentInstruction(vm, proc, &pos, &len);

            vm->guiScheduler.RunStep(true);
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
*/
void RunWindow::EmitGuiSchedule()
{
    emit guiSchedule(new GUISchedulerEvent);
}

void RunWindow::do_gui_schedule(GUISchedulerEvent *)
{
    RunGUIScheduler();
}

void RunWindow::RunGUIScheduler()
{
    if(alreadyRunningScheduler)
        return;

    try
    {
        alreadyRunningScheduler = true;
        while((state == rwNormal || state ==rwTextInput) &&!vm->guiScheduler.isDone()
              )
        {
            if(vm->guiScheduler.RunStep())
            {
                redrawWindow();
            }
            qApp->processEvents(QEventLoop::AllEvents);
        }
        update();
        alreadyRunningScheduler = false;
    }
    catch(VMError err)
    {
        alreadyRunningScheduler = false;
        reportError(err);
        close();
    }
}

void RunWindow::timerEvent(QTimerEvent *)
{
    if(state == rwNormal || state ==rwTextInput)
    {
        vm->guiScheduler.waitRunningOnce(30);
        if(vm->guiScheduler.RunStep())
        {
            redrawWindow();
        }
    }
    else
    {
        killTimer(timerID);
    }
}

void RunWindow::Run()
{
    try
    {
        vm->vmThread->client = client;
        vm->vmThread->runFast = false;
        vm->destroyTheWorldFlag = false;
        vm->destroyer = NULL;
        vm->vmThread->start();
        //timerID = startTimer(0);
        //int x = 0;
        //*
        int pos,  len, oldPos = -1, oldLen = -1;

        while((state == rwNormal || state ==rwTextInput))
        {
            if(vm->destroyTheWorldFlag)
            {
                if(vm->destroyer!=&vm->guiScheduler)
                    vm->worldDestruction.release();
                break;
            }

            bool visualize = client->isWonderfulMonitorEnabled();
            if(visualize)
                vm->guiScheduler.RunStep(true);
            else
                vm->guiScheduler.RunStep(40);

            redrawWindow();
            if(visualize && vm->getMainProcess() && vm->getMainProcess()->state == AwakeProcess)
            {
                client->markCurrentInstruction(vm, vm->getMainProcess(), &pos, &len);
                if((oldPos != pos ) && (oldLen != len))
                {
                    QTime dieTime = QTime::currentTime().addMSecs(client->wonderfulMonitorDelay());
                    while( QTime::currentTime() < dieTime )
                        QCoreApplication::processEvents(QEventLoop::AllEvents, 30);
                }
            }
            oldPos = pos;
            oldLen = len;
            qApp->processEvents(QEventLoop::AllEvents);
        }
        update();
    }
    catch(VMError err)
    {
        reportError(err);
        this->close();
    }
}

void RunWindow::FastRun()
{
    try
    {
        vm->vmThread->client = client;
        vm->vmThread->runFast = true;
        vm->destroyTheWorldFlag = false;
        vm->destroyer = NULL;
        vm->vmThread->start();

        while((state == rwNormal || state ==rwTextInput))
        {
            vm->guiScheduler.FastRunStep(30);

            redrawWindow();

            qApp->processEvents(QEventLoop::AllEvents);
        }
        update();
    }
    catch(VMError err)
    {
        reportError(err);
        this->close();
    }
}

void RunWindow::emitErrorEvent(VMError err)
{
    emit errorSignal(err);
}

void RunWindow::errorEvent(VMError err)
{
    reportError(err);
    close();
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
    box.setWindowTitle(VM::argumentErrors[ArgErr::Kalimat]);

    box.setText(msg);
    box.exec();

    client->handleVMError(err);
}

QString RunWindow::pathOfRunningProgram()
{
    return this->pathOfProgramsFile;
}

QString RunWindow::ensureCompletePath(Process *proc, QString fileName)
{

    QFileInfo fi(fileName);
    if(!fi.isAbsolute())
    {
        if(pathOfRunningProgram() == "")
            assert(proc, false, ArgumentError, VM::argumentErrors.get(ArgErr::CannotUsePartialFileName1, fileName));
        else
        {
            QFileInfo i2(pathOfRunningProgram());
            QFileInfo i3(i2.absoluteDir(), fileName);
            fileName = i3.absoluteFilePath();
        }
    }
    return fileName;
}

void RunWindow::typeCheck(Process *proc, Value *val, IClass *type)
{
    vm->assert(proc, val->type->subclassOf(type), TypeError2, type, val->type);
}

void RunWindow::assert(Process *proc, bool condition, VMErrorType errorType, QString errorMsg)
{
    vm->assert(proc, condition, errorType, errorMsg);
}

void RunWindow::typeError(Process *proc, IClass *expected, IClass *given)
{
    vm->assert(proc, false, TypeError2, expected, given);
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
    {
       update();
    }
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


    if(vm->hasRegisteredEventHandler("collision")
            //        &&!vm->mainScheduler.hasInterrupts()
            )
    {
        spriteLayer.checkCollision(s, &callBack);
    }
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
    if(paintSurface->showCoordinates)
    {
        int x = ev->pos().x(), y = ev->pos().y();
        paintSurface->TX(x);

        paintSurface->mouseLocationForDemo = QPoint(x,y);

        redrawWindow();
    }
    activateMouseEvent(ev, "mousemove");
}

void RunWindow::activateMouseEvent(QMouseEvent *ev, QString evName)
{
    bool aChan = ( NULL != mouseEventChannel) ||
            ( NULL != mouseDownEventChannel) ||
            (NULL != mouseUpEventChannel) ||
            (NULL != mouseMoveEventChannel);

    bool regEv = vm->hasRegisteredEventHandler(evName);
    if(!(regEv || aChan))
    {
        return;
    }
    int x = ev->x();
    int y = ev->y();

    paintSurface->TX(x);


    Value *xval = vm->GetAllocator().newInt(x);
    Value *yval = vm->GetAllocator().newInt(y);

    try
    {
        //*
        // Send to mouse event channel
        if(aChan)
        {
            Value *mouseDataV = vm->GetAllocator().newObject((IClass *) unboxObj(vm->GetType(VMId::get(RId::MouseEventInfo))));
            IObject *mouseData = unboxObj(mouseDataV);
            mouseData->setSlotValue(VMId::get(RId::X), xval);
            mouseData->setSlotValue(VMId::get(RId::Y), yval);

            bool leftBtn = ev->buttons() & Qt::LeftButton;
            bool rightBtn = ev->buttons() & Qt::RightButton;

            mouseData->setSlotValue(VMId::get(RId::LeftButton), vm->GetAllocator().newBool(leftBtn));
            mouseData->setSlotValue(VMId::get(RId::RightButton), vm->GetAllocator().newBool(rightBtn));
            if(mouseEventChannel != NULL)
                unboxChan(mouseEventChannel)->send(mouseDataV, NULL);

            if(evName == "mousedown" && mouseDownEventChannel != NULL)
                unboxChan(mouseDownEventChannel)->send(mouseDataV, NULL);
            else if(evName == "mouseup" && mouseUpEventChannel != NULL)
                unboxChan(mouseUpEventChannel)->send(mouseDataV, NULL);
            else if(evName == "mousemove" && mouseMoveEventChannel != NULL)
                unboxChan(mouseMoveEventChannel)->send(mouseDataV, NULL);
        }
        //*/
        if(regEv)
        {
            QVector<Value *> args;
            args.append(xval);
            args.append(yval);
            vm->ActivateEvent(evName, args);
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

            // to hide the cursor
            textLayer.updateChangedLines(textLayer.cursorLine(), 1);
            Value *v = NULL;
            if(readMethod->readNum)
            {
                v = ConvertStringToNumber(inputText, vm);
            }
            else
            {
                v = vm->GetAllocator().newString(inputText);
            }
            if(v == NULL)
            {
                textLayer.nl();
                textLayer.print(VM::argumentErrors[ArgErr::MustEnterANumber]);
                textLayer.print("\n");
                textLayer.beginInput();
                update();
            }
            else
            {
                state = rwNormal;
                unboxChan(readChannel)->send(v, NULL);
                textLayer.nl();
                update();
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
    bool regEv = vm->hasRegisteredEventHandler(evName);
    bool chan = (kbEventChannel != NULL);
    if(!(regEv || chan))
    {
        return;
    }
    try
    {

        int k = ev->key();

        Value *key = vm->GetAllocator().newInt(k);
        Value *kchar = vm->GetAllocator().newString(ev->text());

        //*
        // Send to KB channel
        if(chan)
        {
            Value *kbInfoV = vm->GetAllocator().newObject((IClass *) unboxObj(vm->GetType(VMId::get(RId::KBEventInfo))));
            IObject *obj = unboxObj(kbInfoV);
            obj->setSlotValue(VMId::get(RId::Key), key);
            obj->setSlotValue(VMId::get(RId::Character), kchar);
            unboxChan(kbEventChannel)->send(kbInfoV, NULL);
        }
        //*/
        if(regEv)
        {
            QVector<Value *> args;
            args.append(key);
            args.append(kchar);
            vm->ActivateEvent(evName, args);
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
    Value *v1 = reinterpret_cast<Value *>(s1->extraValue);
    Value *v2 = reinterpret_cast<Value *>(s2->extraValue);
    if(v1->type != BuiltInTypes::SpriteType || v2->type
            != BuiltInTypes::SpriteType)
    {
        //int breakPoint = 0;
    }
    QVector<Value *> args;
    args.append(v1);
    args.append(v2);
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

void RunWindow::setMouseDemoMode(bool enable)
{
    paintSurface->showCoordinates = enable;
    update();
}

QString RunWindow::translate_error(VMError err)
{
    if(ErrorMap.empty())
    {
        ErrorMap = Utils::prepareErrorMap<VMErrorType>(":/error_map.txt");
    }
    return ErrorMap[err.type];
}

void RunWindow::on_actionGC_triggered()
{
    // vm->gc();
}
