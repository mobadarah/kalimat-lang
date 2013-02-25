/**************************************************************************
**   The Kalimat programming language
**   Copyright (C) 2010 Mohamed Samy Ali - samy2004@gmail.com
**   This project is released under the Apache License version 2.0
**   as described in the included license.txt file
**************************************************************************/

#include <QtGui/QApplication>
#include <QDebug>
#include <QMessageBox>
#include <QTextStream>
#include "runtime/runwindow.h"
#include "utils.h"
#include <iostream>
#include <string>
using namespace std;
QMap<QString, QString> tempConstants;

extern "C"
{
Q_DECL_EXPORT
void SmallVMAddStringConstant(char *symBase64, char *strBase64)
{
    // qDebug() << "adding string constant!";
    QString sym = base64Decode(symBase64);
    QString val = base64Decode(strBase64);
    tempConstants[val] = sym;
}

Q_DECL_EXPORT
void RunSmallVMCodeBase64(wchar_t *pathOfProgramsFile,
                          char *programBase64)
{
    BuiltInTypes::init();
    VM::InitGlobalData();
    //qDebug() << "program gonna run!";
    int argc = 0;
    char **argv = NULL;// = {"aa.exe"};
    QApplication app(argc, argv);

    //qDebug() << "program before decoding:" <<programBase64;
    //qDebug() << "path of prog file:" << QString::fromStdWString(pathOfProgramsFile);
    try
    {
        //qDebug("Launching Run Window");
        RunWindow rw(QString::fromStdWString(pathOfProgramsFile), new NullaryVMClient());
        QString programCode = base64Decode(programBase64);
        //qDebug() << "program after decoding:" << programCode;
        rw.show();
        rw.Init(programCode, tempConstants, QSet<Breakpoint>(), DebugInfo());
        app.exec();
    }
    catch(VMError)
    {
        app.exit(1);
    }
}
}
int main(int argc, char *argv[])
{
    // todo: enable a SmallVM.exe executable to take a string table file
    // and assembly file and run them
    return 0;
}
