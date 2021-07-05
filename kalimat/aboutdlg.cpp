#include "aboutdlg.h"
#include "ui_aboutdlg.h"

AboutDlg::AboutDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AboutDlg)
{
    ui->setupUi(this);
#ifdef ENGLISH_PL
    setLayoutDirection(Qt::LeftToRight);
#endif
}

AboutDlg::~AboutDlg()
{
    delete ui;
}
