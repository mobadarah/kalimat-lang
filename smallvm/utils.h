/**************************************************************************
** The Kalimat programming language
**   Copyright (C) 2010 Mohamed Samy Ali - samy2004@gmail.com
**   This project is released under the Apache License version 2.0
**   as described in the included license.txt file
**************************************************************************/

#ifndef UTILS_H
#define UTILS_H

#include <QFile>
#include <QTextStream>
#include <QString>
#include <QMap>

QString _ws(QStdWString str);
QString readFile(QString path);
QString base64encode(QString other);
QString base64Decode(QString source);

class LineIterator
{
public:
    QTextStream *stream;
    QFile *file;
public:
    ~LineIterator();
    QString readLine();
    QString readAll();
    bool atEnd();
    void close();
};

class Utils
{
public:
    static LineIterator readResourceTextFile(QString fileName);
    static QMap<QString, QString> readAequalBFile(LineIterator &iter);
};

#endif // UTILS_H
