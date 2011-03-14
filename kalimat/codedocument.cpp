/**************************************************************************
**   The Kalimat programming language
**   Copyright (C) 2010 Mohamed Samy Ali - samy2004@gmail.com
**   This project is released under the Apache License version 2.0
**   as described in the included license.txt file
**************************************************************************/

#include <QFile>
#include <QMessageBox>
#include <QTextStream>
#include <QFileDialog>
#include <QStatusBar>
#include <QSettings>
#include <QGraphicsBlurEffect>

#include "codedocument.h"
#include "documentcontainer.h"

//#include "mainwindow.h"

CodeDocument *CodeDocument::newDoc(QString fileName, QTabWidget *tabs, QWidget *tabWidget, DocumentContainer *container)
{
    CodeDocument *ret = new CodeDocument(fileName, tabs, tabWidget, container);
    ret->isNewFile = true;
    return ret;
}

CodeDocument *CodeDocument::openDoc(QString fileName, QTabWidget *tabs, QWidget *tabWidget, DocumentContainer *container)
{
    CodeDocument *ret = new CodeDocument(fileName, tabs, tabWidget, container);
    ret->isNewFile = false;
    ret->load();
    return ret;
}

CodeDocument::CodeDocument(QString fileName, QTabWidget *tabs, QWidget *tabWidget, DocumentContainer *container) : QObject(NULL)
{
    isNewFile = true;            // prevent setFile from updating the recentFileHandler
                                 // upon construction

    this->container = container; // setFileName & setDirty use the 'container' member, always set it first
    initTabLink(tabs, tabWidget);
    setFileName(fileName);
    setDirty(false);

}
CodeDocument::~CodeDocument()
{

}

void CodeDocument::initTabLink(QTabWidget *tabBar, QWidget * tabWidget)
{
    this->tabs = tabBar;
    this->editor = (QTextEdit *) tabWidget;
    //bool ret = QObject::connect(this->editor, SIGNAL(textChanged()), this, SLOT(editor_textChanged()));
    bool ret = QObject::connect(this->editor->document(), SIGNAL(contentsChanged()), this, SLOT(editor_textChanged()));

    ret =  QObject::connect(this->editor, SIGNAL(cursorPositionChanged()), this, SLOT(editor_cursorPositionChanged()));
}
bool CodeDocument::isFileDirty()
{
    return _isDirty;
}
bool CodeDocument::isDocNewFile()
{
    return isNewFile;
}

void CodeDocument::setDirty(bool dirty)
{
    _isDirty = dirty;

    QString str = getTabText().replace("*","");
    if(dirty)
    {
        str = "*" + str;

    }
    setTabText(str);
}

QString CodeDocument::getFileName()
{
    return _fileName;
}

void CodeDocument::setFileName(QString fn)
{
    _fileName = fn;
    if(!isNewFile && container!= NULL)
    {
        container->onFileTouched(fn, this);
    }
}

QTextEdit *CodeDocument::getEditor()
{
    return editor;
}

void CodeDocument::editor_textChanged()
{
    /*
    MainWindow *mw = MainWindow::that;
    if(mw != NULL)
    {
        QString str = QString("Pos = %1").arg(editor->textCursor().position());
        mw->statusBar()->showMessage(str);
    }
    */
    // Clear the selection of error line when user continues editing code.
    // todo: since we use extraselections also to highlight the current
    // code being executed when running the tracer, a use modifying the code while
    // tracing will clear the selection. Users shouldn't do that anyway, and we probably
    // should make the editor read-only when running the tracer.
    editor->extraSelections().clear();
    setDirty(editor->document()->isModified());
}

void CodeDocument::editor_cursorPositionChanged()
{
    /*
    MainWindow *mw = MainWindow::that;
    if(mw != NULL)
    {
        QString str = QString("Pos = %1").arg(editor->textCursor().position());
        mw->statusBar()->showMessage(str);
    }
    */
}

void CodeDocument::load()
{
    QString fn = getFileName();
    if(!fn.isEmpty() && QFile::exists(fn))
    {
        QFileInfo
        setTabText(QFileInfo(fn).fileName());
        QFile f(fn);
        f.open(QIODevice::ReadOnly | QIODevice::Text);
        QTextStream out(&f);
        editor->document()->setPlainText(out.readAll());

        QTextCursor c = editor->textCursor();
        c.setPosition(0);
        editor->setTextCursor(c);
        f.close();
        setDirty(false);
        isNewFile = false;
        setFileName(fn); // 'Touch' the file to update the recent file list
    }
}

void CodeDocument::save()
{
    QString fn = getFileName();
    if(!fn.isEmpty())
    {
        QFile f(fn);
        f.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate);
        QTextStream out(&f);
        out.setCodec("UTF-8");
        out.setGenerateByteOrderMark(true);
        out << editor->document()->toPlainText();
        f.close();
        setDirty(false);
        isNewFile = false;
    }
}

bool CodeDocument::doSave()
{
    bool gotTheName = true;
    if(isNewFile)
        gotTheName = GetSaveFilename();
    if(gotTheName && !getFileName().isEmpty())
    {
        save();
        setFileName(getFileName());// touch the file, see doSaveAs()
        return true;
    }
    return false;
}

void CodeDocument::doSaveAs()
{
    bool r = GetSaveFilename();
    if(r)
    {
        save();
        setFileName(getFileName()); // just touch the file name after saving to update the recent file list
                                    // which doesn't update unless isNewFile is false
    }
}

bool CodeDocument::canDiscard()
{
    if(isFileDirty() == false)
    {
        return true;
    }
    QMessageBox box(QMessageBox::Warning,
                    QString::fromWCharArray(L"كلمات"), QString::fromStdWString(L"حفظ التغييرات في الملف '%1'؟").arg(getFileName()),
                    QMessageBox::Yes|QMessageBox::No| QMessageBox::Cancel,
                    NULL);
    int ret = box.exec();
    if(ret == QMessageBox::Yes)
    {
        return doSave();
    }
    else if(ret == QMessageBox::No)
    {
        return true;
    }
    else if(ret == QMessageBox::Cancel)
    {
        return false;
    }
    return false;
}

bool CodeDocument::GetSaveFilename()
{
    QString dir = "";
    QSettings settings("mohamedsamy", "kalimat 1.0");
    dir = settings.value("last_save_dir", "").toString();
    if(dir != "")
    {
        QDir d = QDir(dir);
        if(!d.exists())
            dir = "";
    }

    QFileDialog dlg(NULL,
                    QString("Save program: ")+ getFileName(),
                    dir,
                    tr("Kalimat code (*.k *.* *)"));
    QString fn;
    dlg.setFileMode(QFileDialog::AnyFile);
    dlg.setAcceptMode(QFileDialog::AcceptSave);
    if(dlg.exec())
        fn = dlg.selectedFiles()[0];
    
    if(!fn.isEmpty())
    {
        QFileInfo f = QFileInfo(fn);
        dir = f.absoluteDir().absolutePath();
        settings.setValue("last_save_dir", dir);

        setFileName(fn);
        setTabText(QFileInfo(fn).fileName());
        return true;
    }
    return false;
}

void CodeDocument::setTabText(QString s)
{
    int tabIndex = tabs->indexOf(editor);
    tabs->setTabText(tabIndex, s);
}

QString CodeDocument::getTabText()
{
    int tabIndex = tabs->indexOf(editor);
    return tabs->tabText(tabIndex);
}
