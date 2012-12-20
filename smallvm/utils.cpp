/**************************************************************************
**   The Kalimat programming language
**   Copyright (C) 2010 Mohamed Samy Ali - samy2004@gmail.com
**   This project is released under the Apache License version 2.0
**   as described in the included license.txt file
**************************************************************************/

#include "utils.h"
#include <QStringList>
//#include <iostream>

#ifdef Q_OS_WIN
    #include <windows.h> // for Sleep
#endif
#include <time.h>

using namespace std;

void mySleep(int ms)
{
    if(ms <= 0)
        return;
#ifdef Q_OS_WIN
    Sleep(uint(ms));
#else
    timespec ts = { ms / 1000, (ms % 1000) * 1000 * 1000 };
    nanosleep(&ts, NULL);
#endif
}

QString str(int i)
{
    return QString("%1").arg(i);
}

QString _ws(QStdWString str)
{
    return QString::fromStdWString(str);
}

QString base64encode(QString other)
{
    QByteArray arr = other.toUtf8();

    QByteArray arr2 = arr.toHex();

    return QString(arr2);
}

QString base64Decode(QString source)
{
    //cout << "byte array count:" << source.count() << endl;

    QByteArray original = QByteArray::fromHex(source.toUtf8());
    QString ret = QString::fromUtf8(original.data());

    //wcout << "String length after decoding:" << ret.length() << endl;
    //cout.flush();
    //wcout.flush();

    return ret;

}

int Labeller::labelOf(QString str)
{
    if(!labelMap.contains(str))
    {
        labelMap[str] = count++;
    }
    return labelMap.value(str, -1);
}

QString readFile(QString path)
{
    QFile file(path);
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    QTextStream out(&file);
    QString ret = out.readAll();
    file.close();
    return ret;
}

LineIterator Utils::readResourceTextFile(QString fileName)
{
    LineIterator iter;

    QFile *inputFile = new QFile(fileName);

    inputFile->open(QIODevice::ReadOnly | QIODevice::Text);
    QTextStream *in = new QTextStream(inputFile);

    iter.file = inputFile;
    iter.stream = in;
    return iter;
}

QMap<QString, QString> Utils::readAequalBFile(LineIterator &iter)
{
    QMap<QString, QString> ret;
    while(!iter.atEnd())
    {
        QString line = iter.readLine();
        QStringList s = line.split('=');
        ret[s[0]] = s[1];
    }
    return ret;
}

QString LineIterator::readLine()
{
    return stream->readLine();
}

QString LineIterator::readAll()
{
    return stream->readAll();
}

bool LineIterator::atEnd()
{
    return stream->atEnd();
}

void LineIterator::close()
{
    stream->device()->close();
}

LineIterator::~LineIterator()
{
    delete stream;
    delete file;
}
