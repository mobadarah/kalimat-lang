# -------------------------------------------------
# Project created by QtCreator 2009-11-23T00:23:29
# -------------------------------------------------
TARGET = kalimat
TEMPLATE = app
QMAKE_CXXFLAGS += -std=gnu++0x -Wno-unused-parameter
#DEFINES += ENGLISH_PL

#QMAKE_CXXFLAGS_RELEASE -= -O2
#QMAKE_CXXFLAGS_RELEASE += -O3
CODECFORTR = UTF-8
TRANSLATIONS = kalimat_en.ts
SOURCES += main.cpp \
    mainwindow.cpp \
    Lexer/token.cpp \
    Lexer/statemachine.cpp \
    Lexer/lexer.cpp \
    Lexer/charpredicate.cpp \
    Lexer/buffer.cpp \
    Parser/parser.cpp \
    Lexer/kalimatlexer.cpp \
    Lexer/action.cpp \
    Parser/ast.cpp \
    syntaxhighlighter.cpp \
    ../smallvm/vm.cpp \
    ../smallvm/value.cpp \
    ../smallvm/method.cpp \
    ../smallvm/instruction.cpp \
    ../smallvm/externalmethod.cpp \
    ../smallvm/vmerror.cpp \
    ../smallvm/frame.cpp \
    savechangedfiles.cpp \
    Compiler/codegenerator.cpp \
    Compiler/compiler.cpp \
    documentcontainer.cpp \
    ../smallvm/allocator.cpp \
    Compiler/codegenhelper.cpp \
    linetracker.cpp \
    myedit.cpp \
    Parser/codeformatter.cpp \
    ../smallvm/references.cpp \
    ../smallvm/classes.cpp \
    Parser/kalimatprettyprintparser.cpp \
    ../smallvm/easyforeignclass.cpp \
    ../smallvm/channel.cpp \
    ../smallvm/process.cpp \
    ../smallvm/metaclass.cpp \
    ../smallvm/vm_ffi.cpp \
    ../smallvm/runtime/vmclient.cpp \
    ../smallvm/runtime/textlayer.cpp \
    ../smallvm/runtime/spritelayer.cpp \
    ../smallvm/runtime/sprite.cpp \
    ../smallvm/runtime/runwindow.cpp \
    ../smallvm/runtime/painttimer.cpp \
    ../smallvm/runtime/paintsurface.cpp \
    ../smallvm/runtime/guieditwidgethandler.cpp \
    ../smallvm/runtime/guicontrols.cpp \
    ../smallvm/runtime/builtinmethods.cpp \
    ../smallvm/debugger.cpp \
    ../smallvm/codedocument.cpp \
    ../smallvm/breakpoint.cpp \
    ../smallvm/debuginfo.cpp \
    ../smallvm/utils.cpp \
    Parser/kalimatparser.cpp \
    Parser/KalimatAst/kalimatast.cpp \
    Parser/parserexception.cpp \
    linenumberarea.cpp \
    ../smallvm/runtime/parserengine.cpp \
    AutoComplete/completioninfo.cpp \
    AutoComplete/completioninfoloading.cpp \
    settingsdlg.cpp \
    aboutdlg.cpp \
    AutoComplete/analyzer.cpp \
    ../smallvm/runtime/spriteclass.cpp \
    mytooltip.cpp \
    mytiplabel.cpp \
    Parser/KalimatAst/kalimat_ast_gen.cpp \
    ../smallvm/runtime/vmrunnerthread.cpp \
    ../smallvm/runtime_identifiers.cpp \
    idemessages.cpp \
    ../smallvm/scheduler.cpp \
    ../smallvm/blockingqueue.cpp \
    stepstopcondition.cpp \
    ../smallvm/runtime/windowproxymethod.cpp \
    programdatabase.cpp \
    ../smallvm/runtime/textbuffer.cpp \
    ../smallvm/runtime/cursor.cpp \
    makeexedlg.cpp \
    ../smallvm/cflowgraph.cpp \
    Compiler/codemodel.cpp
HEADERS += mainwindow.h \
    Lexer/token.h \
    Lexer/statemachine.h \
    Lexer/state.h \
    Lexer/lexer.h \
    Lexer/charpredicate.h \
    Lexer/buffer.h \
    Parser/parser.h \
    Lexer/kalimatlexer.h \
    Lexer/lexer_incl.h \
    Lexer/action.h \
    Lexer/statemachine_incl.h \
    Parser/ast.h \
    Parser/kalimatparser.h \
    Parser/parser_incl.h \
    syntaxhighlighter.h \
    savechangedfiles.h \
    Compiler/codeposition.h \
    Compiler/codegenerator.h \
    Compiler/codegenerator_incl.h \
    Compiler/compiler.h \
    documentcontainer.h \
    Compiler/codegenhelper.h \
    linetracker.h \
    myedit.h \
    Parser/codeformatter.h \
    Parser/kalimatprettyprintparser.h \
    ../smallvm/debuginfo.h \
    ../smallvm/runtime/runwindow.h \
    ../smallvm/runtime/guieditwidgethandler.h \
    ../smallvm/runtime/guicontrols.h \
    ../smallvm/codedocument.h \
    Parser/KalimatAst/formatmaker.h \
    Parser/KalimatAst/kalimatast.h \
    Parser/KalimatAst/kalimatast_incl.h \
    Parser/KalimatParserError.h \
    Parser/parserexception.h \
    linenumberarea.h \
    ../smallvm/runtime/parserengine.h \
    AutoComplete/completioninfo.h \
    AutoComplete/completioninfoloading.h \
    settingsdlg.h \
    aboutdlg.h \
    AutoComplete/analyzer.h \
    ../smallvm/runtime/spriteclass.h \
    mytooltip.h \
    mytiplabel.h \
    Parser/KalimatAst/kalimat_ast_gen.h \
    ../smallvm/runtime/vmrunnerthread.h \
    idemessages.h \
    ../smallvm/blockingqueue.h \
    ../smallvm/vmerror.h \
    stepstopcondition.h \
    programdatabase.h \
    makeexedlg.h \
    Compiler/treeutils.h \
    Compiler/codemodel.h

FORMS += mainwindow.ui \
    savechangedfiles.ui \
    ../smallvm/runtime/runwindow.ui \
    settingsdlg.ui \
    aboutdlg.ui \
    makeexedlg.ui

# QT += testlib
# CONFIG += qtestlib
# QT += webkit
QT += sql
RESOURCES += keywords.qrc \
    icons.qrc \
    ../smallvm/messages.qrc
OTHER_FILES += 
RC_FILE = kalimat.rc

win32:LIBS += -L"$$_PRO_FILE_PWD_/../smallvm/libffi/" -llibffi

#win32:LIBS += -L"$$_PRO_FILE_PWD_/../smallvm/libsmoke/" -lsmokebase
#win32:LIBS += -L"$$_PRO_FILE_PWD_/../smallvm/libsmoke/" -lsmokeqtcore
#win32:LIBS += -L"$$_PRO_FILE_PWD_/../smallvm/libsmoke/" -lsmokeqtgui

unix:LIBS += -lffi
