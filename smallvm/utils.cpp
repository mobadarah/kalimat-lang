/**************************************************************************
**   The Kalimat programming language
**   Copyright (C) 2010 Mohamed Samy Ali - samy2004@gmail.com
**   This project is released under the Apache License version 2.0
**   as described in the included license.txt file
**************************************************************************/

#include "utils.h"
#include <QStringList>
//#include <iostream>
using namespace std;
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
