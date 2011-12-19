#ifndef TYPEEXPRESSION_H
#define TYPEEXPRESSION_H

#include "kalimatast.h"

class TypeExpression : public KalimatAst
{
    Q_OBJECT
    ASTImpl _astImpl;
public:
    TypeExpression(Token pos) : _astImpl(pos) {}
    Token getPos() { return _astImpl.getPos();}
};

class TypeIdentifier : public TypeExpression
{
    Q_OBJECT
public:
    QString name;
    TypeIdentifier(Token pos, QString name);
    QString toString();
    void prettyPrint(CodeFormatter *f);
};

class PointerTypeExpression : public TypeExpression
{
    Q_OBJECT
    shared_ptr<TypeExpression> _pointeeType;
public:
    PointerTypeExpression(Token pos, shared_ptr<TypeExpression> pointeeType);
    QString toString();
    void prettyPrint(CodeFormatter *f);
    TypeExpression *pointeeType() { return _pointeeType.get();}
};

class FunctionTypeExpression : public TypeExpression
{
    Q_OBJECT
    shared_ptr<TypeExpression> _retType;
    QVector<shared_ptr<TypeExpression> > _argTypes;
public:
    FunctionTypeExpression(Token pos,
                           shared_ptr<TypeExpression> retType,
                           QVector<shared_ptr<TypeExpression> > argTypes);
    QString toString();
    void prettyPrint(CodeFormatter *formatter);
    TypeExpression *retType() { return _retType.get(); }
    int argTypeCount() { return _argTypes.count(); }
    TypeExpression *argType(int i) { return _argTypes[i].get(); }
};

#endif // TYPEEXPRESSION_H
