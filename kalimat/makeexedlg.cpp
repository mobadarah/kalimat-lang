#include "makeexedlg.h"
#include "ui_makeexedlg.h"
#include "idemessages.h"
#include <QFileDialog>

MakeExeDlg::MakeExeDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MakeExeDlg)
{
    ui->setupUi(this);
#ifdef ENGLISH_PL
    setLayoutDirection(Qt::LeftToRight);
#endif
    targetFilename = "";
    copyDll = true;
    iconIndex = 0;
    customIconFile = "";
    accepted = false;
}

MakeExeDlg::~MakeExeDlg()
{
    delete ui;
}

void MakeExeDlg::on_optIco1_toggled(bool checked)
{
    if(checked)
    {
        ui->btnSelectIcon->setEnabled(false);
        iconIndex = 0;
    }
}

void MakeExeDlg::on_optIco2_toggled(bool checked)
{
    if(checked)
    {
        ui->btnSelectIcon->setEnabled(false);
        iconIndex = 1;
    }
}

void MakeExeDlg::on_optIco3_toggled(bool checked)
{
    if(checked)
    {
        ui->btnSelectIcon->setEnabled(false);
        iconIndex = 2;
    }
}

void MakeExeDlg::on_btnSelectExeLocation_clicked()
{
    QString selFilter = "Executable file (*.exe)";
    targetFilename = QFileDialog::getSaveFileName(this,
                                                 Ide::msg[IdeMsg::ChooseExePath],
                                                 "", "Executable file (*.exe)",
                                                 &selFilter);
    QFileInfo info(targetFilename);
    if(QFile::exists(info.path()))
    {
        targetPath = info.path();
        ui->txtExePath->setText(targetFilename);
    }
}

void MakeExeDlg::on_optCustomIcon_toggled(bool checked)
{
    if(checked)
    {
        iconIndex = 4;
        ui->btnSelectIcon->setEnabled(true);
    }
}

void MakeExeDlg::on_btnSelectIcon_clicked()
{
    QString selFilter = "Bitmap File(*.bmp)";
    QString filename = QFileDialog::getOpenFileName(this,
                                                 Ide::msg[IdeMsg::ChooseExePath],
                                                 "", "Bitmap File(*.bmp)|PNG file (*.png)",
                                                 &selFilter);
    if(QFile::exists(filename))
    {
        QImage img;
        if(img.load(filename))
        {
            ui->optCustomIcon->setText("");
            ui->optCustomIcon->setIcon(QPixmap::fromImage(img));
            customIconFile = targetFilename;
        }
    }
}

void MakeExeDlg::on_btnCancel_clicked()
{
    this->close();
}

void MakeExeDlg::on_btnMakeExe_clicked()
{
    if(targetFilename == "" || !QFile::exists(targetPath))
    {
        QMessageBox box(QMessageBox::Information,
                        QString::fromStdWString(L"خطأ"),
                        QString::fromStdWString(L"يرجى اختيار مكان الملف التنفيذي"));
        box.exec();
        return;
    }
    if(iconIndex == 3)
    {
        bool hasImg = false;
        if(QFile::exists(customIconFile))
        {
            QImage img;
            hasImg = img.load(customIconFile);
        }
        if(!hasImg)
        {
            QMessageBox box(QMessageBox::Information,
                            QString::fromStdWString(L"خطأ"),
                            QString::fromStdWString(L"خطأ في الصورة الرمزية المختارة"));
            box.exec();
            return;
        }
    }
    accepted = true;
    this->close();
}
