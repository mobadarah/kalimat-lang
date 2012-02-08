#ifndef TOPLEVEL_H
#define TOPLEVEL_H

#include "kalimatast.h"

class TopLevel : public KalimatAst
{
    ASTImpl _astImpl;
public:
    QString attachedComments;
public:
    TopLevel(Token pos);
    Token getPos() { return _astImpl.getPos();}
};

class CompilationUnit: public KalimatAst
{
    ASTImpl _astImpl;
public:
    CompilationUnit(Token pos);
    ~CompilationUnit() {}
    QVector<shared_ptr<StrLiteral> > _usedModules;
    int usedModuleCount() { return _usedModules.count();}
    shared_ptr<StrLiteral> usedModule(int i ) { return _usedModules[i];}
    Token getPos() { return _astImpl.getPos(); }
};

class Program : public CompilationUnit
{
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
    shared_ptr<TopLevel> element(int i) { return _elements[i];}
    void addElement(shared_ptr<TopLevel> element) { _elements.append(element);}
    QString toString();
    void prettyPrint(CodeFormatter *f);
};

class Module : public CompilationUnit
{
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
    shared_ptr<Declaration> decl(int i) { return _declarations[i];}
    shared_ptr<Declaration> declPtr(int i) { return _declarations[i];}
    void addDecl(shared_ptr<Declaration> decl) { _declarations.append(decl);}
    shared_ptr<Identifier> name() { return _name;}

    QString toString();
    void prettyPrint(CodeFormatter *f);
};

#endif // TOPLEVEL_H
