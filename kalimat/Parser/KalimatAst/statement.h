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
    shared_ptr <AssignableExpression> _variable;
    shared_ptr<Expression> _value;
public:
    AssignmentStmt(Token pos, shared_ptr<AssignableExpression> variable, shared_ptr<Expression> value);
    ~AssignmentStmt();
    AssignableExpression *variable() {return _variable.get();}
    Expression *value() {return _value.get();}
    QString toString();
    void prettyPrint(CodeFormatter *f);
};

class IfStmt : public Statement
{
    Q_OBJECT
public:
    shared_ptr<Expression> _condition;
    shared_ptr<Statement> _thenPart;
    shared_ptr<Statement> _elsePart;
public:
    IfStmt(Token pos,
           shared_ptr<Expression> _condition,
           shared_ptr<Statement> _thenPart,
           shared_ptr<Statement> _elsePart);
    ~IfStmt();
    Expression *condition() {return _condition.get();}
    Statement *thenPart() { return _thenPart.get();}
    Statement *elsePart() {return _elsePart.get();}
    QString toString();
    void prettyPrint(CodeFormatter *f);
};

class WhileStmt : public Statement
{
    Q_OBJECT
public:
    shared_ptr<Expression> _condition;
    shared_ptr<Statement> _statement;

public:
    WhileStmt(Token pos,
              shared_ptr<Expression> _condition,
              shared_ptr<Statement> _statement);
    ~WhileStmt();
    Expression *condition() {return _condition.get();}
    Statement *statement() { return _statement.get();}
    QString toString();
    void prettyPrint(CodeFormatter *f);
};

class ForAllStmt : public Statement
{
    Q_OBJECT
public:
    shared_ptr<Identifier> _variable;
    shared_ptr<Expression> _from;
    shared_ptr<Expression> _to;
    shared_ptr<Statement> _statement;

public:
    ForAllStmt(Token pos,
               shared_ptr<Identifier> variable,
               shared_ptr<Expression> from,
               shared_ptr<Expression> to,
               shared_ptr<Statement>  statement);
    ~ForAllStmt();
    Identifier *variable() { return _variable.get(); }
    Expression *from() { return _from.get(); }
    Expression *to() { return _to.get(); }
    Statement *statement() { return _statement.get();}
    QString toString();
    void prettyPrint(CodeFormatter *f);

    virtual QVector<Identifier *> getIntroducedVariables();

};

class ReturnStmt : public Statement
{
    Q_OBJECT
public:
    shared_ptr<Expression> _returnVal;
public:
    ReturnStmt(Token pos, shared_ptr<Expression> returnVal);
    ~ReturnStmt();
    Expression *returnVal() { return _returnVal.get(); }
    QString toString();
    void prettyPrint(CodeFormatter *f);
};

class DelegationStmt : public Statement
{
    Q_OBJECT
public:
    shared_ptr<IInvokation> _invokation;
public:
    DelegationStmt(Token pos, shared_ptr<IInvokation> invokation);
    ~DelegationStmt();
    IInvokation *invokation() { return _invokation.get(); }
    QString toString();
    void prettyPrint(CodeFormatter *f);
    Q_DISABLE_COPY(DelegationStmt)
};

class LaunchStmt : public Statement
{
    Q_OBJECT
public:
    shared_ptr<IInvokation> _invokation;
public:
    LaunchStmt(Token pos, shared_ptr<IInvokation> invokation);
    ~LaunchStmt();
    IInvokation *invokation() { return _invokation.get(); }
    QString toString();
    void prettyPrint(CodeFormatter *f);
    Q_DISABLE_COPY(LaunchStmt)
};

class LabelStmt : public Statement
{
    Q_OBJECT
    shared_ptr<Expression> _target;
public:
    LabelStmt(Token pos, shared_ptr<Expression> target);
    Expression *target() { return _target.get(); }
    QString toString();
    void prettyPrint(CodeFormatter *f);
};


class GotoStmt : public Statement
{
    Q_OBJECT
public:
    bool targetIsNumber;
    shared_ptr<NumLiteral> _numericTarget;
    shared_ptr<Identifier> _idTarget;
public:
    GotoStmt(Token pos, bool _targetIsNumber, shared_ptr<Expression> target);
    Identifier *idTarget() { return _idTarget.get(); }
    NumLiteral *numericTarget() { return _numericTarget.get(); }
    QString toString();
    void prettyPrint(CodeFormatter *f);
};

class PrintStmt : public IOStatement
{
    Q_OBJECT
public:
    shared_ptr<Expression> _fileObject;
    QVector<shared_ptr<Expression > > _args;
    QVector<shared_ptr<Expression> > _widths;
    bool printOnSameLine;
public:
    PrintStmt(Token pos,
              shared_ptr<Expression> fileObject,
              QVector<shared_ptr<Expression> > args,
              QVector<shared_ptr<Expression> > widths,
              bool printOnSameLine);
    int argCount() { return _args.count(); }
    Expression *arg(int i) { return _args[i].get(); }
    Expression *fileObject() { return _fileObject.get();}
    QString toString();
    void prettyPrint(CodeFormatter *f);
};

class ReadStmt : public IOStatement
{
    Q_OBJECT
public:
    shared_ptr<Expression> _fileObject;
    QString prompt;
    QVector<shared_ptr<AssignableExpression> > _variables;
    QVector<bool> readNumberFlags;
public:
    ReadStmt(Token pos,
             shared_ptr<Expression> fileObject,
             QString prompt,
             const QVector<shared_ptr<AssignableExpression> > &variables,
             QVector<bool> readNumberFlags);
    int variableCount() { return _variables.count();}
    AssignableExpression *variable(int i) { return _variables[i].get();}
    Expression *fileObject() { return _fileObject.get();}
    QString toString();
    void prettyPrint(CodeFormatter *f);
};

class DrawPixelStmt : public GraphicsStatement
{
    Q_OBJECT
public:
    shared_ptr<Expression> _x, _y;
    shared_ptr<Expression> _color;
public:
    DrawPixelStmt(Token pos,
                  shared_ptr<Expression> x,
                  shared_ptr<Expression> y,
                  shared_ptr<Expression> color);
    ~DrawPixelStmt();
    Expression *x() { return _x.get();}
    Expression *y() { return _y.get();}
    Expression *color() { return _color.get();}
    QString toString();
    void prettyPrint(CodeFormatter *f);
    Q_DISABLE_COPY(DrawPixelStmt)
};

class DrawLineStmt : public GraphicsStatement
{
    Q_OBJECT
public:
    shared_ptr<Expression> _x1, _y1, _x2, _y2;
    shared_ptr<Expression> _color;
public:
    DrawLineStmt(Token pos,
                 shared_ptr<Expression> x1,
                 shared_ptr<Expression> y1,
                 shared_ptr<Expression> x2,
                 shared_ptr<Expression> y2,
                 shared_ptr<Expression> color);
    ~DrawLineStmt();
    Expression *x1() { return _x1.get();}
    Expression *y1() { return _y1.get();}
    Expression *x2() { return _x2.get();}
    Expression *y2() { return _y2.get();}
    Expression *color() { return _color.get();}
    QString toString();
    void prettyPrint(CodeFormatter *f);
    Q_DISABLE_COPY(DrawLineStmt)
};

class DrawRectStmt : public GraphicsStatement
{
    Q_OBJECT
public:
    shared_ptr<Expression> _x1, _y1, _x2, _y2;
    shared_ptr<Expression> _color;
    shared_ptr<Expression> _filled;
public:
    DrawRectStmt(Token pos,
                 shared_ptr<Expression> x1,
                 shared_ptr<Expression> y1,
                 shared_ptr<Expression> x2,
                 shared_ptr<Expression> y2,
                 shared_ptr<Expression> color,
                 shared_ptr<Expression> filled);
    ~DrawRectStmt();
    Expression *x1() { return _x1.get();}
    Expression *y1() { return _y1.get();}
    Expression *x2() { return _x2.get();}
    Expression *y2() { return _y2.get();}
    Expression *color() { return _color.get();}
    Expression *filled() { return _filled.get(); }
    QString toString();
    void prettyPrint(CodeFormatter *f);
    Q_DISABLE_COPY(DrawRectStmt)
};

class DrawCircleStmt : public GraphicsStatement
{
    Q_OBJECT
public:
    shared_ptr<Expression> _cx, _cy;
    shared_ptr<Expression> _radius;
    shared_ptr<Expression> _color;

    shared_ptr<Expression> _filled;
public:
    DrawCircleStmt(Token pos,
                   shared_ptr<Expression> cx,
                   shared_ptr<Expression> cy,
                   shared_ptr<Expression> radius,
                   shared_ptr<Expression> color,
                   shared_ptr<Expression> filled);
    ~DrawCircleStmt();
    Expression *cx() { return _cx.get();}
    Expression *cy() { return _cy.get();}
    Expression *radius() { return _radius.get();}
    Expression *color() { return _color.get();}
    Expression *filled() { return _filled.get(); }
    QString toString();
    void prettyPrint(CodeFormatter *f);
    Q_DISABLE_COPY(DrawCircleStmt)
};

class DrawSpriteStmt : public GraphicsStatement
{
    Q_OBJECT
public:
    shared_ptr<Expression> _x, _y;
    shared_ptr<Expression> _sprite;

public:
    DrawSpriteStmt(Token pos,
                   shared_ptr<Expression> x,
                   shared_ptr<Expression> y,
                   shared_ptr<Expression> sprite);
    ~DrawSpriteStmt();
    Expression *x() { return _x.get();}
    Expression *y() { return _y.get();}
    Expression *sprite() { return _sprite.get();}
    QString toString();
    void prettyPrint(CodeFormatter *f);
    Q_DISABLE_COPY(DrawSpriteStmt)
};

class ZoomStmt : public GraphicsStatement
{
    Q_OBJECT
public:
    shared_ptr<Expression> _x1, _y1, _x2, _y2;
public:
    ZoomStmt(Token pos,
             shared_ptr<Expression> x1,
             shared_ptr<Expression> y1,
             shared_ptr<Expression> x2,
             shared_ptr<Expression> y2);
    ~ZoomStmt();
    Expression *x1() { return _x1.get();}
    Expression *y1() { return _y1.get();}
    Expression *x2() { return _x2.get();}
    Expression *y2() { return _y2.get();}
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
    Q_OBJECT
public:
    EventType type;
    shared_ptr<Identifier> _handler;
public:
    EventStatement(Token pos, EventType type, shared_ptr<Identifier> handler);
    Identifier *handler() { return _handler.get(); }
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
    shared_ptr<Expression> _value;
    shared_ptr<Expression> _channel;
public:
    SendStmt(Token pos,
             bool signal,
             shared_ptr<Expression> value,
             shared_ptr<Expression> channel);
    ~SendStmt();
    Expression *value() { return _value.get(); }
    Expression *channel() { return _channel.get(); }
    QString toString();
    void prettyPrint(CodeFormatter *f);
     Q_DISABLE_COPY(SendStmt)
};

class ReceiveStmt : public ChannelCommunicationStmt
{
    Q_OBJECT
public:
    bool signal;
    shared_ptr<AssignableExpression> _value;
    shared_ptr<Expression> _channel;
public:
    ReceiveStmt(Token pos,
                bool signal,
                shared_ptr<AssignableExpression> value,
                shared_ptr<Expression> channel);
    ~ReceiveStmt();
    AssignableExpression *value() { return _value.get(); }
    Expression *channel() { return _channel.get(); }
    QString toString();
    void prettyPrint(CodeFormatter *f);
    Q_DISABLE_COPY(ReceiveStmt)
};

class SelectStmt : public Statement
{
    Q_OBJECT
public:
    QVector<shared_ptr<ChannelCommunicationStmt> > _conditions;
    QVector<shared_ptr<Statement> > _actions;
public:
    SelectStmt(Token pos,
               QVector<shared_ptr<ChannelCommunicationStmt> > conditions,
               QVector<shared_ptr<Statement> > actions);
    int count() { return _conditions.count();}
    ChannelCommunicationStmt *condition(int i) { return _conditions[i].get(); }
    Statement *action(int i) { return _actions[i].get(); }

    QString toString();
    void prettyPrint(CodeFormatter *f);
};

class BlockStmt : public Statement
{
    Q_OBJECT
public:
    QVector<shared_ptr<Statement> > _statements;
public:
    BlockStmt(Token pos, QVector<shared_ptr<Statement> > statements);
    int statementCount() { return _statements.count(); }
    Statement * statement(int i) { return _statements[i].get(); }
    QVector<Statement *> getStatements();
    QString toString();
    void prettyPrint(CodeFormatter *f);
};

class InvokationStmt : public Statement
{
    Q_OBJECT
public:
    shared_ptr<Expression> _expression;
public:
    InvokationStmt(Token pos, shared_ptr<Expression> expression);
    ~InvokationStmt();
    Expression *expression() { return _expression.get(); }
    QString toString();
    void prettyPrint(CodeFormatter *f);
    Q_DISABLE_COPY(InvokationStmt)
};

#endif // STATEMENT_H
