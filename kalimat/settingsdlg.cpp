#include "settingsdlg.h"
#include "ui_settingsdlg.h"

#include <QFileDialog>
#include <QFont>

SettingsDlg::SettingsDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::settings)
{
    ui->setupUi(this);
#ifdef ENGLISH_PL
    setLayoutDirection(Qt::LeftToRight);
#endif
}

void SettingsDlg::init(int editorFontSize,
                       QFont editorFont,
                       bool isDemoMode,
                       int codeModelUpdateInterval,
                       QString standardModulePath)
{
    ui->chkShowXYInRunWindow->setChecked(isDemoMode);
    ui->spnEditorFontSize->setValue(editorFontSize);
    ui->fontComboBox->setWritingSystem(QFontDatabase::Arabic);
    ui->fontComboBox->setFont(editorFont);
    ui->spnUpdateCodeModel->setValue(codeModelUpdateInterval);
    ui->lblStandardLibPath->setText(standardModulePath);
}

void SettingsDlg::getResult(int &fontSize,
                            QFont &editorFont,
                            bool &isDemoMode,
                            int &codeModelUpdateInterval,
                            QString &standardModulePath)
{
    fontSize = ui->spnEditorFontSize->value();

    // Workaround for this bug:
    // https://bugreports.qt-project.org/browse/QTBUG-13369
    //ui->fontComboBox->setCurrentIndex(0);
    //ui->fontComboBox->setCurrentIndex(1);

    editorFont = ui->fontComboBox->currentFont();
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
