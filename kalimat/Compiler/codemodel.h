#ifndef CODEMODEL_H
#define CODEMODEL_H

#include "../Parser/KalimatAst/kalimatast_incl.h"
#include "treeutils.h"

class CodeModel;
class ModuleInterface;
class FunctionInterface
{
    CodeModel *owner;
    ModuleInterface *module;
    shared_ptr<ProceduralDecl> declaration;
    QVector<shared_ptr<Identifier> > referrers;
};

class ModuleInterface
{
    CodeModel *owner;
    QVector<ModuleInterface *> users;
    QVector<ModuleInterface *> dependencies;
    QMap<QString, FunctionInterface *> functions;
};

class CodeModel
{
    /*
     The QString is either an absolute file name or in the format
     :/xxxx
     where xxxx is a pointer to a CodeDoc
     */
    QMap<QString, shared_ptr<CompilationUnit> > codes;
    QSet<shared_ptr<ProceduralRef> > unresolvedProcedures;
    QMap<shared_ptr<ProceduralRef>, FunctionInterface *> resolvedProcedures;
};

#endif // CODEMODEL_H
