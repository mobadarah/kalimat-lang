#ifndef TOPLEVEL_H
#define TOPLEVEL_H

#include "kalimatast.h"

class TopLevel : public KalimatAst
{
    Q_OBJECT
    ASTImpl _astImpl;
public:
    QString attachedComments;
public:
    TopLevel(Token pos);
    Token getPos() { return _astImpl.getPos();}
};

class CompilationUnit: public KalimatAst
{
    Q_OBJECT
    ASTImpl _astImpl;
public:
    CompilationUnit(Token pos);
    QVector<QSharedPointer<StrLiteral> > _usedModules;
    int usedModuleCount() { return _usedModules.count();}
    StrLiteral *usedModule(int i ) { return _usedModules[i].data();}
    Token getPos() { return _astImpl.getPos(); }
};

class Program : public CompilationUnit
{
    Q_OBJECT
public:
    QVector<QSharedPointer<TopLevel > > _elements;

    // Original layout without collecting top-level statements
    // into a 'main' function. Used for pretty-printing...etc
    QVector<QSharedPointer<TopLevel > >  _originalElements;
public:
    Program(Token pos, QVector<TopLevel *> program, QVector<StrLiteral *> usedModules, QVector<TopLevel* > originalElements);
    int elementCount() { return _elements.count(); }
    TopLevel *element(int i) { return _elements[i].data();}
    void addElement(TopLevel *element) { _elements.append(QSharedPointer<TopLevel>(element));}
    QString toString();
    void prettyPrint(CodeFormatter *f);
};

class Module : public CompilationUnit
{
    Q_OBJECT
public:
    QVector<QSharedPointer<Declaration > > _declarations;
    QScopedPointer<Identifier> _name;
public:
    Module(Token pos, Identifier *name, QVector<Declaration *> module, QVector<StrLiteral *> usedModules);
    ~Module();
    int declCount() { return _declarations.count(); }
    Declaration *decl(int i) { return _declarations[i].data();}
    void addDecl(Declaration *decl) { _declarations.append(QSharedPointer<Declaration>(decl));}
    Identifier *name() { return _name.data();}

    QString toString();
    void prettyPrint(CodeFormatter *f);
    Q_DISABLE_COPY(Module)
};

#endif // TOPLEVEL_H
