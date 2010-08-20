/**************************************************************************
**   The Kalimat programming language
**   Copyright (C) 2010 Mohamed Samy Ali - samy2004@gmail.com
**   This project is released under the Apache License version 2.0
**   as described in the included license.txt file
**************************************************************************/

#include "vm_incl.h"
#include "vm.h"

#include "mainwindow.h"
#include "ui_mainwindow.h"


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setCentralWidget(ui->textEdit);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::changeEvent(QEvent *e)
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

void MainWindow::on_actionGo_triggered()
{
    VM vm;
    vm.Register("print", new Print());
    QString o="";
    try
    {
        vm.Load(ui->textEdit->document()->toPlainText());
        vm.Init();

        while(vm.isRunning())
        {
            vm.RunStep();
        }
    }
    catch(VMError err)
    {
        o = error_message_from(err);
    }

    if(o=="")
        o = QString("%1").arg( vm.__top()->v.intVal);
    this->setWindowTitle(o);
}
QString MainWindow::error_message_from(VMError )
{
    return "";
}
