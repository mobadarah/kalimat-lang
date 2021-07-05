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
#include "idemessages.h"
#include "../smallvm/utils.h"

class DocumentClient
{
public:
    virtual void LoadDocIntoWidget(CodeDocument *doc, QWidget *widget) = 0;
    virtual QWidget *GetParentWindow() = 0;
    virtual QWidget *CreateEditorWidget() = 0;
};

typedef void (*DocProc)(CodeDocument *, QWidget *, void *);

class DocumentContainer : public QObject, public RecentFileHandler
{
    Q_OBJECT
private:
    Translation<IdeMsg::IdeMessage> &msg;
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
                      QMenu *recentFileMenu,
                      Translation<IdeMsg::IdeMessage> &msg);
    virtual ~DocumentContainer();
    void handleNew(QString prefix,QWidget *editor);
    void handleOpen();
    void handleSave();
    void handleSaveAs();
    void handleClose(QCloseEvent *ev);
    void handleTabCloseRequested(int index);
    QString OpenExistingFiles(const QStringList& fileNames);
    void OpenOrSwitch(const QString fileName);

    void updateRecentFiles();
    CodeDocument *getDocumentFromTab(int index);
    CodeDocument *addDocument(QString title, QString fileName, QWidget *editor, bool createNew);
    CodeDocument *addInitialEmptyDocument();
    bool hasOpenDocument(CodeDocument *doc);
    void removeInitialEmptyDocument();
    CodeDocument *getCurrentDocument();
    CodeDocument *getDocumentFromWidget(QWidget *w);
    void setCurrentDocument(CodeDocument *doc);
    CodeDocument *getDocumentFromPath(QString path, bool addIfNeeded);
    int documentCount() { return widgetDocs.count(); }
    void onFileTouched(QString fileName, CodeDocument *doc);
    void forAll(DocProc, void *);
    void forAll(DocProc);
private slots:
    void recentfile_triggered();
};

#endif // DOCUMENTCONTAINER_H
