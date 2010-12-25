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
#include <QInputDialog>
#include <QCloseEvent>
#include <QSplitter>
#include <QVBoxLayout>
#include <QGraphicsTextItem>
#include <QDir>
#include <QToolBar>

MainWindow *MainWindow::that = NULL;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    MainWindow::that = this;
    ui->setupUi(this);

    QToolBar *notice = new QToolBar("");
    notice->addAction(QString::fromStdWString(L"هذه هي النسخة الأولية لشهر ديسمبر 2010. حمل أحدث نسخة من www.kalimat-lang.com"));

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

    docContainer->addDocument(QString::fromWCharArray(L"بدون عنوان"), "untitled", CreateEditorWidget(), true);
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
        AST * tree = parser.parse();
        ui->outputView->clear();
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
        RunWindow *rw = new RunWindow(this, path);
        connect(this, SIGNAL(destroyed(QObject*)), rw, SLOT(parentDestroyed(QObject *)));
        rw->show();
        rw->Init(output, compiler.generator.getStringConstants());
    }
    catch(UnexpectedCharException ex)
    {
        //show_error(ex->buildMessage());
        show_error(QString::fromStdWString(L"خطأ في تركيب البرنامج"));
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
    catch(VMError err)
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
    ui->editorTabs->setCurrentWidget(editor);
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

void MainWindow::handleVMError(VMError err)
{
    if(!err.callStack.empty())
    {
        Frame f = err.callStack.top();
        Instruction i = f.getPreviousRunningInstruction();
        int key = i.extra;
        CodePosition p = PositionInfo[key];
        visualizeCallStack(err.callStack, ui->graphicsView);
        if(p.doc != NULL)
        {
            // p.doc can be null if the source of the error is from a module
            // that's compiled from a file but not loaded into the editor.
            // todo: later we would create a CodeDocument * object and pass it to the
            // compiler even for modules not open in the editor, that way we can be like
            // Visual Studio (tm) and open the error-source file on the fly and
            // hilight the problem when there's an error!
            QTextEdit *editor = p.doc->getEditor();

            setWindowTitle(QString("Error position index=%1, file=%2").arg(key).arg(p.doc->getFileName()));
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
            ui->lblFindStatus->setText(QString::fromStdWString(L"وصلنا لبداية الملف"));
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
            ui->lblFindStatus->setText(QString::fromStdWString(L"وصلنا لنهاية الملف"));
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
        ui->lblFindStatus->setText("");
        QString searchStr = ui->txtSearchString->text();
        // See the comment in on_btnReplaceNext_clicked() to explain
        // why we clear selection first
        QTextCursor c =editor->textCursor();
        c.setPosition(c.selectionStart());
        editor->setTextCursor(c);
        editor->textCursor().clearSelection();

        bool result = editor->find(searchStr, QTextDocument::FindBackward);
        if(result)
        {
            editor->textCursor().insertText(ui->txtReplacementString->text());
            int n =  ui->txtReplacementString->text().length();
            QTextCursor c = editor->textCursor();
            c.movePosition(QTextCursor::PreviousCharacter, QTextCursor::KeepAnchor, n);
            editor->setTextCursor(c);
        }
        else
        {
            ui->lblFindStatus->setText(QString::fromStdWString(L"وصلنا لبداية الملف"));
        }
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

        QTextCursor c =editor->textCursor();
        c.setPosition(c.selectionStart());
        editor->setTextCursor(c);
        editor->textCursor().clearSelection();
        ui->lblFindStatus->setText("");
        QString searchStr = ui->txtSearchString->text();
        bool result = editor->find(searchStr);
        if(result)
        {
            editor->textCursor().insertText(ui->txtReplacementString->text());
            int n =  ui->txtReplacementString->text().length();
            QTextCursor c = editor->textCursor();
            c.movePosition(QTextCursor::PreviousCharacter, QTextCursor::KeepAnchor, n);
            editor->setTextCursor(c);

        }
        else
        {
            ui->lblFindStatus->setText(QString::fromStdWString(L"وصلنا لنهاية الملف"));
        }
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

void MainWindow::on_action_software_update_triggered()
{

}
