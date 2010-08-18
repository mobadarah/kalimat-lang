/****************************************************************************
** Meta object code from reading C++ file 'savechangedfiles.h'
**
** Created: Tue Apr 6 02:46:59 2010
**      by: The Qt Meta Object Compiler version 62 (Qt 4.6.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "savechangedfiles.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'savechangedfiles.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.6.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_SaveChangedFiles[] = {

 // content:
       4,       // revision
       0,       // classname
       0,    0, // classinfo
       1,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      25,   18,   17,   17, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_SaveChangedFiles[] = {
    "SaveChangedFiles\0\0button\0"
    "on_buttonBox_clicked(QAbstractButton*)\0"
};

const QMetaObject SaveChangedFiles::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_SaveChangedFiles,
      qt_meta_data_SaveChangedFiles, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &SaveChangedFiles::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *SaveChangedFiles::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *SaveChangedFiles::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_SaveChangedFiles))
        return static_cast<void*>(const_cast< SaveChangedFiles*>(this));
    return QDialog::qt_metacast(_clname);
}

int SaveChangedFiles::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: on_buttonBox_clicked((*reinterpret_cast< QAbstractButton*(*)>(_a[1]))); break;
        default: ;
        }
        _id -= 1;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
