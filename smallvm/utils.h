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
#include <QMessageBox>


void mySleep(int ms);

QString _ws(QStdWString str);
QString readFile(QString path);
QString base64encode(QString other);
QString base64Decode(QString source);

QString str(int i);

template<class T> QSet<T> setOf(T t1)
{
    QSet<T> ret;
    ret.insert(t1);
    return ret;
}

template<class T> QSet<T> setOf(T t1, T t2)
{
    QSet<T> ret;
    ret.insert(t1);
    ret.insert(t2);
    return ret;
}

template<class T> QSet<T> setOf(T t1, T t2, T t3)
{
    QSet<T> ret;
    ret.insert(t1);
    ret.insert(t2);
    ret.insert(t3);
    return ret;
}

template<class T> QSet<T> setOf(T t1, T t2, T t3, T t4)
{
    QSet<T> ret;
    ret.insert(t1);
    ret.insert(t2);
    ret.insert(t3);
    ret.insert(t4);
    return ret;
}

template <typename T> bool isa(void * obj)
{
    T value = dynamic_cast<T>(obj);
    return value != NULL;
}

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

struct Labeller
{
    QMap<QString, int> labelMap;
    int count;
    Labeller() { count = 0;}
    int labelOf(QString str);
};

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
    static QVector<QString> prepareErrorVector(QString fileName)
    {
        QVector<QString> errorVector;
        LineIterator in = Utils::readResourceTextFile(fileName);
        while(!in.atEnd())
        {
            QString val = in.readLine().trimmed();
            errorVector.append(val);
        }
        in.close();
        return errorVector;
    }
};

template<class ErrTypeEnum> class Translation
{
    QVector<QString> table;
    QString filename;
public:
    Translation(QString filename): filename(filename)
    {

    }

    inline QString operator[](ErrTypeEnum msgId)
    {
        return get(msgId);
    }

    QString get(ErrTypeEnum msgId)
    {
        if(table.empty())
        {
            table = Utils::prepareErrorVector(filename);
        }

        return table[msgId];
    }

    QString get(ErrTypeEnum msgId, QString arg0)
    {
        if(table.empty())
            table = Utils::prepareErrorVector(filename);
        return table[msgId].arg(arg0);
    }

    QString get(ErrTypeEnum msgId, QString arg0, QString arg1)
    {
        if(table.empty())
            table = Utils::prepareErrorVector(filename);
        return table[msgId].arg(arg0).arg(arg1);
    }

    QString get(ErrTypeEnum msgId, QString arg0, QString arg1, QString arg2)
    {
        if(table.empty())
            table = Utils::prepareErrorVector(filename);
        return table[msgId].arg(arg0).arg(arg1).arg(arg2);
    }
};

#endif // UTILS_H
