/****************************************************************************
** Resource object code
**
** Created: Tue Apr 6 02:45:49 2010
**      by: The Resource Compiler for Qt version 4.6.1
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <QtCore/qglobal.h>

static const unsigned char qt_resource_data[] = {
  // /home/samy/code/smallvm/test1.txt
  0x0,0x0,0x0,0x0,
  
  
};

static const unsigned char qt_resource_name[] = {
  // asm
  0x0,0x3,
  0x0,0x0,0x68,0x9d,
  0x0,0x61,
  0x0,0x73,0x0,0x6d,
    // test1.txt
  0x0,0x9,
  0xa,0x74,0xae,0x74,
  0x0,0x74,
  0x0,0x65,0x0,0x73,0x0,0x74,0x0,0x31,0x0,0x2e,0x0,0x74,0x0,0x78,0x0,0x74,
  
};

static const unsigned char qt_resource_struct[] = {
  // :
  0x0,0x0,0x0,0x0,0x0,0x2,0x0,0x0,0x0,0x1,0x0,0x0,0x0,0x1,
  // :/asm
  0x0,0x0,0x0,0x0,0x0,0x2,0x0,0x0,0x0,0x1,0x0,0x0,0x0,0x2,
  // :/asm/test1.txt
  0x0,0x0,0x0,0xc,0x0,0x0,0x0,0x0,0x0,0x1,0x0,0x0,0x0,0x0,

};

QT_BEGIN_NAMESPACE

extern Q_CORE_EXPORT bool qRegisterResourceData
    (int, const unsigned char *, const unsigned char *, const unsigned char *);

extern Q_CORE_EXPORT bool qUnregisterResourceData
    (int, const unsigned char *, const unsigned char *, const unsigned char *);

QT_END_NAMESPACE


int QT_MANGLE_NAMESPACE(qInitResources_asm)()
{
    QT_PREPEND_NAMESPACE(qRegisterResourceData)
        (0x01, qt_resource_struct, qt_resource_name, qt_resource_data);
    return 1;
}

Q_CONSTRUCTOR_FUNCTION(QT_MANGLE_NAMESPACE(qInitResources_asm))

int QT_MANGLE_NAMESPACE(qCleanupResources_asm)()
{
    QT_PREPEND_NAMESPACE(qUnregisterResourceData)
       (0x01, qt_resource_struct, qt_resource_name, qt_resource_data);
    return 1;
}

Q_DESTRUCTOR_FUNCTION(QT_MANGLE_NAMESPACE(qCleanupResources_asm))

