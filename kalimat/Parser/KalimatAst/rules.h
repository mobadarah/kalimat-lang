#ifndef RULES_H
#define RULES_H

#include "kalimatast.h"
#include "declaration.h"
#include "expression.h"

class PegExpr : public KalimatAst
{
    ASTImpl _astImpl;
public:
    PegExpr(Token pos) :
        _astImpl(pos)
    {

    }
    virtual ~PegExpr() {}
    virtual QSet<QString> getAllAssociatedVars()=0;
    Token getPos() { return _astImpl.getPos();}
};


class PegPrimary : public PegExpr
{
    shared_ptr<Identifier> _associatedVar;
public:
    PegPrimary(Token pos,
               shared_ptr<Identifier> associatedVar)
        :PegExpr(pos),
          _associatedVar(associatedVar)
    {

    }
    shared_ptr<Identifier> associatedVar() { return _associatedVar; }
    void setAssociatedVar(shared_ptr<Identifier> v)
    {
        _associatedVar = v;
    }
    virtual QSet<QString> getAllAssociatedVars();
};

class PegSequence : public PegExpr
{
    QVector<shared_ptr<PegExpr> > elements;
public:
    PegSequence(Token pos,QVector<shared_ptr<PegExpr> > elements)
        :PegExpr(pos),
          elements(elements)
    {

    }
    ~PegSequence() {}
    int elementCount() {return elements.count();}
    shared_ptr<PegExpr> element(int i) {return elements[i];}
    void prettyPrint(CodeFormatter *);
    QString toString();
    virtual QSet<QString> getAllAssociatedVars();
};

/*
class PegOr : public PegExpr
{
    QVector<shared_ptr<PegExpr> > elements;
public:
    PegOr(QVector<shared_ptr<PegExpr> > elements)
        :elements(elements)
    {

    }
    int elementCount() {return elements.count();}
    shared_ptr<PegExpr> element(int i) {return elements[i];}
};
*/

class PegRuleInvokation : public PegPrimary
{
    shared_ptr<Identifier> _ruleName;
public:
    PegRuleInvokation(Token pos,
                      shared_ptr<Identifier> associatedVar,
                      shared_ptr<Identifier> _ruleName)
        : PegPrimary(pos, associatedVar),
          _ruleName(_ruleName)
    {

    }
    ~PegRuleInvokation() {}
    shared_ptr<Identifier> ruleName() { return _ruleName;}
    void prettyPrint(CodeFormatter *);
    QString toString();
};

class PegLiteral : public PegPrimary
{
    shared_ptr<StrLiteral> _value;
public:
    PegLiteral(Token pos,
                      shared_ptr<Identifier> associatedVar,
                      shared_ptr<StrLiteral> _value)
        : PegPrimary(pos, associatedVar),
          _value(_value)
    {

    }
    ~PegLiteral() {}
    shared_ptr<StrLiteral> value() { return _value;}
    void prettyPrint(CodeFormatter *);
    QString toString() { return _value->toString(); }
};

class PegCharRange : public PegPrimary
{
    shared_ptr<StrLiteral> _value1;
    shared_ptr<StrLiteral> _value2;
public:
    PegCharRange(Token pos,
                      shared_ptr<Identifier> associatedVar,
                      shared_ptr<StrLiteral> _value1,
                      shared_ptr<StrLiteral> _value2)
        : PegPrimary(pos, associatedVar),
          _value1(_value1), _value2(_value2)
    {

    }
    ~PegCharRange() {}
    shared_ptr<StrLiteral> value1() { return _value1;}
    shared_ptr<StrLiteral> value2() { return _value2;}
    void prettyPrint(CodeFormatter *);
    QString toString()
    {
        return QString::fromStdWString(
                    L"(من %1 إلى %2)").arg(_value1->toString())
                .arg(_value2->toString());
    }
};

/*
  rep(stmt)
  expr = term:t1 rep(x=t1, "+" term:t2, x=binOp("+", t1,t2)):x
*/
class PegRepetion : public PegPrimary
{
public:
    bool hasResult;
private:
    shared_ptr<Identifier> _resultVar;
    shared_ptr<PegExpr> _subExpr;
    shared_ptr<AssignmentStmt> _stepAssignment;
public:
};

class RuleOption : public KalimatAst
{
    ASTImpl _astImpl;
    shared_ptr<PegExpr> _expression;
    shared_ptr<Expression> _resultExpr;
public:
    RuleOption(Token pos, shared_ptr<PegExpr> _expression,
               shared_ptr<Expression> _resultExpr)
        : _astImpl(pos),
          _expression(_expression),
          _resultExpr(_resultExpr)
    {

    }
    shared_ptr<PegExpr> expression() { return _expression;}
    shared_ptr<Expression> resultExpr() { return _resultExpr;}
    Token getPos() { return _astImpl.getPos();}
    QString toString();
    void prettyPrint(CodeFormatter *formatter);
};

struct RuleDecl : public KalimatAst
{
    ASTImpl _astImpl;
    QString ruleName;
    QVector<shared_ptr<RuleOption> > options;
    RuleDecl(Token pos, QString ruleName, QVector<shared_ptr<RuleOption> > options)
        : _astImpl(pos),
          ruleName(ruleName),
          options(options)
    {

    }
    virtual QSet<QString> getAllAssociatedVars();
    Token getPos() { return _astImpl.getPos();}
    QString toString();
    void prettyPrint(CodeFormatter *formatter);
};

class RulesDecl : public Declaration
{
    QVector<shared_ptr<RuleDecl> > _subRules;
    shared_ptr<Identifier> _ruleName;
public:
    RulesDecl(Token pos,
              shared_ptr<Identifier> name,
              QVector<shared_ptr<RuleDecl> > subRules,
              bool isPublic);
    int subRuleCount() { return _subRules.count(); }
    shared_ptr<RuleDecl> subRule(int i) { return _subRules[i]; }
    shared_ptr<Identifier> name() { return _ruleName; }
    void prettyPrint(CodeFormatter *);
    QString toString();
    virtual QSet<QString> getAllAssociatedVars();
};

#endif // RULES_H
