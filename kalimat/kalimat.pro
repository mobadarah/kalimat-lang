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
    Parser/kalimatparser.cpp \
    Parser/kalimatast.cpp \
    syntaxhighlighter.cpp \
    runwindow.cpp \
    ../smallvm/vm.cpp \
    ../smallvm/value.cpp \
    ../smallvm/method.cpp \
    ../smallvm/instruction.cpp \
    ../smallvm/externalmethod.cpp \
    ../smallvm/vmerror.cpp \
    ../smallvm/frame.cpp \
    sprite.cpp \
    codedocument.cpp \
    savechangedfiles.cpp \
    utils.cpp \
    Compiler/codegenerator.cpp \
    Compiler/compiler.cpp \
    documentcontainer.cpp \
    ../smallvm/allocator.cpp \
    Compiler/codegenhelper.cpp \
    painttimer.cpp \
    linetracker.cpp \
    myedit.cpp \
    textlayer.cpp \
    spritelayer.cpp \
    builtinmethods.cpp \
    paintsurface.cpp \
    guieditwidgethandler.cpp \
    Parser/codeformatter.cpp \
    ../smallvm/references.cpp \
    ../smallvm/classes.cpp \
    Parser/kalimatprettyprintparser.cpp \
    ../smallvm/easyforeignclass.cpp
HEADERS += mainwindow.h \
    codedocument.h \
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
    Parser/kalimatast.h \
    syntaxhighlighter.h \
    runwindow.h \
    sprite.h \
    savechangedfiles.h \
    Compiler/codeposition.h \
    utils.h \
    Compiler/codegenerator.h \
    Compiler/codegenerator_incl.h \
    Compiler/compiler.h \
    documentcontainer.h \
    Compiler/codegenhelper.h \
    painttimer.h \
    linetracker.h \
    myedit.h \
    textlayer.h \
    spritelayer.h \
    builtinmethods.h \
    paintsurface.h \
    guieditwidgethandler.h \
    Parser/codeformatter.h \
    Parser/kalimatprettyprintparser.h
FORMS += mainwindow.ui \
    runwindow.ui \
    savechangedfiles.ui

# QT += testlib
# CONFIG += qtestlib
# QT += webkit
RESOURCES += keywords.qrc \
    icons.qrc
OTHER_FILES += 
# QMAKE_CXXFLAGS += -std=c++0x
RC_FILE = kalimat.rc
