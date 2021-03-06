/**************************************************************************
**   The Kalimat programming language
**   Copyright (C) 2010 Mohamed Samy Ali - samy2004@gmail.com
**   This project is released under the Apache License version 2.0
**   as described in the included license.txt file
**************************************************************************/

#include "Lexer/lexer_incl.h"
#include "Lexer/kalimatlexer.h"

#include "Parser/parser_incl.h"
#include "Parser/KalimatAst/kalimatast_incl.h"
#include "Parser/KalimatAst/kalimat_ast_gen.h"
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
#include "settingsdlg.h"
#include "aboutdlg.h"
#include "makeexedlg.h"
#include "mytooltip.h"

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
#include <QWheelEvent>
#include <QScrollBar>
#include <QDesktopServices>
#include <QtConcurrentRun>
#include <QToolTip>
#include <QIcon>
#include <QSqlQuery>
#include <QImageWriter>

MainWindow *MainWindow::that = NULL;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      codeAnalyzer(Ide::msg),
      functionNavigationCombo(NULL),
      ui(new Ui::MainWindow)
{
    MainWindow::that = this;
    VM::InitGlobalData();
    BuiltInTypes::init();
    helpWindowVisible = false;
    helpSplitter = NULL;

    ui->setupUi(this);
#ifdef ENGLISH_PL
    setLayoutDirection(Qt::LeftToRight);
    ui->menuBar->setLayoutDirection(Qt::LeftToRight);
    ui->dockSearchReplace->setLayoutDirection(Qt::LeftToRight);
    ui->menuBar->removeAction(ui->mnuSpecialSymbol->menuAction());
#endif
    settingsOrganizationName = "mohamedsamy";
    settingsApplicationName = "kalimat 1.0";

    QSettings settings(settingsOrganizationName, settingsApplicationName, this);

    QString here = QCoreApplication::applicationDirPath() + "/stdlib/";
    this->standardModulePath = settings.value("standard_module_path",
                                              here).toString();

    QString editorFontName = settings.value("editor_font_name",
                                            this->font().family()).toString();

    this->editorFontSize = settings.value("editor_font_size", 18).toInt();

    editorFont = QFont(editorFontName, editorFontSize);
    this->codeModelUpdateInterval = settings.value("codemodel_update_interval", 5000).toInt();

    this->isDemoMode = settings.value("is_demo_mode", false).toBool();

    QToolBar *notice = new QToolBar("");
    notice->addAction(
                Ide::msg[IdeMsg::UpdateBanner],
            this,
            SLOT(do_goto_kalimatlangdotcom_triggered())
            );
    insertToolBarBreak(ui->functionNavigationToolbar);
    insertToolBar(ui->functionNavigationToolbar, notice);
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
    ui->mnuSpecialSymbol->setVisible(false);
    docContainer = new DocumentContainer(settingsOrganizationName,
                                         settingsApplicationName,
                                         tr("Kalimat code (*.k *.* *)"),
                                         ui->editorTabs,
                                         this,
                                         MaxRecentFiles,
                                         ui->mnuRecentFiles_2,
                                         Ide::msg);

    QSplitter *splitter = new QSplitter(Qt::Vertical, this);
    splitter->addWidget(ui->editorTabs);
    splitter->addWidget(ui->tabWidget);
    setCentralWidget(splitter);

    splitter->setStretchFactor(0, 4);
    splitter->setStretchFactor(1, 0);

    this->setWindowTitle(Ide::msg[IdeMsg::Kalimat]);
    this->showMaximized();
    shouldHideFunctionTooltip = false;
    docContainer->addInitialEmptyDocument();

    stoppedRunWindow = NULL;
    atBreak = false;
    currentDebuggerProcess = NULL;
    lastCodeDocToRun = NULL;
    _isWonderfulMonitorEnabled = false;

    setAcceptDrops(true);

    ui->editorTabs->setAcceptDrops(true);
    ui->txtSearchString->installEventFilter(this);
    ui->txtReplacementString->installEventFilter(this);
    parserErrorMap = Utils::prepareErrorMap<KalimatParserError>(":/parser_error_map.txt");

    //functionNavigationCombo = new QComboBox(this);
    functionNavigationComboIsUpdating = false;
    ui->functionNavigationToolbar->hide();
    ui->functionNavigationToolbar->addWidget(functionNavigationCombo);
    //connect(functionNavigationCombo, SIGNAL(currentIndexChanged(int)), SLOT(do_functionNavigationCombo_currentIndexChanged(int)));
    //ui->functionNavigationToolbar->show();
    generatingProgramModel = false;
    // codeModelUpdateTimerId = startTimer(codeModelUpdateInterval);

    connect(this, SIGNAL(markCurrentInstructionEvent(VM*,Process*,int*,int*)),
            this, SLOT(markCurrentInstructionSlot(VM*,Process*,int*,int*)),
            Qt::BlockingQueuedConnection);

    connect(this, SIGNAL(breakEvent(BreakSource::Src,int,Frame*,Process*)),this,
            SLOT(breakSlot(BreakSource::Src,int,Frame*,Process*)));
    currentStepStopCondition = NullaryStepStopCondition::instance();

    // No .exes for other operating systems for now...
#ifndef Q_OS_WIN
    ui->actionMake_exe->setVisible(false);
#endif
    ui->action_go_to_definition->setVisible(false);
}

void MainWindow::outputMsg(QString s)
{
    ui->outputView->append(s);
}

QString MainWindow::translateParserError(ParserException ex)
{
    QString message = ex.message;
    if(ex.hasType())
    {
        message = parserErrorMap[(KalimatParserError)ex.errType()];
        if(ex.hasPosInfo)
        {
            message += Ide::msg.get(IdeMsg::TokenPos3, str(ex.pos.Line), str(ex.pos.Column), str(ex.pos.Pos));
        }
    }
    return message;
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
    CodeDocument *doc = docContainer->getCurrentDocument();
    if(doc == NULL)
        return NULL;
    return doc->getEditor();
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

    edit->connect(edit,SIGNAL(linkClickedEvent(MyEdit*,QString)), this,SLOT(do_editor_linkClicked(MyEdit*,QString)));

    syn = new SyntaxHighlighter(edit->document(), new KalimatLexer(), Ide::msg);
    edit->textCursor().setVisualNavigation(true);

    edit->setFont(editorFont);
    edit->updateLineNumberAreaFont();
    // If the next line wasn't there, the syntaxhilighter sends a document modified
    // event on the initial display of the text editor, causing the document
    // to be initially 'dirty'.
    edit->setPlainText("");
    edit->viewport()->installEventFilter(edit);

    return edit;
}

void MainWindow::setLineIndicators(int line, int column, QTextEdit *editor)
{
    lblEditorCurrentLine->setText(Ide::msg.get(IdeMsg::Line1, str(line)));
    lblEditorCurrentColumn->setText(Ide::msg.get(IdeMsg::Column1, str(column)));

    MyEdit *ed = (MyEdit *) editor;

    setFunctionNavigationComboSelection(editor);
    //if(!QToolTip::isVisible())
    //    shouldHideFunctionTooltip = false;
    if(shouldHideFunctionTooltip)
    {
        bool success = false;
        Token t2;
        Token t1 = ed->getMatchingParen(funcToolTipParenTokenIndex,t2);
        int minz;
        int maxz;
        if(!isOpenParen(t2.Lexeme))
        {
            // Actually no open paren under cursor,
            // so no func(
            // so no need to show tooltip
            success = true;
        }
        // if t1 is invalid, the function call still has no
        // closing paren: it is func(...
        // we will keep the tooltip as long as the cursor is
        // between the ( and the end of document
        else if(t1.Type == TokenInvalid)
        {
            minz = t2.Pos;
            maxz = ed->document()->toPlainText().length();
            int pos = ed->textCursor().position();
            if(pos >maxz
                    || pos <=minz)
            {
                success = true;
            }
        }
        else if(t1.Type != TokenInvalid)
        {
            // there's a matching paren, check if
            // cursor's not within range
            minz = min(t1.Pos, t2.Pos);
            maxz = max(t1.Pos + t1.Lexeme.count(),
                       t2.Pos + t2.Lexeme.count());
            int pos = ed->textCursor().position();
            // pos+1 because the closing )
            // hasn't been entered yet

            if(pos+1>maxz
                    || pos <=minz)
            {
                success = true;
            }
        }
        if(success)
        {
            MyToolTip::hideText();
            shouldHideFunctionTooltip = false;
        }
    }
}

void MainWindow::setFunctionNavigationComboSelection(QTextEdit *editor)
{
    if(!functionNavigationCombo)
        return;
    int pos = editor->textCursor().position();
    QSqlQuery q = progdb.q("SELECT name FROM definitions JOIN function_definitions "
                           "ON definitions.id=function_definitions.def_id AND "
                           " definitions.module_filename=function_definitions.module_filename "
                           " WHERE ? >= defining_token_pos AND ? <= ending_token_pos;", pos, pos);
    if(q.next())
    {
        QString d = q.value(0).toString();
        functionNavigationComboIsUpdating = true;
        functionNavigationCombo->setCurrentIndex(
                    functionNavigationCombo->findText(q.value(0).toString()));
        functionNavigationComboIsUpdating = false;
    }
    else
    {
        if(functionNavigationCombo->count() > 0)
            functionNavigationCombo->setCurrentIndex(0);
    }
}

void MainWindow::timerEvent(QTimerEvent *event)
{
    on_actionUpdate_code_model_triggered();
}

shared_ptr<CompilationUnit> MainWindow::parseCurrentDocumentWithRecovery(QVector<Token> &tokens)
{
    KalimatLexer lxr;
    KalimatParser parser;
    shared_ptr<CompilationUnit> ret;
    if(!currentEditor())
        return ret;
    try
    {
        parser.withRecovery = true;
        parser.init(currentEditor()->document()->toPlainText(), &lxr, NULL);
        tokens = parser.getTokens();
        shared_ptr<AST> tree = parser.parse();
        ret = dynamic_pointer_cast<CompilationUnit>(
                    tree);

    }
    catch(UnexpectedCharException ex)
    {
        //ui->outputView->append(ex.buildMessage());
    }
    catch(UnexpectedEndOfFileException ex)
    {
        //ui->outputView->append("Unexpected end of file");
    }
    catch(ParserException ex)
    {
        //QString msg = translateParserError(ex);
        //ui->outputView->append(msg);
    }
    return ret;
}

void MainWindow::on_actionLexize_triggered()
{
    KalimatLexer lxr;

    lxr.init(currentEditor()->document()->toPlainText());
    try
    {
        ui->tabWidget->setCurrentWidget(ui->outputView);
        long t1 = get_time();
        lxr.tokenize();
        QVector<Token> tokens = lxr.getTokens();
        ui->outputView->clear();
        clock_t t2 = get_time();
        double secs = (double)(t2-t1) / 1000000;
        ui->outputView->append(QString("Compiled in %1 seconds.").arg(secs));
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
        clock_t t1= clock();
        parser.init(currentEditor()->document()->toPlainText(), &lxr, NULL);
        shared_ptr<AST> tree = parser.parse();
        clock_t t2 = clock();
        double secs = (double)(t2-t1) / CLOCKS_PER_SEC;
        ui->outputView->append(QString("Compiled in %1 seconds.").arg(secs));
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
        QString msg = translateParserError(ex);
        ui->outputView->append(msg);
    }
}

void MainWindow::on_actionParse_with_recovery_triggered()
{
    KalimatLexer lxr;
    KalimatParser parser;

    try
    {
        ui->tabWidget->setCurrentWidget(ui->outputView);
        clock_t t1 = clock();
        parser.withRecovery = true;
        parser.init(currentEditor()->document()->toPlainText(), &lxr, NULL);

        shared_ptr<AST> tree = parser.parse();
        //ui->outputView->clear();
        clock_t t2 = clock();
        double secs = (double)(t2-t1) / CLOCKS_PER_SEC;
        ui->outputView->append(QString("Compiled in %1 seconds.").arg(secs));
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
        QString msg = translateParserError(ex);
        ui->outputView->append(msg);
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
        clock_t t1 = clock();
        Compiler compiler(docContainer, standardModulePath);

        QString output;

        if(doc->isDocNewFile() || doc->isFileDirty())
            output = compiler.CompileFromCode(doc->getEditor()->document()->toPlainText(), doc);
        else
            output = compiler.CompileFromFile(doc->getFileName(), NULL);
        clock_t t2 = clock();
        double secs = (double)(t2-t1) / CLOCKS_PER_SEC;
        ui->outputView->append(QString("Compiled in %1 seconds.").arg(secs));
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
        QString msg = translateParserError(ex);

        ui->outputView->append(QString("File %1:\t%2").arg(fn).arg(msg));
    }
    catch(CompilerException ex)
    {
        show_error(ex.getMessage());

        if(doc != NULL)
        {
            CodeDocument *dc = NULL;

            if(ex.source && ex.source->getPos().tag != NULL)
            {
                dc = (CodeDocument *) ex.source->getPos().tag;
            }
            else if(ex.source)
            {
                dc = docContainer->getDocumentFromPath(ex.fileName, true);
            }
            if(dc)
            {
                highlightLine(dc->getEditor(), ex.source->getPos().Pos);
            }
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
        Compiler compiler(docContainer, standardModulePath);

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
        QString msg = translateParserError(ex);
        ui->outputView->append(msg);
    }
    catch(CompilerException ex)
    {
        show_error(ex.getMessage());

        if(doc != NULL)
        {
            CodeDocument *dc = NULL;

            if(ex.source && ex.source->getPos().tag != NULL)
            {
                dc = (CodeDocument *) ex.source->getPos().tag;
            }
            else if(ex.source)
            {
                dc = docContainer->getDocumentFromPath(ex.fileName, true);
            }
            if(dc)
            {
                highlightLine(dc->getEditor(), ex.source->getPos().Pos);
            }
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
        QMessageBox box(QMessageBox::Information, Ide::msg[IdeMsg::CannotRunProgram],
                Ide::msg[IdeMsg::CannotRunNewProgramAtMiddleOfBreakPoint]);
        box.exec();
        return;
    }
    try
    {
        currentEditor()->setExtraSelections(QList<QTextEdit::ExtraSelection>());
        saveAll();
        ui->outputView->clear();
        doc = docContainer->getCurrentDocument();
        Compiler compiler(docContainer, standardModulePath);

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
        rw->paintSurface->showCoordinates = isDemoMode;

        connect(this, SIGNAL(destroyed(QObject*)), rw, SLOT(parentDestroyed(QObject *)));

        stoppedRunWindow = rw;
        atBreak = false;
        lastCodeDocToRun = doc;
        rw->show();

        //KalimatLexer lxr;
        //KalimatParser parser;
        //parser.init(doc->getEditor()->document()->toPlainText(), &lxr, doc);
        //rw->parseTree = dynamic_pointer_cast<KalimatAst>(parser.parse());
        rw->Init(output, compiler.generator.getStringConstants(), breakPoints, debugInfo);

    }
    catch(UnexpectedCharException ex)
    {
        //show_error(ex->buildMessage());
        CodeDocument *dc = NULL;
        if(ex.getCulprit() == "<EOF>")
        {
            show_error(Ide::msg[IdeMsg::UnexpectedEof]);
        }
        else
        {
            show_error(Ide::msg.get(IdeMsg::UnexpectedToken1, ex.getCulprit()));
        }
        if(QFile::exists(ex.fileName))
            dc = docContainer->getDocumentFromPath(ex.fileName, true);
        if(dc != NULL)
        {
            highlightLine(dc->getEditor(), ex.getPos());
        }
    }
    catch(UnexpectedEndOfFileException ex)
    {
        show_error(Ide::msg[IdeMsg::UnexpectedEof]);
    }
    catch(ParserException ex)
    {
        // QString message = translateParserError(ex);
        //show_error(ex->message);
        show_error(Ide::msg[IdeMsg::SyntaxError]);
        if(doc != NULL && ex.hasPosInfo)
        {
            CodeDocument *dc = NULL;
            if(ex.pos.tag == NULL)
            {
                dc = docContainer->getDocumentFromPath(ex.fileName, true);
            }
            if(ex.pos.tag != NULL)
            {
                dc = (CodeDocument *) ex.pos.tag;
            }
            if(dc)
            {
                highlightLine(dc->getEditor(), ex.pos.Pos);
            }
        }
    }
    catch(CompilerException ex)
    {
        if(ex.getError() == CannotRunAModule)
        {
            if(lastCodeDocToRun && docContainer->hasOpenDocument(lastCodeDocToRun))
            {
                QString message = Ide::msg.get(IdeMsg::CannotExecuteUnit2, doc->getFileName(), lastCodeDocToRun->getFileName());
                QMessageBox box(QMessageBox::Question,
                                Ide::msg[IdeMsg::CannotExecuteUnit],
                        message,
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

        if(doc != NULL)
        {
            CodeDocument *dc = NULL;

            if(ex.source && ex.source->getPos().tag != NULL)
            {
                dc = (CodeDocument *) ex.source->getPos().tag;
            }
            else if(ex.source)
            {
                dc = docContainer->getDocumentFromPath(ex.fileName, true);
            }
            if(dc)
            {
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
    // todo: doesn't highlight whole line when
    // the file is empty (e.g when using a module that's an empty file)
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
    return _isWonderfulMonitorEnabled;
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

void MainWindow::visualizeCallStacks(QSet<QQueue<Process *> *> callStacks, QGraphicsView *view)
{
    // todo: Visualize the call stacks; as the function name says :(
    if(callStacks.empty())
        return;
    for(QSet<QQueue<Process *> *>::const_iterator i = callStacks.begin(); i!=callStacks.end(); ++i)
    {
        const QQueue<Process *> * q = (*i);
        if(!q->empty())
            visualizeCallStack(q->front()->stack, view);
        break; // only the first call stack for now
    }
}

void MainWindow::visualizeCallStack(Frame *callStack, QGraphicsView *view)
{
    QGraphicsScene *scene = new QGraphicsScene();

    float left = 0.0f;
    Frame *f = callStack;
    while(f != NULL)
    {
        QString frepr;
        frepr = "<" + f->currentMethod->getName() + ">\n";
        for(int j=0; j<f->currentMethod->Locals.count(); j++)
        {
            QString var = f->currentMethod->Locals.keys().at(j);
            Value *v = f->local(var);
            if(!v)
                continue;
            QString val = f->local(var)->toString();
            if(var.startsWith("%"))
                continue;
            if(var.length() > 5)
                var = var.mid(0,4) +"-";
            if(val.length() > 10)
                val = val.mid(0, 9) + "-";

            QString out = QString("%1  =  %2").arg(var)
                    .arg(val);
            frepr +=out;
            if(j<f->currentMethod->Locals.count()-1)
                frepr += "\n";
        }

        QGraphicsTextItem *txt = scene->addText(frepr, ui->graphicsView->font());

        QGraphicsRectItem *rct = scene->addRect(txt->boundingRect(), QPen(Qt::black), QBrush(Qt::NoBrush));
        txt->setParentItem(rct);
        rct->setPos(left, 5.0f);
        left += 5.0f + txt->boundingRect().width();

        f = f->next;
    }
#ifndef ENGLISH_PL
    view->setAlignment(Qt::AlignRight| Qt::AlignTop);
    view->setLayoutDirection(Qt::RightToLeft);
#endif
    ui->tabWidget->setCurrentWidget(ui->graphicsView);
    QGraphicsScene *oldScene = view->scene();
    if(oldScene)
        delete oldScene;
    view->setScene(scene);
}

void MainWindow::postMarkCurrentInstruction(VM *vm, Process *proc, int *pos, int *length)
{
    emit markCurrentInstructionEvent(vm, proc, pos, length);
}

void MainWindow::markCurrentInstructionSlot(VM *vm, Process *proc, int *pos, int *length)
{
    markCurrentInstruction(vm, proc, pos, length);
}

void MainWindow::markCurrentInstruction(VM *vm, Process *proc, int *pos, int *length)
{
    if(!proc->isFinished())
    {
        const Instruction &i = proc->getCurrentInstruction();
        // todo: critical: call stack in wonderful monitor
        //visualizeCallStacks(vm->mainScheduler.getCallStacks(), ui->graphicsView);
        visualizeCallStack(vm->getMainProcess()->stack, ui->graphicsView);
        int key = i.extra;
        if(key ==-1)
            return;
        CodePosition p = PositionInfo[key];

        if(p.doc != NULL)
        {
            QTextEdit *editor = p.doc->getEditor();

            ui->editorTabs->setCurrentWidget(editor);
            *pos = p.pos;
            *length = p.ast->getPos().Lexeme.length();
            highlightToken(editor, *pos, *length);
        }
    }
}

void MainWindow::handleVMError(VMError err)
{
    if(err.callStack != NULL)
    {
        Frame *f = err.callStack;
        highlightRunningInstruction(f);
        visualizeCallStack(err.callStack, ui->graphicsView);
    }
}

void MainWindow::highlightRunningInstruction(Frame *f)
{
    highlightRunningInstruction(f, Qt::yellow);
}

void MainWindow::highlightRunningInstruction(Frame *f, QColor clr)
{
    CodePosition p;
    if(!getPositionOfRunningInstruction(p, f, true))
    {
        return;
    }

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

bool MainWindow::getPositionOfRunningInstruction(CodePosition &p, Frame *f, bool previous)
{
    /*
     We can come to a break for one of two reasons:
        1- A stepStopCondition was reached when stepping
        2- Executing a break instruction caused by a breakpoint,

     - In the first case, the current running instruction
     is method[ip-1] because the current instruction has already been executed and ip
     was incremented.

     - In the second case the the 'break' has replaced an existing instruction,
       and execution has gone back a step, so we should not decrement ip
       since it has already been decremented
     */
    try
    {
        const Instruction *i;

        if(previous)
            i = &f->getPreviousRunningInstruction();
        else
            i= &f->getRunningInstruction();

        // const Instruction &i = f->currentMethod->Get(f->ip);
        int key = i->extra;
        p = PositionInfo[key];
        return true;
    }
    catch(VMError err)
    {
        handleVMError(err);
        return false;
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

void MainWindow::show_error(QString message)
{
    QMessageBox box;
    box.setWindowTitle(Ide::msg[IdeMsg::Kalimat]);
    box.setText(message);
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
            ui->lblFindStatus->setText(Ide::msg[IdeMsg::StartOfFileReached]);

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
            ui->lblFindStatus->setText(Ide::msg[IdeMsg::EndOfFileReached]);

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
        QString result = removeExtraSpaces(output.join(""));
        QApplication::clipboard()->setText(result);
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
        QString result = removeExtraSpaces(output.join(""));
        QApplication::clipboard()->setText(result);
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
            QString code = fmt.getOutput();
            // Remove some spurious spaces
            // todo: fix the pretty printing itself
            // instead of this
            QString ac1 = QString::fromStdWString(L" ??");
            QString ac2 = QString::fromStdWString(L"??");
            code = code.replace(" ,", ",")
                    .replace(ac1, ac2)
                    .replace(" (", "(")
                    .replace(" )", ")");
            editor->setText(code);
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

void MainWindow::breakSlot(BreakSource::Src source, int offset, Frame *frame, Process *process)
{
    Break(source, offset, frame, process);
}

void MainWindow::postBreak(BreakSource::Src source, int offset, Frame *frame, Process *process)
{
    emit breakEvent(source, offset, frame, process);
}

void MainWindow::Break(BreakSource::Src source, int offset, Frame *frame, Process *process)
{
    this->setWindowTitle(Ide::msg[IdeMsg::BreakpointReached]);
    //this->activateWindow();
    atBreak = true;
    breakSource = source;
    if(currentStepStopCondition != NullaryStepStopCondition::instance())
    {
        delete currentStepStopCondition;
        currentStepStopCondition = NullaryStepStopCondition::instance();
    }

    //highlightRunningInstruction(frame, QColor(255, 0,0));

    CodeDocument *doc;
    int line;
    QString methodName = frame->currentMethod->getName();
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

void MainWindow::currentBreakCondition(Process *process)
{
    currentStepStopCondition->stopNow(process);
}

void MainWindow::step(Process *proc)
{
    bool prev;
    if(this->breakSource == BreakSource::FromInstruction)
        prev = false;
    else if(this->breakSource == BreakSource::FromStepping)
        prev = true;
    else
    {
        // should be unreachable
        //int dummy = 0;
    }

    CodePosition p;
    if(!getPositionOfRunningInstruction(p, proc->currentFrame(), prev))
        return;

    StepStopCondition *cond = new SingleStepCondition(p.doc, p.ast->getPos().Line, proc->currentFrame(), this);
    genericStep(proc, cond);
}

void MainWindow::stepOver(Process *proc)
{
    bool prev;
    if(this->breakSource == BreakSource::FromInstruction)
        prev = false;
    else
        prev = true;

    CodePosition p;
    if(!getPositionOfRunningInstruction(p, proc->currentFrame(), prev))
        return;

    StepStopCondition *cond = new StepOverCondition(p.doc, p.ast->getPos().Line, proc->currentFrame(), this);
    genericStep(proc, cond);
}

void MainWindow::genericStep(Process *proc, StepStopCondition *cond)
{
    if(!stoppedRunWindow)
        return;
    if(!proc)
        return;
    if(proc->isFinished())
        return;

    currentStepStopCondition = cond;
    // ??????????! ??????????
    atBreak = false;
    stoppedRunWindow->resume();
    stoppedRunWindow->Run();
}

void MainWindow::setDebuggedProcess(Process *p)
{
    currentDebuggerProcess = p;
}

void MainWindow::programStopped(RunWindow *rw)
{
    currentDebuggerProcess = NULL;
    atBreak = false;
    currentStepStopCondition = NullaryStepStopCondition::instance();
    stoppedRunWindow = NULL;
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
        editor->toggleBreakpoint(line);
        //highlightLine(editor, editor->textCursor().position(), QColor(170, 170, 170));
    }
    else
    {
        //removeLineHighlights(editor, line);
        editor->toggleBreakpoint(line);
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
        setWindowTitle(Ide::msg[IdeMsg::Kalimat]);
        removeLineHighlights(editor, stoppedAtBreakPoint.line);
        highlightLine(editor, editor->textCursor().position(), QColor(170, 170, 170));
        atBreak = false;
        stoppedRunWindow->resume();
        stoppedRunWindow->Run();

        // We need to check again since stoppedRunWindow::Run()
        // might encounter and exception and close, thus
        // notifying the MainWindow that execution has
        // stopped and setting stoppedRunWindow to NULL
        if(!stoppedRunWindow)
            return;
        //stoppedRunWindow->singleStep(currentDebuggerProcess);
        stoppedRunWindow->setBreakpoint(stoppedAtBreakPoint, debugInfo);
        //stoppedRunWindow->Run();
    }
    catch(VMError err)
    {
    }
}

void MainWindow::on_action_step_triggered()
{
    if(!currentDebuggerProcess || currentDebuggerProcess->isFinished())
        return;
    step(currentDebuggerProcess);
}

void MainWindow::on_action_step_procedure_triggered()
{
    if(!currentDebuggerProcess || currentDebuggerProcess->isFinished())
        return;
    stepOver(currentDebuggerProcess);
}

void MainWindow::on_actionMake_exe_triggered()
{
    MakeExeDlg dlg(this);
    dlg.exec();
    if(!dlg.accepted)
        return;
    QString targetFile = dlg.targetFilename;
    CodeDocument *doc = docContainer->getCurrentDocument();

    QString fn = doc ? doc->getFileName() : "untitled";
    fn = QFileInfo(fn).fileName();

    //QString kalimatBaseDir = "c:/kalimat-release";
    QString kalimatBaseDir = qApp->applicationDirPath();
    QString stagingArea = kalimatBaseDir + "/stagingarea";

    QString pasFileName = stagingArea + "/" + fn + ".pas";
    QString oFileName = stagingArea + "/" + fn + ".o";
    QString orFileName = stagingArea + "/" + fn + ".or";
    QString aFileName = stagingArea + "/libimp" + fn + ".a";
    QString exeFileName = stagingArea + "/" + fn + ".exe";
    QString icoFileName = stagingArea + "/" + "app.ico";

    if(targetFile == exeFileName)
    {
        QMessageBox box(QMessageBox::Information, "Invalid .exe path",
                        QString("The path '%1' cannot be used as a location for creating .exe files")
                        .arg(stagingArea));
        box.exec();
        return;
    }
    QImage iconImg;

    switch(dlg.iconIndex)
    {
    case 0:
        iconImg.load(":/icons/icons/exe1.ico");
        break;
    case 1:
        iconImg.load(":/icons/icons/exe2.ico");
        break;
    case 2:
        iconImg.load(":/icons/icons/exe3.ico");
        break;
    case 3:
        iconImg.load(dlg.customIconFile);
        break;
    }
    if(QFile::exists(icoFileName))
        QFile::remove(icoFileName);

    // Remove any previous temp. exe from the staging area
    if(QFile::exists(exeFileName))
        QFile::remove(exeFileName);
    iconImg.save(icoFileName);

    try
    {
        currentEditor()->setExtraSelections(QList<QTextEdit::ExtraSelection>());
        saveAll();
        ui->outputView->clear();
        doc = docContainer->getCurrentDocument();
        Compiler compiler(docContainer, standardModulePath);

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

        QStringList strConstants;

        for(QMap<QString, QString>::const_iterator i = compiler.generator.getStringConstants().begin()
            ; i!=compiler.generator.getStringConstants().end(); ++i)
        {
            QString sym = base64encode(i.value());
            QString data = base64encode(i.key());

            QStringList dataSegments = Utils::segmentStringForPascal(data, 200);

            strConstants.append(QString("SmallVMAddStringConstant('%1',%2)")
                                .arg(sym)
                                .arg(dataSegments.join("+")));
        }
        QString vmCode = compiler.generator.getOutput();

        QString vmCode64 = base64encode(vmCode);

        QStringList segments = Utils::segmentStringForPascal(vmCode64, 200);

        //                        {$APPTYPE GUI}
        QString haveProjectRc = QFile::exists(icoFileName)? "{$R project.rc}" : "";

        QString pascalProgram = QString(" {$APPTYPE GUI}  %1 {$LONGSTRINGS ON} \n\
                                        program RunSmallVM;\n\
                procedure RunSmallVMCodeBase64(A:PUnicodeChar;B:PChar);stdcall ;external 'smallvm.dll';\n\
                procedure SmallVMAddStringConstant(A:PChar; B:PChar); stdcall; external 'smallvm.dll';\n\
                begin\n\
                %2\n%3\n\
                RunSmallVMCodeBase64(PUnicodeChar(AnsiToUtf8(ParamStr(0))),%4);\n\
                end.")
                .arg(haveProjectRc)
                .arg(strConstants.join(";\n"))
                .arg(strConstants.count() >0? ";\n":"")
                .arg(segments.join("+"));



        QFile pascal(pasFileName);

        pascal.open(QIODevice::WriteOnly|QIODevice::Text | QIODevice::Truncate);
        QTextStream p(&pascal);
        p.setCodec("UTF-8");
        p << pascalProgram;
        pascal.close();

        QProcess fpc;

        QStringList argz;
        QString problem;
        argz.append(pasFileName);
        if(!QFile::exists(stagingArea + "/fpc.exe"))
        {
            problem = QString::fromStdWString(L"Cannot find fpc.exe");
        }


        fpc.start(stagingArea + "/fpc.exe", argz);
        fpc.waitForFinished(10000);
        bool success = false;


        if(fpc.exitCode() == 0)
        {
            if(!QFile::exists(exeFileName))
            {
                problem = "Failed to compile generated program.";
                problem += " - " + fpc.errorString() + ". ";
                problem += fpc.readAllStandardError();

            }
            else
            {
                bool testCopy = QFile::copy(exeFileName, targetFile);
                if(!testCopy || !QFile::exists(targetFile))
                {
                    problem = "Failed to copy executable to destination";
                }
                else
                {
                    success = true;
                }
            }
        }
        else
        {
            problem = QString("Failed to compile generated program");
            problem += " " + fpc.errorString() + ". ";
            problem += fpc.readAllStandardOutput();
        }
        if(success && dlg.copyDll && dlg.targetPath != kalimatBaseDir)
        {
            QStringList dlls;
            dlls << "smallvm" << "QtCore4" << "QtGui4" << "mingwm10"
                 << "libstdc++-6" << "libgcc_s_dw2-1" << "libffi-5";
            bool overwriteAllDecided = false;
            for(int i=0; i<dlls.count(); ++i)
            {
                QString src = kalimatBaseDir + "/" + dlls[i] + ".dll";
                QString dest = dlg.targetPath + "/" + dlls[i] + ".dll";

                bool overwriteAll = false;

                if(QFile::exists(dest))
                {
                    bool overwrite = false;
                    if(overwriteAllDecided)
                    {
                        overwrite = overwriteAll;
                    }
                    else
                    {
                        QMessageBox box(QMessageBox::Information,
                                        QString::fromStdWString(L"?????????? ?????????? ????????????"),
                                        QString::fromStdWString(L"?????????? '%1' ?????????? ???? ???????????? '%2' ???? ???????? ?????????????? ??????????")
                                        .arg(dlls[i]+ ".dll")
                                        .arg(dlg.targetPath), QMessageBox::Yes|QMessageBox::YesAll|
                                        QMessageBox::No| QMessageBox::NoAll
                                        );
                        box.exec();
                        if(box.result() == QMessageBox::Yes)
                        {
                            overwrite = true;
                        }
                        else if(box.result() == QMessageBox::No)
                        {
                            overwrite = false;
                        }
                        else if(box.result() == QMessageBox::YesToAll)
                        {
                            overwrite = true;
                            overwriteAll = true;
                            overwriteAllDecided = true;
                        }
                        else if(box.result() == QMessageBox::NoToAll)
                        {
                            overwrite = false;
                            overwriteAll = false;
                            overwriteAllDecided = true;
                        }
                    }
                    if(QFile::exists(src) && overwrite)
                    {
                        QFile::remove(dest);
                    }
                }
                QFile::copy(src, dest);
            }
        }

        if(!success)
        {
            QMessageBox box(QMessageBox::Information, Ide::msg[IdeMsg::CreatingExecutable],
                    Ide::msg.get(IdeMsg::ErrorCreatingExecutable1, problem));
            box.exec();
        }
        else
        {
            QMessageBox box(QMessageBox::Information,  Ide::msg[IdeMsg::CreatingExecutable],
                    Ide::msg[IdeMsg::SuccessCreatingExe]);
            box.exec();
        }
    }
    catch(UnexpectedCharException ex)
    {
        //show_error(ex->buildMessage());
        show_error(Ide::msg.get(IdeMsg::UnexpectedToken1, ex.getCulprit()));
        if(doc != NULL)
        {
            CodeDocument *dc = doc;
            highlightLine(dc->getEditor(), ex.getLine());
        }
    }
    catch(UnexpectedEndOfFileException ex)
    {
        show_error(Ide::msg[IdeMsg::UnexpectedEof]);
    }
    catch(ParserException ex)
    {
        //show_error(ex->message);
        show_error(Ide::msg[IdeMsg::SyntaxError]);
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
        //  show_error(msg[IdeMsg::SyntaxError]);
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
    // cleanup
    //*
    if(QFile::exists(exeFileName))
        QFile::remove(exeFileName);
    if(QFile::exists(pasFileName))
        QFile::remove(pasFileName);
    if(QFile::exists(oFileName))
        QFile::remove(oFileName);
    if(QFile::exists(orFileName))
        QFile::remove(orFileName);
    if(QFile::exists(aFileName))
        QFile::remove(aFileName);
    if(QFile::exists(icoFileName))
        QFile::remove(icoFileName);
    //*/
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
    {
        event->acceptProposedAction();
    }
    else
    {
        qDebug("Error: Only Files can be dragged to this window");
    }
}

void MainWindow::wheelEvent(QWheelEvent *ev)
{
    if(ev->modifiers() & Qt::ControlModifier)
    {
        float numDegrees = ev->delta() / 8;
        float numSteps = numDegrees / 15;

        editorFontSize += numSteps*1.5;

        if(editorFontSize < 5)
            editorFontSize = 5;
        else if(editorFontSize > 40)
            editorFontSize = 40;

        setEditorFontSize(editorFontSize);
        ev->accept();
    }
}

QString combinePath(QString parent, QString child)
{
    return QFileInfo(parent, child).absoluteFilePath();
}

void MainWindow::do_editor_linkClicked(MyEdit *source, QString href)
{
    CodeDocument *doc = docContainer->getDocumentFromWidget(source);
    if(!doc)
        return;

    QString linkedFile = getImportedModuleFile(doc, href);
    if(linkedFile == "")
    {
        QMessageBox box(QMessageBox::Question,Ide::msg[IdeMsg::FileNotFound],
                Ide::msg.get(IdeMsg::FileNotFoundCreateIt1, linkedFile),
                QMessageBox::Yes|QMessageBox::No);
        box.exec();
        if( box.result() == QMessageBox::Yes)
        {
            QFile file(linkedFile);
            if(file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate))
            {
                file.close();
            }
            else
            {
                return;
            }
        }
        else
        {
            return;
        }
    }
    docContainer->OpenOrSwitch(linkedFile);
}

QString MainWindow::getImportedModuleFile(CodeDocument *importer, QString name)
{
    if(importer->isDocNewFile())
    {
        QString stdMod = combinePath(standardModulePath, name);
        if(QFile::exists(stdMod))
        {
            return stdMod;
        }
        return "";
    }
    QString docPath = importer->getFileName();
    QFileInfo f = QFileInfo(docPath);
    QString dir = f.absoluteDir().absolutePath();
    QString linkedFile = dir + "/" + name;
    if(!QFile::exists(linkedFile))
        return "";
    return linkedFile;
}

bool MainWindow::eventFilter(QObject *sender, QEvent *event)
{

    // this filter applies to the find/replace
    // text boxes
    if (event->type() == QEvent::KeyPress)
    {
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

void MainWindow::on_action_options_triggered()
{
    SettingsDlg s(this);
    s.init(getEditorFontSize(), editorFont, isDemoMode, codeModelUpdateInterval, standardModulePath);
    if(s.exec() == QDialog::Accepted)
    {
        int fontSize;
        s.getResult(fontSize, editorFont, isDemoMode, codeModelUpdateInterval, standardModulePath);
        QSettings settings(settingsOrganizationName, settingsApplicationName, this);
        settings.setValue("editor_font_size", fontSize);
        settings.setValue("editor_font_name", editorFont.family());
        settings.setValue("codemodel_update_interval", codeModelUpdateInterval);
        settings.setValue("standard_module_path", standardModulePath);
        settings.setValue("is_demo_mode", isDemoMode);
        editorFont.setPointSize(fontSize);
        setEditorFont(editorFont);
        killTimer(codeModelUpdateTimerId);
        codeModelUpdateTimerId = startTimer(codeModelUpdateInterval);
    }
}

int MainWindow::getEditorFontSize()
{
    return editorFontSize;
}

struct NameSize
{
    QString name;
    int size;
};

void setAllEditsProc(CodeDocument *, QWidget *edit, void *newFont)
{
    QFont *font = ((QFont *) newFont);
    MyEdit *ed = dynamic_cast<MyEdit *>(edit);
    if(ed)
    {
        ed->setFont(*font);
        ed->updateLineNumberAreaFont();
    }
}

void setAllEditsSizeProc(CodeDocument *, QWidget *edit, void *fp)
{
    int size = *((int *) fp);
    MyEdit *ed = dynamic_cast<MyEdit *>(edit);
    if(ed)
    {
        QFont f = ed->font();
        f.setPointSize(size);
        ed->setFont(f);
        ed->updateLineNumberAreaFont();
    }
}

void MainWindow::setEditorFont(QFont font)
{
    editorFontSize = font.pointSize();
    docContainer->forAll(setAllEditsProc, &font);
}

void MainWindow::setEditorFontSize(int size)
{
    editorFontSize = size;
    docContainer->forAll(setAllEditsSizeProc, &size);
}

void MainWindow::on_action_about_kalimat_triggered()
{
    AboutDlg dlg(this);
    dlg.exec();
}

void MainWindow::on_actionUpdate_code_model_triggered()
{

    if(generatingProgramModel)
        return;
    generatingProgramModel = true;
    if(!currentEditor())
    {
        generatingProgramModel = false;
        return;
    }

    CodeDocument *doc = docContainer->getCurrentDocument();
    if(!doc)
    {
        generatingProgramModel = false;
        return;
    }

    if(!currentEditor()->document()->isModified())
    {
        generatingProgramModel = false;
        return;
    }

    QVector<Token> tokens;
    long t1 = get_time();
    shared_ptr<CompilationUnit> cu = parseCurrentDocumentWithRecovery(tokens);
    long t2 = get_time();
    qDebug() << "Parsed in: " << (t2-t1)/ 1000 << " ms";
    if(!cu)
    {
        generatingProgramModel = false;
        return;
    }
    QString fileName = doc->isDocNewFile()? QString("%%1").arg((uint) doc): doc->getFileName();
    t1 = get_time();
    if(!progdb.isOpen())
        progdb.open();

    QVector<QString> imports;
    for(int i=0; i<cu->usedModuleCount(); ++i)
    {
        QString mod = cu->usedModule(i)->value();
        mod = getImportedModuleFile(doc, mod);
        imports.append(mod);
    }
    progdb.updateModule(fileName, doc, cu);
    progdb.updateImportsOfModule(fileName, imports);
    progdb.updateModuleTokens(fileName, tokens);
    progdb.updateModuleDefinitions(fileName, cu);
    t2 = get_time();
    qDebug() << "Updated DB in: " << (t2-t1)/ 1000 << " ms";
    fillFunctionNavigationCombo(doc, fileName);
    functionNavigationCombo->setMinimumWidth(
                0.6f * (float) ui->functionNavigationToolbar->width() );

    /*
    functionNavigationInfo = codeAnalyzer.analyzeCompilationUnit(cu);
    if(functionNavigationInfo .funcNameToAst.count() == 0)
    {
        ui->functionNavigationToolbar->hide();
    }
    else
    {
        functionNavigationComboIsUpdating = true;
        functionNavigationCombo->clear();
        functionNavigationCombo->addItem(Ide::msg[IdeMsg::TopLevel]);

        for(QMap<QString, shared_ptr<ProceduralDecl> >::const_iterator i = functionNavigationInfo.funcNameToAst.begin();
            i != functionNavigationInfo.funcNameToAst.end(); ++i)
        {

            functionNavigationCombo->addItem(i.key());
        }
        functionNavigationComboIsUpdating = false;

        ui->functionNavigationToolbar->show();
        functionNavigationCombo->setMinimumWidth(
                    0.6f * (float) ui->functionNavigationToolbar->width() );
        setFunctionNavigationComboSelection(currentEditor());
    }
    */
    generatingProgramModel = false;
}

void MainWindow::fillFunctionNavigationCombo(CodeDocument *doc, QString filename)
{
    if(!functionNavigationCombo)
        return;
    functionNavigationComboIsUpdating = true;
    functionNavigationCombo->clear();
    functionNavigationCombo->addItem(Ide::msg[IdeMsg::TopLevel]);
    QSqlQuery q = progdb.q("SELECT name, defining_token_pos FROM function_definitions JOIN definitions"
                           " ON definitions.id=function_definitions.def_id AND"
                           " definitions.module_filename = function_definitions.module_filename "
                           "WHERE definitions.module_filename=?", filename);
    while(q.next())
    {
        functionNavigationCombo->addItem(q.value(0).toString(), q.value(1));
    }
    setFunctionNavigationComboSelection(currentEditor());
    functionNavigationComboIsUpdating = false;
}

void MainWindow::do_functionNavigationCombo_currentIndexChanged(int index)
{
    if(functionNavigationComboIsUpdating)
        return;
    if(index == -1 || index == 0)
        return;
    if(!currentEditor())
        return;

    int pos = functionNavigationCombo->itemData(index).toInt();

    ((MyEdit *) currentEditor())->jumpToPos(pos);
    currentEditor()->setFocus();
}

Token MainWindow::getTokenUnderCursor(MyEdit *editor, TokenType type, bool ignoreTypeFilter)
{
    int index;
    return getTokenUnderCursor(editor, type, index, ignoreTypeFilter);
}

Token MainWindow::getTokenUnderCursor(MyEdit *editor, TokenType type, int &index, bool ignoreTypeFilter)
{
    KalimatLexer lxr;
    int cursorPos = editor->textCursor().position();
    try
    {
        lxr.init(currentEditor()->document()->toPlainText());
        lxr.tokenize(true);
        QVector<Token> tokens = lxr.getTokens();

        for(int i=0; i<tokens.count(); i++)
        {
            Token t = tokens[i];
            if(t.Pos > cursorPos)
                continue;
            if(!ignoreTypeFilter && t.Type != type)
                continue;
            if(t.Pos + t.Lexeme.length() >= cursorPos)
            {
                index = i;
                return t;
            }
        }
    }
    catch(UnexpectedCharException ex)
    {
        ui->outputView->append(ex.buildMessage());
    }
    catch(UnexpectedEndOfFileException ex)
    {
        ui->outputView->append(tr("Unexpected end of file"));
    }
    return Token();
}

Token MainWindow::getTokenBeforeCursor(MyEdit *editor, TokenType type, bool ignoreTypeFilter)
{
    int index;
    return getTokenBeforeCursor(editor, type, index, ignoreTypeFilter);
}

Token MainWindow::getTokenBeforeCursor(MyEdit *editor, TokenType type, int &index, bool ignoreTypeFilter)
{
    KalimatLexer lxr;
    int cursorPos = editor->textCursor().position();
    cursorPos --;
    try
    {
        lxr.init(currentEditor()->document()->toPlainText());
        lxr.tokenize(true);
        QVector<Token> tokens = lxr.getTokens();

        for(int i=0; i<tokens.count(); i++)
        {
            Token t = tokens[i];
            if(t.Pos < cursorPos)
            {
                if((i+1)< tokens.count())
                {
                    Token t2 = tokens[i+1];
                    if(t2.Pos>=cursorPos)
                    {
                        if(t.Type == type)
                            return t;
                        else
                            return t;
                    }
                }
                else
                {
                    if(t.Type == type)
                        return t;
                    else
                        return t;
                }
            }

        }
    }
    catch(UnexpectedCharException ex)
    {
        ui->outputView->append(ex.buildMessage());
    }
    catch(UnexpectedEndOfFileException ex)
    {
        ui->outputView->append(tr("Unexpected end of file"));
    }
    return Token();
}

void MainWindow::jumpToFunctionNamed(QString name, MyEdit *editor)
{
    if(!functionNavigationInfo.funcNameToAst.contains(name))
        return;
    shared_ptr<ProceduralDecl> proc = functionNavigationInfo.funcNameToAst[name];
    if(proc)
    {
        Token pos = proc->getPos();
        if(pos.Type == TokenNone || pos.Type == TokenInvalid)
            return;
        QTextCursor cursor = editor->textCursor();
        cursor.setPosition(pos.Pos);
        editor->setTextCursor(cursor);
        editor->centerCursorVerticallyIfNeeded();
        editor->setFocus();
    }
}

void MainWindow::on_action_go_to_definition_triggered()
{
    /*
    if(!currentEditor())
        return;

    MyEdit *editor = dynamic_cast<MyEdit *>(currentEditor());

    Token t = getTokenUnderCursor(editor, IDENTIFIER);
    if(t.Type != TokenInvalid)
    {
        QString funcName = t.Lexeme;
        jumpToFunctionNamed(funcName, editor);
    }
    */

    if(!currentEditor())
        return;

    MyEdit *editor = dynamic_cast<MyEdit *>(currentEditor());
    CodeDocument *doc = docContainer->getCurrentDocument();
    QString filename = doc->isDocNewFile()? QString("%%1").arg((uint) doc) : doc->getFileName();

    int cursorPos = editor->textCursor().position();
    QSqlQuery q = progdb.q("SELECT defining_token_pos FROM function_definitions JOIN definitions "
                           " ON definitions.id=function_definitions.def_id AND "
                           " definitions.module_filename = function_definitions.module_filename "
                           " WHERE definitions.module_filename = ?"
                           " AND definitions.name in  (SELECT lexeme FROM tokens WHERE pos <=? "
                           " AND pos + length >=?)", filename, cursorPos, cursorPos);
    if(q.next())
    {
        editor->jumpToPos(q.value(0).toInt());
        return;
    }

    q = progdb.q("SELECT defining_token_pos, function_definitions.module_filename FROM function_definitions JOIN definitions "
                 " ON definitions.id=function_definitions.def_id AND "
                 " definitions.module_filename = function_definitions.module_filename "
                 " WHERE definitions.module_filename IN (SELECT imported FROM"
                 " module_imports WHERE importer=?)"
                 " AND definitions.name in  (SELECT lexeme FROM tokens WHERE pos <=? "
                 " AND pos + length >=?)", filename, cursorPos, cursorPos);

    if(q.next())
    {
        docContainer->OpenOrSwitch(q.value(1).toString());
        ((MyEdit *)currentEditor())->jumpToPos(q.value(0).toInt());
        return;
    }

}

void MainWindow::triggerAutocomplete(MyEdit *editor)
{
    analyzeForAutocomplete();
    int indexOfTok;
    Token t = getTokenBeforeCursor(editor, IDENTIFIER, indexOfTok);

    if(t.Type == TokenInvalid)
        return;
    if(!varInfos.contains(t.Pos))
        return; // among other reasons for not being in
    // varinfos is that it might not
    // actually be a variable
    // but any identifier
    VarUsageInfo vi = varInfos[t.Pos];
    showCompletionCombo(editor, vi);

}

void MainWindow::showCompletionCombo(MyEdit *editor, VarUsageInfo vi)
{
    QString typeName;
    shared_ptr<Identifier> declPoint = vi.pointOfDeclaration;
    int pos = declPoint->getPos().Pos;
    if(!varTypeInfo.contains(pos))
        return;
    typeName = varTypeInfo[pos];
    if(!classInfoData.contains(typeName))
        return;
    shared_ptr<ClassDecl> cd = classInfoData[typeName];
    if(cd->methodCount() == 0)
        return;

    autoCompleteCombo = new QComboBox(editor);
    //autoCompleteCombo->setIconSize(QSize(32,32));
    for(QMap<QString, shared_ptr<MethodDecl> >::const_iterator i=cd->_methods.begin();
        i != cd->_methods.end();
        ++i)
    {
        shared_ptr<MethodDecl> md = i.value();
        /*
        QIcon icon;
        if(md->isFunctionNotProcedure)
            icon = QIcon(":/icons/icons/response.bmp");
        else
            icon = QIcon(":/icons/icons/reply.bmp");
        //*/
        autoCompleteCombo->addItem(md->getTooltip(), md->procName()->name());
    }

    autoCompleteCombo->setWindowOpacity(0.8);
    autoCompleteCombo->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    autoCompleteCombo->setLayoutDirection(Qt::RightToLeft);
    autoCompleteCombo->adjustSize();
    int x = editor->cursorRect().topLeft().x();
    int y = editor->cursorRect().topLeft().y();
    autoCompleteCombo->move(x - autoCompleteCombo->width(),
                            y);
    connect(autoCompleteCombo, SIGNAL(activated(int)),SLOT(autoCompleteBoxActivated(int)));

    autoCompleteCombo->showPopup();
}

void MainWindow::autoCompleteBoxActivated(int index)
{
    if(index == -1)
        return;
    QString sel = autoCompleteCombo->itemText(index);
    QString methodName = autoCompleteCombo->itemData(index)
            .toString() + "(";
    if(sel.trimmed() !="" && currentEditor() != NULL)
        currentEditor()->textCursor().insertText(methodName);
}

void MainWindow::triggerFunctionTips(MyEdit *editor)
{
    int indexOfTok;
    Token t = getTokenUnderCursor(editor, IDENTIFIER, indexOfTok);

    if(t.Type == TokenInvalid)
        return;
    QString funcName = t.Lexeme;

    if(!functionNavigationInfo.funcNameToAst.contains(funcName))
        return;

    shared_ptr<ProceduralDecl> proc =
            functionNavigationInfo.funcNameToAst[funcName];
    QString tip = proc->getTooltip();
    QRect cr = editor->cursorRect();
    toolTipPoint = editor->mapToGlobal(cr.topRight());

    MyToolTip::showText(toolTipPoint, tip, editor, QRect(), false);
    toolTipEditor = editor;
    shouldHideFunctionTooltip = true;

    funcToolTipParenTokenIndex = indexOfTok + 1;
}

void MainWindow::analyzeForAutocomplete()
{
    KalimatLexer lxr;
    KalimatParser parser;
    CodeDocument *doc = NULL;
    try
    {
        ui->tabWidget->setCurrentWidget(ui->outputView);
        ui->outputView->clear();
        doc = docContainer->getCurrentDocument();
        if(doc == NULL)
            return;
        QString code = doc->getEditor()->document()->toPlainText();
        parser.withRecovery = true;
        parser.init(code, &lxr, NULL);
        shared_ptr<AST> tree = parser.parse();
        shared_ptr<Module> mod = dynamic_pointer_cast<Module>(
                    tree);

        CodeGenerator gen;
        gen.Init();
        gen.mode = AnalysisMode;
        if(mod)
        {
            gen.compileModule(mod, "", doc);
        }
        shared_ptr<Program> prog = dynamic_pointer_cast<Program>(
                    tree);
        if(prog)
        {
            gen.generate(prog, "", doc);
        }
        varInfos = gen.varInfos;
        classInfoData = gen.allClasses;
        varTypeInfo = gen.varTypeInfo;
    }
    catch(UnexpectedCharException ex)
    {
        ui->outputView->append(ex.buildMessage());
    }
    catch(UnexpectedEndOfFileException ex)
    {
        ui->outputView->append(tr("Unexpected end of file"));
    }
    catch(ParserException ex)
    {
        QString fn;
        if(ex.pos.tag!= NULL)
        {
            fn = ((CodeDocument *) ex.pos.tag)->getFileName();
        }
        QString msg = translateParserError(ex);

        ui->outputView->append(QString("File %1:\t%2").arg(fn).arg(msg));
    }
    catch(CompilerException ex)
    {
        ui->outputView->append(ex.getMessage());
        if(doc != NULL)
        {
            CodeDocument *dc = doc;
            if(ex.source)
            {
                if(ex.source->getPos().tag != NULL)
                {
                    dc = (CodeDocument *) ex.source->getPos().tag;
                }
                highlightLine(dc->getEditor(), ex.source->getPos().Pos);
            }
        }
    }
}

void launchKalimatSite()
{
#ifndef ENGLISH_PL
    QDesktopServices::openUrl(QUrl("http://www.kalimat-lang.com", QUrl::TolerantMode));
#else
    QDesktopServices::openUrl(QUrl("http://code.google.com/p/kalimat/downloads", QUrl::TolerantMode));
#endif
}

void MainWindow::do_goto_kalimatlangdotcom_triggered()
{
    QtConcurrent::run(launchKalimatSite);
}

void MainWindow::insertInEditor(QString s)
{
    QTextEdit *editor = currentEditor();
    if(!editor)
        return;
    editor->textCursor().insertText(s);
}

void MainWindow::on_actionSpecialSymbol_dot_triggered()
{
    insertInEditor(".");
}

void MainWindow::on_action_SpecialSymbol_comma_triggered()
{
    insertInEditor(QString::fromStdWString(L"?? "));
}

void MainWindow::on_action_SpecialSymbol_openBracket_triggered()
{
    insertInEditor("[");
}

void MainWindow::on_action_SpecialSymbol_closeBracket_triggered()
{
    insertInEditor("]");
}

void MainWindow::on_action_SpecialSymbol_openBrace_triggered()
{
    insertInEditor("{");
}

void MainWindow::on_action_SpecialSymbol_closeBrace_triggered()
{
    insertInEditor("}");
}

void MainWindow::on_action_SpecialSymbol_lambda_triggered()
{
    insertInEditor(QString::fromStdWString(L"??"));
}

void MainWindow::on_actionLambda_transformation_triggered()
{

}

void MainWindow::on_action_wonderfulmonitor_toggled(bool arg1)
{
    _isWonderfulMonitorEnabled = ui->action_wonderfulmonitor->isChecked();
}
