/****************************************************************************
** Meta object code from reading C++ file 'testkalimatlexer.h'
**
** Created: Sat Apr 17 13:28:35 2010
**      by: The Qt Meta Object Compiler version 62 (Qt 4.6.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "Lexer/testkalimatlexer.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'testkalimatlexer.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.6.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_TestKalimatLexer[] = {

 // content:
       4,       // revision
       0,       // classname
       0,    0, // classinfo
       6,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      18,   17,   17,   17, 0x08,
      44,   17,   17,   17, 0x08,
      65,   17,   17,   17, 0x08,
      86,   17,   17,   17, 0x08,
     102,   17,   17,   17, 0x08,
     126,   17,   17,   17, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_TestKalimatLexer[] = {
    "TestKalimatLexer\0\0testNumberLiterals_data()\0"
    "testNumberLiterals()\0testOperators_data()\0"
    "testOperators()\0testComplexCases_data()\0"
    "testComplexCases()\0"
};

const QMetaObject TestKalimatLexer::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_TestKalimatLexer,
      qt_meta_data_TestKalimatLexer, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &TestKalimatLexer::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *TestKalimatLexer::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *TestKalimatLexer::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_TestKalimatLexer))
        return static_cast<void*>(const_cast< TestKalimatLexer*>(this));
    return QObject::qt_metacast(_clname);
}

int TestKalimatLexer::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: testNumberLiterals_data(); break;
        case 1: testNumberLiterals(); break;
        case 2: testOperators_data(); break;
        case 3: testOperators(); break;
        case 4: testComplexCases_data(); break;
        case 5: testComplexCases(); break;
        default: ;
        }
        _id -= 6;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
