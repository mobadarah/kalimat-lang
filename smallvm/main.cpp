/**************************************************************************
**   The Kalimat programming language
**   Copyright (C) 2010 Mohamed Samy Ali - samy2004@gmail.com
**   This project is released under the Apache License version 2.0
**   as described in the included license.txt file
**************************************************************************/

#include <QtGui/QApplication>
//#include <iostream>
#include <QTextStream>
#include "runtime/runwindow.h"

//using namespace std;
Q_DECL_EXPORT
void RunSmallVMCode(QWidget *parent,
                    QString pathOfProgramsFile,
                    VMClient *client,
                    QString program,
                    QMap<QString, QString> stringConstants,
                    QSet<Breakpoint> breakPoints,
                    DebugInfo debugInfo)
{
    RunWindow *rw = new RunWindow(parent, pathOfProgramsFile, client);
    rw->show();
    rw->Init(program, stringConstants, breakPoints, debugInfo);
}

QString decodeBase64(QString source)
{
    QByteArray base64(source.toAscii());
    //cout << "byte array count:" << source.count() << endl;

    QByteArray original = QByteArray::fromBase64(base64);
    QString ret = QString::fromUtf8(original.data(), original.count());

    //wcout << "String after decoding:" << ret.toStdWString() << endl;
    //cout.flush();
    //wcout.flush();

    return ret;

}

extern "C"
{
Q_DECL_EXPORT
void RunSmallVMCodeBase64(char *pathOfProgramsFile,
                          char *programBase64)
{
    int argc = 1;
    char *argv[1] = {"aa.exe"};
    QApplication app(argc, argv);

    //cout << "program before decoding:" <<programBase64 << endl;
    RunWindow *rw = new RunWindow(NULL, pathOfProgramsFile, new NullaryVMClient());
    QString programCode = decodeBase64(programBase64);
    rw->show();
    rw->Init(programCode, QMap<QString, QString>(), QSet<Breakpoint>(), DebugInfo());
    app.exec();
}
}
int main(int argc, char *argv[])
{

    return 0;
}
