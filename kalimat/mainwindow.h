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
#include "codedocument.h"
#include "../smallvm/vm_incl.h"
#include "../smallvm/vm.h"
#include "../smallvm/externalmethod.h"
#include "Compiler/codeposition.h"
#include "Compiler/codegenerator_incl.h"
#include "documentcontainer.h"

#include <QQueue>
#include <QGraphicsView>

namespace Ui
{
    class MainWindow;
}

const int MaxRecentFiles = 4;
class MainWindow : public QMainWindow, public DocumentClient
{
    Q_OBJECT

public:
    static MainWindow *that;
    MainWindow(QWidget *parent = 0);
    ~MainWindow();
    bool isWonderfulMonitorEnabled();
    void markCurrentInstruction(VM *vm, int &pos, int &length);
    void handleVMError(VMError err);
    void highlightLine(QTextEdit *editor, int pos);
    void highlightToken(QTextEdit *editor, int pos, int length);

    void visualizeCallStack(QStack<Frame> &callStack, QGraphicsView *view);

    DocumentContainer *docContainer;
    QMap<int, CodePosition> PositionInfo;
    QStack<QString> pathsOfModuleClients; // must always be absolute paths
private:
    Ui::MainWindow *ui;
    SyntaxHighlighter *syn;
    VM vm;

    void show_error(QString);
    virtual void LoadDocIntoWidget(CodeDocument *doc, QWidget *widget);
    virtual QWidget *GetParentWindow();
    virtual QWidget *CreateEditorWidget();
    QTextEdit *currentEditor();
private slots:
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
    void on_actionLoad_Compilation_unit_triggered();
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

};

#endif // MAINWINDOW_H
