#ifndef TYPEEXPRESSION_H
#define TYPEEXPRESSION_H

class TypeExpression : public KalimatAst
{
    ASTImpl _astImpl;
public:
    TypeExpression(Token pos) : _astImpl(pos) {}
    Token getPos() { return _astImpl.getPos();}
};

class TypeIdentifier : public TypeExpression
{

public:
    QString name;
    TypeIdentifier(Token pos, QString name);
    QString toString();
    void prettyPrint(CodeFormatter *f);

};

class PointerTypeExpression : public TypeExpression
{

    QScopedPointer<TypeExpression> _pointeeType;
public:
    PointerTypeExpression(Token pos, TypeExpression *pointeeType);
    QString toString();
    void prettyPrint(CodeFormatter *f);
    TypeExpression *pointeeType() { return _pointeeType.data();}

};

class FunctionTypeExpression : public TypeExpression
{

    QScopedPointer<TypeExpression> _retType;
    QVector<QSharedPointer<TypeExpression> > _argTypes;
public:
    FunctionTypeExpression(Token pos, TypeExpression *retType, QVector<TypeExpression *> argTypes);
    QString toString();
    void prettyPrint(CodeFormatter *formatter);
    TypeExpression *retType() { return _retType.data(); }
    int argTypeCount() { return _argTypes.count(); }
    TypeExpression *argType(int i) { return _argTypes[i].data(); }
};

#endif // TYPEEXPRESSION_H
