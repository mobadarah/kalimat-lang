/**************************************************************************
**   The Kalimat programming language
**   Copyright (C) 2010 Mohamed Samy Ali - samy2004@gmail.com
**   This project is released under the Apache License version 2.0
**   as described in the included license.txt file
**************************************************************************/

#include "savechangedfiles.h"
#include "ui_savechangedfiles.h"

#include <QMessageBox>
#include <QDesktopWidget>
void CenterWidget(QWidget *widget)
{
        QDesktopWidget desktopWidget;
        QRect desktopRect(desktopWidget.availableGeometry(desktopWidget.primaryScreen()));
        QRect widgetRect(widget->rect());
        widget->move(desktopRect.center() - widgetRect.center());
}

SaveChangedFiles::SaveChangedFiles(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SaveChangedFiles)
{
    ui->setupUi(this);
    CenterWidget(this);
}

void SaveChangedFiles::addFile(int index, QString fileName)
{
    ui->listWidget->addItem(fileName);
    ui->listWidget->selectAll();
}

QVector<int> SaveChangedFiles::getSelectedIndices()
{
    QVector<int> ret;
    for(int i=0; i<ui->listWidget->count();i++)
    {
        if(ui->listWidget->item(i)->isSelected())
            ret.append(i);
   }
    return ret;
}

SaveChangedFiles::~SaveChangedFiles()
{
    delete ui;
}

void SaveChangedFiles::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}


void SaveChangedFiles::on_buttonBox_clicked(QAbstractButton* button)
{
    QDialogButtonBox::ButtonRole role = ui->buttonBox->buttonRole(button);
    int res;
    switch(role)
    {
    case QDialogButtonBox::DestructiveRole:
        res = QMessageBox::Discard;
        break;
    case QDialogButtonBox::AcceptRole:
        res = QMessageBox::Save;
        break;
    case QDialogButtonBox::RejectRole:
        res = QMessageBox::Cancel;
        break;
    default: // skip warning for other dialog roles
        break;
    }
    setResult(res);
    done(res);
}
