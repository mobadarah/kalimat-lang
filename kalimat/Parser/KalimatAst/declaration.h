#ifndef DECLARATION_H
#define DECLARATION_H

#include "kalimatast.h"

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
    QSharedPointer<Identifier> _procName;
    QVector<QSharedPointer<Identifier > > _formals;
    QVector<QSharedPointer<Identifier > > _allReferences;
    QSharedPointer<BlockStmt> _body;
    Token _endingToken;
public:
    ProceduralDecl(Token pos,
                   Token endingToken,
                   QSharedPointer<Identifier> procName,
                   QVector<QSharedPointer<Identifier> > formals,
                   QSharedPointer<BlockStmt> body,
                   bool isPublic);
    Identifier *procName() {return _procName.data();}
    int formalCount() { return _formals.count();}
    Identifier *formal(int i) { return _formals[i].data();}
    Identifier *allReferences(int i) { return _allReferences[i].data();}
    void addReference(Identifier *id) { _allReferences.append(QSharedPointer<Identifier>(id));}
    BlockStmt *body() {return _body.data();}
    void body(QSharedPointer<BlockStmt> stmt) { _body = stmt;}
    virtual QVector<QSharedPointer<Identifier> > getIntroducedVariables();
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
                  QSharedPointer<Identifier> procName,
                  QVector<QSharedPointer<Identifier> > formals,
                  QSharedPointer<BlockStmt> body,
                  bool isPublic);
    QString toString();
    void prettyPrint(CodeFormatter *f);
};

class FunctionDecl : public ProceduralDecl, public IFunction
{
public:
    FunctionDecl(Token pos,
                 Token endingToken,
                 QSharedPointer<Identifier> procName,
                 QVector<QSharedPointer<Identifier> > formals,
                 QSharedPointer<BlockStmt> body,
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
    QVector<QSharedPointer<Declaration> > _decls;
public:
    FFILibraryDecl(Token pos, QString libName, QVector<QSharedPointer<Declaration> > decls, bool isPublic);
    int declCount() { return _decls.count(); }
    Declaration *decl(int index) { return _decls[index].data();}
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
    QScopedPointer<TypeExpression> _returnType; // NULL -> It's a procedure
    QVector<QSharedPointer<TypeExpression> > _paramTypes;
public:
    FFIProceduralDecl(Token pos, bool isFunctionNotProc,
        TypeExpression *returnType,
        QVector<TypeExpression*> paramTypes,
        QString procName,
        QString symbol,
        bool isPublic);
    TypeExpression *returnType() { return _returnType.data(); }
    int paramTypeCount() { return _paramTypes.count(); }
    TypeExpression *paramType(int index) { return _paramTypes.at(index).data(); }
    void prettyPrint(CodeFormatter *formatter);
    QString toString();
};

class FFIStructDecl : public Declaration
{
public:
    QVector<int> fieldBatches; // For grouping HAS a, b, c; HAS d,e declarations for pretty printing
private:
    QScopedPointer<Identifier> _name;
    QVector<QSharedPointer<Identifier> > _fieldNames;
    QVector<QSharedPointer<TypeExpression> > _fieldTypes;
public:
    FFIStructDecl(Token pos,
                  Identifier *name,
                  QVector<Identifier*> fieldNames,
                  QVector<TypeExpression*> fieldTypes,
                  QVector<int> fieldBatches,
                  bool isPublic);
    int fieldCount() { return _fieldNames.count();}
    Identifier *name() { return _name.data(); }
    Identifier *fieldName(int index) { return _fieldNames[index].data();}
    TypeExpression *fieldType(int index) { return _fieldTypes[index].data();}
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
    QSharedPointer<Identifier> name;
    QVector<QSharedPointer<Identifier> > params;
    void prettyPrint(CodeFormatter *f);

    ConcreteResponseInfo(Identifier *_name)
    {
        name = QSharedPointer<Identifier>(_name);
    }
    void add(Identifier *param)
    {
        params.append(QSharedPointer<Identifier>(param));
    }
};

struct Has : public ClassInternalDecl
{
    QMap<QString, QSharedPointer<TypeExpression> > _fieldMarshallAs;
    QVector<QSharedPointer<Identifier> > fields;
    void prettyPrint(CodeFormatter *f);
    void add(Identifier *field)
    {
        fields.append(QSharedPointer<Identifier>(field));
    }
};

struct RespondsTo : public ClassInternalDecl
{
    bool isFunctions;
    QVector<QSharedPointer<ConcreteResponseInfo> > methods;

    RespondsTo(bool _isFunctions) { isFunctions = _isFunctions;}
    void prettyPrint(CodeFormatter *f);
    void add(ConcreteResponseInfo *mi)
    {
        methods.append(QSharedPointer<ConcreteResponseInfo>(mi));
    }
};

class ClassDecl : public Declaration
{
public:
    QScopedPointer<Identifier> _name;
    QVector<QSharedPointer<Identifier > > _fields;
    QMap<QString, QSharedPointer<TypeExpression> > _fieldMarshallAs;
    QMap<QString, MethodInfo>  _methodPrototypes;
    QMap<QString, QSharedPointer<MethodDecl> > _methods;
    QScopedPointer<Identifier> _ancestorName;
    QSharedPointer<ClassDecl> _ancestorClass;
    QVector<QSharedPointer<ClassInternalDecl> > _internalDecls; // For pretty printing...etc
public:
    ClassDecl(Token pos,
              Identifier *name,
              QVector<Identifier *> fields,
              QMap<QString,MethodInfo> methodPrototypes,
              QVector<QSharedPointer<ClassInternalDecl> > internalDecls,
              QMap<QString, TypeExpression *> fieldMarshalAs,
              bool isPublic);
    ClassDecl(Token pos,
              Identifier *ancestorName,
              Identifier *name,
              QVector<Identifier *> fields,
              QMap<QString,MethodInfo> methodPrototypes,
              QVector<QSharedPointer<ClassInternalDecl> > internalDecls,
              QMap<QString, TypeExpression *> fieldMarshalAs,
              bool isPublic);
    Identifier *name() { return _name.data();}
    int fieldCount() { return _fields.count();}
    Identifier *field(int i) { return _fields[i].data();}
    bool containsMethod(QString name);
    bool containsPrototype(QString name);
    int methodCount() { return _methods.count();}
    MethodDecl *method(QString name);
    MethodDecl *method(int i) { return _methods.values().at(i).data();}
    MethodInfo methodPrototype(QString name);
    int prototypeCount() { return _methodPrototypes.count();}
    Identifier *ancestor() { return _ancestorName.data();}
    bool fieldHasMarshalType(QString fieldName) { return _fieldMarshallAs.contains(fieldName); }
    TypeExpression *marshalTypeOf(QString fieldName) { return _fieldMarshallAs[fieldName].data(); }

    void setAncestorClass(QSharedPointer<ClassDecl> cd);
    void insertMethod(QString name, QSharedPointer<MethodDecl> m);
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
    QScopedPointer<Identifier> _className;
    QScopedPointer<Identifier> _receiverName;
    bool isFunctionNotProcedure;
public:
    MethodDecl(Token pos, Token endingToken, Identifier *className, Identifier *receiverName,
               Identifier *methodName, QVector<Identifier *> formals, BlockStmt *body
               , bool isFunctionNotProcedure);
    Identifier *className() { return _className.data();}
    Identifier *receiverName() { return _receiverName.data();}
    QString toString();
    void prettyPrint(CodeFormatter *f);
};

#endif // DECLARATION_H
