/**************************************************************************
**   The Kalimat programming language
**   Copyright (C) 2010 Mohamed Samy Ali - samy2004@gmail.com
**   This project is released under the Apache License version 2.0
**   as described in the included license.txt file
**************************************************************************/

#include "documentcontainer.h"

#include <QSettings>
#include <QDir>
#include <QFileDialog>
#include <QMessageBox>
#include <QCloseEvent>
#include <QAction>
#include <QMenu>
#include "savechangedfiles.h"

DocumentContainer::DocumentContainer(QString settingsOrganizationName,
                                     QString settingsApplicationName,
                                     QString documentFilter,
                                     QTabWidget *tabWidget,
                                     DocumentClient *client,
                                     int MaxRecentFiles,
                                     QMenu *recentFileMenu)
    :QObject(client->GetParentWindow())
{
    newDocCount = 0;
    hasInitialEmptyDocument = false;
    this->settingsApplicationName = settingsApplicationName;
    this->settingsOrganizationName = settingsOrganizationName;
    this->documentFilter = documentFilter;
    this->tabWidget = tabWidget;
    this->client = client;
    this->MaxRecentFiles = MaxRecentFiles;
    this->recentFileMenu = recentFileMenu;
    recentFileMenu->clear();

    for(int i=0; i<MaxRecentFiles; i++)
    {
        QAction *rf = recentFileMenu->addAction("");
        rf->setVisible(false);
        connect(rf, SIGNAL(triggered()), this, SLOT(recentfile_triggered()));
        recentFileActions.append(rf);
    }
    updateRecentFiles();


}
DocumentContainer::~DocumentContainer()
{

}

void DocumentContainer::handleNew(QString prefix, QWidget *editor)
{
    removeInitialEmptyDocument();
    QString name = QString("%1 %2").arg(prefix).arg(++newDocCount);
    addDocument(name, name, editor, true);
}

CodeDocument *DocumentContainer::addDocument(QString title, QString fileName, QWidget *editor, bool createNew)
{
    CodeDocument *doc;

    int tabIndex = tabWidget->addTab(editor, title);
    tabWidget->setCurrentIndex(tabIndex);

    
    if(!createNew)
    {
        doc = CodeDocument::openDoc(fileName, tabWidget, editor, this);
    }
    else
    {
        doc = CodeDocument::newDoc(fileName, tabWidget, editor, this);
    }
    widgetDocs[editor] = doc;
    editor->setFocus();
    return doc;
}

CodeDocument *DocumentContainer::addInitialEmptyDocument()
{
    addDocument(QString::fromWCharArray(L"بدون عنوان"), "untitled", client->CreateEditorWidget(), true);
    hasInitialEmptyDocument = true;
}

bool DocumentContainer::hasOpenDocument(CodeDocument *doc)
{
    for(int i=0; i<tabWidget->count(); i++)
    {
        if(widgetDocs.contains(tabWidget->widget(i)))
        {
            if(widgetDocs[tabWidget->widget(i)] == doc)
                return true;
        }
    }
    return false;
}

void DocumentContainer::removeInitialEmptyDocument()
{
    if(hasInitialEmptyDocument &&tabWidget->count() !=0)
    {
        hasInitialEmptyDocument = false;
        if(getCurrentDocument()->isDocNewFile() && !getCurrentDocument()->isFileDirty())
        {
            handleTabCloseRequested(0);
        }
    }
}

CodeDocument *DocumentContainer::getCurrentDocument()
{
    if(tabWidget->count() == 0)
        return NULL;
    return widgetDocs[tabWidget->currentWidget()];
}

CodeDocument *DocumentContainer::getDocumentFromWidget(QWidget *w)
{
    if(tabWidget->count() == 0)
        return NULL;
    if(!widgetDocs.contains(w))
        return NULL;
    return widgetDocs[w];
}

bool DocumentContainer::setCurrentDocument(CodeDocument *doc)
{
    tabWidget->setCurrentWidget(doc->getEditor());
}

CodeDocument *DocumentContainer::getDocumentFromPath(QString path, bool addIfNeeded)
{
    for(int i=0; i<widgetDocs.values().count(); i++)
    {
        if(!widgetDocs.values()[i]->isDocNewFile() && widgetDocs.values()[i]->getFileName() == path)
            return widgetDocs.values()[i];
    }
    if(addIfNeeded)
    {
        CodeDocument *doc = addDocument(QFileInfo(path).fileName(), path, client->CreateEditorWidget(), false);
        client->LoadDocIntoWidget(doc, doc->getEditor());
        return doc;
    }
    return NULL;
}

CodeDocument *DocumentContainer::getDocumentFromTab(int index)
{
    return widgetDocs[tabWidget->widget(index)];
}

void DocumentContainer::updateRecentFiles()
{
    QSettings settings(settingsOrganizationName, settingsApplicationName);

    QStringList recent = settings.value("recent_files", QStringList()).toStringList();
    recentFileList.clear();
    recentFileList.append(recent);

    for(int i=0; i<recentFileList.count(); i++)
    {
        recentFileActions[i]->setText(QFileInfo(recentFileList.at(i)).fileName());
        recentFileActions[i]->setData(recentFileList.at(i));
        recentFileActions[i]->setVisible(true);
    }
    for(int j=recentFileList.count(); j< MaxRecentFiles; j++)
    {
        recentFileActions[j]->setVisible(false);
    }
}

void DocumentContainer::recentfile_triggered()
{
    QString fileName = "";
    QAction *action = qobject_cast<QAction *>(sender());
    if (action)
        fileName = action->data().toString();

    if(!fileName.isEmpty() && QFile::exists(fileName))
    {
        // If the file is already open; we switch to its
        // existing tab instead of opening it in a new tab
        bool found = false;
        for(int i=0; i<tabWidget->count(); i++)
        {
            CodeDocument *doc = getDocumentFromTab(i);
            if(doc->getFileName() == fileName)
            {
                found = true;
                tabWidget->setCurrentIndex(i);
                break;
            }
        }
        if(!found)
        {
            removeInitialEmptyDocument();
            addDocument(QFileInfo(fileName).fileName(), fileName, client->CreateEditorWidget(), false);
        }
    }
}

void DocumentContainer::handleOpen()
{
    QString dir = "";
    QSettings settings(settingsOrganizationName, settingsApplicationName);
    dir = settings.value("last_open_dir", "").toString();
    if(dir != "")
    {
        QDir d = QDir(dir);
        if(!d.exists())
            dir = "";
    }
    QFileDialog dlg(client->GetParentWindow(),
                    QString("Open program"),
                    dir,
                    documentFilter);
    QString fileName;
    dlg.setFileMode(QFileDialog::ExistingFiles);
    dlg.setAcceptMode(QFileDialog::AcceptOpen);
    if(dlg.exec())
    {
        int n = dlg.selectedFiles().count();
        bool anyLoaded = false;
        dir = OpenExistingFiles(dlg.selectedFiles());

        // set last opened Dir
        if (!dir.isEmpty())
        {
            settings.setValue("last_open_dir", dir);
        }
    }
}

void DocumentContainer::handleSave()
{
    CodeDocument *doc = getCurrentDocument();
    if(doc != NULL)
    {
        doc->doSave();
    }
}

void DocumentContainer::handleSaveAs()
{
    CodeDocument *doc = getCurrentDocument();
    if(doc != NULL)
    {
        doc->doSaveAs();
    }
}

void DocumentContainer::handleClose(QCloseEvent *ev)
{
    bool moveOn;

    QMap<int, CodeDocument*> dirtyDirtyFiles;
    QVector<int> indices;
    for(int i=0; i<tabWidget->count();i++)
    {
        CodeDocument *doc = getDocumentFromTab(i);
        if(doc->isFileDirty())
        {
            dirtyDirtyFiles[i] = doc;
            indices.append(i);
        }
    }
    if(dirtyDirtyFiles.count() == 0)
    {
        moveOn = true;
    }
    else if(dirtyDirtyFiles.count()==1)
    {
        CodeDocument *doc = dirtyDirtyFiles.values()[0];
        moveOn = doc->canDiscard();
    }
    else
    {
        SaveChangedFiles dlg;
        for(int i=0; i<indices.count(); i++)
        {
            dlg.addFile(indices[i], dirtyDirtyFiles[indices[i]]->getFileName());
        }
        dlg.setWindowTitle("");
        //dlg.setModal(true);
        int result = dlg.exec();
        result = dlg.result();
        if(result == QMessageBox::Save)
        {
            QVector<int> selectedIndices = dlg.getSelectedIndices();
            bool ret = true;
            for(int i=0; i<selectedIndices.count(); i++)
            {
                CodeDocument *doc = dirtyDirtyFiles[indices[selectedIndices[i]]];
                ret = ret & doc->doSave();
            }
            moveOn = ret;
        }
        else if(result == QMessageBox::Discard)
        {
            moveOn = true;
        }
        else if(result == QMessageBox::Cancel)
        {
            moveOn = false;
        }
        else
        {

        }
    }

    if(moveOn)
    {
        ev->accept();
    }
    else
    {
        ev->ignore();
    }
}

void DocumentContainer::handleTabCloseRequested(int index)
{
    CodeDocument *doc = getDocumentFromTab(index);
    if(doc->canDiscard())
    {
        QWidget *widget = doc->getEditor();
        widgetDocs.remove(widget);
        tabWidget->removeTab(index);
    }
}

void DocumentContainer::onFileTouched(QString fileName, CodeDocument *doc)
{
    recentFileList.removeAll(fileName);
    recentFileList.prepend(fileName);
    if(recentFileList.count() > MaxRecentFiles)
        recentFileList.removeLast();
    QSettings settings(settingsOrganizationName, settingsApplicationName);
    QStringList recent;
    recent.append(recentFileList);
    settings.setValue("recent_files", recent);
    updateRecentFiles();
}

void DocumentContainer::OpenOrSwitch(const QString fileName)
{
    // First, try to switch
    for(int i=0; i<tabWidget->count();i++)
    {
        CodeDocument *doc = getDocumentFromTab(i);
        if(!doc->isDocNewFile() &&
                doc->getFileName() == fileName)
        {
            setCurrentDocument(doc);
            return;
        }
    }
    // If we reached here, we have to open
    CodeDocument *doc = addDocument(QFileInfo(fileName).fileName(), fileName, client->CreateEditorWidget(), false);
    client->LoadDocIntoWidget(doc, doc->getEditor());
}

// Opens an existing file, and returns the parent directory path
QString DocumentContainer::OpenExistingFiles(const QStringList& fileNames)
{
    int     n = fileNames.count();
    bool    anyLoaded = false;
    QString fileName;
    QString dir = "";

    for(int i=0; i<n; i++)
    {
        fileName = fileNames[i];

        if(!fileName.isEmpty())
        {
            if(!anyLoaded)
            {
                anyLoaded = true;
                removeInitialEmptyDocument();
            }
            QFileInfo f = QFileInfo(fileName);
            dir = f.absoluteDir().absolutePath();
            CodeDocument *doc = addDocument(QFileInfo(fileName).fileName(), fileName, client->CreateEditorWidget(), false);
            client->LoadDocIntoWidget(doc, doc->getEditor());
        }
    }

    return dir;
}

