#ifndef MAKEEXEDLG_H
#define MAKEEXEDLG_H

#include <QDialog>

namespace Ui {
class MakeExeDlg;
}

class MakeExeDlg : public QDialog
{
    Q_OBJECT
    
public:
    explicit MakeExeDlg(QWidget *parent = 0);
    ~MakeExeDlg();
public:
    QString targetFilename;
    QString targetPath;
    int iconIndex;
    QString customIconFile;
    bool accepted;
    bool copyDll;
private slots:
    void on_optIco1_toggled(bool checked);

    void on_optIco2_toggled(bool checked);

    void on_optIco3_toggled(bool checked);

    void on_btnSelectExeLocation_clicked();

    void on_optCustomIcon_toggled(bool checked);

    void on_btnSelectIcon_clicked();

    void on_btnCancel_clicked();

    void on_btnMakeExe_clicked();

private:
    Ui::MakeExeDlg *ui;

};

#endif // MAKEEXEDLG_H
