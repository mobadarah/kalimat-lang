#include "settingsdlg.h"
#include "ui_settingsdlg.h"

SettingsDlg::SettingsDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::settings)
{
    ui->setupUi(this);
}

void SettingsDlg::init(int editorFontSize, bool isDemoMode, int codeModelUpdateInterval)
{
    ui->chkShowXYInRunWindow->setChecked(isDemoMode);
    ui->spnEditorFontSize->setValue(editorFontSize);
    ui->spnUpdateCodeModel->setValue(codeModelUpdateInterval);
}

void SettingsDlg::getResult(int &fontSize, bool &isDemoMode, int &codeModelUpdateInterval)
{
    fontSize = ui->spnEditorFontSize->value();
    isDemoMode = ui->chkShowXYInRunWindow->isChecked();
    codeModelUpdateInterval = ui->spnUpdateCodeModel->value();
}

SettingsDlg::~SettingsDlg()
{
    delete ui;
}
