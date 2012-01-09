/**************************************************************************
**   The Kalimat programming language
**   Copyright (C) 2010 Mohamed Samy Ali - samy2004@gmail.com
**   This project is released under the Apache License version 2.0
**   as described in the included license.txt file
**************************************************************************/

#include <QtGui/QApplication>
#include <iostream>
#include <QMessageBox>
#include <QTextStream>
#include "runtime/runwindow.h"
#include "utils.h"

using namespace std;
QMap<QString, QString> tempConstants;

Q_DECL_EXPORT
void RunSmallVMCode(QWidget *parent,
                    QString pathOfProgramsFile,
                    VMClient *client,
                    QString program,
                    QMap<QString, QString> stringConstants,
                    QSet<Breakpoint> breakPoints,
                    DebugInfo debugInfo)
{
    RunWindow rw(parent, pathOfProgramsFile, client);
    rw.show();
    rw.Init(program, stringConstants, breakPoints, debugInfo);
}

extern "C"
{
Q_DECL_EXPORT
void SmallVMAddStringConstant(char *symBase64, char *strBase64)
{
    //cout << "adding string constant!" << endl;
    //cout.flush();
    QString sym = base64Decode(symBase64);
    QString val = base64Decode(strBase64);
    tempConstants[val] = sym;
}

Q_DECL_EXPORT
void RunSmallVMCodeBase64(wchar_t *pathOfProgramsFile,
                          char *programBase64)
{
    //cout << "program gonna run!" << endl;
    int argc = 0;
    char **argv = NULL;// = {"aa.exe"};
    QApplication app(argc, argv);

    //cout << "program before decoding:" <<programBase64 << endl;
    //cout.flush();
    //cout << "path of prog file:" << pathOfProgramsFile << endl;
    //cout.flush();
    try
    {
        RunWindow rw(QString::fromStdWString(pathOfProgramsFile), new NullaryVMClient());
        QString programCode = base64Decode(programBase64);
        //cout << "program after decoding:" << programCode.toStdString() << endl;
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

    return 0;
}
