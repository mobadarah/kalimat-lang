/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created: Sun 5. Sep 21:22:11 2010
**      by: Qt User Interface Compiler version 4.6.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QDockWidget>
#include <QtGui/QGraphicsView>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QMainWindow>
#include <QtGui/QMenu>
#include <QtGui/QMenuBar>
#include <QtGui/QPushButton>
#include <QtGui/QStatusBar>
#include <QtGui/QTabWidget>
#include <QtGui/QTextBrowser>
#include <QtGui/QToolBar>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QAction *actionLexize;
    QAction *actionParse;
    QAction *actionEvaluate;
    QAction *action;
    QAction *action_walkthroughs;
    QAction *action_report_bug;
    QAction *action_software_update;
    QAction *action_contact_us;
    QAction *action_about_kalimat;
    QAction *action_help_keyboard;
    QAction *mnuProgramRun;
    QAction *actionCompile;
    QAction *action_save;
    QAction *action_open;
    QAction *action_verify;
    QAction *action_new;
    QAction *action_saveas;
    QAction *action_exit;
    QAction *action_edit_cut;
    QAction *action_edit_copy;
    QAction *action_edit_paste;
    QAction *action_delete;
    QAction *action_undo;
    QAction *action_redo;
    QAction *garbageCollect;
    QAction *actionDummy;
    QAction *actionLoad_Compilation_unit;
    QAction *action_wonderfulmonitor;
    QAction *action_shift;
    QAction *action_2;
    QAction *action_shift_2;
    QAction *action_shift_3;
    QAction *action_shift_4;
    QAction *action_shift_5;
    QAction *action_shift_6;
    QAction *action_find;
    QAction *action_replace;
    QAction *actionSpeedFast;
    QAction *actionSpeedMedium;
    QAction *actionSpeedSlow;
    QAction *actionCompile_without_tags;
    QAction *actionGo_to_position;
    QWidget *centralWidget;
    QTabWidget *editorTabs;
    QTabWidget *tabWidget;
    QWidget *tab;
    QVBoxLayout *verticalLayout_2;
    QTextBrowser *outputView;
    QWidget *tab_2;
    QVBoxLayout *verticalLayout;
    QGraphicsView *graphicsView;
    QMenuBar *menuBar;
    QMenu *mnuFile;
    QMenu *mnuRecentFiles_2;
    QMenu *mnuEdit;
    QMenu *mnuPRogram;
    QMenu *mnuHelp;
    QMenu *mnuTest;
    QToolBar *mainToolBar;
    QStatusBar *statusBar;
    QDockWidget *dockSearchReplace;
    QWidget *dockWidgetContents;
    QLabel *label;
    QLabel *label_2;
    QLineEdit *txtSearchString;
    QLineEdit *txtReplacementString;
    QPushButton *btnFindPrev;
    QPushButton *btnFindNext;
    QPushButton *btnReplacePrev;
    QPushButton *btnReplaceNext;
    QLabel *lblFindStatus;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QString::fromUtf8("MainWindow"));
        MainWindow->resize(782, 678);
        MainWindow->setLayoutDirection(Qt::RightToLeft);
        MainWindow->setAutoFillBackground(false);
        MainWindow->setDockNestingEnabled(false);
        actionLexize = new QAction(MainWindow);
        actionLexize->setObjectName(QString::fromUtf8("actionLexize"));
        actionParse = new QAction(MainWindow);
        actionParse->setObjectName(QString::fromUtf8("actionParse"));
        actionEvaluate = new QAction(MainWindow);
        actionEvaluate->setObjectName(QString::fromUtf8("actionEvaluate"));
        action = new QAction(MainWindow);
        action->setObjectName(QString::fromUtf8("action"));
        action_walkthroughs = new QAction(MainWindow);
        action_walkthroughs->setObjectName(QString::fromUtf8("action_walkthroughs"));
        action_report_bug = new QAction(MainWindow);
        action_report_bug->setObjectName(QString::fromUtf8("action_report_bug"));
        action_software_update = new QAction(MainWindow);
        action_software_update->setObjectName(QString::fromUtf8("action_software_update"));
        action_contact_us = new QAction(MainWindow);
        action_contact_us->setObjectName(QString::fromUtf8("action_contact_us"));
        action_about_kalimat = new QAction(MainWindow);
        action_about_kalimat->setObjectName(QString::fromUtf8("action_about_kalimat"));
        action_help_keyboard = new QAction(MainWindow);
        action_help_keyboard->setObjectName(QString::fromUtf8("action_help_keyboard"));
        mnuProgramRun = new QAction(MainWindow);
        mnuProgramRun->setObjectName(QString::fromUtf8("mnuProgramRun"));
        QIcon icon;
        icon.addFile(QString::fromUtf8(":/icons/icons/runIcon.png"), QSize(), QIcon::Normal, QIcon::Off);
        mnuProgramRun->setIcon(icon);
        actionCompile = new QAction(MainWindow);
        actionCompile->setObjectName(QString::fromUtf8("actionCompile"));
        action_save = new QAction(MainWindow);
        action_save->setObjectName(QString::fromUtf8("action_save"));
        QIcon icon1;
        icon1.addFile(QString::fromUtf8(":/icons/icons/saveIcon.png"), QSize(), QIcon::Normal, QIcon::Off);
        action_save->setIcon(icon1);
        action_open = new QAction(MainWindow);
        action_open->setObjectName(QString::fromUtf8("action_open"));
        QIcon icon2;
        icon2.addFile(QString::fromUtf8(":/icons/icons/openIcon.png"), QSize(), QIcon::Normal, QIcon::Off);
        action_open->setIcon(icon2);
        action_verify = new QAction(MainWindow);
        action_verify->setObjectName(QString::fromUtf8("action_verify"));
        QIcon icon3;
        icon3.addFile(QString::fromUtf8(":/icons/icons/checkIcon.png"), QSize(), QIcon::Normal, QIcon::Off);
        action_verify->setIcon(icon3);
        action_new = new QAction(MainWindow);
        action_new->setObjectName(QString::fromUtf8("action_new"));
        QIcon icon4;
        icon4.addFile(QString::fromUtf8(":/icons/icons/newIcon.png"), QSize(), QIcon::Normal, QIcon::Off);
        action_new->setIcon(icon4);
        action_saveas = new QAction(MainWindow);
        action_saveas->setObjectName(QString::fromUtf8("action_saveas"));
        action_exit = new QAction(MainWindow);
        action_exit->setObjectName(QString::fromUtf8("action_exit"));
        action_edit_cut = new QAction(MainWindow);
        action_edit_cut->setObjectName(QString::fromUtf8("action_edit_cut"));
        action_edit_copy = new QAction(MainWindow);
        action_edit_copy->setObjectName(QString::fromUtf8("action_edit_copy"));
        action_edit_paste = new QAction(MainWindow);
        action_edit_paste->setObjectName(QString::fromUtf8("action_edit_paste"));
        action_delete = new QAction(MainWindow);
        action_delete->setObjectName(QString::fromUtf8("action_delete"));
        action_undo = new QAction(MainWindow);
        action_undo->setObjectName(QString::fromUtf8("action_undo"));
        action_redo = new QAction(MainWindow);
        action_redo->setObjectName(QString::fromUtf8("action_redo"));
        garbageCollect = new QAction(MainWindow);
        garbageCollect->setObjectName(QString::fromUtf8("garbageCollect"));
        actionDummy = new QAction(MainWindow);
        actionDummy->setObjectName(QString::fromUtf8("actionDummy"));
        actionLoad_Compilation_unit = new QAction(MainWindow);
        actionLoad_Compilation_unit->setObjectName(QString::fromUtf8("actionLoad_Compilation_unit"));
        action_wonderfulmonitor = new QAction(MainWindow);
        action_wonderfulmonitor->setObjectName(QString::fromUtf8("action_wonderfulmonitor"));
        action_wonderfulmonitor->setCheckable(true);
        action_shift = new QAction(MainWindow);
        action_shift->setObjectName(QString::fromUtf8("action_shift"));
        action_2 = new QAction(MainWindow);
        action_2->setObjectName(QString::fromUtf8("action_2"));
        action_shift_2 = new QAction(MainWindow);
        action_shift_2->setObjectName(QString::fromUtf8("action_shift_2"));
        action_shift_3 = new QAction(MainWindow);
        action_shift_3->setObjectName(QString::fromUtf8("action_shift_3"));
        action_shift_4 = new QAction(MainWindow);
        action_shift_4->setObjectName(QString::fromUtf8("action_shift_4"));
        action_shift_5 = new QAction(MainWindow);
        action_shift_5->setObjectName(QString::fromUtf8("action_shift_5"));
        action_shift_6 = new QAction(MainWindow);
        action_shift_6->setObjectName(QString::fromUtf8("action_shift_6"));
        action_find = new QAction(MainWindow);
        action_find->setObjectName(QString::fromUtf8("action_find"));
        action_replace = new QAction(MainWindow);
        action_replace->setObjectName(QString::fromUtf8("action_replace"));
        actionSpeedFast = new QAction(MainWindow);
        actionSpeedFast->setObjectName(QString::fromUtf8("actionSpeedFast"));
        actionSpeedFast->setCheckable(true);
        actionSpeedMedium = new QAction(MainWindow);
        actionSpeedMedium->setObjectName(QString::fromUtf8("actionSpeedMedium"));
        actionSpeedMedium->setCheckable(true);
        actionSpeedMedium->setChecked(true);
        actionSpeedSlow = new QAction(MainWindow);
        actionSpeedSlow->setObjectName(QString::fromUtf8("actionSpeedSlow"));
        actionSpeedSlow->setCheckable(true);
        actionSpeedSlow->setChecked(false);
        actionCompile_without_tags = new QAction(MainWindow);
        actionCompile_without_tags->setObjectName(QString::fromUtf8("actionCompile_without_tags"));
        actionGo_to_position = new QAction(MainWindow);
        actionGo_to_position->setObjectName(QString::fromUtf8("actionGo_to_position"));
        centralWidget = new QWidget(MainWindow);
        centralWidget->setObjectName(QString::fromUtf8("centralWidget"));
        QSizePolicy sizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
        sizePolicy.setHorizontalStretch(100);
        sizePolicy.setVerticalStretch(100);
        sizePolicy.setHeightForWidth(centralWidget->sizePolicy().hasHeightForWidth());
        centralWidget->setSizePolicy(sizePolicy);
        centralWidget->setMaximumSize(QSize(600, 348));
        centralWidget->setLayoutDirection(Qt::LeftToRight);
        centralWidget->setAutoFillBackground(false);
        editorTabs = new QTabWidget(centralWidget);
        editorTabs->setObjectName(QString::fromUtf8("editorTabs"));
        editorTabs->setGeometry(QRect(9, 9, 581, 231));
        QSizePolicy sizePolicy1(QSizePolicy::Ignored, QSizePolicy::Expanding);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(5);
        sizePolicy1.setHeightForWidth(editorTabs->sizePolicy().hasHeightForWidth());
        editorTabs->setSizePolicy(sizePolicy1);
        editorTabs->setTabsClosable(true);
        tabWidget = new QTabWidget(centralWidget);
        tabWidget->setObjectName(QString::fromUtf8("tabWidget"));
        tabWidget->setGeometry(QRect(9, 235, 473, 229));
        QSizePolicy sizePolicy2(QSizePolicy::Expanding, QSizePolicy::Expanding);
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(2);
        sizePolicy2.setHeightForWidth(tabWidget->sizePolicy().hasHeightForWidth());
        tabWidget->setSizePolicy(sizePolicy2);
        tabWidget->setTabShape(QTabWidget::Triangular);
        tabWidget->setDocumentMode(true);
        tab = new QWidget();
        tab->setObjectName(QString::fromUtf8("tab"));
        verticalLayout_2 = new QVBoxLayout(tab);
        verticalLayout_2->setSpacing(6);
        verticalLayout_2->setContentsMargins(11, 11, 11, 11);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        outputView = new QTextBrowser(tab);
        outputView->setObjectName(QString::fromUtf8("outputView"));
        QSizePolicy sizePolicy3(QSizePolicy::Ignored, QSizePolicy::Expanding);
        sizePolicy3.setHorizontalStretch(0);
        sizePolicy3.setVerticalStretch(2);
        sizePolicy3.setHeightForWidth(outputView->sizePolicy().hasHeightForWidth());
        outputView->setSizePolicy(sizePolicy3);
        outputView->setMinimumSize(QSize(451, 0));

        verticalLayout_2->addWidget(outputView);

        tabWidget->addTab(tab, QString());
        tab_2 = new QWidget();
        tab_2->setObjectName(QString::fromUtf8("tab_2"));
        verticalLayout = new QVBoxLayout(tab_2);
        verticalLayout->setSpacing(6);
        verticalLayout->setContentsMargins(11, 11, 11, 11);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        graphicsView = new QGraphicsView(tab_2);
        graphicsView->setObjectName(QString::fromUtf8("graphicsView"));
        QFont font;
        font.setFamily(QString::fromUtf8("Courier New"));
        font.setPointSize(14);
        graphicsView->setFont(font);

        verticalLayout->addWidget(graphicsView);

        tabWidget->addTab(tab_2, QString());
        MainWindow->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(MainWindow);
        menuBar->setObjectName(QString::fromUtf8("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 782, 18));
        menuBar->setLayoutDirection(Qt::RightToLeft);
        mnuFile = new QMenu(menuBar);
        mnuFile->setObjectName(QString::fromUtf8("mnuFile"));
        mnuRecentFiles_2 = new QMenu(mnuFile);
        mnuRecentFiles_2->setObjectName(QString::fromUtf8("mnuRecentFiles_2"));
        mnuEdit = new QMenu(menuBar);
        mnuEdit->setObjectName(QString::fromUtf8("mnuEdit"));
        mnuPRogram = new QMenu(menuBar);
        mnuPRogram->setObjectName(QString::fromUtf8("mnuPRogram"));
        mnuHelp = new QMenu(menuBar);
        mnuHelp->setObjectName(QString::fromUtf8("mnuHelp"));
        mnuHelp->setTearOffEnabled(false);
        mnuHelp->setSeparatorsCollapsible(false);
        mnuTest = new QMenu(menuBar);
        mnuTest->setObjectName(QString::fromUtf8("mnuTest"));
        MainWindow->setMenuBar(menuBar);
        mainToolBar = new QToolBar(MainWindow);
        mainToolBar->setObjectName(QString::fromUtf8("mainToolBar"));
        mainToolBar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
        mainToolBar->setFloatable(false);
        MainWindow->addToolBar(Qt::TopToolBarArea, mainToolBar);
        statusBar = new QStatusBar(MainWindow);
        statusBar->setObjectName(QString::fromUtf8("statusBar"));
        MainWindow->setStatusBar(statusBar);
        dockSearchReplace = new QDockWidget(MainWindow);
        dockSearchReplace->setObjectName(QString::fromUtf8("dockSearchReplace"));
        dockSearchReplace->setMinimumSize(QSize(70, 100));
        dockSearchReplace->setFeatures(QDockWidget::AllDockWidgetFeatures);
        dockWidgetContents = new QWidget();
        dockWidgetContents->setObjectName(QString::fromUtf8("dockWidgetContents"));
        label = new QLabel(dockWidgetContents);
        label->setObjectName(QString::fromUtf8("label"));
        label->setGeometry(QRect(720, 10, 51, 20));
        label_2 = new QLabel(dockWidgetContents);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        label_2->setGeometry(QRect(710, 40, 61, 20));
        txtSearchString = new QLineEdit(dockWidgetContents);
        txtSearchString->setObjectName(QString::fromUtf8("txtSearchString"));
        txtSearchString->setGeometry(QRect(332, 10, 351, 20));
        txtReplacementString = new QLineEdit(dockWidgetContents);
        txtReplacementString->setObjectName(QString::fromUtf8("txtReplacementString"));
        txtReplacementString->setGeometry(QRect(332, 40, 351, 20));
        btnFindPrev = new QPushButton(dockWidgetContents);
        btnFindPrev->setObjectName(QString::fromUtf8("btnFindPrev"));
        btnFindPrev->setGeometry(QRect(300, 10, 21, 23));
        btnFindNext = new QPushButton(dockWidgetContents);
        btnFindNext->setObjectName(QString::fromUtf8("btnFindNext"));
        btnFindNext->setGeometry(QRect(280, 10, 21, 23));
        btnReplacePrev = new QPushButton(dockWidgetContents);
        btnReplacePrev->setObjectName(QString::fromUtf8("btnReplacePrev"));
        btnReplacePrev->setGeometry(QRect(300, 40, 21, 23));
        btnReplaceNext = new QPushButton(dockWidgetContents);
        btnReplaceNext->setObjectName(QString::fromUtf8("btnReplaceNext"));
        btnReplaceNext->setGeometry(QRect(280, 40, 21, 23));
        lblFindStatus = new QLabel(dockWidgetContents);
        lblFindStatus->setObjectName(QString::fromUtf8("lblFindStatus"));
        lblFindStatus->setGeometry(QRect(10, 10, 181, 16));
        dockSearchReplace->setWidget(dockWidgetContents);
        MainWindow->addDockWidget(static_cast<Qt::DockWidgetArea>(4), dockSearchReplace);

        menuBar->addAction(mnuFile->menuAction());
        menuBar->addAction(mnuEdit->menuAction());
        menuBar->addAction(mnuPRogram->menuAction());
        menuBar->addAction(mnuTest->menuAction());
        menuBar->addAction(mnuHelp->menuAction());
        mnuFile->addAction(action_new);
        mnuFile->addAction(action_open);
        mnuFile->addAction(mnuRecentFiles_2->menuAction());
        mnuFile->addAction(action_save);
        mnuFile->addAction(action_saveas);
        mnuFile->addSeparator();
        mnuFile->addAction(action_exit);
        mnuRecentFiles_2->addAction(actionDummy);
        mnuEdit->addAction(action_undo);
        mnuEdit->addAction(action_redo);
        mnuEdit->addSeparator();
        mnuEdit->addAction(action_edit_cut);
        mnuEdit->addAction(action_edit_copy);
        mnuEdit->addAction(action_edit_paste);
        mnuEdit->addAction(action_delete);
        mnuEdit->addSeparator();
        mnuEdit->addAction(action_find);
        mnuEdit->addAction(action_replace);
        mnuPRogram->addAction(mnuProgramRun);
        mnuPRogram->addAction(action_verify);
        mnuPRogram->addAction(garbageCollect);
        mnuPRogram->addAction(action_wonderfulmonitor);
        mnuPRogram->addAction(actionSpeedFast);
        mnuPRogram->addAction(actionSpeedMedium);
        mnuPRogram->addAction(actionSpeedSlow);
        mnuHelp->addAction(action);
        mnuHelp->addAction(action_help_keyboard);
        mnuHelp->addSeparator();
        mnuHelp->addAction(action_walkthroughs);
        mnuHelp->addAction(action_report_bug);
        mnuHelp->addAction(action_software_update);
        mnuHelp->addAction(action_contact_us);
        mnuHelp->addSeparator();
        mnuHelp->addAction(action_about_kalimat);
        mnuTest->addAction(actionLexize);
        mnuTest->addAction(actionParse);
        mnuTest->addAction(actionCompile);
        mnuTest->addAction(actionCompile_without_tags);
        mnuTest->addAction(actionEvaluate);
        mnuTest->addAction(actionLoad_Compilation_unit);
        mnuTest->addAction(actionGo_to_position);
        mainToolBar->addAction(action_new);
        mainToolBar->addAction(action_open);
        mainToolBar->addAction(action_save);
        mainToolBar->addSeparator();
        mainToolBar->addAction(mnuProgramRun);

        retranslateUi(MainWindow);

        tabWidget->setCurrentIndex(0);


        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QApplication::translate("MainWindow", "MainWindow", 0, QApplication::UnicodeUTF8));
        actionLexize->setText(QApplication::translate("MainWindow", "Lexize", 0, QApplication::UnicodeUTF8));
        actionLexize->setShortcut(QApplication::translate("MainWindow", "Ctrl+L", 0, QApplication::UnicodeUTF8));
        actionParse->setText(QApplication::translate("MainWindow", "Parse", 0, QApplication::UnicodeUTF8));
        actionParse->setShortcut(QApplication::translate("MainWindow", "Ctrl+P", 0, QApplication::UnicodeUTF8));
        actionEvaluate->setText(QApplication::translate("MainWindow", "Evaluate", 0, QApplication::UnicodeUTF8));
        action->setText(QApplication::translate("MainWindow", "\330\247\331\204\330\250\330\261\331\205\330\254\330\251 \330\250\331\203\331\204\331\205\330\247\330\252", 0, QApplication::UnicodeUTF8));
        action_walkthroughs->setText(QApplication::translate("MainWindow", "\331\205\330\255\330\247\330\266\330\261\330\247\330\252 \330\271\331\205\331\204\331\212\330\251", 0, QApplication::UnicodeUTF8));
        action_report_bug->setText(QApplication::translate("MainWindow", "\330\245\330\250\331\204\330\247\330\272 \330\271\331\206 \330\256\330\267\330\243 \330\250\330\247\331\204\330\250\330\261\331\206\330\247\331\205\330\254", 0, QApplication::UnicodeUTF8));
        action_software_update->setText(QApplication::translate("MainWindow", "\330\252\330\255\330\257\331\212\330\253 \331\204\330\242\330\256\330\261 \331\206\330\263\330\256\330\251", 0, QApplication::UnicodeUTF8));
        action_contact_us->setText(QApplication::translate("MainWindow", "\330\247\330\252\330\265\331\204 \330\250\331\206\330\247", 0, QApplication::UnicodeUTF8));
        action_about_kalimat->setText(QApplication::translate("MainWindow", "\330\271\331\206 \331\203\331\204\331\205\330\247\330\252...", 0, QApplication::UnicodeUTF8));
        action_help_keyboard->setText(QApplication::translate("MainWindow", "\330\247\331\204\330\271\331\204\330\247\331\205\330\247\330\252 \330\247\331\204\330\256\330\247\330\265\330\251 \331\210\331\204\331\210\330\255\330\251 \330\247\331\204\331\205\331\201\330\247\330\252\331\212\330\255", 0, QApplication::UnicodeUTF8));
        mnuProgramRun->setText(QApplication::translate("MainWindow", "\330\252\331\206\331\201\331\212\330\260", 0, QApplication::UnicodeUTF8));
        mnuProgramRun->setShortcut(QApplication::translate("MainWindow", "Ctrl+R", 0, QApplication::UnicodeUTF8));
        actionCompile->setText(QApplication::translate("MainWindow", "Compile", 0, QApplication::UnicodeUTF8));
        actionCompile->setShortcut(QApplication::translate("MainWindow", "Ctrl+M", 0, QApplication::UnicodeUTF8));
        action_save->setText(QApplication::translate("MainWindow", "\330\255\331\201\330\270", 0, QApplication::UnicodeUTF8));
        action_save->setShortcut(QApplication::translate("MainWindow", "Ctrl+S", 0, QApplication::UnicodeUTF8));
        action_open->setText(QApplication::translate("MainWindow", "\331\201\330\252\330\255", 0, QApplication::UnicodeUTF8));
        action_open->setShortcut(QApplication::translate("MainWindow", "Ctrl+O", 0, QApplication::UnicodeUTF8));
        action_verify->setText(QApplication::translate("MainWindow", "\330\252\330\255\331\202\331\221\331\202", 0, QApplication::UnicodeUTF8));
        action_new->setText(QApplication::translate("MainWindow", "\330\254\330\257\331\212\330\257", 0, QApplication::UnicodeUTF8));
        action_new->setShortcut(QApplication::translate("MainWindow", "Ctrl+N", 0, QApplication::UnicodeUTF8));
        action_saveas->setText(QApplication::translate("MainWindow", "\330\255\331\201\330\270 \330\250\330\245\330\263\331\205...", 0, QApplication::UnicodeUTF8));
        action_exit->setText(QApplication::translate("MainWindow", "\330\256\330\261\331\210\330\254", 0, QApplication::UnicodeUTF8));
        action_edit_cut->setText(QApplication::translate("MainWindow", "\331\202\330\265", 0, QApplication::UnicodeUTF8));
        action_edit_cut->setShortcut(QApplication::translate("MainWindow", "Ctrl+X", 0, QApplication::UnicodeUTF8));
        action_edit_copy->setText(QApplication::translate("MainWindow", "\331\206\330\263\330\256", 0, QApplication::UnicodeUTF8));
        action_edit_copy->setShortcut(QApplication::translate("MainWindow", "Ctrl+C", 0, QApplication::UnicodeUTF8));
        action_edit_paste->setText(QApplication::translate("MainWindow", "\331\204\330\265\331\202", 0, QApplication::UnicodeUTF8));
        action_edit_paste->setShortcut(QApplication::translate("MainWindow", "Ctrl+V", 0, QApplication::UnicodeUTF8));
        action_delete->setText(QApplication::translate("MainWindow", "\331\205\330\263\330\255", 0, QApplication::UnicodeUTF8));
        action_undo->setText(QApplication::translate("MainWindow", "\330\252\330\261\330\247\330\254\330\271", 0, QApplication::UnicodeUTF8));
        action_undo->setShortcut(QApplication::translate("MainWindow", "Ctrl+Z", 0, QApplication::UnicodeUTF8));
        action_redo->setText(QApplication::translate("MainWindow", "\330\245\330\271\330\247\330\257\330\251", 0, QApplication::UnicodeUTF8));
        garbageCollect->setText(QApplication::translate("MainWindow", "\330\254\331\205\330\271 \330\247\331\204\331\205\331\207\331\205\331\204\330\247\330\252", 0, QApplication::UnicodeUTF8));
        actionDummy->setText(QApplication::translate("MainWindow", "dummy", 0, QApplication::UnicodeUTF8));
        actionLoad_Compilation_unit->setText(QApplication::translate("MainWindow", "Load Compilation unit", 0, QApplication::UnicodeUTF8));
        action_wonderfulmonitor->setText(QApplication::translate("MainWindow", "\330\247\331\204\331\205\330\261\330\247\331\202\330\250 \330\247\331\204\330\271\330\254\331\212\330\250", 0, QApplication::UnicodeUTF8));
        action_shift->setText(QApplication::translate("MainWindow", "<     (shift+\330\254)", 0, QApplication::UnicodeUTF8));
        action_2->setText(QApplication::translate("MainWindow", ">     (shift+\330\257)", 0, QApplication::UnicodeUTF8));
        action_shift_2->setText(QApplication::translate("MainWindow", "\330\214     (shift+\331\206)", 0, QApplication::UnicodeUTF8));
        action_shift_3->setText(QApplication::translate("MainWindow", "[     (shift+\330\250)", 0, QApplication::UnicodeUTF8));
        action_shift_4->setText(QApplication::translate("MainWindow", "]     (shift+\331\212)", 0, QApplication::UnicodeUTF8));
        action_shift_5->setText(QApplication::translate("MainWindow", ".     (shift+\330\262)", 0, QApplication::UnicodeUTF8));
        action_shift_6->setText(QApplication::translate("MainWindow", "\330\245     (shift+\330\272)", 0, QApplication::UnicodeUTF8));
        action_find->setText(QApplication::translate("MainWindow", "\330\250\330\255\330\253...", 0, QApplication::UnicodeUTF8));
        action_find->setShortcut(QApplication::translate("MainWindow", "Ctrl+F", 0, QApplication::UnicodeUTF8));
        action_replace->setText(QApplication::translate("MainWindow", "\330\247\330\263\330\252\330\250\330\257\330\247\331\204...", 0, QApplication::UnicodeUTF8));
        actionSpeedFast->setText(QApplication::translate("MainWindow", "\330\263\330\261\331\212\330\271", 0, QApplication::UnicodeUTF8));
        actionSpeedFast->setShortcut(QApplication::translate("MainWindow", "Alt+Q", 0, QApplication::UnicodeUTF8));
        actionSpeedMedium->setText(QApplication::translate("MainWindow", "\331\205\330\252\331\210\330\263\330\267", 0, QApplication::UnicodeUTF8));
        actionSpeedMedium->setShortcut(QApplication::translate("MainWindow", "Alt+A", 0, QApplication::UnicodeUTF8));
        actionSpeedSlow->setText(QApplication::translate("MainWindow", "\330\250\330\267\331\212\330\241", 0, QApplication::UnicodeUTF8));
        actionSpeedSlow->setShortcut(QApplication::translate("MainWindow", "Alt+Z", 0, QApplication::UnicodeUTF8));
        actionCompile_without_tags->setText(QApplication::translate("MainWindow", "Compile without tags", 0, QApplication::UnicodeUTF8));
        actionGo_to_position->setText(QApplication::translate("MainWindow", "Go to position", 0, QApplication::UnicodeUTF8));
        actionGo_to_position->setShortcut(QApplication::translate("MainWindow", "Ctrl+G", 0, QApplication::UnicodeUTF8));
        tabWidget->setTabText(tabWidget->indexOf(tab), QApplication::translate("MainWindow", "\330\247\331\204\330\261\330\263\330\247\330\246\331\204", 0, QApplication::UnicodeUTF8));
        tabWidget->setTabText(tabWidget->indexOf(tab_2), QApplication::translate("MainWindow", "\330\247\331\204\331\205\330\252\330\272\331\212\330\261\330\247\330\252", 0, QApplication::UnicodeUTF8));
        mnuFile->setTitle(QApplication::translate("MainWindow", "\331\205\331\204\331\201", 0, QApplication::UnicodeUTF8));
        mnuRecentFiles_2->setTitle(QApplication::translate("MainWindow", "\331\205\331\204\331\201\330\247\330\252 \330\263\330\247\330\250\331\202\330\251", 0, QApplication::UnicodeUTF8));
        mnuEdit->setTitle(QApplication::translate("MainWindow", "\330\252\330\255\330\261\331\212\330\261", 0, QApplication::UnicodeUTF8));
        mnuPRogram->setTitle(QApplication::translate("MainWindow", "\330\250\330\261\331\206\330\247\331\205\330\254", 0, QApplication::UnicodeUTF8));
        mnuHelp->setTitle(QApplication::translate("MainWindow", "\331\205\330\263\330\247\330\271\330\257\330\251", 0, QApplication::UnicodeUTF8));
        mnuTest->setTitle(QApplication::translate("MainWindow", "\330\247\330\256\330\252\330\250\330\247\330\261", 0, QApplication::UnicodeUTF8));
        label->setText(QApplication::translate("MainWindow", "\330\247\330\250\330\255\330\253 \330\271\331\206", 0, QApplication::UnicodeUTF8));
        label_2->setText(QApplication::translate("MainWindow", "\331\210\330\247\330\263\330\252\330\250\330\257\331\204 \330\250\331\207", 0, QApplication::UnicodeUTF8));
        btnFindPrev->setText(QApplication::translate("MainWindow", "<", 0, QApplication::UnicodeUTF8));
        btnFindNext->setText(QApplication::translate("MainWindow", ">", 0, QApplication::UnicodeUTF8));
        btnReplacePrev->setText(QApplication::translate("MainWindow", "<", 0, QApplication::UnicodeUTF8));
        btnReplaceNext->setText(QApplication::translate("MainWindow", ">", 0, QApplication::UnicodeUTF8));
        lblFindStatus->setText(QString());
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
