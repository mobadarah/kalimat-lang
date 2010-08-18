/****************************************************************************
** Meta object code from reading C++ file 'mainwindow.h'
**
** Created: Sat Apr 17 13:44:31 2010
**      by: The Qt Meta Object Compiler version 62 (Qt 4.6.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "mainwindow.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'mainwindow.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.6.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_MainWindow[] = {

 // content:
       4,       // revision
       0,       // classname
       0,    0, // classinfo
      13,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      12,   11,   11,   11, 0x08,
      38,   11,   11,   11, 0x08,
      74,   68,   11,   11, 0x08,
     111,   11,   11,   11, 0x08,
     138,   11,   11,   11, 0x08,
     167,   11,   11,   11, 0x08,
     194,   11,   11,   11, 0x08,
     221,   11,   11,   11, 0x08,
     247,   11,   11,   11, 0x08,
     276,   11,   11,   11, 0x08,
     305,   11,   11,   11, 0x08,
     333,   11,   11,   11, 0x08,
     360,   11,   11,   11, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_MainWindow[] = {
    "MainWindow\0\0on_recentfile_triggered()\0"
    "on_garbageCollect_triggered()\0index\0"
    "on_editorTabs_tabCloseRequested(int)\0"
    "on_action_exit_triggered()\0"
    "on_action_saveas_triggered()\0"
    "on_action_save_triggered()\0"
    "on_action_open_triggered()\0"
    "on_action_new_triggered()\0"
    "on_actionCompile_triggered()\0"
    "on_mnuProgramRun_triggered()\0"
    "on_actionLexize_triggered()\0"
    "on_actionParse_triggered()\0"
    "closeEvent(QCloseEvent*)\0"
};

const QMetaObject MainWindow::staticMetaObject = {
    { &QMainWindow::staticMetaObject, qt_meta_stringdata_MainWindow,
      qt_meta_data_MainWindow, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &MainWindow::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *MainWindow::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *MainWindow::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_MainWindow))
        return static_cast<void*>(const_cast< MainWindow*>(this));
    if (!strcmp(_clname, "RecentFileHandler"))
        return static_cast< RecentFileHandler*>(const_cast< MainWindow*>(this));
    if (!strcmp(_clname, "ICompilerClient"))
        return static_cast< ICompilerClient*>(const_cast< MainWindow*>(this));
    return QMainWindow::qt_metacast(_clname);
}

int MainWindow::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: on_recentfile_triggered(); break;
        case 1: on_garbageCollect_triggered(); break;
        case 2: on_editorTabs_tabCloseRequested((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 3: on_action_exit_triggered(); break;
        case 4: on_action_saveas_triggered(); break;
        case 5: on_action_save_triggered(); break;
        case 6: on_action_open_triggered(); break;
        case 7: on_action_new_triggered(); break;
        case 8: on_actionCompile_triggered(); break;
        case 9: on_mnuProgramRun_triggered(); break;
        case 10: on_actionLexize_triggered(); break;
        case 11: on_actionParse_triggered(); break;
        case 12: closeEvent((*reinterpret_cast< QCloseEvent*(*)>(_a[1]))); break;
        default: ;
        }
        _id -= 13;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
