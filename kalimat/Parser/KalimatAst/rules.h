#ifndef RULES_H
#define RULES_H

#include "kalimatast.h"
#include "declaration.h"

class PegExpr : public KalimatAst
{
    ASTImpl _astImpl;
public:
    PegExpr(Token pos) :
        _astImpl(pos)
    {

    }

    Token getPos() { return _astImpl.getPos();}
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
    int elementCount() {return elements.count();}
    shared_ptr<PegExpr> element(int i) {return elements[i];}
    void prettyPrint(CodeFormatter *);
    QString toString();
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
};

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
    shared_ptr<Identifier> ruleName() { return _ruleName;}
    void prettyPrint(CodeFormatter *);
    QString toString();
};

class RuleOption
{
    shared_ptr<PegExpr> _expression;
    shared_ptr<Expression> _resultExpr;
public:
    RuleOption(shared_ptr<PegExpr> _expression,
               shared_ptr<Expression> _resultExpr)
        : _expression(_expression), _resultExpr(_resultExpr)
    {

    }
    shared_ptr<PegExpr> expression() { return _expression;}
    shared_ptr<Expression> resultExpr() { return _resultExpr;}
};

struct RuleDecl
{
    QString ruleName;
    QVector<shared_ptr<RuleOption> > options;
    RuleDecl(QString ruleName, QVector<shared_ptr<RuleOption> > options)
        :ruleName(ruleName),
          options(options)
    {

    }
};

class RulesDecl : public Declaration
{
    Q_OBJECT
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
};

#endif // RULES_H
