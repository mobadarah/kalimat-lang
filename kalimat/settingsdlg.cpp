#include "settingsdlg.h"
#include "ui_settings.h"

SettingsDlg::SettingsDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::settings)
{
    ui->setupUi(this);
}

void SettingsDlg::init(int editorFontSize, bool isDemoMode)
{
    ui->chkShowXYInRunWindow->setChecked(isDemoMode);
    ui->spnEditorFontSize->setValue(editorFontSize);
}

void SettingsDlg::getResult(int &fontSize, bool &isDemoMode)
{
    fontSize = ui->spnEditorFontSize->value();
    isDemoMode = ui->chkShowXYInRunWindow->isChecked();
}

SettingsDlg::~SettingsDlg()
{
    delete ui;
}
