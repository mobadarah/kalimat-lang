#ifndef STATEMENT_H
#define STATEMENT_H

#include "toplevel.h"
#include "kalimatast.h"

class Statement : public TopLevel
{
    Q_OBJECT
public:
    Statement(Token pos);
};

class IOStatement : public Statement
{
    Q_OBJECT
public:
    IOStatement(Token pos);
};

class GraphicsStatement : public Statement
{
    Q_OBJECT
public:
    GraphicsStatement(Token pos);
};

class AssignmentStmt : public Statement
{
    Q_OBJECT
public:
    QSharedPointer <AssignableExpression> _variable;
    QSharedPointer<Expression> _value;
public:
    AssignmentStmt(Token pos, QSharedPointer<AssignableExpression> variable, QSharedPointer<Expression> value);
    ~AssignmentStmt();
    AssignableExpression *variable() {return _variable.data();}
    Expression *value() {return _value.data();}
    QString toString();
    void prettyPrint(CodeFormatter *f);
};

class IfStmt : public Statement
{
    Q_OBJECT
public:
    QSharedPointer<Expression> _condition;
    QSharedPointer<Statement> _thenPart;
    QSharedPointer<Statement> _elsePart;
public:
    IfStmt(Token pos,
           QSharedPointer<Expression> _condition,
           QSharedPointer<Statement> _thenPart,
           QSharedPointer<Statement> _elsePart);
    ~IfStmt();
    Expression *condition() {return _condition.data();}
    Statement *thenPart() { return _thenPart.data();}
    Statement *elsePart() {return _elsePart.data();}
    QString toString();
    void prettyPrint(CodeFormatter *f);
};

class WhileStmt : public Statement
{
    Q_OBJECT
public:
    QSharedPointer<Expression> _condition;
    QSharedPointer<Statement> _statement;

public:
    WhileStmt(Token pos,
              QSharedPointer<Expression> _condition,
              QSharedPointer<Statement> _statement);
    ~WhileStmt();
    Expression *condition() {return _condition.data();}
    Statement *statement() { return _statement.data();}
    QString toString();
    void prettyPrint(CodeFormatter *f);
};

class ForAllStmt : public Statement
{
    Q_OBJECT
public:
    QSharedPointer<Identifier> _variable;
    QSharedPointer<Expression> _from;
    QSharedPointer<Expression> _to;
    QSharedPointer<Statement> _statement;

public:
    ForAllStmt(Token pos,
               QSharedPointer<Identifier> variable,
               QSharedPointer<Expression> from,
               QSharedPointer<Expression> to,
               QSharedPointer<Statement>  statement);
    ~ForAllStmt();
    Identifier *variable() { return _variable.data(); }
    Expression *from() { return _from.data(); }
    Expression *to() { return _to.data(); }
    Statement *statement() { return _statement.data();}
    QString toString();
    void prettyPrint(CodeFormatter *f);

    virtual QVector<Identifier *> getIntroducedVariables();

};

class ReturnStmt : public Statement
{
    Q_OBJECT
public:
    QSharedPointer<Expression> _returnVal;
public:
    ReturnStmt(Token pos, QSharedPointer<Expression> returnVal);
    ~ReturnStmt();
    Expression *returnVal() { return _returnVal.data(); }
    QString toString();
    void prettyPrint(CodeFormatter *f);
};

class DelegationStmt : public Statement
{
    Q_OBJECT
public:
    QScopedPointer<IInvokation> _invokation;
public:
    DelegationStmt(Token pos, IInvokation *invokation);
    ~DelegationStmt();
    IInvokation *invokation() { return _invokation.data(); }
    QString toString();
    void prettyPrint(CodeFormatter *f);
    Q_DISABLE_COPY(DelegationStmt)
};

class LaunchStmt : public Statement
{
    Q_OBJECT
public:
    QScopedPointer<IInvokation> _invokation;
public:
    LaunchStmt(Token pos, IInvokation *invokation);
    ~LaunchStmt();
    IInvokation *invokation() { return _invokation.data(); }
    QString toString();
    void prettyPrint(CodeFormatter *f);
    Q_DISABLE_COPY(LaunchStmt)
};

class LabelStmt : public Statement
{
    Q_OBJECT
    QSharedPointer<Expression> _target;
public:
    LabelStmt(Token pos, Expression *target);
    Expression *target() { return _target.data(); }
    QString toString();
    void prettyPrint(CodeFormatter *f);
};


class GotoStmt : public Statement
{
    Q_OBJECT
public:
    bool targetIsNumber;
    QSharedPointer<NumLiteral> _numericTarget;
    QSharedPointer<Identifier> _idTarget;
public:
    GotoStmt(Token pos, bool _targetIsNumber, Expression *target);
    Identifier *idTarget() { return _idTarget.data(); }
    NumLiteral *numericTarget() { return _numericTarget.data(); }
    QString toString();
    void prettyPrint(CodeFormatter *f);
};

class PrintStmt : public IOStatement
{
    Q_OBJECT
public:
    QSharedPointer<Expression> _fileObject;
    QVector<QSharedPointer<Expression > > _args;
    QVector<QSharedPointer<Expression> > _widths;
    bool printOnSameLine;
public:
    PrintStmt(Token pos, Expression *fileObject, QVector<Expression *> args, QVector<Expression *> widths, bool printOnSameLine);
    int argCount() { return _args.count(); }
    Expression *arg(int i) { return _args[i].data(); }
    Expression *fileObject() { return _fileObject.data();}
    QString toString();
    void prettyPrint(CodeFormatter *f);
};

class ReadStmt : public IOStatement
{
    Q_OBJECT
public:
    QString prompt;
    QSharedPointer<Expression> _fileObject;
    QVector<QSharedPointer<AssignableExpression> > _variables;
    QVector<bool> readNumberFlags;
public:
    ReadStmt(Token pos, Expression *fileObject, QString prompt, const QVector<AssignableExpression*> &variables, QVector<bool> readNumberFlags);
    int variableCount() { return _variables.count();}
    AssignableExpression *variable(int i) { return _variables[i].data();}
    Expression *fileObject() { return _fileObject.data();}
    QString toString();
    void prettyPrint(CodeFormatter *f);
};

class DrawPixelStmt : public GraphicsStatement
{
    Q_OBJECT
public:
    QScopedPointer<Expression> _x, _y;
    QScopedPointer<Expression> _color;
public:
    DrawPixelStmt(Token pos, Expression *x, Expression *y, Expression *color);
    ~DrawPixelStmt();
    Expression *x() { return _x.data();}
    Expression *y() { return _y.data();}
    Expression *color() { return _color.data();}
    QString toString();
    void prettyPrint(CodeFormatter *f);
    Q_DISABLE_COPY(DrawPixelStmt)
};

class DrawLineStmt : public GraphicsStatement
{
    Q_OBJECT
public:
    QScopedPointer<Expression> _x1, _y1, _x2, _y2;
    QScopedPointer<Expression> _color;
public:
    DrawLineStmt(Token pos, Expression *x1, Expression *y1, Expression *x2, Expression *y2, Expression *color);
    ~DrawLineStmt();
    Expression *x1() { return _x1.data();}
    Expression *y1() { return _y1.data();}
    Expression *x2() { return _x2.data();}
    Expression *y2() { return _y2.data();}
    Expression *color() { return _color.data();}
    QString toString();
    void prettyPrint(CodeFormatter *f);
    Q_DISABLE_COPY(DrawLineStmt)
};

class DrawRectStmt : public GraphicsStatement
{
    Q_OBJECT
public:
    QScopedPointer<Expression> _x1, _y1, _x2, _y2;
    QScopedPointer<Expression> _color;
    QSharedPointer<Expression> _filled;
public:
    DrawRectStmt(Token pos, Expression *x1, Expression *y1, Expression *x2, Expression *y2, Expression *color, Expression *filled);
    ~DrawRectStmt();
    Expression *x1() { return _x1.data();}
    Expression *y1() { return _y1.data();}
    Expression *x2() { return _x2.data();}
    Expression *y2() { return _y2.data();}
    Expression *color() { return _color.data();}
    Expression *filled() { return _filled.data(); }
    QString toString();
    void prettyPrint(CodeFormatter *f);
    Q_DISABLE_COPY(DrawRectStmt)
};

class DrawCircleStmt : public GraphicsStatement
{
    Q_OBJECT
public:
    QScopedPointer<Expression> _cx, _cy;
    QScopedPointer<Expression> _radius;
    QScopedPointer<Expression> _color;

    QSharedPointer<Expression> _filled;
public:
    DrawCircleStmt(Token pos, Expression *cx, Expression *cy, Expression *radius, Expression *color, Expression *filled);
    ~DrawCircleStmt();
    Expression *cx() { return _cx.data();}
    Expression *cy() { return _cy.data();}
    Expression *radius() { return _radius.data();}
    Expression *color() { return _color.data();}
    Expression *filled() { return _filled.data(); }
    QString toString();
    void prettyPrint(CodeFormatter *f);
    Q_DISABLE_COPY(DrawCircleStmt)
};

class DrawSpriteStmt : public GraphicsStatement
{
    Q_OBJECT
public:
    QScopedPointer<Expression> _x, _y;
    QScopedPointer<Expression> _sprite;

public:
    DrawSpriteStmt(Token pos, Expression *x, Expression *y, Expression *sprite);
    ~DrawSpriteStmt();
    Expression *x() { return _x.data();}
    Expression *y() { return _y.data();}
    Expression *sprite() { return _sprite.data();}
    QString toString();
    void prettyPrint(CodeFormatter *f);
    Q_DISABLE_COPY(DrawSpriteStmt)
};

class ZoomStmt : public GraphicsStatement
{
    Q_OBJECT
public:
    QScopedPointer<Expression> _x1, _y1, _x2, _y2;
public:
    ZoomStmt(Token pos, Expression *x1, Expression *y1, Expression *x2, Expression *y2);
    ~ZoomStmt();
    Expression *x1() { return _x1.data();}
    Expression *y1() { return _y1.data();}
    Expression *x2() { return _x2.data();}
    Expression *y2() { return _y2.data();}
    QString toString();
    void prettyPrint(CodeFormatter *f);
     Q_DISABLE_COPY(ZoomStmt)
};

enum EventType
{
    KalimatKeyDownEvent,
    KalimatKeyUpEvent,
    KalimatKeyPressEvent,
    KalimatMouseUpEvent,
    KalimatMouseDownEvent,
    KalimatMouseMoveEvent,
    KalimatSpriteCollisionEvent
};
class EventStatement : public Statement
{
public:
    EventType type;
    QScopedPointer<Identifier> _handler;
public:
    EventStatement(Token pos, EventType type, Identifier *handler);
    Identifier *handler() { return _handler.data(); }
    QString toString();
    void prettyPrint(CodeFormatter *f);
};

class ChannelCommunicationStmt : public Statement
{
    Q_OBJECT
public:
    ChannelCommunicationStmt(Token pos);
};

class SendStmt : public ChannelCommunicationStmt
{
    Q_OBJECT
public:
    bool signal;
    QScopedPointer<Expression> _value;
    QScopedPointer<Expression> _channel;
public:
    SendStmt(Token pos, bool signal, Expression *value, Expression *channel);
    ~SendStmt();
    Expression *value() { return _value.data(); }
    Expression *channel() { return _channel.data(); }
    QString toString();
    void prettyPrint(CodeFormatter *f);
     Q_DISABLE_COPY(SendStmt)
};

class ReceiveStmt : public ChannelCommunicationStmt
{
    Q_OBJECT
public:
    bool signal;
    QScopedPointer<AssignableExpression> _value;
    QScopedPointer<Expression> _channel;
public:
    ReceiveStmt(Token pos, bool signal, AssignableExpression *value, Expression *channel);
    ~ReceiveStmt();
    AssignableExpression *value() { return _value.data(); }
    Expression *channel() { return _channel.data(); }
    QString toString();
    void prettyPrint(CodeFormatter *f);
    Q_DISABLE_COPY(ReceiveStmt)
};

class SelectStmt : public Statement
{
    Q_OBJECT
public:
    QVector<QSharedPointer<ChannelCommunicationStmt> > _conditions;
    QVector<QSharedPointer<Statement> > _actions;
public:
    SelectStmt(Token pos, QVector<ChannelCommunicationStmt *> conditions, QVector<Statement *> actions);
    int count() { return _conditions.count();}
    ChannelCommunicationStmt *condition(int i) { return _conditions[i].data(); }
    Statement *action(int i) { return _actions[i].data(); }

    QString toString();
    void prettyPrint(CodeFormatter *f);
};

class BlockStmt : public Statement
{
    Q_OBJECT
public:
    QVector<QSharedPointer<Statement> > _statements;
public:
    BlockStmt(Token pos, QVector<QSharedPointer<Statement> > statements);
    int statementCount() { return _statements.count(); }
    Statement * statement(int i) { return _statements[i].data(); }
    QVector<Statement *> getStatements();
    QString toString();
    void prettyPrint(CodeFormatter *f);
};

class InvokationStmt : public Statement
{
    Q_OBJECT
public:
    QSharedPointer<Expression> _expression;
public:
    InvokationStmt(Token pos, QSharedPointer<Expression> expression);
    ~InvokationStmt();
    Expression *expression() { return _expression.data(); }
    QString toString();
    void prettyPrint(CodeFormatter *f);
    Q_DISABLE_COPY(InvokationStmt)
};

#endif // STATEMENT_H
