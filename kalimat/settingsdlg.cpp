#include "settingsdlg.h"
#include "ui_settingsdlg.h"

#include <QFileDialog>
SettingsDlg::SettingsDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::settings)
{
    ui->setupUi(this);
}

void SettingsDlg::init(int editorFontSize,
                       bool isDemoMode,
                       int codeModelUpdateInterval,
                       QString standardModulePath)
{
    ui->chkShowXYInRunWindow->setChecked(isDemoMode);
    ui->spnEditorFontSize->setValue(editorFontSize);
    ui->spnUpdateCodeModel->setValue(codeModelUpdateInterval);
    ui->lblStandardLibPath->setText(standardModulePath);
}

void SettingsDlg::getResult(int &fontSize,
                            bool &isDemoMode,
                            int &codeModelUpdateInterval,
                            QString &standardModulePath)
{
    fontSize = ui->spnEditorFontSize->value();
    isDemoMode = ui->chkShowXYInRunWindow->isChecked();
    codeModelUpdateInterval = ui->spnUpdateCodeModel->value();
    standardModulePath = ui->lblStandardLibPath->text();
}

SettingsDlg::~SettingsDlg()
{
    delete ui;
}

void SettingsDlg::on_btnSelectStandardLibPath_clicked()
{
    QString s = QFileDialog::getExistingDirectory(this);
    if(s != "")
        ui->lblStandardLibPath->setText(s);
}
