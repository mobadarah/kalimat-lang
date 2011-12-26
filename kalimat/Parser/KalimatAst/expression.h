#ifndef EXPRESSION_H
#define EXPRESSION_H

#include "kalimatast.h"

class Pattern;
class Expression : public KalimatAst
{
    Q_OBJECT
    ASTImpl _astImpl;
public:
    Expression(Token pos);
    Token getPos() { return _astImpl.getPos();}
};

class AssignableExpression : public Expression
{
    Q_OBJECT
public:
    AssignableExpression(Token pos);
};

class Literal : public Expression
{
    Q_OBJECT
public:
    Literal(Token pos): Expression(pos) {}
};

class SimpleLiteral : public Literal
{
    Q_OBJECT
public:
    SimpleLiteral(Token pos) : Literal(pos) {}
};

class BinaryOperation : public Expression
{
    Q_OBJECT
public:
    QString _operator;
    shared_ptr<Expression> _operand1, _operand2;
public:
    BinaryOperation(Token pos,
                    QString op,
                    shared_ptr<Expression> op1,
                    shared_ptr<Expression> op2);
    shared_ptr<Expression> operand1() { return _operand1;}
    shared_ptr<Expression> operand2() { return _operand2;}
    QString toString();
    void prettyPrint(CodeFormatter *f);
};

class IsaOperation : public Expression
{
    Q_OBJECT
public:
    shared_ptr<Expression> _expression;
    shared_ptr<Identifier> _type;
public:
    IsaOperation(Token pos, shared_ptr<Expression> expression, shared_ptr<Identifier> type);
    shared_ptr<Expression> expression() { return _expression; }
    shared_ptr<Identifier> type() { return _type; }
    QString toString();
    void prettyPrint(CodeFormatter *f);
};

class MatchOperation : public Expression
{
    Q_OBJECT
public:
    shared_ptr<Expression> _expression;
    shared_ptr<Pattern> _pattern;
public:
    MatchOperation(Token pos,
                   shared_ptr<Expression> expression,
                   shared_ptr<Pattern> pattern);
    ~MatchOperation();
    shared_ptr<Expression> expression() { return _expression; }
    shared_ptr<Pattern> pattern() { return _pattern; }
    QString toString();
    void prettyPrint(CodeFormatter *f);
};

class UnaryOperation : public Expression
{
    Q_OBJECT
public:
    QString _operator;
    shared_ptr<Expression> _operand;
public:
    UnaryOperation(Token pos, QString operation,shared_ptr<Expression> operand);
    shared_ptr<Expression> operand() { return _operand;}
    QString toString();
    void prettyPrint(CodeFormatter *f);
};

class Identifier : public AssignableExpression
{
    Q_OBJECT
public:
    QString name;
public:
    Identifier(Token pos, QString name);
    QString toString();
    void prettyPrint(CodeFormatter *f);
};

class NumLiteral : public SimpleLiteral
{
    Q_OBJECT
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
    Q_OBJECT
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
    Q_OBJECT
public:
    NullLiteral(Token pos);
    QString toString();
    QString repr();
    void prettyPrint(CodeFormatter *f);
};
class BoolLiteral : public SimpleLiteral
{
    Q_OBJECT
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
    Q_OBJECT
public:
    QVector<shared_ptr<Expression > > _data;
public:
    ArrayLiteral(Token pos, QVector<shared_ptr<Expression> > data);
    int dataCount() {return _data.count();}
    Expression *data(int i) { return _data[i].get(); }
    QVector<shared_ptr<Expression> > dataVector() { return _data;}
    QString toString();
    void prettyPrint(CodeFormatter *f);
};

class MapLiteral : public Literal
{
    Q_OBJECT
public:
    QVector<shared_ptr<Expression > > _data;
public:
    MapLiteral(Token pos, QVector<shared_ptr<Expression> > data);
    int dataCount() {return _data.count();}
    Expression *data(int i) { return _data[i].get(); }
    QVector<shared_ptr<Expression> > dataVector() { return _data;}
    QString toString();
    void prettyPrint(CodeFormatter *f);
};

class IInvokation : public Expression
{
    Q_OBJECT
public:
    IInvokation(Token pos);
};

class Invokation : public IInvokation
{
    Q_OBJECT
public:
    shared_ptr<Expression> _functor;
    QVector<shared_ptr<Expression > > _arguments;
public:
    Invokation(Token pos,
               shared_ptr<Expression> functor,
               QVector<shared_ptr<Expression> > arguments);
    Expression *functor() { return _functor.get();}
    int argumentCount() {return _arguments.count();}
    Expression *argument(int i) { return _arguments[i].get(); }
    virtual QString toString();
    virtual void prettyPrint(CodeFormatter *f);
};

class MethodInvokation : public IInvokation
{
    Q_OBJECT
public:
    shared_ptr<Expression> _receiver;
    shared_ptr<Identifier> _methodSelector;
    QVector<shared_ptr<Expression > > _arguments;
public:
    MethodInvokation(Token pos,
                     shared_ptr<Expression> receiver,
                     shared_ptr<Identifier> methodSelector,
                     QVector<shared_ptr<Expression> > arguments);
    Expression *receiver() {return _receiver.get();}
    Identifier *methodSelector() { return _methodSelector.get();}
    int argumentCount() { return _arguments.count();}
    Expression *argument(int i) {return _arguments[i].get();}
    QString toString();
    void prettyPrint(CodeFormatter *f);
};

class Idafa : public AssignableExpression
{
    Q_OBJECT
public:
    shared_ptr<Identifier> _modaf;
    shared_ptr<Expression> _modaf_elaih;
public:
    Idafa(Token pos, shared_ptr<Identifier> modaf, shared_ptr<Expression> modaf_elaih);
    shared_ptr<Identifier> modaf() {return _modaf;}
    shared_ptr<Expression> modaf_elaih() {return _modaf_elaih;}
    QString toString();
    void prettyPrint(CodeFormatter *f);
};

class ArrayIndex : public AssignableExpression
{
    Q_OBJECT
public:
    shared_ptr<Expression> _array;
    shared_ptr<Expression> _index;
public:
    ArrayIndex(Token pos, shared_ptr<Expression> array, shared_ptr<Expression> index);
    shared_ptr<Expression> array() {return _array;}
    shared_ptr<Expression> index() {return _index;}
    QString toString();
    void prettyPrint(CodeFormatter *f);
};

class MultiDimensionalArrayIndex : public AssignableExpression
{
    Q_OBJECT
public:
    shared_ptr<Expression> _array;
    QVector<shared_ptr<Expression> > _indexes;
public:
    MultiDimensionalArrayIndex(Token pos,
                               shared_ptr<Expression> array,
                               QVector<shared_ptr<Expression> > indexes);
    shared_ptr<Expression> array() {return _array;}
    int indexCount() { return _indexes.count();}
    shared_ptr<Expression> index(int i) {return _indexes[i];}
    QVector<shared_ptr<Expression> > indexes() {return _indexes;}
    QString toString();
    void prettyPrint(CodeFormatter *f);
};

class ObjectCreation : public Expression
{
    Q_OBJECT
public:
    shared_ptr<Identifier> _className;
public:
    ObjectCreation(Token pos, shared_ptr<Identifier> className);
    shared_ptr<Identifier> className() { return _className;}
    QString toString();
    void prettyPrint(CodeFormatter *f);
};

#endif // EXPRESSION_H
