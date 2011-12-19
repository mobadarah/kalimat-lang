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
    QVector<shared_ptr<StrLiteral> > _usedModules;
    int usedModuleCount() { return _usedModules.count();}
    StrLiteral *usedModule(int i ) { return _usedModules[i].get();}
    Token getPos() { return _astImpl.getPos(); }
};

class Program : public CompilationUnit
{
    Q_OBJECT
public:
    QVector<shared_ptr<TopLevel > > _elements;

    // Original layout without collecting top-level statements
    // into a 'main' function. Used for pretty-printing...etc
    QVector<shared_ptr<TopLevel> >  _originalElements;
public:
    Program(Token pos, QVector<shared_ptr<TopLevel> > elements,
            QVector<shared_ptr<StrLiteral> > usedModules,
            QVector<shared_ptr<TopLevel> > originalElements);
    int elementCount() { return _elements.count(); }
    TopLevel *element(int i) { return _elements[i].get();}
    void addElement(TopLevel *element) { _elements.append(shared_ptr<TopLevel>(element));}
    QString toString();
    void prettyPrint(CodeFormatter *f);
};

class Module : public CompilationUnit
{
    Q_OBJECT
public:
    shared_ptr<Identifier> _name;
    QVector<shared_ptr<Declaration > > _declarations;
public:
    Module(Token pos,
           shared_ptr<Identifier> name,
           QVector<shared_ptr<Declaration> > module,
           QVector<shared_ptr<StrLiteral> > usedModules);
    ~Module();
    int declCount() { return _declarations.count(); }
    Declaration *decl(int i) { return _declarations[i].get();}
    void addDecl(Declaration *decl) { _declarations.append(shared_ptr<Declaration>(decl));}
    Identifier *name() { return _name.get();}

    QString toString();
    void prettyPrint(CodeFormatter *f);
    Q_DISABLE_COPY(Module)
};

#endif // TOPLEVEL_H
