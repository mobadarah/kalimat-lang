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
    void init(int editorFontSize, bool isDemoMode, int codeModelUpdateInterval);
    void getResult(int &fontSize, bool &isDemoMode, int &codeModelUpdateInterval);
    ~SettingsDlg();

private:
    Ui::settings *ui;
};

#endif // SETTINGS_H
