/****************************************************************************
** Meta object code from reading C++ file 'runwindow.h'
**
** Created: Tue Apr 6 12:47:22 2010
**      by: The Qt Meta Object Compiler version 62 (Qt 4.6.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "runwindow.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'runwindow.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.6.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_RunWindow[] = {

 // content:
       4,       // revision
       0,       // classname
       0,    0, // classinfo
       8,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      11,   10,   10,   10, 0x08,
      41,   10,   10,   10, 0x08,
      67,   10,   10,   10, 0x08,
      98,   92,   10,   10, 0x08,
     125,   10,   10,   10, 0x08,
     150,   10,   10,   10, 0x08,
     175,   10,   10,   10, 0x08,
     181,   10,   10,   10, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_RunWindow[] = {
    "RunWindow\0\0mousePressEvent(QMouseEvent*)\0"
    "keyPressEvent(QKeyEvent*)\0"
    "paintEvent(QPaintEvent*)\0event\0"
    "resizeEvent(QResizeEvent*)\0"
    "closeEvent(QCloseEvent*)\0"
    "timerEvent(QTimerEvent*)\0Run()\0"
    "on_actionGC_triggered()\0"
};

const QMetaObject RunWindow::staticMetaObject = {
    { &QMainWindow::staticMetaObject, qt_meta_stringdata_RunWindow,
      qt_meta_data_RunWindow, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &RunWindow::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *RunWindow::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *RunWindow::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_RunWindow))
        return static_cast<void*>(const_cast< RunWindow*>(this));
    return QMainWindow::qt_metacast(_clname);
}

int RunWindow::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: mousePressEvent((*reinterpret_cast< QMouseEvent*(*)>(_a[1]))); break;
        case 1: keyPressEvent((*reinterpret_cast< QKeyEvent*(*)>(_a[1]))); break;
        case 2: paintEvent((*reinterpret_cast< QPaintEvent*(*)>(_a[1]))); break;
        case 3: resizeEvent((*reinterpret_cast< QResizeEvent*(*)>(_a[1]))); break;
        case 4: closeEvent((*reinterpret_cast< QCloseEvent*(*)>(_a[1]))); break;
        case 5: timerEvent((*reinterpret_cast< QTimerEvent*(*)>(_a[1]))); break;
        case 6: Run(); break;
        case 7: on_actionGC_triggered(); break;
        default: ;
        }
        _id -= 8;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
