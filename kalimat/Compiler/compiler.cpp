/**************************************************************************
**   The Kalimat programming language
**   Copyright (C) 2010 Mohamed Samy Ali - samy2004@gmail.com
**   This project is released under the Apache License version 2.0
**   as described in the included license.txt file
**************************************************************************/

#include "Lexer/lexer_incl.h"
#include "Lexer/kalimatlexer.h"
#include "Parser/parser_incl.h"
#include "Parser/kalimatast/kalimatast_incl.h"
#include "Parser/kalimatparser.h"
#include "../../smallvm/codedocument.h"
#include "documentcontainer.h"
#include "codegenerator.h"

#include "compiler.h"
#include "../../smallvm/utils.h"

Compiler::Compiler(DocumentContainer *container)
{
    this->documentContainer = container;
}

shared_ptr<AST> parseModule(Parser *p)
{
    return ((KalimatParser *) p)->module();
}

shared_ptr<Program> Compiler::loadProgram(QString path, CodeDocument *doc)
{
    parser.init(loadFileContents(path), &lexer, doc, path);
    shared_ptr<Program> p = dynamic_pointer_cast<Program>(parser.parse());

    for(int i=0; i<p->usedModuleCount(); i++)
    {
        QString m2 = p->usedModule(i)->value;
        QString fullPath = getPathOfModule(m2, path);
        if(!loadedModules.contains(fullPath))
            loadModule(fullPath);
        Module *importedMod = loadedModules[fullPath].get();
        pathsOfModules[importedMod] = fullPath;
        /*
        for(int i=0; i<importedMod->declCount(); i++)
        {
            Declaration *decl = importedMod->decl(i);
            if(decl->isPublic())
            {
                p->addElement(decl);
            }
        }
        //*/
    }
    return p;
}

shared_ptr<Module> Compiler::loadModule(QString path)
{
    parser.init(loadFileContents(path), &lexer, documentContainer->getDocumentFromPath(path), path);
    shared_ptr<Module> m = dynamic_pointer_cast<Module>(parser.parse(parseModule));
    loadedModules[path] = m;
    pathsOfModules[m.get()] = path;
    for(int i=0; i<m->usedModuleCount(); i++)
    {
        QString m2 = m->usedModule(i)->value;
        QString fullPath = getPathOfModule(m2, path);
        if(!loadedModules.contains(fullPath))
            loadModule(fullPath);
        shared_ptr<Module> importedMod = loadedModules[fullPath];

        pathsOfModules[importedMod.get()] = fullPath;
        /*
        for(int i=0; i<importedMod->declCount(); i++)
        {
            Declaration *decl = importedMod->decl(i);
            if(decl->isPublic())
            {
                m->addDecl(decl);
            }
        }
        //*/
    }
    return m;
}

void Compiler::generateAllLoadedModules()
{
    for(QMap<QString,shared_ptr<Module> >::iterator i= loadedModules.begin(); i!=loadedModules.end(); ++i)
    {
        shared_ptr<Module> mod = *i;
        CodeDocument *doc = documentContainer->getDocumentFromPath(pathsOfModules[mod.get()]);
        generator.compileModule(mod, doc);
    }
}

QString Compiler::getPathOfModule(QString name, QString pathOfImportingModule)
{
    QString directory = directoryFromFullPath(pathOfImportingModule);
    QString ret = combinePath(directory, name);
    return ret;
}

QString Compiler::directoryFromFullPath(QString path)
{
    return QFileInfo(path).absolutePath();
}

QString Compiler::combinePath(QString parent, QString child)
{
    return QFileInfo(parent, child).absoluteFilePath();
}

QString Compiler::CompileFromCode(QString source, CodeDocument *doc)
{
    parser.init(source, &lexer, doc);
    shared_ptr<Program> p = dynamic_pointer_cast<Program>(parser.parse());

    if(p->usedModuleCount()!=0)
        throw CompilerException(p, ProgramsCannotUseExternalModulesWithoutSavingThemFirst);

    generator.generate(p, doc);
    return generator.getOutput();
}

QString Compiler::CompileFromFile(QString pathToMainCompilationUnit, CodeDocument *doc)
{
    shared_ptr<Program> p = loadProgram(pathToMainCompilationUnit, doc);
    generateAllLoadedModules();
    generator.generate(p, doc);
    return generator.getOutput();
}

QString Compiler::loadFileContents(QString path)
{
    return readFile(path);
}
