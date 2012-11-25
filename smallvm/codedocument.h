/**************************************************************************
** The Kalimat programming language
**   Copyright (C) 2010 Mohamed Samy Ali - samy2004@gmail.com
**   This project is released under the Apache License version 2.0
**   as described in the included license.txt file
**************************************************************************/

#ifndef CODEDOCUMENT_H
#define CODEDOCUMENT_H

#include <QTextEdit>
#include <QTabWidget>
#include "utils.h"

class CodeDocument;
class DocumentContainer;
class RecentFileHandler
{
public:
    virtual void onFileTouched(QString fileName, CodeDocument *doc) = 0;
};

enum CodeDocumentMessage
{
    SaveChangesInFile1,
    AppName0
};

class CodeDocument : public QObject
{
    Q_OBJECT
    static Translation<CodeDocumentMessage> docMessages;
private:
    bool _isDirty, isNewFile;
    QString _fileName;
    QTextEdit *editor;
    QTabWidget *tabs;
    RecentFileHandler *listener;
    CodeDocument(QString fileName, QTabWidget *tabs, QWidget *tabWidget, RecentFileHandler *listener);
public:

    static CodeDocument *newDoc(QString fileName, QTabWidget *tabs, QWidget *tabWidget, RecentFileHandler *listener);
    static CodeDocument *openDoc(QString fileName, QTabWidget *tabs, QWidget *tabWidget, RecentFileHandler *listener);
    virtual ~CodeDocument();
    QTextEdit *getEditor();
    QTabWidget *getTab() {return tabs;}
    void setDirty(bool dirty);
    bool isFileDirty();
    bool isDocNewFile();
    bool canDiscard();
    QString getFileName();
    bool doSave();
    void doSaveAs();
    void setFileName(QString);
private:
    void initTabLink(QTabWidget *tabBar, QWidget * tabWidget);
    void load();
    void save();

    bool GetSaveFilename();
    void setTabText(QString);
    QString getTabText();
public slots:
    void editor_textChanged();
    void editor_cursorPositionChanged();

};

#endif // CODEDOCUMENT_H
