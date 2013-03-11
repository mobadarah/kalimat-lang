# -------------------------------------------------
# Project created by QtCreator 2010-03-04T20:06:04
# -------------------------------------------------
TARGET = smallvm
QMAKE_CXXFLAGS += -std=gnu++0x -Wno-unused-parameter
TEMPLATE = lib
CONFIG += dll
#CONFIG -= console
#DEFINES += ENGLISH_PL
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
    debuginfo.cpp \
    runtime/parserengine.cpp \
    runtime/spriteclass.cpp \
    runtime/vmrunnerthread.cpp \
    runtime_identifiers.cpp \
    scheduler.cpp \
    blockingqueue.cpp \
    stack.cpp \
    runtime/windowproxymethod.cpp \
    framepool.cpp \
    runtime/cursor.cpp \
    runtime/textbuffer.cpp \
    cflowgraph.cpp

HEADERS += value.h \
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
    runtime/guieditwidgethandler.h \
    runtime/parserengine.h \
    runtime/spriteclass.h \
    runtime/vmrunnerthread.h \
    runtime_identifiers.h \
    scheduler.h \
    blockingqueue.h \
    stack.h \
    processiterator.h \
    runtime/windowproxymethod.h \
    framepool.h \
    operandstack.h \
    runtime/cursor.h \
    runtime/textbuffer.h \
    cflowgraph.h
FORMS +=  runtime/runwindow.ui

RESOURCES += \
    messages.qrc

qt += opengl
win32:LIBS += -L"$$_PRO_FILE_PWD_/../smallvm/libffi/" -llibffi

#win32:LIBS += -L"$$_PRO_FILE_PWD_/../smallvm/libsmoke/" -lsmokebase
#win32:LIBS += -L"$$_PRO_FILE_PWD_/../smallvm/libsmoke/" -lsmokeqtcore
#win32:LIBS += -L"$$_PRO_FILE_PWD_/../smallvm/libsmoke/" -lsmokeqtgui

unix:LIBS += -lffi

OTHER_FILES += \
    runlib_errors.txt \
    instruction_defs.h
