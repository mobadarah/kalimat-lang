# -------------------------------------------------
# Project created by QtCreator 2009-11-23T00:23:29
# -------------------------------------------------
TARGET = kalimat
TEMPLATE = app
SOURCES += main.cpp \
    mainwindow.cpp \
    Lexer/token.cpp \
    Lexer/statemachine.cpp \
    Lexer/lexer.cpp \
    Lexer/charpredicate.cpp \
    Lexer/buffer.cpp \
    Parser/parser.cpp \
    Lexer/kalimatlexer.cpp \ # ../../Qt/2009.04/share/qtcreator/gdbmacros/gdbmacros.cpp \
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
    ../smallvm/vmutils.cpp \
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
    Parser/astreifier.cpp \
    Parser/kalimatparser.cpp \
    ../smallvm/qobjectforeignclass.cpp \
    Parser/KalimatAst/kalimatast.cpp
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
    Parser/astreifier.h \
    Parser/kalimatast/toplevel.h \
    Parser/kalimatast/statement.h \
    Parser/kalimatast/typeexpression.h \
    Parser/kalimatast/expression.h \
    Parser/kalimatast/declaration.h \
    Parser/kalimatast/pattern.h \
    Parser/kalimatast/formatmaker.h \
    Parser/KalimatAst/kalimatast.h \
    Parser/kalimatast/kalimatast_incl.h \
    Parser/kalimatast/rules.h \
    Parser/kalimatast/ruledecl.h

FORMS += mainwindow.ui \
    savechangedfiles.ui \
    ../smallvm/runtime/runwindow.ui

# QT += testlib
# CONFIG += qtestlib
# QT += webkit
RESOURCES += keywords.qrc \
    icons.qrc \
    ../smallvm/messages.qrc
OTHER_FILES += 
# QMAKE_CXXFLAGS += -std=c++0x
RC_FILE = kalimat.rc

win32:LIBS += -L"$$_PRO_FILE_PWD_/../smallvm/libffi/" -llibffi

unix:LIBS += -lffi

