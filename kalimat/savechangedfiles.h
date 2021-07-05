/**************************************************************************
** The Kalimat programming language
**   Copyright (C) 2010 Mohamed Samy Ali - samy2004@gmail.com
**   This project is released under the Apache License version 2.0
**   as described in the included license.txt file
**************************************************************************/

#ifndef SAVECHANGEDFILES_H
#define SAVECHANGEDFILES_H

#include <QDialog>
#include <QVector>
#include <QAbstractButton>
namespace Ui {
    class SaveChangedFiles;
}

class SaveChangedFiles : public QDialog {
    Q_OBJECT
public:
    SaveChangedFiles(QWidget *parent = 0);
    ~SaveChangedFiles();
    void addFile(int index, QString fileName);
    QVector<int> getSelectedIndices();
protected:
    void changeEvent(QEvent *e);

private:
    Ui::SaveChangedFiles *ui;

private slots:
    void on_buttonBox_clicked(QAbstractButton* button);
};

#endif // SAVECHANGEDFILES_H
