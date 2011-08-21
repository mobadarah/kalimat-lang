/**************************************************************************
**   The Kalimat programming language
**   Copyright (C) 2010 Mohamed Samy Ali - samy2004@gmail.com
**   This project is released under the Apache License version 2.0
**   as described in the included license.txt file
**************************************************************************/

#include "utils.h"

QString _ws(QStdWString str)
{
    return QString::fromStdWString(str);
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
