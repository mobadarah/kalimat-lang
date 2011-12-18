#ifndef RULES_H
#define RULES_H

#include "kalimatast.h"
class PegExpr
{

};

class RuleOption
{
    QSharedPointer<PegExpr> expression;
    QSharedPointer<Expression> resultExpr;
public:
    RuleOption(QSharedPointer<PegExpr> _expression, QSharedPointer<Expression> _resultExpr)
        : expression(_expression), resultExpr(_resultExpr)
    {

    }

};

struct RuleDecl
{
    QString ruleName;
    QVector<QSharedPointer<RuleOption> > options;
    RuleDecl(QString ruleName, QVector<QSharedPointer<RuleOption> > options);
};

class RulesDecl : public Declaration
{
    QVector<QSharedPointer<RuleDecl> > _subRules;
    QScopedPointer<Identifier> _ruleName;
public:
    RulesDecl(Token pos, Identifier *name, QVector<RulesDecl *> subRules, bool isPublic);
    int subRuleCount() { return _subRules.count(); }
    RuleDecl *subRule(int i) { return _subRules[i].data(); }
    Identifier *name() { return _ruleName.data(); }

};

#endif // RULES_H
