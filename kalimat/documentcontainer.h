/**************************************************************************
** The Kalimat programming language
**   Copyright (C) 2010 Mohamed Samy Ali - samy2004@gmail.com
**   This project is released under the Apache License version 2.0
**   as described in the included license.txt file
**************************************************************************/

#ifndef DOCUMENTCONTAINER_H
#define DOCUMENTCONTAINER_H

#include <QWidget>
#include <QQueue>
#include <QVector>
#include <QMap>
#include <QString>

#include "../smallvm/codedocument.h"

class DocumentClient
{
public:
    virtual void LoadDocIntoWidget(CodeDocument *doc, QWidget *widget) = 0;
    virtual QWidget *GetParentWindow() = 0;
    virtual QWidget *CreateEditorWidget() = 0;
};

class DocumentContainer : public QObject, public RecentFileHandler
{
    Q_OBJECT
private:
    int MaxRecentFiles;
    QMenu *recentFileMenu;
    DocumentClient *client;
    QString settingsOrganizationName, settingsApplicationName, documentFilter;
    int newDocCount;
    QTabWidget *tabWidget;
    QQueue<QString> recentFileList;
    QVector<QAction *> recentFileActions;
    QMap<QWidget *, CodeDocument *> widgetDocs;
    bool hasInitialEmptyDocument;
public:
    DocumentContainer(QString settingsOrganizationName,
                      QString settingsApplicationName,
                      QString documentFilter,
                      QTabWidget *tabWidget,
                      DocumentClient *client,
                      int MaxRecentFiles,
                      QMenu *recentFileMenu);
    virtual ~DocumentContainer();
    void handleNew(QString prefix,QWidget *editor);
    void handleOpen();
    void handleSave();
    void handleSaveAs();
    void handleClose(QCloseEvent *ev);
    void handleTabCloseRequested(int index);

    void updateRecentFiles();
    CodeDocument *getDocumentFromTab(int index);
    CodeDocument *addDocument(QString title, QString fileName, QWidget *editor, bool createNew);
    CodeDocument *addInitialEmptyDocument();
    void removeInitialEmptyDocument();
    CodeDocument *getCurrentDocument();
    CodeDocument *getDocumentFromPath(QString path);
    int documentCount() { return widgetDocs.count(); }
    void onFileTouched(QString fileName, CodeDocument *doc);
private slots:
    void recentfile_triggered();
};

#endif // DOCUMENTCONTAINER_H
