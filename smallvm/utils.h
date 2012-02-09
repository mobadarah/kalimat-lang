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
#include <QSet>
QString _ws(QStdWString str);
QString readFile(QString path);
QString base64encode(QString other);
QString base64Decode(QString source);

template<class T> void appendAll(QVector<T> &a, QVector<T> b)
{
    for(int i=0; i<b.count(); i++)
        a.append(b[i]);
}

template<class T> void appendAll(QSet<T> &a, QSet<T> b)
{
    auto i=b.begin();
    while(i != b.end())
    {
        a.insert(*i);
        ++i;
    }
}

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
    template<class ErrTypeEnum> static QMap<ErrTypeEnum, QString> prepareErrorMap(QString fileName)
    {
        QMap<ErrTypeEnum, QString> ErrorMap;
        LineIterator in = Utils::readResourceTextFile(fileName);
        int i=0;
        while(!in.atEnd())
        {
            QString val = in.readLine().trimmed();
            ErrorMap[(ErrTypeEnum) i++] = val;
        }
        in.close();
        return ErrorMap;
    }
};

#endif // UTILS_H
