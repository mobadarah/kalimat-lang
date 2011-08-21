# -------------------------------------------------
# Project created by QtCreator 2010-03-04T20:06:04
# -------------------------------------------------
TARGET = smallvm
TEMPLATE = lib
CONFIG += dll
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
    vmutils.cpp \
    vm_ffi.cpp \
    runtime/textlayer.cpp \
    runtime/spritelayer.cpp \
    runtime/sprite.cpp \
    runtime/runwindow.cpp \
    runtime/painttimer.cpp \
    runtime/paintsurface.cpp \
    runtime/guicontrols.cpp \
    runtime/builtinmethods.cpp \
    codedocument.cpp \
    breakpoint.cpp \
    utils.cpp \
    runtime/vmclient.cpp \
    runtime/guieditwidgethandler.cpp \
    debuginfo.cpp

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
    tracelistener.h \
    vm_ffi.h \
    runtime/textlayer.h \
    runtime/spritelayer.h \
    runtime/sprite.h \
    runtime/runwindow.h \
    runtime/painttimer.h \
    runtime/paintsurface.h \
    runtime/guicontrols.h \
    runtime/builtinmethods.h \
    debuginfo.h \
    codedocument.h \
    breakpoint.h \
    utils.h \
    runtime/vmclient.h \
    runtime/guieditwidgethandler.h
FORMS +=  runtime/runwindow.ui

RESOURCES += \
    messages.qrc

win32:LIBS += -L"$$_PRO_FILE_PWD_/../smallvm/libffi/" -llibffi
unix:LIBS += -lffi
