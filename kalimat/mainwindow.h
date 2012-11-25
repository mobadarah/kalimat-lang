/**************************************************************************
** The Kalimat programming language
**   Copyright (C) 2010 Mohamed Samy Ali - samy2004@gmail.com
**   This project is released under the Apache License version 2.0
**   as described in the included license.txt file
**************************************************************************/

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui/QMainWindow>
#include "syntaxhighlighter.h"
#include "../smallvm/codedocument.h"

#ifndef VM_H
    #include "../smallvm/vm_incl.h"
    #include "../smallvm/vm.h"
#endif

#include "../smallvm/externalmethod.h"
#include "Compiler/codeposition.h"
#include "Compiler/codegenerator_incl.h"
#include "documentcontainer.h"
#include "myedit.h"
#include "../smallvm/breakpoint.h"
#include "../smallvm/runtime/vmclient.h"
#include "Lexer/token.h"
#include "Parser/parserexception.h"
#include "Parser/KalimatParserError.h"
#include "AutoComplete/analyzer.h"
#include <QQueue>
#include <QGraphicsView>
#include <QActionGroup>
#include <QLabel>
#include <QSplitter>
#include <QComboBox>

namespace Ui
{
    class MainWindow;
}

const int MaxRecentFiles = 8;
class MainWindow;
class RunWindow;

enum StepMode
{
    StepSingle, StepCall, StepParams
};

struct StepStopCondition
{
    virtual bool stopNow(CodeDocument *doc1, int line1, Frame *frame1, CodeDocument *doc2, int line2, Frame *frame2)=0;
};

class MainWindow : public QMainWindow, public DocumentClient, public VMClient
{
    Q_OBJECT

public:
    QString settingsOrganizationName;
    QString settingsApplicationName;
    static MainWindow *that;
    QLabel *lblEditorCurrentLine, *lblEditorCurrentColumn;
    bool helpWindowVisible;
    bool isDemoMode;
    int editorFontSize;
    QFont editorFont;

    Analyzer codeAnalyzer;
    QTimer codeParseTimer;

    // Info on all variables, including (currently) declaration point
    // generated frequently for autocomplete
    // maps from position of token of identifier
    // to its varinfo
    QMap<int, VarUsageInfo> varInfos;
    QMap<int, QString> varTypeInfo;
    QMap<QString, shared_ptr<ClassDecl> > classInfoData;

    shared_ptr<CompilationUnit> parseCurrentDocumentWithRecovery();
    QComboBox *functionNavigationCombo;
    QComboBox *autoCompleteCombo; // todo:allocated on each
                                 // autocompletion. Does this leak?
    bool functionNavigationComboIsUpdating;
    CompilationUnitInfo functionNavigationInfo;
    void setFunctionNavigationComboSelection(QTextEdit *editor);
    int codeModelUpdateInterval;
    int codeModelUpdateTimerId;
    bool generatingProgramModel;

    Token getTokenUnderCursor(MyEdit *editor, TokenType type, bool ignoreTypeFilter=false);
    Token getTokenUnderCursor(MyEdit *editor, TokenType type, int &index, bool ignoreTypeFilter=false);

    Token getTokenBeforeCursor(MyEdit *editor, TokenType type, bool ignoreTypeFilter=false);
    Token getTokenBeforeCursor(MyEdit *editor, TokenType type, int &index, bool ignoreTypeFilter=false);

    void analyzeForAutocomplete();

    void jumpToFunctionNamed(QString name, MyEdit *editor);
    void triggerAutocomplete(MyEdit *editor);
    void showCompletionCombo(MyEdit *editor, VarUsageInfo vi);
    void triggerFunctionTips(MyEdit *editor);
    bool shouldHideFunctionTooltip;
    int funcToolTipParenTokenIndex;
    QPoint toolTipPoint;
    MyEdit *toolTipEditor;

    QString standardModulePath;

    MainWindow(QWidget *parent = 0);
    ~MainWindow();

    bool isWonderfulMonitorEnabled();
    int wonderfulMonitorDelay();
    CodePosition getPositionOfRunningInstruction(Frame *f);
    CodePosition getPositionOfInstruction(QString method, int offset);
    void markCurrentInstruction(VM *vm, Process *proc, int &pos, int &length);

    void handleVMError(VMError err);
    void highlightRunningInstruction(Frame *f);
    void highlightRunningInstruction(Frame *f, QColor clr);
    void highlightLine(QTextEdit *editor, int pos);
    void highlightLine(QTextEdit *editor, int pos, QColor color);
    void highlightToken(QTextEdit *editor, int pos, int length);
    void removeLineHighlights(MyEdit *editor, int line);
    void setLineIndicators(int line, int column, QTextEdit *editor);
    void visualizeCallStacks(QSet<QQueue<Process *> *> callStacks, QGraphicsView *view);
    void visualizeCallStack(QStack<Frame> &callStack, QGraphicsView *view);

    void outputMsg(QString s);
    QString translateParserError(ParserException ex);

    void Break(QString methodName, int offset, Frame *frame, Process *process);
    void programStopped(RunWindow *);
    bool eventFilter(QObject *sender, QEvent *event);

    DocumentContainer *docContainer;
    QMap<int, CodePosition> PositionInfo;
    QStack<QString> pathsOfModuleClients; // must always be absolute paths

    int getEditorFontSize();
    void setEditorFont(QFont font);
    void setEditorFontSize(int);
    void insertInEditor(QString);
private:
    QActionGroup *speedGroup;
    Ui::MainWindow *ui;
    SyntaxHighlighter *syn;
    VM vm;
    RunWindow *stoppedRunWindow;
    QMap<KalimatParserError, QString> parserErrorMap;
    void show_error(QString);
    virtual void LoadDocIntoWidget(CodeDocument *doc, QWidget *widget);
    virtual QWidget *GetParentWindow();
    virtual QWidget *CreateEditorWidget();

    QSplitter *helpSplitter;
    QWidget *oldMainWidget;
    //QWebView *helpWebView;
    void showHelpWindow();
    void hideHelpWindow();

    QTextEdit *currentEditor();
    CodeDocument *lastCodeDocToRun;
    void saveAll();

    // The breakpoint 'reservations' recorded here will be passed to the code generator
    QSet<Breakpoint> breakPoints;
    DebugInfo debugInfo;
    Breakpoint stoppedAtBreakPoint;
    Process *currentDebuggerProcess;
    bool atBreak;

    void genericStep(Process *proc, StepStopCondition &cond);
    void step(Process *proc);
    void setDebuggedProcess(Process *);
private slots:
    void on_actionGo_to_position_triggered();
    void on_actionCompile_without_tags_triggered();
    void on_btnReplaceNext_clicked();
    void on_btnReplacePrev_clicked();
    void on_btnFindNext_clicked();
    void on_btnFindPrev_clicked();
    void on_action_replace_triggered();
    void on_action_find_triggered();
    void on_action_redo_triggered();
    void on_action_undo_triggered();
    void on_action_delete_triggered();
    void on_action_edit_paste_triggered();
    void on_action_edit_copy_triggered();
    void on_action_edit_cut_triggered();
    void on_garbageCollect_triggered();
    void on_editorTabs_tabCloseRequested(int index);
    void on_action_exit_triggered();
    void on_action_saveas_triggered();
    void on_action_save_triggered();
    void on_action_open_triggered();
    void on_action_new_triggered();
    void on_actionCompile_triggered();
    void on_mnuProgramRun_triggered();
    void on_actionLexize_triggered();
    void on_actionParse_triggered();
    void closeEvent(QCloseEvent *);
    void on_actionKalimatManual_triggered();
    void on_action_copy_as_html_triggered();
    void on_action_copy_as_wiki_triggered();
    void on_action_copy_literate_html_triggered();
    void on_action_autoFormat_triggered();
    void on_action_breakpoint_triggered();
    void on_action_resume_triggered();
    void on_action_step_triggered();
    void on_action_step_procedure_triggered();
    void on_actionMake_exe_triggered();
    void makeDrag();
    void on_editor_linkClicked(MyEdit *source, QString href);
    void on_actionParse_with_recovery_triggered();

    void on_action_options_triggered();

    void on_action_about_kalimat_triggered();

    void on_actionUpdate_code_model_triggered();
    void on_functionNavigationCombo_currentIndexChanged(int);
    void on_goto_kalimatlangdotcom_triggered();
    void on_actionSpecialSymbol_dot_triggered();

    void on_action_SpecialSymbol_comma_triggered();

    void on_action_SpecialSymbol_openBracket_triggered();

    void on_action_SpecialSymbol_closeBracket_triggered();

    void on_action_SpecialSymbol_openBrace_triggered();

    void on_action_SpecialSymbol_closeBrace_triggered();

    void on_action_go_to_definition_triggered();
    void autoCompleteBoxActivated(int);

    void on_action_SpecialSymbol_lambda_triggered();

    void on_actionLambda_transformation_triggered();

protected:
     void dropEvent(QDropEvent *de);
     void dragMoveEvent(QDragMoveEvent *de);
     void dragEnterEvent(QDragEnterEvent *event);

     void wheelEvent(QWheelEvent *);
     void timerEvent(QTimerEvent *event); // for code model update
};

#endif // MAINWINDOW_H
