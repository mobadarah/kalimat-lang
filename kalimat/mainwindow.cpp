/**************************************************************************
**   The Kalimat programming language
**   Copyright (C) 2010 Mohamed Samy Ali - samy2004@gmail.com
**   This project is released under the Apache License version 2.0
**   as described in the included license.txt file
**************************************************************************/

#include "Lexer/lexer_incl.h"
#include "Lexer/kalimatlexer.h"

#include "Parser/parser_incl.h"
#include "Parser/kalimatast/kalimatast_incl.h"
#include "Parser/kalimatparser.h"
#include "../smallvm/codedocument.h"
#include "Compiler/codegenerator.h"
#include "Compiler/compiler.h"

#include "../smallvm/runtime/runwindow.h"
#include "../smallvm/utils.h"
#include "syntaxhighlighter.h"

#include "Parser/kalimatprettyprintparser.h"
#include "savechangedfiles.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QMessageBox>
#include <QFileDialog>
#include <QInputDialog>
#include <QCloseEvent>
#include <QVBoxLayout>
#include <QGraphicsTextItem>
#include <QDir>
#include <QToolBar>
#include <QClipboard>
#include <QTextEdit>
#include <QProcess>
#include <QTextStream>
MainWindow *MainWindow::that = NULL;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    MainWindow::that = this;

    helpWindowVisible = false;
    helpSplitter = NULL;

    ui->setupUi(this);

    QToolBar *notice = new QToolBar("");
    notice->addAction(QString::fromStdWString(L"هذه هي النسخة الأولية لشهر ديسمبر 2011. حمل أحدث نسخة من www.kalimat-lang.com"));

    addToolBarBreak();
    addToolBar(Qt::TopToolBarArea, notice);
    speedGroup = new QActionGroup(this);
    speedGroup->addAction(ui->actionSpeedFast);
    speedGroup->addAction(ui->actionSpeedMedium);
    speedGroup->addAction(ui->actionSpeedSlow);
    speedGroup->addAction(ui->actionSpeedSuper);
    lblEditorCurrentLine = new QLabel();
    lblEditorCurrentColumn = new QLabel();

    ui->statusBar->addWidget(lblEditorCurrentLine, 0.3);
    ui->statusBar->addWidget(lblEditorCurrentColumn, 0.3);
    lblEditorCurrentLine->show();
    ui->dockSearchReplace->hide();
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

    docContainer->addInitialEmptyDocument();
    stoppedRunWindow = NULL;
    atBreak = false;
    currentDebuggerProcess = NULL;
    lastCodeDocToRun = NULL;

    setAcceptDrops(true);
    ui->editorTabs->setAcceptDrops(true);
    ui->txtSearchString->installEventFilter(this);
    ui->txtReplacementString->installEventFilter(this);
}

void MainWindow::outputMsg(QString s)
{
    ui->outputView->append(s);
}

void MainWindow::showHelpWindow()
{
#if false
    if(helpWindowVisible)
        return;
    helpWindowVisible = true;
    if(helpSplitter == NULL)
    {
        helpSplitter = new QSplitter(Qt::Horizontal, this);
        oldMainWidget = centralWidget();
        helpWebView = new QWebView(helpSplitter);
        helpSplitter->addWidget(oldMainWidget);
        helpSplitter->addWidget(helpWebView);
    }
    setCentralWidget(helpSplitter);

    //QString html = readFile(QCoreApplication::applicationDirPath()+"/help/index.html");
    helpWebView->load(QUrl::fromLocalFile(QCoreApplication::applicationDirPath()+"/help/index.html"));
#endif
}

void MainWindow::hideHelpWindow()
{
    if(!helpWindowVisible)
        return;
    helpWindowVisible = false;
    setCentralWidget(oldMainWidget);
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
}

MainWindow::~MainWindow()
{
    delete ui;
    delete speedGroup;
    delete lblEditorCurrentLine;
}

QWidget *MainWindow::CreateEditorWidget()
{
    MyEdit *edit = new MyEdit(this);
    syn = new SyntaxHighlighter(edit->document(), new KalimatLexer());
    edit->textCursor().setVisualNavigation(true);
    QFont font = edit->font();
    font.setPointSize(18);

    edit->setFont(font);
    // If the next line wasn't there, the syntaxhilighter sends a document modified
    // event on the initial display of the text editor, causing the document
    // to be initially 'dirty'.
    edit->setPlainText("");
    return edit;
}

void MainWindow::setLineIndicators(int line, int column)
{
    lblEditorCurrentLine->setText(QString::fromStdWString(L"السطر: %1").arg(line));
    lblEditorCurrentColumn->setText(QString::fromStdWString(L"العمود: %1").arg(column));
}

void MainWindow::on_actionLexize_triggered()
{
    KalimatLexer lxr;

    lxr.init(currentEditor()->document()->toPlainText());
    try
    {
        ui->tabWidget->setCurrentWidget(ui->outputView);
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
    catch(UnexpectedCharException ex)
    {
        ui->outputView->append(ex.buildMessage());
    }
    catch(ColonUnsupportedInIdentifiersException ex)
    {

        ui->outputView->append("Cannot have a ':' in a non-keyword");
    }
    catch(UnexpectedEndOfFileException ex)
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
        ui->tabWidget->setCurrentWidget(ui->outputView);
        parser.init(currentEditor()->document()->toPlainText(), &lxr, NULL);
        shared_ptr<AST> tree = parser.parse();
        //ui->outputView->clear();
        ui->outputView->append(tree->toString());
    }
    catch(UnexpectedCharException ex)
    {
        ui->outputView->append(ex.buildMessage());
    }
    catch(UnexpectedEndOfFileException ex)
    {
        ui->outputView->append("Unexpected end of file");
    }
    catch(ParserException ex)
    {
        ui->outputView->append(ex.message);
    }
}

void MainWindow::on_actionCompile_triggered()
{
    KalimatLexer lxr;
    KalimatParser parser;
    CodeDocument *doc = NULL;
    try
    {
        ui->tabWidget->setCurrentWidget(ui->outputView);
        ui->outputView->clear();
        doc = docContainer->getCurrentDocument();
        Compiler compiler(docContainer);

        QString output;
        if(doc->isDocNewFile() || doc->isFileDirty())
            output = compiler.CompileFromCode(doc->getEditor()->document()->toPlainText(), doc);
        else
            output = compiler.CompileFromFile(doc->getFileName(), NULL);

        ui->outputView->append(output);
    }
    catch(UnexpectedCharException ex)
    {
        ui->outputView->append(ex.buildMessage());
    }
    catch(UnexpectedEndOfFileException ex)
    {
        ui->outputView->append("Unexpected end of file");
    }
    catch(ParserException ex)
    {
        QString fn;
        if(ex.pos.tag!= NULL)
        {
            fn = ((CodeDocument *) ex.pos.tag)->getFileName();
        }
        ui->outputView->append(QString("File %1:\t%2").arg(fn).arg(ex.message));
    }
    catch(CompilerException ex)
    {
        ui->outputView->append(ex.getMessage());
        if(doc != NULL)
        {
            CodeDocument *dc = doc;
            if(ex.source->getPos().tag != NULL)
            {
                dc = (CodeDocument *) ex.source->getPos().tag;
            }
            highlightLine(dc->getEditor(), ex.source->getPos().Pos);
        }
    }
}

void MainWindow::on_actionCompile_without_tags_triggered()
{
    KalimatLexer lxr;
    KalimatParser parser;
    CodeDocument *doc = NULL;
    try
    {
        ui->tabWidget->setCurrentWidget(ui->outputView);
        ui->outputView->clear();
        doc = docContainer->getCurrentDocument();
        Compiler compiler(docContainer);

        QString output;
        if(doc->isDocNewFile() || doc->isFileDirty())
            output = compiler.CompileFromCode(doc->getEditor()->document()->toPlainText(), doc);
        else
            output = compiler.CompileFromFile(doc->getFileName(), NULL);

        output = compiler.generator.getStringConstantsAsOpCodes() + output;
        QRegExp rx("@([^\n])*\n");
        output = output.replace(rx, "\r\n");
        ui->outputView->append(output);
    }
    catch(UnexpectedCharException ex)
    {
        ui->outputView->append(ex.buildMessage());
    }
    catch(UnexpectedEndOfFileException ex)
    {
        ui->outputView->append("Unexpected end of file");
    }
    catch(ParserException ex)
    {
        ui->outputView->append(ex.message);
    }
    catch(CompilerException ex)
    {
        ui->outputView->append(ex.getMessage());
        if(doc != NULL)
        {
            CodeDocument *dc = doc;
            if(ex.source->getPos().tag != NULL)
            {
                dc = (CodeDocument *) ex.source->getPos().tag;
            }
            highlightLine(dc->getEditor(), ex.source->getPos().Pos);
        }
    }
}

void MainWindow::saveAll()
{
    for(int i=0; i<docContainer->documentCount(); i++)
    {
        CodeDocument *doc = docContainer->getDocumentFromTab(i);
        if(!doc->isDocNewFile())
        {
            doc->doSave();
        }
    }
}

void MainWindow::on_mnuProgramRun_triggered()
{
    top:
    CodeDocument *doc = NULL;
    if((currentDebuggerProcess != NULL) && atBreak)
    {
        QMessageBox box(QMessageBox::Information, QString::fromStdWString(L"لا يمكن التنفيذ"),
                              QString::fromStdWString(L"لا يمكن تنفيذ برنامج جديد بينما نحن في نقطة توقف للبرنامج الحالي"));
        box.exec();
        return;
    }
    try
    {
        currentEditor()->setExtraSelections(QList<QTextEdit::ExtraSelection>());
        saveAll();
        ui->outputView->clear();
        doc = docContainer->getCurrentDocument();
        Compiler compiler(docContainer);

        QString output;
        QString path;
        if(doc->isDocNewFile() || doc->isFileDirty())
        {
            output = compiler.CompileFromCode(doc->getEditor()->document()->toPlainText(), doc);
        }
        else
        {
            output = compiler.CompileFromFile(doc->getFileName(), doc);
        }

        if(doc->isDocNewFile())
        {
            path = "";
        }
        else
        {
            path = doc->getFileName();
        }
        //ui->outputView->append(output);


        this->PositionInfo = compiler.generator.getPositionInfo();
        debugInfo = compiler.generator.debugInfo;

        RunWindow *rw = new RunWindow(this, path, this);

        connect(this, SIGNAL(destroyed(QObject*)), rw, SLOT(parentDestroyed(QObject *)));

        stoppedRunWindow = rw;
        atBreak = false;
        lastCodeDocToRun = doc;
        rw->show();

        KalimatLexer lxr;
        KalimatParser parser;
        parser.init(doc->getEditor()->document()->toPlainText(), &lxr, doc);
        rw->parseTree = dynamic_pointer_cast<KalimatAst>(parser.parse());
        rw->Init(output, compiler.generator.getStringConstants(), breakPoints, debugInfo);

    }
    catch(UnexpectedCharException ex)
    {
        //show_error(ex->buildMessage());
        show_error(QString::fromStdWString(L"لا يمكن كتابة هذا الرمز هنا '%1'").arg(ex.getCulprit()));
        if(doc != NULL)
        {
            CodeDocument *dc = doc;
            highlightLine(dc->getEditor(), ex.getLine());
        }
    }
    catch(UnexpectedEndOfFileException ex)
    {
        //show_error("Unexpected end of file");
        //show_error(QString::fromStdWString(L"خطأ في تركيب البرنامج"));
        show_error(QString::fromStdWString(L"انتهى البرنامج قبل أن يكون له معنى"));
    }
    catch(ParserException ex)
    {
        //show_error(ex->message);
        show_error(QString::fromStdWString(L"خطأ في تركيب البرنامج"));
        if(doc != NULL && ex.hasPosInfo)
        {
            CodeDocument *dc = doc;
            if(ex.pos.tag != NULL)
            {
                dc = (CodeDocument *) ex.pos.tag;
            }
            highlightLine(dc->getEditor(), ex.pos.Pos);
        }
    }
    catch(CompilerException ex)
    {
        if(ex.getError() == CannotRunAModule)
        {
            if(lastCodeDocToRun && docContainer->hasOpenDocument(lastCodeDocToRun))
            {
                QString msg = QString::fromStdWString(L"لا يمكن تنفيذ %1 لأنه وحدة، هل تريد تنفيذ البرنامج %2 بدلا منه؟")
                        .arg(doc->getFileName())
                        .arg(lastCodeDocToRun->getFileName());
                QMessageBox box(QMessageBox::Question,
                                QString::fromStdWString(L"لا يمكن تنفيذ وحدة"),
                                msg,
                                QMessageBox::Yes | QMessageBox::No);
                if(box.exec() == QMessageBox::Yes)
                {
                    docContainer->setCurrentDocument(lastCodeDocToRun);
                    goto top;
                }
                else
                {
                    return;
                }
            }
        }
       show_error(ex.getMessage());
       // show_error(QString(L"خطأ في تركيب البرنامج"));
       if(doc != NULL)
       {
           CodeDocument *dc = doc;
           if(ex.source && ex.source->getPos().tag != NULL)
           {
               dc = (CodeDocument *) ex.source->getPos().tag;
               highlightLine(dc->getEditor(), ex.source->getPos().Pos);
           }

       }
    }
    catch(VMError err)
    {

    }
}

void MainWindow::highlightLine(QTextEdit *editor, int pos)
{
   highlightLine(editor, pos, QColor(Qt::yellow));
}

void MainWindow::highlightLine(QTextEdit *editor, int pos, QColor color)
{
    QList<QTextEdit::ExtraSelection> selections;
    QTextEdit::ExtraSelection sel;

    QTextCursor cur = editor->textCursor();
    cur.setPosition(pos);

    editor->setTextCursor(cur);
    editor->ensureCursorVisible();

    sel.cursor = cur;
    sel.format.setProperty(QTextFormat::FullWidthSelection, true);
    sel.format.setBackground(QBrush(color));
    selections.append(sel);
    editor->setExtraSelections(selections);
    ui->editorTabs->setCurrentWidget(editor);
}

void MainWindow::removeLineHighlights(MyEdit *editor, int line)
{
    QList<QTextEdit::ExtraSelection> extra = editor->extraSelections();
    for(int i=0; i<extra.count(); i++)
    {
        QTextEdit::ExtraSelection sel = extra[i];
        if(editor->lineOfPos(sel.cursor.position()) == line)
        {
            extra.removeAt(i);
            break;
        }
    }
    editor->setExtraSelections(extra);
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

int MainWindow::wonderfulMonitorDelay()
{
    if(ui->actionSpeedFast->isChecked())
        return 100;
    if(ui->actionSpeedMedium->isChecked())
        return 500;
    if(ui->actionSpeedSlow->isChecked())
        return 1500;
    if(ui->actionSpeedSuper->isChecked())
        return 5;
    return 500;
}

void MainWindow::visualizeCallStacks(QQueue<Process *> &callStacks, QGraphicsView *view)
{
    // todo: Visualize the call stacks; as the function name says :(
    if(callStacks.empty())
        return;
    visualizeCallStack(callStacks.front()->stack, view);
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
    ui->tabWidget->setCurrentWidget(ui->graphicsView);
    view->setScene(scene);
}

void MainWindow::markCurrentInstruction(VM *vm, int &pos, int &length)
{
    if(vm->hasRunningInstruction())
    {
        Instruction i = vm->getCurrentInstruction();
        visualizeCallStacks(vm->getCallStacks(), ui->graphicsView);
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

void MainWindow::handleVMError(VMError err)
{
    if(!err.callStack.empty())
    {
        Frame f = err.callStack.top();
        highlightRunningInstruction(&f);
        visualizeCallStack(err.callStack, ui->graphicsView);
    }
}

void MainWindow::highlightRunningInstruction(Frame *f)
{
    highlightRunningInstruction(f, Qt::yellow);
}

void MainWindow::highlightRunningInstruction(Frame *f, QColor clr)
{
    CodePosition p = getPositionOfRunningInstruction(f);

    if(p.doc != NULL)
    {
        // p.doc can be null if the source of the error is from a module
        // that's compiled from a file but not loaded into the editor.
        // todo: later we would create a CodeDocument * object and pass it to the
        // compiler even for modules not open in the editor, that way we can be like
        // Visual Studio (tm) and open the error-source file on the fly and
        // hilight the problem when there's an error!
        QTextEdit *editor = p.doc->getEditor();

        //setWindowTitle(QString("Error position index=%1, file=%2").arg(key).arg(p.doc->getFileName()));
        highlightLine(editor, p.pos, clr);
    }
}

CodePosition MainWindow::getPositionOfRunningInstruction(Frame *f)
{
    Instruction i = f->getPreviousRunningInstruction();
    int key = i.extra;
    CodePosition p = PositionInfo[key];
    return p;
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
    docContainer->handleOpen();
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

void MainWindow::on_action_edit_cut_triggered()
{
    QTextEdit *editor = currentEditor();
    if(editor != NULL)
    {
        editor->cut();
    }
}

void MainWindow::on_action_edit_copy_triggered()
{
    QTextEdit *editor = currentEditor();
    if(editor != NULL)
    {
        editor->copy();
    }
}

void MainWindow::on_action_edit_paste_triggered()
{
    QTextEdit *editor = currentEditor();
    if(editor != NULL)
    {
        editor->paste();
    }
}

void MainWindow::on_action_delete_triggered()
{
    QTextEdit *editor = currentEditor();
    if(editor != NULL)
    {
        editor->textCursor().removeSelectedText();
    }
}

void MainWindow::on_action_undo_triggered()
{
    QTextEdit *editor = currentEditor();
    if(editor != NULL)
    {
        editor->undo();
    }
}

void MainWindow::on_action_redo_triggered()
{
    QTextEdit *editor = currentEditor();
    if(editor != NULL)
    {
        editor->redo();
    }
}

void MainWindow::on_action_find_triggered()
{
    ui->dockSearchReplace->show();
    ui->txtSearchString->setFocus();
    ui->txtSearchString->selectAll();
}

void MainWindow::on_action_replace_triggered()
{
    ui->dockSearchReplace->show();
}

void ensurePositionBeforeAnchor(QTextEdit *editor)
{
    QTextCursor c = editor->textCursor();
    int p = c.selectionStart();
    int n = c.selectionEnd() - p;
    c.clearSelection();
    c.setPosition(p + n);
    c.movePosition(QTextCursor::PreviousCharacter, QTextCursor::KeepAnchor, n);
    editor->setTextCursor(c);
}

void MainWindow::on_btnFindPrev_clicked()
{
    QTextEdit *editor = currentEditor();
    if(editor != NULL)
    {
        ui->lblFindStatus->setText("");
        QString searchStr = ui->txtSearchString->text();
        bool result = editor->find(searchStr, QTextDocument::FindBackward);
        if(!result)
        {
            ui->lblFindStatus->setText(QString::fromStdWString(L"وصلنا لبداية الملف"));

            // set the cursor at the end of the document
            QTextCursor c = editor->textCursor();
            c.clearSelection();
            c.movePosition(QTextCursor::End, QTextCursor::MoveAnchor);
            editor->setTextCursor(c);
        }
        else
            ensurePositionBeforeAnchor(editor);
    }
}

void MainWindow::on_btnFindNext_clicked()
{
    QTextEdit *editor = currentEditor();
    if(editor != NULL)
    {
        ui->lblFindStatus->setText("");
        QString searchStr = ui->txtSearchString->text();
        bool result = editor->find(searchStr);
        if(!result)
        {
            ui->lblFindStatus->setText(QString::fromStdWString(L"وصلنا لنهاية الملف"));

            // set the cursor at the begining of the document
            QTextCursor c = editor->textCursor();
            c.clearSelection();
            c.setPosition(0, QTextCursor::MoveAnchor);
            editor->setTextCursor(c);
        }
        else
        {
            ensurePositionBeforeAnchor(editor);
        }
    }
}

void MainWindow::on_btnReplacePrev_clicked()
{
    QTextEdit *editor = currentEditor();
    if(editor != NULL)
    {
        // get selected text..
        ui->lblFindStatus->setText("");
        bool bResult = (editor->textCursor().selectedText().compare(ui->txtSearchString->text()) == 0);

        // if matches search string
        if(bResult)
        {
            // replace it..
            editor->textCursor().insertText(ui->txtReplacementString->text());

            // move cursor before it
            int n =  ui->txtReplacementString->text().length();
            QTextCursor c = editor->textCursor();
            c.movePosition(QTextCursor::PreviousCharacter, QTextCursor::MoveAnchor, n);
            editor->setTextCursor(c);
        }

        // then search for the next match
        this->on_btnFindPrev_clicked();
    }
}

void MainWindow::on_btnReplaceNext_clicked()
{
    QTextEdit *editor = currentEditor();
    if(editor != NULL)
    {
        /* We want the user to be able to enter a word and a replacement, press 'find', verify
           that he actually wants to replace the word, then click replace.
           Unfortunately 'find' makes the found string selected, a replace in QT 4.6.2 begins it's
           own find operation from the selection end, so it begins from the next word after the found one.

           To work around this, we clear the selection before calling replace() on the text editor.
        */

        // get selected text..
        ui->lblFindStatus->setText("");
        bool bResult = (editor->textCursor().selectedText().compare(ui->txtSearchString->text()) == 0);

        // if matches search string
        if(bResult)
        {
            // replace it..
            editor->textCursor().insertText(ui->txtReplacementString->text());
        }

        // then search for the next match
        this->on_btnFindNext_clicked();
    }
}

void MainWindow::on_actionGo_to_position_triggered()
{
    bool ok;
    QString text = QInputDialog::getText(this, "Go to position",
                                         "pos:", QLineEdit::Normal,
                                         "0", &ok);
    if (ok && !text.isEmpty())
    {
        long pos = text.toLong(&ok, 10);
        if(ok)
        {
            QTextCursor c = currentEditor()->textCursor();
            c.setPosition(pos);
            currentEditor()->setTextCursor(c);
        }
    }
}

void MainWindow::on_actionKalimatManual_triggered()
{
    if(!helpWindowVisible)
        showHelpWindow();
    else
        hideHelpWindow();
}

void MainWindow::on_action_copy_as_html_triggered()
{
    QTextEdit *editor = currentEditor();
    if(editor != NULL)
    {
        //QString html = editor->document()->toHtml();
        //QApplication::clipboard()->setText(html);
        QString program = editor->document()->toPlainText();
        QStringList output;
        syn->highlightToHtml(program, output);
        QApplication::clipboard()->setText(output.join(""));
    }
}

void MainWindow::on_action_copy_as_wiki_triggered()
{
    QTextEdit *editor = currentEditor();
    if(editor != NULL)
    {
        //QString html = editor->document()->toHtml();
        //QApplication::clipboard()->setText(html);
        QString program = editor->document()->toPlainText();
        QStringList output;
        syn->highlightToWiki(program, output);
        QApplication::clipboard()->setText(output.join(""));
    }
}

void MainWindow::on_action_copy_literate_html_triggered()
{
    QTextEdit *editor = currentEditor();
    if(editor != NULL)
    {
        //QString html = editor->document()->toHtml();
        //QApplication::clipboard()->setText(html);
        QString program = editor->document()->toPlainText();
        QStringList output;
        syn->highlightLiterateHtml(program, output);
        QApplication::clipboard()->setText(output.join(""));
    }
}

void MainWindow::on_action_autoFormat_triggered()
{
    QTextEdit *editor = currentEditor();
    if(editor != NULL)
    {
        //QString html = editor->document()->toHtml();
        //QApplication::clipboard()->setText(html);
        QString program = editor->document()->toPlainText();
        KalimatLexer lxr;
        //KalimatParser parser;
        KalimatPrettyprintParser parser;
        SimpleCodeFormatter fmt;
        try
        {
            parser.init(program, &lxr, NULL);
            shared_ptr<AST> tree = parser.parse();
                tree->prettyPrint(&fmt);
            editor->setText(fmt.getOutput());
        }
        catch(UnexpectedCharException ex)
        {
        }
        catch(UnexpectedEndOfFileException ex)
        {
        }
        catch(ParserException ex)
        {
        }
    }
}

void MainWindow::Break(QString methodName, int offset, Frame *frame, Process *process)
{
    this->setWindowTitle(QString::fromStdWString(L"وصلت لنقطة توقف!!"));
    this->activateWindow();
    atBreak = true;

    //highlightRunningInstruction(frame, QColor(255, 0,0));

    CodeDocument *doc;
    int line;
    if(debugInfo.lineFromInstruction(methodName, offset, doc, line))
    {
        stoppedAtBreakPoint = Breakpoint(doc, line);
        MyEdit *editor = (MyEdit*) doc->getEditor();
        highlightLine(editor, editor->startPosOfLine(line));
    }
    else
    {
        QString msg = QString("Instruction [%1:%2] has no corresponding line").arg(methodName).arg(offset);
        this->setWindowTitle(msg);
    }
}

void MainWindow::genericStep(Process *proc, StepStopCondition &cond)
{
    if(!currentDebuggerProcess)
        return;
    VM *vm = stoppedRunWindow->getVM();
    if(vm->processIsFinished(currentDebuggerProcess))
        return;

    CodeDocument *doc;
    int line;
    Frame *frame;

    int offset;
    QString method;

    vm->getCodePos(currentDebuggerProcess, method, offset, frame);
    if(!debugInfo.lineFromInstruction(method, offset, doc, line))
    {
        CodePosition p = getPositionOfRunningInstruction(frame);
        if(p.doc == NULL)
            return;
        doc = p.doc;
        line = p.ast->getPos().Line;
    }
    // دايزر! انطلق
    vm->reactivate();
    atBreak = false;
    while(true)
    {
        if(vm->processIsFinished(currentDebuggerProcess))
            break;
        stoppedRunWindow->singleStep(currentDebuggerProcess);

        CodeDocument *doc2;
        int line2;
        Frame *frame2;
        vm->getCodePos(proc, method, offset, frame2);

        if(!debugInfo.lineFromInstruction(method, offset, doc2, line2))
        {
            CodePosition p = getPositionOfRunningInstruction(frame2);
            if(p.doc == NULL)
                break;
            doc2 = p.doc;
            line2 = p.ast->getPos().Line;
        }

        if(cond.stopNow(doc, line, frame, doc2, line2, frame2))
        {
            Break(method, offset, frame2, currentDebuggerProcess);
            break;
        }
        doc = doc2;
        line = line2;
        frame = frame2;
    }
}

void MainWindow::step(Process *proc)
{

    struct StopOnOtherLine : public StepStopCondition
    {
        bool stopNow(CodeDocument *doc1, int line1, Frame *frame1, CodeDocument *doc2, int line2, Frame *frame2)
        {
            return (line1 != line2 || frame1 != frame2 || doc1 != doc2);
        }
    } cond;
    genericStep(proc, cond);
}

void MainWindow::setDebuggedProcess(Process *p)
{
    currentDebuggerProcess = p;
}

void MainWindow::programStopped(RunWindow *rw)
{
    currentDebuggerProcess = NULL;
}

void MainWindow::on_action_breakpoint_triggered()
{
    CodeDocument *doc = docContainer->getCurrentDocument();
    MyEdit *editor = (MyEdit *) currentEditor();
    int line = editor->line();

    Breakpoint it = Breakpoint(doc, line);
    bool enabled = false;

    if(breakPoints.contains(it))
    {
        breakPoints.remove(it);
    }
    else
    {
        breakPoints.insert(it);
        enabled = true;
    }

    if(enabled)
    {
        highlightLine(editor, editor->textCursor().position(), QColor(170, 170, 170));
    }
    else
    {
        removeLineHighlights(editor, line);
    }
}

void MainWindow::on_action_resume_triggered()
{
    try
    {
        if(!stoppedRunWindow || !stoppedRunWindow->isVisible() || !this->atBreak)
        {
            this->on_mnuProgramRun_triggered();
            return;
        }
        if(!currentDebuggerProcess)
            return;

        MyEdit *editor = (MyEdit *)stoppedAtBreakPoint.doc->getEditor();
        setWindowTitle(QString::fromStdWString(L"كلمات"));
        removeLineHighlights(editor, stoppedAtBreakPoint.line);
        highlightLine(editor, editor->textCursor().position(), QColor(170, 170, 170));
        atBreak = false;
        stoppedRunWindow->resume();
        stoppedRunWindow->reactivateVM();
        stoppedRunWindow->singleStep(currentDebuggerProcess);
        stoppedRunWindow->setBreakpoint(stoppedAtBreakPoint, debugInfo);
        stoppedRunWindow->Run();
    }
    catch(VMError err)
    {
    }
}

void MainWindow::on_action_step_triggered()
{
    step(currentDebuggerProcess);
}

void MainWindow::on_action_step_procedure_triggered()
{

    if(!currentDebuggerProcess)
        return; // We're not actually debugging a program
    VM *vm = stoppedRunWindow->getVM();

    CodeDocument *doc;
    int line;
    Frame *frame;

    int offset;
    QString method;

    if(vm->getCodePos(currentDebuggerProcess, method, offset, frame))
    {

        if(frame->currentMethod->Get(frame->ip).opcode == Ret)
        {
            step(currentDebuggerProcess);
            return;
        }
        // Todo: we should check frame level equality instead of actual
        // frame equality in order to work well with tail-call elimination
        struct StopOnOtherLineSameFrameLevel : public StepStopCondition
        {
            Frame *originalFrame;
            bool stopNow(CodeDocument *doc1, int line1, Frame *frame1, CodeDocument *doc2, int line2, Frame *frame2)
            {
                return ((line1 != line2)  && (frame2 == originalFrame));
            }
        } cond;
        cond.originalFrame = frame;
        genericStep(currentDebuggerProcess, cond);
    }
}

void MainWindow::on_actionMake_exe_triggered()
{
    QFileDialog saveDlg;
    QStringList filters;
    filters << "Executable file (*.exe)";
    saveDlg.setNameFilters(filters);
    saveDlg.setDefaultSuffix("exe");
    QString selFilter = "Executable file (*.exe)";
    QString targetFile = saveDlg.getSaveFileName(this,
                                                 QString::fromStdWString(L"اختر مكان واسم الملف التنفيذي"),
                                                 "", "Executable file (*.exe)",
                                                 &selFilter);

    CodeDocument *doc = NULL;

    // QString stagingArea = "/home/samy/Desktop/kalimatstagingarea";
    // QString stagingArea = "c:/Users/samy/Desktop/stagingarea";

    try
    {
        currentEditor()->setExtraSelections(QList<QTextEdit::ExtraSelection>());
        saveAll();
        ui->outputView->clear();
        doc = docContainer->getCurrentDocument();
        Compiler compiler(docContainer);

        QString output;

        QString path;
        if(doc->isDocNewFile() || doc->isFileDirty())
        {
            output = compiler.CompileFromCode(doc->getEditor()->document()->toPlainText(), doc);
        }
        else
        {
            output = compiler.CompileFromFile(doc->getFileName(), doc);
        }

        if(doc->isDocNewFile())
        {
            path = "";
        }
        else
        {
            path = doc->getFileName();
        }
        //ui->outputView->append(output);

        QString stagingArea = qApp->applicationDirPath() + "/stagingarea";

        QStringList strConstants;

        for(QMap<QString, QString>::const_iterator i = compiler.generator.getStringConstants().begin()
            ; i!=compiler.generator.getStringConstants().end(); ++i)
        {
            QString sym = base64encode(i.value());
            QString data = base64encode(i.key());
            strConstants.append(QString("SmallVMAddStringConstant('%1','%2')").arg(sym).arg(data));
        }
        QString bb = compiler.generator.getOutput();

        QString baha = base64encode(bb);
        QStringList segments;
        while(baha.length() > 0)
        {
            segments.append("'" + baha.left(80)+"'");
            baha = baha.mid(80);
        }
        //                        {$APPTYPE GUI}


        QString pascalProgram = QString(" {$APPTYPE GUI} {$LONGSTRINGS ON} \n\
                                        program RunSmallVM;\n\
        procedure RunSmallVMCodeBase64(A:PChar;B:PChar);stdcall ;external 'smallvm.dll';\n\
        procedure SmallVMAddStringConstant(A:PChar; B:PChar); stdcall; external 'smallvm.dll';\n\
                                        begin\n\
                %1\n%2\n\
                RunSmallVMCodeBase64(argv^,%3);\n\
                            end.")
                .arg(strConstants.join(";\n"))
                .arg(strConstants.count() >0? ";\n":"")
                .arg(segments.join("+"));


        QString fn = doc ? doc->getFileName() : "untitled";
        fn = QFileInfo(fn).fileName();
        QString pasFileName = stagingArea + "/" + fn + ".pas";
        QString oFileName = stagingArea + "/" + fn + ".o";
        QString aFileName = stagingArea + "/libimp" + fn + ".a";
        QString exeFileName = stagingArea + "/" + fn + ".exe";
        QFile pascal(pasFileName);

        pascal.open(QIODevice::WriteOnly|QIODevice::Text);
        QTextStream p(&pascal);
        p.setCodec("UTF-8");
        p << pascalProgram;
        pascal.close();

        QProcess fpc;

        QStringList argz;
        argz.append(pasFileName);
        fpc.start(stagingArea + "/fpc.exe", argz);
        fpc.waitForFinished(10000);
        bool success = false;
        if(fpc.exitCode() == 0)
        {
            success = QFile::copy(exeFileName, targetFile);
            QFile::remove(exeFileName);
            QFile::remove(pasFileName);
            QFile::remove(oFileName);
            QFile::remove(aFileName);
        }
        if(!success)
        {
            QMessageBox box(QMessageBox::Information, QString::fromStdWString(L"عمل ملف تنفيذي"),
                                  QString::fromStdWString(L"خطأ في عمل الملف التنفيذي"));
            box.exec();
        }
        else
        {
            QMessageBox box(QMessageBox::Information, QString::fromStdWString(L"عمل ملف تنفيذي"),
                                  QString::fromStdWString(L"تم عمل الملف التنفيذي. لكي يعمل برنامجك يجب أن تضع معه الملفات الموجودة في الفهرس المسمى dll المتفرع من مجلد كلمات على جهازك"));
            box.exec();
        }
    }
    catch(UnexpectedCharException ex)
    {
        //show_error(ex->buildMessage());
        show_error(QString::fromStdWString(L"لا يمكن كتابة هذا الرمز هنا '%1'").arg(ex.getCulprit()));
        if(doc != NULL)
        {
            CodeDocument *dc = doc;
            highlightLine(dc->getEditor(), ex.getLine());
        }
    }
    catch(UnexpectedEndOfFileException ex)
    {
        //show_error("Unexpected end of file");
        //show_error(QString::fromStdWString(L"خطأ في تركيب البرنامج"));
        show_error(QString::fromStdWString(L"انتهى البرنامج قبل أن يكون له معنى"));
    }
    catch(ParserException ex)
    {
        //show_error(ex->message);
        show_error(QString::fromStdWString(L"خطأ في تركيب البرنامج"));
        if(doc != NULL && ex.hasPosInfo)
        {
            CodeDocument *dc = doc;
            if(ex.pos.tag != NULL)
            {
                dc = (CodeDocument *) ex.pos.tag;
            }
            highlightLine(dc->getEditor(), ex.pos.Pos);
        }
    }
    catch(CompilerException ex)
    {
       show_error(ex.getMessage());
       // show_error(QString(L"خطأ في تركيب البرنامج"));
       if(doc != NULL)
       {
           CodeDocument *dc = doc;
           if(ex.source->getPos().tag != NULL)
           {
               dc = (CodeDocument *) ex.source->getPos().tag;
           }
           highlightLine(dc->getEditor(), ex.source->getPos().Pos);
       }
    }

}

void MainWindow::makeDrag()
{
    QDrag *dr = new QDrag(this);
     // The data to be transferred by the drag and drop operation is contained in a QMimeData object
     QMimeData *data = new QMimeData;
     data->setText("This is a test");
     // Assign ownership of the QMimeData object to the QDrag object.
     dr->setMimeData(data);
     // Start the drag and drop operation
     dr->start();
}

void MainWindow::dragMoveEvent(QDragMoveEvent *de)
{
    // The event needs to be accepted here
    de->accept();
}

void MainWindow::dropEvent(QDropEvent *event)
{
    // Unpack dropped data and handle it the way you want
    QList<QUrl> urlList =  event->mimeData()->urls();
    QStringList fileNames;

    int len = urlList.size();
    for (int i = 0; i<len; i++)
    {
        qDebug() << "Formats:"<<  urlList.at(i).toLocalFile();
        fileNames << urlList.at(i).toLocalFile();
    }

    docContainer->OpenExistingFiles(fileNames);
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
    // Set the drop action to be the proposed action.
     if (event->mimeData()->hasUrls())
        event->acceptProposedAction();
     else
         qDebug("Error: Only Files can be dragged to this window");
}


bool MainWindow::eventFilter(QObject *sender, QEvent *event)
{
    if (event->type() == QEvent::KeyPress) {

        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        qDebug("Filter - key pressed !! %d", keyEvent->key());
        if (keyEvent->key() == Qt::Key_Enter || keyEvent->key() == Qt::Key_Return)
        {
            if (sender == ui->txtSearchString)
                this->on_btnFindNext_clicked();
            else if (sender == ui->txtReplacementString)
                this->on_btnReplaceNext_clicked();
            return true;
        }
        else if (keyEvent->key() == Qt::Key_Escape)
        {
            ui->dockSearchReplace->hide();
            return true;
        }
        else
            return false;
    }
    return false;
}
