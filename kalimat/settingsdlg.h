#ifndef SETTINGS_H
#define SETTINGS_H

#include <QDialog>

namespace Ui {
    class settings;
}

class SettingsDlg : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDlg(QWidget *parent = 0);
    void init(int editorFontSize,
              QFont editorFont,
              bool isDemoMode,
              int codeModelUpdateInterval, QString standardModulePath);

    void getResult(int &fontSize,
                   QFont & editorFontName,
                   bool &isDemoMode,
                   int &codeModelUpdateInterval,
                   QString &standardModulePath);
    ~SettingsDlg();

private slots:
    void on_btnSelectStandardLibPath_clicked();

private:
    Ui::settings *ui;
};

#endif // SETTINGS_H
