/**************************************************************************
**   The Kalimat programming language
**   Copyright (C) 2010 Mohamed Samy Ali - samy2004@gmail.com
**   This project is released under the Apache License version 2.0
**   as described in the included license.txt file
**************************************************************************/

#include "Lexer/lexer_incl.h"
#include "Lexer/kalimatlexer.h"

#include "Parser/parser_incl.h"
#include "Parser/kalimatast.h"
#include "Parser/kalimatparser.h"
#include "codedocument.h"
#include "Compiler/codegenerator.h"
#include "Compiler/compiler.h"

#include "runwindow.h"
#include "syntaxhighlighter.h"

#include "savechangedfiles.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QMessageBox>
#include <QFileDialog>
#include <QCloseEvent>
#include <QSplitter>
#include <QVBoxLayout>
#include <QGraphicsTextItem>
#include <QDir>

MainWindow *MainWindow::that = NULL;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
    ui(new Ui::MainWindow)



{
    MainWindow::that = this;
    ui->setupUi(this);
    docContainer = new DocumentContainer("mohamedsamy",
                                         "kalimat 1.0",
                                         tr("Kalimat code (*.k *.* *)"),
                                         ui->editorTabs,
                                         this,
                                         MaxRecentFiles,
                                         ui->mnuRecentFiles_2);


    QSplitter *splitter = new QSplitter(Qt::Vertical, this);
    splitter->addWidget(ui->editorTabs);
    splitter->addWidget(ui->tabWidget);
    setCentralWidget(splitter);

    this->setWindowTitle(QString::fromWCharArray(L"كلمات"));
    this->showMaximized();

    docContainer->addDocument(QString::fromWCharArray(L"بدون عنوان"), "untitled", this->CreateEditorWidget(), true);
}
QWidget *MainWindow::GetParentWindow()
{
    return this;
}

QTextEdit *MainWindow::currentEditor()
{
    return docContainer->getCurrentDocument()->getEditor();
}

void MainWindow::LoadDocIntoWidget(CodeDocument *doc, QWidget *widget)
{
    // Does nothing, the CodeDocument
    // already loads the file contents into the
    // QTextEdit. This is for possible future extensiblity
    // where we might have editing widgets that are not
    // QTextEdit
}

MainWindow::~MainWindow()
{
    delete ui;
}

class MyEdit : public QTextEdit
{
public slots:
    void keyPressEvent(QKeyEvent *);
};

bool isAfterNumber(QTextEdit *edit)
{
    bool replace = false;
    if(edit->textCursor().position()>=1)
    {
        QChar at = edit->document()->characterAt(edit->textCursor().position()-1);
        if(at.isDigit())
        {
            replace = true;
        }
    }
    return replace;
}
bool isAfterArabicNumber(QTextEdit *edit)
{
    bool replace = false;
    if(edit->textCursor().position()>=1)
    {
        QChar at = edit->document()->characterAt(edit->textCursor().position()-1);
        if(at.unicode() >= L'٠' && at.unicode() < L'٩' )
        {
            replace = true;
        }
    }
    return replace;
}
void MyEdit::keyPressEvent(QKeyEvent *ev)
{
    static QString arabComma = QString::fromWCharArray(L"،");
    if(ev->key() == Qt::Key_Tab)
    {
        //QKeyEvent *otherEvent = new QKeyEvent(ev->type(), Qt::Key_Right, ev->modifiers(), ev->text(), ev->isAutoRepeat(), ev->count());
        //QTextEdit::keyPressEvent(otherEvent);
        this->insertPlainText("    ");
    }
    else if(ev->key() == Qt::Key_Left)
    {
        QKeyEvent *otherEvent = new QKeyEvent(ev->type(), Qt::Key_Right, ev->modifiers(), ev->text(), ev->isAutoRepeat(), ev->count());
        QTextEdit::keyPressEvent(otherEvent);
    }
    else if(ev->key() == Qt::Key_Right)
    {
        QKeyEvent *otherEvent = new QKeyEvent(ev->type(), Qt::Key_Left, ev->modifiers(), ev->text(), ev->isAutoRepeat(), ev->count());;
        QTextEdit::keyPressEvent(otherEvent);
    }
    else if(ev->text() == "," || ev->text() == arabComma)
    {
        bool rightAfterNumber = isAfterNumber(this);
        QTextEdit::keyPressEvent(ev);
        if(rightAfterNumber)
        {
            QKeyEvent *otherEvent = new QKeyEvent(QEvent::KeyPress, Qt::Key_Space, 0, " ", false, 1);
            QTextEdit::keyPressEvent(otherEvent);
        }
    }
    else if(ev->text() == ".")
    {
        bool replace = isAfterArabicNumber(this);
        if(!replace)
            QTextEdit::keyPressEvent(ev);
        else
            this->insertPlainText(QString::fromWCharArray(L"٫"));
    }

    else
    {
        QTextEdit::keyPressEvent(ev);
    }
}

QWidget *MainWindow::CreateEditorWidget()
{
    MyEdit *edit = new MyEdit();
    syn = new SyntaxHighlighter(edit->document(), new KalimatLexer());
    edit->textCursor().setVisualNavigation(true);
    QFont font = edit->font();
    font.setPointSize(18);

    edit->setFont(font);
    return edit;
}

void MainWindow::on_actionLexize_triggered()
{
    KalimatLexer lxr;

    lxr.init(currentEditor()->document()->toPlainText());
    try
    {
        lxr.tokenize();
        QVector<Token> tokens = lxr.getTokens();
        ui->outputView->clear();
        for(int i=0; i<tokens.size(); i++)
        {
           Token  t = tokens[i];
           QString lexeme = t.Lexeme;

           lexeme = lexeme.replace("\n", "\\n");
           QString msg = QString("%1/type=%2, line=%3\n").arg(t.Lexeme).arg(TokenNameFromId(t.Type)).arg(t.Line);
           ui->outputView->append(msg);
        }
    }
    catch(UnexpectedCharException *ex)
    {
        ui->outputView->append(ex->buildMessage());
    }
    catch(ColonUnsupportedInIdentifiersException *ex)
    {
        ui->outputView->append("Cannot have a ':' in a non-keyword");
    }
    catch(UnexpectedEndOfFileException *ex)
    {
        ui->outputView->append("Unexpected end of file");
    }
}

void MainWindow::on_actionParse_triggered()
{
    KalimatLexer lxr;
    KalimatParser parser;

    try
    {
        parser.init(currentEditor()->document()->toPlainText(), &lxr);
        AST * tree = parser.parse();
        ui->outputView->clear();
        ui->outputView->append(tree->toString());
    }
    catch(UnexpectedCharException *ex)
    {
        ui->outputView->append(ex->buildMessage());
    }
    catch(UnexpectedEndOfFileException *ex)
    {
        ui->outputView->append("Unexpected end of file");
    }
    catch(ParserException *ex)
    {
        ui->outputView->append(ex->message);
    }
}

void MainWindow::on_actionCompile_triggered()
{
    KalimatLexer lxr;
    KalimatParser parser;

    try
    {
        ui->outputView->clear();
        CodeDocument *doc = docContainer->getCurrentDocument();
        Compiler compiler;

        QString output;
        if(doc->isDocNewFile() || doc->isFileDirty())
            output = compiler.CompileFromCode(doc->getEditor()->document()->toPlainText(), doc);
        else
            output = compiler.CompileFromFile(doc->getFileName(), NULL);

        ui->outputView->append(output);
    }
    catch(UnexpectedCharException *ex)
    {
        ui->outputView->append(ex->buildMessage());
    }
    catch(UnexpectedEndOfFileException *ex)
    {
        ui->outputView->append("Unexpected end of file");
    }
    catch(ParserException *ex)
    {
        ui->outputView->append(ex->message);
    }
    catch(CompilerException ex)
    {
        ui->outputView->append(ex.getMessage());
    }


}

void MainWindow::on_mnuProgramRun_triggered()
{
    KalimatLexer lxr;
    KalimatParser parser;
    CodeDocument *doc;
    try
    {
        currentEditor()->setExtraSelections(QList<QTextEdit::ExtraSelection>());

        ui->outputView->clear();
        doc = docContainer->getCurrentDocument();
        Compiler compiler;

        QString output;
        QString path;
        if(doc->isDocNewFile() || doc->isFileDirty())
        {
            output = compiler.CompileFromCode(doc->getEditor()->document()->toPlainText(), doc);
            path = "";
        }
        else
        {
            output = compiler.CompileFromFile(doc->getFileName(), doc);
            path = doc->getFileName();

        }

        //ui->outputView->append(output);


        this->PositionInfo = compiler.generator.getPositionInfo();
        RunWindow *rw = new RunWindow(this, path);
        connect(this, SIGNAL(destroyed(QObject*)), rw, SLOT(parentDestroyed(QObject *)));
        rw->show();
        rw->Init(output, compiler.generator.getStringConstants());
    }
    catch(UnexpectedCharException *ex)
    {
        //show_error(ex->buildMessage());
        show_error(QString::fromStdWString(L"خطأ في تركيب البرنامج"));
    }
    catch(UnexpectedEndOfFileException *ex)
    {
        //show_error("Unexpected end of file");
        //show_error(QString::fromStdWString(L"خطأ في تركيب البرنامج"));
        show_error(QString::fromStdWString(L"انتهى البرنامج قبل أن يكون له معنى"));
    }
    catch(ParserException *ex)
    {
        //show_error(ex->message);
        show_error(QString::fromStdWString(L"خطأ في تركيب البرنامج"));
        if(doc != NULL && ex->hasPosInfo)
            highlightLine(doc->getEditor(), ex->pos.Pos);
    }
    catch(CompilerException ex)
    {
       show_error(ex.getMessage());
       // show_error(QString(L"خطأ في تركيب البرنامج"));
    }
    catch(VMError *err)
    {

    }
}
void MainWindow::highlightLine(QTextEdit *editor, int pos)
{
    QList<QTextEdit::ExtraSelection> selections;
    QTextEdit::ExtraSelection sel;

    QTextCursor cur = editor->textCursor();
    cur.setPosition(pos);

    editor->setTextCursor(cur);
    editor->ensureCursorVisible();

    sel.cursor = cur;
    sel.format.setProperty(QTextFormat::FullWidthSelection, true);
    sel.format.setBackground(QBrush(QColor(Qt::yellow)));
    selections.append(sel);
    editor->setExtraSelections(selections);


}
void MainWindow::highlightToken(QTextEdit *editor, int pos, int length)
{
    QList<QTextEdit::ExtraSelection> selections;
    QTextEdit::ExtraSelection sel;

    //editor->textCursor().setPosition(pos);
    QTextCursor cur = editor->textCursor();
    QTextCursor cur2 = cur;
    cur.setPosition(pos);

    editor->setTextCursor(cur);
    editor->ensureCursorVisible();

    cur.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, length);

    sel.cursor = cur;
    sel.format.setBackground(QBrush(QColor(Qt::yellow)));
    selections.append(sel);
    editor->setExtraSelections(selections);

}


bool MainWindow::isWonderfulMonitorEnabled()
{
    return ui->action_wonderfulmonitor->isChecked();
}

void MainWindow::visualizeCallStack(QStack<Frame> &callStack, QGraphicsView *view)
{
    QGraphicsScene *scene = new QGraphicsScene();

    float left = 0.0f;
    for(int i= callStack.count() -1; i>=0; i--)
    {
        Frame f = callStack.at(i);
        QString frepr;
        frepr = "<" + f.currentMethod->getName() + ">\n";
        for(int j=0; j<f.Locals.count(); j++)
        {
            QString var = f.Locals.keys().at(j);
            QString val = f.Locals.values().at(j)->toString();

            if(var.length() > 5)
                var = var.mid(0,4) +"-";
            if(val.length() > 10)
                val = val.mid(0, 9) + "-";

            QString out = QString("%1  =  %2").arg(var)
                          .arg(val);
            frepr +=out;
            if(j<f.Locals.count()-1)
                frepr += "\n";
        }
    
        QGraphicsTextItem *txt = scene->addText(frepr, ui->graphicsView->font());

        QGraphicsRectItem *rct = scene->addRect(txt->boundingRect(), QPen(Qt::black), QBrush(Qt::NoBrush));
        txt->setParentItem(rct);
        rct->setPos(left, 5.0f);
        left += 5.0f + txt->boundingRect().width();
    }
    view->setAlignment(Qt::AlignRight| Qt::AlignTop);
    view->setLayoutDirection(Qt::RightToLeft);


    view->setScene(scene);
}

void MainWindow::markCurrentInstruction(VM *vm, int &pos, int &length)
{
    if(vm->hasRunningInstruction())
    {
        Instruction i = vm->getCurrentInstruction();
        visualizeCallStack(vm->getCallStack(), ui->graphicsView);
        int key = i.extra;
        if(key ==-1)
            return;
        CodePosition p = PositionInfo[key];

        if(p.doc != NULL)
        {
            QTextEdit *editor = p.doc->getEditor();

            ui->editorTabs->setCurrentWidget(editor);
            pos = p.pos;
            length = p.ast->getPos().Lexeme.length();
            highlightToken(editor, pos, length);
        }
    }
}

void MainWindow::handleVMError(VMError *err)
{
    if(!err->callStack.empty())
    {
        Frame f = err->callStack.top();
        Instruction i = f.getPreviousRunningInstruction();
        int key = i.extra;
        CodePosition p = PositionInfo[key];

        if(p.doc != NULL)
        {
            // p.doc can be null if the source of the error is from a module
            // that's compiled from a file but not loaded into the editor.
            // todo: later we would create a CodeDocument * object and pass it to the
            // compiler even for modules not open in the editor, that way we can be like
            // Visual Studio (tm) and open the error-source file on the fly and
            // hilight the problem when there's an error!
            QTextEdit *editor = p.doc->getEditor();

            ui->editorTabs->setCurrentWidget(editor);
            setWindowTitle(QString("Error position index=%1").arg(key));
            highlightLine(editor, p.pos);
        }
    }
}

void MainWindow::on_action_new_triggered()
{
    docContainer->handleNew("new program ", CreateEditorWidget());
}
void MainWindow::closeEvent(QCloseEvent *ev)
{
    docContainer->handleClose(ev);
    QObject::connectNotify("mainWindowClosed");
}
void MainWindow::on_editorTabs_tabCloseRequested(int index)
{
    docContainer->handleTabCloseRequested(index);
}
void MainWindow::on_action_open_triggered()
{
    docContainer->handleOpen(CreateEditorWidget());
}

void MainWindow::on_action_save_triggered()
{
   docContainer->handleSave();
}

void MainWindow::on_action_saveas_triggered()
{
    docContainer->handleSaveAs();
}


void MainWindow::on_action_exit_triggered()
{
    this->close();
}
void MainWindow::show_error(QString msg)
{
    QMessageBox box;
    box.setWindowTitle(QString::fromWCharArray(L"كلمات"));
    box.setText(msg);
    box.exec();
}



void MainWindow::on_garbageCollect_triggered()
{

}

void MainWindow::on_actionLoad_Compilation_unit_triggered()
{
    try
    {
        Compiler compiler;
        //QString output = compiler.CompileFromFile("c:/code/kalimat/examples/module1.k");
        QString output = compiler.CompileFromFile("c:/code/kalimat/examples/program1.k", NULL);

        ui->outputView->append(output);
    }
    catch(UnexpectedCharException *ex)
    {
        show_error(ex->buildMessage());
    }
    catch(UnexpectedEndOfFileException *ex)
    {
        show_error("Unexpected end of file");
    }
    catch(ParserException *ex)
    {
        show_error(ex->message);
    }
    catch(CompilerException ex)
    {
        show_error(ex.getMessage());
    }
    catch(VMError *err)
    {

    }

}
