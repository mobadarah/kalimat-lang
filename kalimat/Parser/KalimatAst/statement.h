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
    shared_ptr<AssignableExpression> variable() {return _variable;}
    shared_ptr<Expression> value() {return _value;}
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
    shared_ptr<Expression> condition() {return _condition;}
    shared_ptr<Statement> thenPart() { return _thenPart;}
    shared_ptr<Statement> elsePart() {return _elsePart;}
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
    shared_ptr<Expression> condition() {return _condition;}
    shared_ptr<Statement> statement() { return _statement;}
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
    shared_ptr<Identifier> variable() { return _variable; }
    shared_ptr<Expression> from() { return _from; }
    shared_ptr<Expression> to() { return _to; }
    shared_ptr<Statement> statement() { return _statement;}
    QString toString();
    void prettyPrint(CodeFormatter *f);

    virtual QVector<shared_ptr<Identifier> > getIntroducedVariables();

};

class ReturnStmt : public Statement
{
    Q_OBJECT
public:
    shared_ptr<Expression> _returnVal;
public:
    ReturnStmt(Token pos, shared_ptr<Expression> returnVal);
    ~ReturnStmt();
    shared_ptr<Expression> returnVal() { return _returnVal; }
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
    shared_ptr<IInvokation> invokation() { return _invokation; }
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
    shared_ptr<IInvokation> invokation() { return _invokation; }
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
    shared_ptr<Expression> target() { return _target; }
    QString toString();
    void prettyPrint(CodeFormatter *f);
};

class GotoStmt : public Statement
{
    Q_OBJECT
public:
    shared_ptr<Expression> _target;
public:
    GotoStmt(Token pos, shared_ptr<Expression> target);
    shared_ptr<Expression> target() { return _target; }
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
    shared_ptr<Expression> arg(int i) { return _args[i]; }
    shared_ptr<Expression> fileObject() { return _fileObject;}
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
    shared_ptr<AssignableExpression> variable(int i) { return _variables[i];}
    shared_ptr<Expression> fileObject() { return _fileObject;}
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
    shared_ptr<Expression> x() { return _x;}
    shared_ptr<Expression> y() { return _y;}
    shared_ptr<Expression> color() { return _color;}
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
    shared_ptr<Expression> x1() { return _x1;}
    shared_ptr<Expression> y1() { return _y1;}
    shared_ptr<Expression> x2() { return _x2;}
    shared_ptr<Expression> y2() { return _y2;}
    shared_ptr<Expression> color() { return _color;}
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
    shared_ptr<Expression> x1() { return _x1;}
    shared_ptr<Expression> y1() { return _y1;}
    shared_ptr<Expression> x2() { return _x2;}
    shared_ptr<Expression> y2() { return _y2;}
    shared_ptr<Expression> color() { return _color;}
    shared_ptr<Expression> filled() { return _filled; }
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
    shared_ptr<Expression> cx() { return _cx;}
    shared_ptr<Expression> cy() { return _cy;}
    shared_ptr<Expression> radius() { return _radius;}
    shared_ptr<Expression> color() { return _color;}
    shared_ptr<Expression> filled() { return _filled; }
    QString toString();
    void prettyPrint(CodeFormatter *f);
    Q_DISABLE_COPY(DrawCircleStmt)
};

class DrawImageStmt : public GraphicsStatement
{
    Q_OBJECT
public:
    shared_ptr<Expression> _x, _y;
    shared_ptr<Expression> _image;

public:
    DrawImageStmt(Token pos,
                   shared_ptr<Expression> x,
                   shared_ptr<Expression> y,
                   shared_ptr<Expression> image);
    ~DrawImageStmt();
    shared_ptr<Expression> x() { return _x;}
    shared_ptr<Expression> y() { return _y;}
    shared_ptr<Expression> image() { return _image;}
    QString toString();
    void prettyPrint(CodeFormatter *f);
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
    shared_ptr<Expression> x() { return _x;}
    shared_ptr<Expression> y() { return _y;}
    shared_ptr<Expression> sprite() { return _sprite;}
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
    shared_ptr<Expression> x1() { return _x1;}
    shared_ptr<Expression> y1() { return _y1;}
    shared_ptr<Expression> x2() { return _x2;}
    shared_ptr<Expression> y2() { return _y2;}
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
    shared_ptr<Identifier> handler() { return _handler; }
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
    shared_ptr<Expression> value() { return _value; }
    shared_ptr<Expression> channel() { return _channel; }
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
    shared_ptr<AssignableExpression> value() { return _value; }
    shared_ptr<Expression> channel() { return _channel; }
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
    shared_ptr<ChannelCommunicationStmt> condition(int i) { return _conditions[i]; }
    shared_ptr<Statement> action(int i) { return _actions[i]; }

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
    shared_ptr<Statement> statement(int i) { return _statements[i]; }
    QVector<shared_ptr<Statement> > getStatements();
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
    shared_ptr<Expression> expression() { return _expression; }
    QString toString();
    void prettyPrint(CodeFormatter *f);
    Q_DISABLE_COPY(InvokationStmt)
};

#endif // STATEMENT_H
