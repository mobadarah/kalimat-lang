/**************************************************************************
** The Kalimat programming language
**   Copyright (C) 2010 Mohamed Samy Ali - samy2004@gmail.com
**   This project is released under the Apache License version 2.0
**   as described in the included license.txt file
**************************************************************************/

#ifndef COMPILER_H
#define COMPILER_H

class Compiler
{
public:
    KalimatLexer lexer;
    KalimatParser parser;
    CodeGenerator generator;
public:
    QMap<QString, Module *> loadedModules; // Maps from absolute path -> Module
    QStack<QString> pathStack;
    QString currentSource;
public:
    Compiler();
    Module *loadModule(QString path);
    Program *loadProgram(QString path);

    QString CompileFromFile(QString pathToMainCompilationUnit, CodeDocument *doc);
    QString CompileFromCode(QString source, CodeDocument *doc);
    QString getCompilerOutput();
private:
    void generateAllLoadedModules();
    QString loadFileContents(QString path);
    QString getPathOfModule(QString name, QString pathOfImportingModule);
    QString directoryFromFullPath(QString path);
    QString combinePath(QString parent, QString child);
};

#endif // COMPILER_H
