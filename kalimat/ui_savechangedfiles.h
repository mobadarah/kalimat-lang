/********************************************************************************
** Form generated from reading UI file 'savechangedfiles.ui'
**
** Created: Wed 1. Sep 01:54:13 2010
**      by: Qt User Interface Compiler version 4.6.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_SAVECHANGEDFILES_H
#define UI_SAVECHANGEDFILES_H

#include <QtCore/QLocale>
#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QDialog>
#include <QtGui/QDialogButtonBox>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QListWidget>

QT_BEGIN_NAMESPACE

class Ui_SaveChangedFiles
{
public:
    QDialogButtonBox *buttonBox;
    QListWidget *listWidget;
    QLabel *label;

    void setupUi(QDialog *SaveChangedFiles)
    {
        if (SaveChangedFiles->objectName().isEmpty())
            SaveChangedFiles->setObjectName(QString::fromUtf8("SaveChangedFiles"));
        SaveChangedFiles->setWindowModality(Qt::ApplicationModal);
        SaveChangedFiles->resize(391, 340);
        SaveChangedFiles->setLayoutDirection(Qt::RightToLeft);
        SaveChangedFiles->setLocale(QLocale(QLocale::Arabic, QLocale::Egypt));
        SaveChangedFiles->setModal(true);
        buttonBox = new QDialogButtonBox(SaveChangedFiles);
        buttonBox->setObjectName(QString::fromUtf8("buttonBox"));
        buttonBox->setGeometry(QRect(30, 290, 341, 32));
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Discard|QDialogButtonBox::Save);
        buttonBox->setCenterButtons(true);
        listWidget = new QListWidget(SaveChangedFiles);
        listWidget->setObjectName(QString::fromUtf8("listWidget"));
        listWidget->setGeometry(QRect(10, 60, 371, 211));
        listWidget->setSelectionMode(QAbstractItemView::MultiSelection);
        listWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
        listWidget->setUniformItemSizes(true);
        label = new QLabel(SaveChangedFiles);
        label->setObjectName(QString::fromUtf8("label"));
        label->setGeometry(QRect(50, 10, 331, 41));
        label->setLayoutDirection(Qt::RightToLeft);
        label->setLocale(QLocale(QLocale::Arabic, QLocale::Egypt));

        retranslateUi(SaveChangedFiles);

        QMetaObject::connectSlotsByName(SaveChangedFiles);
    } // setupUi

    void retranslateUi(QDialog *SaveChangedFiles)
    {
        SaveChangedFiles->setWindowTitle(QString());
        label->setText(QApplication::translate("SaveChangedFiles", "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
"<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">\n"
"p, li { white-space: pre-wrap; }\n"
"</style></head><body style=\" font-family:'Sans'; font-size:10pt; font-weight:400; font-style:normal;\">\n"
"<table border=\"0\" style=\"-qt-table-type: root; margin-top:4px; margin-bottom:4px; margin-left:4px; margin-right:4px;\">\n"
"<tr>\n"
"<td style=\"border: none;\">\n"
"<p align=\"right\" style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-size:12pt;\">\331\207\330\260\331\207 \330\247\331\204\331\205\331\204\331\201\330\247\330\252 \331\202\330\257 \330\252\330\272\331\212\330\261\330\252! \330\255\331\201\330\270 \330\247\331\204\330\252\330\272\331\212\331\212\330\261\330\247\330\252\330\237</span></p></td></tr></table></body></html>", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class SaveChangedFiles: public Ui_SaveChangedFiles {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_SAVECHANGEDFILES_H
