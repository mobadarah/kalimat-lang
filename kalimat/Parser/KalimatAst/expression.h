#ifndef EXPRESSION_H
#define EXPRESSION_H

class Pattern;
class Expression : public AST
{

    ASTImpl _astImpl;
public:
    Expression(Token pos);
    Token getPos() { return _astImpl.getPos();}
};

class AssignableExpression : public Expression
{

public:
    AssignableExpression(Token pos);
};

class Literal : public Expression
{

public:
    Literal(Token pos): Expression(pos) {}
};

class SimpleLiteral : public Literal
{

public:
    SimpleLiteral(Token pos) : Literal(pos) {}
};

class BinaryOperation : public Expression
{
public:
    QString _operator;
    QScopedPointer <Expression> _operand1, _operand2;
public:
    BinaryOperation(Token pos, QString op,Expression *op1, Expression *op2);
    Expression *operand1() { return _operand1.data();}
    Expression *operand2() { return _operand2.data();}
    QString toString();
    void prettyPrint(CodeFormatter *f);
};

class IsaOperation : public Expression
{
public:
    QScopedPointer<Expression> _expression;
    QScopedPointer<Identifier> _type;
public:
    IsaOperation(Token pos, Expression *expression, Identifier *type);
    Expression *expression() { return _expression.data(); }
    Identifier *type() { return _type.data(); }
    QString toString();
    void prettyPrint(CodeFormatter *f);
};

class MatchOperation : public Expression
{
public:
    QScopedPointer<Expression> _expression;
    QScopedPointer<Pattern> _pattern;
public:
    MatchOperation(Token pos, Expression *expression, Pattern *pattern);
    Expression *expression() { return _expression.data(); }
    Pattern *pattern() { return _pattern.data(); }
    QString toString();
    void prettyPrint(CodeFormatter *f);
};

class UnaryOperation : public Expression
{
public:
    QString _operator;
    QScopedPointer <Expression> _operand;
public:
    UnaryOperation(Token pos, QString operation,Expression *operand);
    Expression *operand() { return _operand.data();}
    QString toString();
    void prettyPrint(CodeFormatter *f);
};

class Identifier : public AssignableExpression
{
public:
    QString name;
public:
    Identifier(Token pos, QString name);
    QString toString();
    void prettyPrint(CodeFormatter *f);
};

class NumLiteral : public SimpleLiteral
{
public:
    long lValue;
    double dValue;
    bool longNotDouble;
    bool valueRecognized;
public:
    NumLiteral(Token pos, QString lexeme);
    QString toString();
    QString repr();
    void prettyPrint(CodeFormatter *f);
};

class StrLiteral : public SimpleLiteral
{
public:
    QString value;
public:
    StrLiteral(Token pos, QString value);
    QString toString();
    QString repr();
    void prettyPrint(CodeFormatter *f);
};
class NullLiteral : public SimpleLiteral
{
public:
    NullLiteral(Token pos);
    QString toString();
    QString repr();
    void prettyPrint(CodeFormatter *f);
};
class BoolLiteral : public SimpleLiteral
{
public:
    bool value;
public:
    BoolLiteral(Token pos, bool value);
    QString toString();
    QString repr();
    void prettyPrint(CodeFormatter *f);
};

class ArrayLiteral : public Literal
{
public:
    QVector<QSharedPointer<Expression > > _data;
public:
    ArrayLiteral(Token pos, QVector<Expression *> data);
    int dataCount() {return _data.count();}
    Expression *data(int i) { return _data[i].data(); }
    QVector<QSharedPointer<Expression> > dataVector() { return _data;}
    QString toString();
    void prettyPrint(CodeFormatter *f);
};

class MapLiteral : public Literal
{
public:
    QVector<QSharedPointer<Expression > > _data;
public:
    MapLiteral(Token pos, QVector<Expression *> data);
    int dataCount() {return _data.count();}
    Expression *data(int i) { return _data[i].data(); }
    QVector<QSharedPointer<Expression> > dataVector() { return _data;}
    QString toString();
    void prettyPrint(CodeFormatter *f);
};

class IInvokation : public Expression
{
public:
    IInvokation(Token pos);
};

class Invokation : public IInvokation
{
public:
    QScopedPointer<Expression> _functor;
    QVector<QSharedPointer<Expression > > _arguments;
public:
    Invokation(Token pos, Expression *functor, QVector<Expression *> arguments);
    Expression *functor() { return _functor.data();}
    int argumentCount() {return _arguments.count();}
    Expression *argument(int i) { return _arguments[i].data(); }
    virtual QString toString();
    virtual void prettyPrint(CodeFormatter *f);
};

class MethodInvokation : public IInvokation
{
public:
    QScopedPointer<Expression> _receiver;
    QScopedPointer<Identifier> _methodSelector;
    QVector<QSharedPointer<Expression > > _arguments;
public:
    MethodInvokation(Token pos, Expression *receiver, Identifier *methodSelector, QVector<Expression *> arguments);
    Expression *receiver() {return _receiver.data();}
    Identifier *methodSelector() { return _methodSelector.data();}
    int argumentCount() { return _arguments.count();}
    Expression *argument(int i) {return _arguments[i].data();}
    QString toString();
    void prettyPrint(CodeFormatter *f);
};

class Idafa : public AssignableExpression
{
public:
    QScopedPointer<Identifier> _modaf;
    QScopedPointer<Expression> _modaf_elaih;
public:
    Idafa(Token pos, Identifier *modaf, Expression *modaf_elaih);
    Identifier *modaf() {return _modaf.data();}
    Expression *modaf_elaih() {return _modaf_elaih.data();}
    QString toString();
    void prettyPrint(CodeFormatter *f);
};

class ArrayIndex : public AssignableExpression
{
public:
    QScopedPointer<Expression> _array;
    QScopedPointer<Expression> _index;
public:
    ArrayIndex(Token pos, Expression *array, Expression *index);
    Expression *array() {return _array.data();}
    Expression *index() {return _index.data();}
    QString toString();
    void prettyPrint(CodeFormatter *f);
};

class MultiDimensionalArrayIndex : public AssignableExpression
{
public:
    QScopedPointer<Expression> _array;
    QVector<QSharedPointer<Expression> > _indexes;
public:
    MultiDimensionalArrayIndex(Token pos, Expression *array, QVector<Expression *> indexes);
    Expression *array() {return _array.data();}
    int indexCount() { return _indexes.count();}
    Expression *index(int i) {return _indexes[i].data();}
    QVector<QSharedPointer<Expression> > indexes() {return _indexes;}
    QString toString();
    void prettyPrint(CodeFormatter *f);
};

class ObjectCreation : public Expression
{
public:
    QScopedPointer<Identifier> _className;
public:
    ObjectCreation(Token pos, Identifier *className);
    Identifier *className() { return _className.data();}
    QString toString();
    void prettyPrint(CodeFormatter *f);
};

#endif // EXPRESSION_H
