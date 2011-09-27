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
#include <QQueue>
#include <QGraphicsView>
#include <QActionGroup>
#include <QLabel>
#include <QSplitter>

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
    static MainWindow *that;
    QLabel *lblEditorCurrentLine, *lblEditorCurrentColumn;
    bool helpWindowVisible;
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

    bool isWonderfulMonitorEnabled();
    int wonderfulMonitorDelay();
    CodePosition getPositionOfRunningInstruction(Frame *f);
    CodePosition getPositionOfInstruction(QString method, int offset);
    void markCurrentInstruction(VM *vm, int &pos, int &length);

    void handleVMError(VMError err);
    void highlightRunningInstruction(Frame *f);
    void highlightRunningInstruction(Frame *f, QColor clr);
    void highlightLine(QTextEdit *editor, int pos);
    void highlightLine(QTextEdit *editor, int pos, QColor color);
    void highlightToken(QTextEdit *editor, int pos, int length);
    void removeLineHighlights(MyEdit *editor, int line);
    void setLineIndicators(int line, int column);
    void visualizeCallStacks(QQueue<Process *> &callStacks, QGraphicsView *view);
    void visualizeCallStack(QStack<Frame> &callStack, QGraphicsView *view);

    void outputMsg(QString s);

    void Break(QString methodName, int offset, Frame *frame, Process *process);
    void programStopped(RunWindow *);
    bool eventFilter(QObject *sender, QEvent *event);

    DocumentContainer *docContainer;
    QMap<int, CodePosition> PositionInfo;
    QStack<QString> pathsOfModuleClients; // must always be absolute paths
private:
    QActionGroup *speedGroup;
    Ui::MainWindow *ui;
    SyntaxHighlighter *syn;
    VM vm;
    RunWindow *stoppedRunWindow;
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
protected:
     void dropEvent(QDropEvent *de);
     void dragMoveEvent(QDragMoveEvent *de);
     void dragEnterEvent(QDragEnterEvent *event);
};

#endif // MAINWINDOW_H
