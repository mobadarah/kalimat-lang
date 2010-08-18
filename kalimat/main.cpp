/**************************************************************************
**   The Kalimat programming language
**   Copyright (C) 2010 Mohamed Samy Ali - samy2004@gmail.com
**   This project is released under the Apache License version 2.0
**   as described in the included license.txt file
**************************************************************************/

#include <QtGui/QApplication>

#include "../smallvm/vm_incl.h"
#include "../smallvm/vm.h"
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    bool testing;
    testing = false;

    qApp->setLayoutDirection(Qt::RightToLeft);

    QApplication a(argc, argv);
    MainWindow w;
    w.show();
}
