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

void myMessageOutput(QtMsgType type, const char *msg)
{
    switch (type) {
    case QtDebugMsg:
        fprintf(stderr, "Debug: %s\n", msg);
        break;
    case QtWarningMsg:
        fprintf(stderr, "Warning: %s\n", msg);
        break;
    case QtCriticalMsg:
        fprintf(stderr, "Critical: %s\n", msg);
        break;
    case QtFatalMsg:
        fprintf(stderr, "Fatal: %s\n", msg);
      abort();
    }
}

int main(int argc, char *argv[])
{
    qInstallMsgHandler(myMessageOutput);
    QApplication a(argc, argv);
    a.setLayoutDirection(Qt::RightToLeft);
    qApp->setLayoutDirection(Qt::RightToLeft);

    MainWindow w;
    w.show();
    return a.exec();
}
