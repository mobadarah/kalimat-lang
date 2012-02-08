#ifndef DECLARATION_H
#define DECLARATION_H

#include "kalimatast.h"
#include "toplevel.h"
#include "statement.h"

class Declaration: public TopLevel
{
    bool _isPublic;
public:
    Declaration(Token pos, bool isPublic);
    bool isPublic();
};

class ProceduralDecl : public Declaration, public IScopeIntroducer
{
public:
    shared_ptr<Identifier> _procName;
        shared_ptr<BlockStmt> _body;
    QVector<shared_ptr<Identifier > > _formals;

//    QVector<shared_ptr<Identifier > > _allReferences;

    Token _endingToken;
public:
    ProceduralDecl(Token pos,
                   Token endingToken,
                   shared_ptr<Identifier> procName,
                   QVector<shared_ptr<Identifier> > formals,
                   shared_ptr<BlockStmt> body,
                   bool isPublic);
    shared_ptr<Identifier> procName() {return _procName;}
    int formalCount() { return _formals.count();}
    shared_ptr<Identifier> formal(int i) { return _formals[i];}
//    shared_ptr<Identifier> allReferences(int i) { return _allReferences[i];}
//    void addReference(shared_ptr<Identifier> id) { _allReferences.append(id);}
    shared_ptr<BlockStmt> body() {return _body;}
    void body(shared_ptr<BlockStmt> stmt) { _body = stmt;}
    virtual QVector<shared_ptr<Identifier> > getIntroducedVariables();
};

class IProcedure
{

};

class IFunction
{

};

class ProcedureDecl : public ProceduralDecl, public IProcedure
{
public:
    ProcedureDecl(Token pos,
                  Token endingToken,
                  shared_ptr<Identifier> procName,
                  QVector<shared_ptr<Identifier> > formals,
                  shared_ptr<BlockStmt> body,
                  bool isPublic);
    QString toString();
    void prettyPrint(CodeFormatter *f);
};

class FunctionDecl : public ProceduralDecl, public IFunction
{
public:
    FunctionDecl(Token pos,
                 Token endingToken,
                 shared_ptr<Identifier> procName,
                 QVector<shared_ptr<Identifier> > formals,
                 shared_ptr<BlockStmt> body,
                 bool isPublic);
    QString toString();
    void prettyPrint(CodeFormatter *f);
};

class FFIProceduralDecl;

class FFILibraryDecl : public Declaration
{
public:
    QString libName;
private:
    QVector<shared_ptr<Declaration> > _decls;
public:
    FFILibraryDecl(Token pos, QString libName, QVector<shared_ptr<Declaration> > decls, bool isPublic);
    int declCount() { return _decls.count(); }
    shared_ptr<Declaration> decl(int index) { return _decls[index];}
    void prettyPrint(CodeFormatter *formatter);
    QString toString();
};

class FFIProceduralDecl : public Declaration
{
public:
    bool isFunctionNotProc;
    QString procName;
    QString symbol;
private:
    shared_ptr<TypeExpression>_returnType; // NULL -> It's a procedure
    QVector<shared_ptr<TypeExpression> > _paramTypes;
public:
    FFIProceduralDecl(Token pos, bool isFunctionNotProc,
        shared_ptr<TypeExpression> returnType,
        QVector<shared_ptr<TypeExpression> > paramTypes,
        QString procName,
        QString symbol,
        bool isPublic);
    shared_ptr<TypeExpression> returnType() { return _returnType; }
    int paramTypeCount() { return _paramTypes.count(); }
    shared_ptr<TypeExpression> paramType(int index) { return _paramTypes.at(index); }
    void prettyPrint(CodeFormatter *formatter);
    QString toString();
};

class FFIStructDecl : public Declaration
{
private:
    shared_ptr<Identifier> _name;
    QVector<shared_ptr<Identifier> > _fieldNames;
    QVector<shared_ptr<TypeExpression> > _fieldTypes;
public:
    QVector<int> fieldBatches; // For grouping HAS a, b, c; HAS d,e declarations for pretty printing
public:
    FFIStructDecl(Token pos,
                  shared_ptr<Identifier> name,
                  QVector<shared_ptr<Identifier> > fieldNames,
                  QVector<shared_ptr<TypeExpression> > fieldTypes,
                  QVector<int> fieldBatches,
                  bool isPublic);
    int fieldCount() { return _fieldNames.count();}
    shared_ptr<Identifier> name() { return _name; }
    shared_ptr<Identifier> fieldName(int index) { return _fieldNames[index];}
    shared_ptr<TypeExpression> fieldType(int index) { return _fieldTypes[index];}
    void prettyPrint(CodeFormatter *formatter);
    QString toString();
};

struct ClassInternalDecl : public PrettyPrintable
{
    virtual void prettyPrint(CodeFormatter *f) = 0;
};

struct MethodInfo
{
    int arity;
    bool isFunction;
    MethodInfo(int arity, bool isFunction);
    MethodInfo();
};

struct ConcreteResponseInfo : public PrettyPrintable
{
    shared_ptr<Identifier> name;
    QVector<shared_ptr<Identifier> > params;
    void prettyPrint(CodeFormatter *f);

    ConcreteResponseInfo(shared_ptr<Identifier> _name)
        :name(_name)
    {
    }

    void add(shared_ptr<Identifier> param)
    {
        params.append(param);
    }
};

struct Has : public ClassInternalDecl
{
    QMap<QString, shared_ptr<TypeExpression> > _fieldMarshallAs;
    QVector<shared_ptr<Identifier> > fields;
    void prettyPrint(CodeFormatter *f);
    void add(shared_ptr<Identifier> field)
    {
        fields.append(shared_ptr<Identifier>(field));
    }
    int fieldCount() { return fields.count(); }
    shared_ptr<Identifier> field(int i) { return fields[i]; }
    shared_ptr<TypeExpression> marshallingTypeOf(QString field)
    {
        if(_fieldMarshallAs.contains(field))
            return _fieldMarshallAs[field];
        else
            return shared_ptr<TypeExpression>();
    }
};

struct RespondsTo : public ClassInternalDecl
{
    bool isFunctions;
    QVector<shared_ptr<ConcreteResponseInfo> > methods;

    RespondsTo(bool _isFunctions) { isFunctions = _isFunctions;}
    void prettyPrint(CodeFormatter *f);
    void add(shared_ptr<ConcreteResponseInfo> mi)
    {
        methods.append(mi);
    }
    shared_ptr<ConcreteResponseInfo> method(int i) { return methods[i]; }
    int methodCount() { return methods.count(); }
};

class ClassDecl : public Declaration
{
public:
    shared_ptr<Identifier> _ancestorName;
    shared_ptr<Identifier> _name;
    QVector<shared_ptr<Identifier > > _fields;
    QMap<QString, MethodInfo>  _methodPrototypes;
    QVector<shared_ptr<ClassInternalDecl> > _internalDecls; // For pretty printing...etc
    QMap<QString, shared_ptr<TypeExpression> > _fieldMarshallAs;
    QMap<QString, shared_ptr<MethodDecl> > _methods;
    shared_ptr<ClassDecl> _ancestorClass;

public:
    ClassDecl(Token pos,
              shared_ptr<Identifier> name,
              QVector<shared_ptr<Identifier> > fields,
              QMap<QString,MethodInfo> methodPrototypes,
              QVector<shared_ptr<ClassInternalDecl> > internalDecls,
              QMap<QString, shared_ptr<TypeExpression> > fieldMarshalAs,
              bool isPublic);
    ClassDecl(Token pos,
              shared_ptr<Identifier> ancestorName,
              shared_ptr<Identifier> name,
              QVector<shared_ptr<Identifier> > fields,
              QMap<QString,MethodInfo> methodPrototypes,
              QVector<shared_ptr<ClassInternalDecl> > internalDecls,
              QMap<QString, shared_ptr<TypeExpression> > fieldMarshalAs,
              bool isPublic);
    shared_ptr<Identifier> name() { return _name;}
    int fieldCount() { return _fields.count();}
    shared_ptr<Identifier> field(int i) { return _fields[i];}
    bool containsMethod(QString name);
    bool containsPrototype(QString name);
    int methodCount() { return _methods.count();}
    shared_ptr<MethodDecl> method(QString name);
    shared_ptr<MethodDecl> method(int i) { return _methods.values().at(i);}
    MethodInfo methodPrototype(QString name);
    int prototypeCount() { return _methodPrototypes.count();}
    shared_ptr<Identifier> ancestor() { return _ancestorName;}
    bool fieldHasMarshalType(QString fieldName) { return _fieldMarshallAs.contains(fieldName); }
    shared_ptr<TypeExpression> marshalTypeOf(QString fieldName) { return _fieldMarshallAs[fieldName]; }

    void setAncestorClass(shared_ptr<ClassDecl> cd);
    void insertMethod(QString name, shared_ptr<MethodDecl> m);
    QString toString();
    void prettyPrint(CodeFormatter *f);
};

class GlobalDecl : public Declaration
{
public:
    QString varName;
public:
    GlobalDecl(Token pos, QString varName, bool isPublic);
    QString toString();
    void prettyPrint(CodeFormatter *f);
};

class MethodDecl : public ProceduralDecl
{
public:
    shared_ptr<Identifier> _className;
    shared_ptr<Identifier> _receiverName;
    bool isFunctionNotProcedure;
public:
    MethodDecl(Token pos,
               Token endingToken,
               shared_ptr<Identifier> className,
               shared_ptr<Identifier> receiverName,
               shared_ptr<Identifier> methodName,
               QVector<shared_ptr<Identifier> > formals,
               shared_ptr<BlockStmt> body,
               bool isFunctionNotProcedure);
    shared_ptr<Identifier> className() { return _className;}
    shared_ptr<Identifier> receiverName() { return _receiverName;}
    QString toString();
    void prettyPrint(CodeFormatter *f);
};

#endif // DECLARATION_H
