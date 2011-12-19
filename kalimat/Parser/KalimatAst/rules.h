#ifndef RULES_H
#define RULES_H

#include "kalimatast.h"
#include "declaration.h"

class PegExpr
{

};

class RuleOption
{
    shared_ptr<PegExpr> expression;
    shared_ptr<Expression> resultExpr;
public:
    RuleOption(shared_ptr<PegExpr> _expression, shared_ptr<Expression> _resultExpr)
        : expression(_expression), resultExpr(_resultExpr)
    {

    }

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
              Identifier *name,
              QVector<shared_ptr<RulesDecl> > subRules,
              bool isPublic);
    int subRuleCount() { return _subRules.count(); }
    RuleDecl *subRule(int i) { return _subRules[i].get(); }
    Identifier *name() { return _ruleName.get(); }

};

#endif // RULES_H
