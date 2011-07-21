# -------------------------------------------------
# Project created by QtCreator 2010-03-04T20:06:04
# -------------------------------------------------
TARGET = smallvm
TEMPLATE = app
SOURCES += main.cpp \
    value.cpp \
    instruction.cpp \
    vm.cpp \
    method.cpp \
    externalmethod.cpp \
    frame.cpp \
    vmerror.cpp \
    multidimensionalarray.cpp \
    allocator.cpp \
    classes.cpp \
    references.cpp \
    easyforeignclass.cpp \
    process.cpp \
    channel.cpp \
    metaclass.cpp \
    debugger.cpp \
    tracelistener.cpp \
    vmutils.cpp
HEADERS += \
    value.h \
    instruction.h \
    vm.h \
    method.h \
    externalmethod.h \
    vm_incl.h \
    frame.h \
    vmerror.h \
    multidimensionalarray.h \
    allocator.h \
    dequeue.h \
    classes.h \
    references.h \
    easyforeignclass.h \
    process.h \
    channel.h \
    metaclass.h \
    debugger.h \
    tracelistener.h
FORMS +=
RESOURCES +=

LIBS += -L"$$_PRO_FILE_PWD_/libffi/" -llibffi
