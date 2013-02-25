#ifndef KALIMAT_AST_GEN_H
#define KALIMAT_AST_GEN_H

#include "kalimatast.h"
class IInvokation;
class Declaration;
class AST;
class FormalParam;
class StrLiteral;
class IScopeIntroducer;
class TypeExpression;
class Pattern;
class Identifier;
class PrettyPrintable;
class MethodDecl;
class KalimatAst  :   public AST
{
public:
    Token _pos;
    Token _endingpos;
public:

    KalimatAst(Token _pos,
               Token _endingpos);
    Token pos() { return _pos; }
    Token endingpos() { return _endingpos; }
    virtual QString toString()=0;
    virtual void prettyPrint(CodeFormatter *f)=0;

    virtual void traverse(shared_ptr<PrettyPrintable> p, Traverser *tv)=0;

    virtual void traverseChildren(Traverser *tv);

    virtual QString childrenToString();
    Token getPos() { return _pos; }
    Token getEndingPos() { return _endingpos; }
    void setEndingPos(Token t) { _endingpos= t; }
};


class TopLevel  :   public KalimatAst
{
public:
public:

    TopLevel(Token _pos,
             Token _endingpos);
    virtual QString toString()=0;
    virtual void prettyPrint(CodeFormatter *f)=0;

    virtual void traverse(shared_ptr<PrettyPrintable> p, Traverser *tv)=0;

    virtual void traverseChildren(Traverser *tv);

    virtual QString childrenToString();
    QString attachedComments;
};


class Statement  :   public TopLevel
{
public:
public:

    Statement(Token _pos,
              Token _endingpos);
    virtual QString toString()=0;
    virtual void prettyPrint(CodeFormatter *f)=0;

    virtual void traverse(shared_ptr<PrettyPrintable> p, Traverser *tv)=0;

    virtual void traverseChildren(Traverser *tv);

    virtual QString childrenToString();
};


class Expression  :   public KalimatAst
{
public:
public:

    Expression(Token _pos,
               Token _endingpos);
    virtual QString toString()=0;
    virtual void prettyPrint(CodeFormatter *f)=0;

    virtual void traverse(shared_ptr<PrettyPrintable> p, Traverser *tv)=0;

    virtual void traverseChildren(Traverser *tv);

    virtual QString childrenToString();
};


class IOStatement  :   public Statement
{
public:
public:

    IOStatement(Token _pos,
                Token _endingpos);
    virtual QString toString()=0;
    virtual void prettyPrint(CodeFormatter *f)=0;

    virtual void traverse(shared_ptr<PrettyPrintable> p, Traverser *tv)=0;

    virtual void traverseChildren(Traverser *tv);

    virtual QString childrenToString();
};


class GraphicsStatement  :   public Statement
{
public:
public:

    GraphicsStatement(Token _pos,
                      Token _endingpos);
    virtual QString toString()=0;
    virtual void prettyPrint(CodeFormatter *f)=0;

    virtual void traverse(shared_ptr<PrettyPrintable> p, Traverser *tv)=0;

    virtual void traverseChildren(Traverser *tv);

    virtual QString childrenToString();
};


class ChannelCommunicationStmt  :   public Statement
{
public:
public:

    ChannelCommunicationStmt(Token _pos,
                             Token _endingpos);
    virtual QString toString()=0;
    virtual void prettyPrint(CodeFormatter *f)=0;

    virtual void traverse(shared_ptr<PrettyPrintable> p, Traverser *tv)=0;

    virtual void traverseChildren(Traverser *tv);

    virtual QString childrenToString();
};


class CompilationUnit  :   public KalimatAst
{
public:
    QVector<shared_ptr<StrLiteral> > usedModules;
public:

    CompilationUnit(Token _pos,
                    Token _endingpos,
                    QVector<shared_ptr<StrLiteral> > usedModules);
    int usedModuleCount() { return usedModules.count(); }
    shared_ptr<StrLiteral> usedModule(int index) { return usedModules[index]; }
    virtual QString toString()=0;
    virtual void prettyPrint(CodeFormatter *f)=0;

    virtual void traverse(shared_ptr<PrettyPrintable> p, Traverser *tv)=0;

    virtual void traverseChildren(Traverser *tv);

    virtual QString childrenToString();
};


class Program  :   public CompilationUnit
{
public:
    QVector<shared_ptr<TopLevel> > elements;
    // Original layout without collecting top-level statements
    // into a 'main' function. Used for pretty-printing...etc
    QVector<shared_ptr<TopLevel> > originalElements;
public:

    Program(Token _pos,
            Token _endingpos,
            QVector<shared_ptr<StrLiteral> > usedModules,
            QVector<shared_ptr<TopLevel> > elements,
            QVector<shared_ptr<TopLevel> > originalElements);
    int elementCount() { return elements.count(); }
    shared_ptr<TopLevel> element(int index) { return elements[index]; }
    int originalElementCount() { return originalElements.count(); }
    shared_ptr<TopLevel> originalElement(int index) { return originalElements[index]; }
    void addOriginalElement(shared_ptr<TopLevel> arg) { originalElements.append(arg);}
    virtual QString toString();
    virtual void prettyPrint(CodeFormatter *f);

    virtual void traverse(shared_ptr<PrettyPrintable> p, Traverser *tv);

    virtual void traverseChildren(Traverser *tv);

    virtual QString childrenToString();
};


class Module  :   public CompilationUnit
{
public:
    shared_ptr<Identifier> _name;
    QVector<shared_ptr<Declaration> > declarations;
public:

    Module(Token _pos,
           Token _endingpos,
           QVector<shared_ptr<StrLiteral> > usedModules,
           shared_ptr<Identifier> _name,
           QVector<shared_ptr<Declaration> > declarations);
    shared_ptr<Identifier> name() { return _name; }
    int declarationCount() { return declarations.count(); }
    shared_ptr<Declaration> declaration(int index) { return declarations[index]; }
    void addDeclaration(shared_ptr<Declaration> arg) { declarations.append(arg);}
    virtual QString toString();
    virtual void prettyPrint(CodeFormatter *f);

    virtual void traverse(shared_ptr<PrettyPrintable> p, Traverser *tv);

    virtual void traverseChildren(Traverser *tv);

    virtual QString childrenToString();
};


class AssignableExpression  :   public Expression
{
public:
public:

    AssignableExpression(Token _pos,
                         Token _endingpos);
    virtual QString toString()=0;
    virtual void prettyPrint(CodeFormatter *f)=0;

    virtual void traverse(shared_ptr<PrettyPrintable> p, Traverser *tv)=0;

    virtual void traverseChildren(Traverser *tv);

    virtual QString childrenToString();
};


class Literal  :   public Expression
{
public:
public:

    Literal(Token _pos,
            Token _endingpos);
    virtual QString toString()=0;
    virtual void prettyPrint(CodeFormatter *f)=0;

    virtual void traverse(shared_ptr<PrettyPrintable> p, Traverser *tv)=0;

    virtual void traverseChildren(Traverser *tv);

    virtual QString childrenToString();
};


class SimpleLiteral  :   public Literal
{
public:
public:

    SimpleLiteral(Token _pos,
                  Token _endingpos);
    virtual QString toString()=0;
    virtual void prettyPrint(CodeFormatter *f)=0;

    virtual void traverse(shared_ptr<PrettyPrintable> p, Traverser *tv)=0;

    virtual void traverseChildren(Traverser *tv);

    virtual QString childrenToString();
};


class AssignmentStmt  :   public Statement
{
public:
    shared_ptr<AssignableExpression> _variable;
    shared_ptr<Expression> _value;
    shared_ptr<TypeExpression> _type;
public:

    AssignmentStmt(Token _pos,
                   Token _endingpos,
                   shared_ptr<AssignableExpression> _variable,
                   shared_ptr<Expression> _value,
                   shared_ptr<TypeExpression> _type);
    shared_ptr<AssignableExpression> variable() { return _variable; }
    shared_ptr<Expression> value() { return _value; }
    shared_ptr<TypeExpression> type() { return _type; }
    virtual QString toString();
    virtual void prettyPrint(CodeFormatter *f);

    virtual void traverse(shared_ptr<PrettyPrintable> p, Traverser *tv);

    virtual void traverseChildren(Traverser *tv);

    virtual QString childrenToString();
};


class IfStmt  :   public Statement
{
public:
    shared_ptr<Expression> _condition;
    shared_ptr<Statement> _thenPart;
    shared_ptr<Statement> _elsePart;
public:

    IfStmt(Token _pos,
           Token _endingpos,
           shared_ptr<Expression> _condition,
           shared_ptr<Statement> _thenPart,
           shared_ptr<Statement> _elsePart);
    shared_ptr<Expression> condition() { return _condition; }
    shared_ptr<Statement> thenPart() { return _thenPart; }
    shared_ptr<Statement> elsePart() { return _elsePart; }
    virtual QString toString();
    virtual void prettyPrint(CodeFormatter *f);

    virtual void traverse(shared_ptr<PrettyPrintable> p, Traverser *tv);

    virtual void traverseChildren(Traverser *tv);

    virtual QString childrenToString();
};


class WhileStmt  :   public Statement
{
public:
    shared_ptr<Expression> _condition;
    shared_ptr<Statement> _statement;
public:

    WhileStmt(Token _pos,
              Token _endingpos,
              shared_ptr<Expression> _condition,
              shared_ptr<Statement> _statement);
    shared_ptr<Expression> condition() { return _condition; }
    shared_ptr<Statement> statement() { return _statement; }
    virtual QString toString();
    virtual void prettyPrint(CodeFormatter *f);

    virtual void traverse(shared_ptr<PrettyPrintable> p, Traverser *tv);

    virtual void traverseChildren(Traverser *tv);

    virtual QString childrenToString();
};


class ForAllStmt  :   public Statement
{
public:
    shared_ptr<Identifier> _variable;
    shared_ptr<Expression> _from;
    shared_ptr<Expression> _to;
    shared_ptr<Expression> _step;
    shared_ptr<Statement> _statement;
    bool _downTo;
public:

    ForAllStmt(Token _pos,
               Token _endingpos,
               shared_ptr<Identifier> _variable,
               shared_ptr<Expression> _from,
               shared_ptr<Expression> _to,
               shared_ptr<Expression> _step,
               shared_ptr<Statement> _statement,
               bool _downTo);
    shared_ptr<Identifier> variable() { return _variable; }
    shared_ptr<Expression> from() { return _from; }
    shared_ptr<Expression> to() { return _to; }
    shared_ptr<Expression> step() { return _step; }
    shared_ptr<Statement> statement() { return _statement; }
    bool downTo() { return _downTo; }
    virtual QString toString();
    virtual void prettyPrint(CodeFormatter *f);

    virtual void traverse(shared_ptr<PrettyPrintable> p, Traverser *tv);

    virtual void traverseChildren(Traverser *tv);

    virtual QString childrenToString();
    virtual QVector<shared_ptr<Identifier> > getIntroducedVariables();};


class ForEachStmt  :   public Statement
{
public:
    shared_ptr<Identifier> _variable;
    shared_ptr<Expression> _enumerable;
    shared_ptr<Statement> _statement;
public:

    ForEachStmt(Token _pos,
                Token _endingpos,
                shared_ptr<Identifier> _variable,
                shared_ptr<Expression> _enumerable,
                shared_ptr<Statement> _statement);
    shared_ptr<Identifier> variable() { return _variable; }
    shared_ptr<Expression> enumerable() { return _enumerable; }
    shared_ptr<Statement> statement() { return _statement; }
    virtual QString toString();
    virtual void prettyPrint(CodeFormatter *f);

    virtual void traverse(shared_ptr<PrettyPrintable> p, Traverser *tv);

    virtual void traverseChildren(Traverser *tv);

    virtual QString childrenToString();

    virtual QVector<shared_ptr<Identifier> > getIntroducedVariables();
};


class ReturnStmt  :   public Statement
{
public:
    shared_ptr<Expression> _returnVal;
public:

    ReturnStmt(Token _pos,
               Token _endingpos,
               shared_ptr<Expression> _returnVal);
    shared_ptr<Expression> returnVal() { return _returnVal; }
    virtual QString toString();
    virtual void prettyPrint(CodeFormatter *f);

    virtual void traverse(shared_ptr<PrettyPrintable> p, Traverser *tv);

    virtual void traverseChildren(Traverser *tv);

    virtual QString childrenToString();
};


class DelegationStmt  :   public Statement
{
public:
    shared_ptr<IInvokation> _invokation;
public:

    DelegationStmt(Token _pos,
                   Token _endingpos,
                   shared_ptr<IInvokation> _invokation);
    shared_ptr<IInvokation> invokation() { return _invokation; }
    virtual QString toString();
    virtual void prettyPrint(CodeFormatter *f);

    virtual void traverse(shared_ptr<PrettyPrintable> p, Traverser *tv);

    virtual void traverseChildren(Traverser *tv);

    virtual QString childrenToString();
};


class LaunchStmt  :   public Statement
{
public:
    shared_ptr<IInvokation> _invokation;
public:

    LaunchStmt(Token _pos,
               Token _endingpos,
               shared_ptr<IInvokation> _invokation);
    shared_ptr<IInvokation> invokation() { return _invokation; }
    virtual QString toString();
    virtual void prettyPrint(CodeFormatter *f);

    virtual void traverse(shared_ptr<PrettyPrintable> p, Traverser *tv);

    virtual void traverseChildren(Traverser *tv);

    virtual QString childrenToString();
};


class LabelStmt  :   public Statement
{
public:
    shared_ptr<Expression> _target;
public:

    LabelStmt(Token _pos,
              Token _endingpos,
              shared_ptr<Expression> _target);
    shared_ptr<Expression> target() { return _target; }
    virtual QString toString();
    virtual void prettyPrint(CodeFormatter *f);

    virtual void traverse(shared_ptr<PrettyPrintable> p, Traverser *tv);

    virtual void traverseChildren(Traverser *tv);

    virtual QString childrenToString();
};


class GotoStmt  :   public Statement
{
public:
    shared_ptr<Expression> _target;
public:

    GotoStmt(Token _pos,
             Token _endingpos,
             shared_ptr<Expression> _target);
    shared_ptr<Expression> target() { return _target; }
    virtual QString toString();
    virtual void prettyPrint(CodeFormatter *f);

    virtual void traverse(shared_ptr<PrettyPrintable> p, Traverser *tv);

    virtual void traverseChildren(Traverser *tv);

    virtual QString childrenToString();
};


class PrintStmt  :   public IOStatement
{
public:
    shared_ptr<Expression> _fileObject;
    QVector<shared_ptr<Expression> > args;
    QVector<shared_ptr<Expression> > widths;
    bool _printOnSameLine;
public:

    PrintStmt(Token _pos,
              Token _endingpos,
              shared_ptr<Expression> _fileObject,
              QVector<shared_ptr<Expression> > args,
              QVector<shared_ptr<Expression> > widths,
              bool _printOnSameLine);
    shared_ptr<Expression> fileObject() { return _fileObject; }
    int argCount() { return args.count(); }
    shared_ptr<Expression> arg(int index) { return args[index]; }
    int widthCount() { return widths.count(); }
    shared_ptr<Expression> width(int index) { return widths[index]; }
    bool printOnSameLine() { return _printOnSameLine; }
    virtual QString toString();
    virtual void prettyPrint(CodeFormatter *f);

    virtual void traverse(shared_ptr<PrettyPrintable> p, Traverser *tv);

    virtual void traverseChildren(Traverser *tv);

    virtual QString childrenToString();
};


class ReadStmt  :   public IOStatement
{
public:
    shared_ptr<Expression> _fileObject;
    QString _prompt;
    QVector<shared_ptr<AssignableExpression> > variables;
    QVector<bool > readNumberFlags;
public:

    ReadStmt(Token _pos,
             Token _endingpos,
             shared_ptr<Expression> _fileObject,
             QString _prompt,
             QVector<shared_ptr<AssignableExpression> > variables,
             QVector<bool > readNumberFlags);
    shared_ptr<Expression> fileObject() { return _fileObject; }
    QString prompt() { return _prompt; }
    int variableCount() { return variables.count(); }
    shared_ptr<AssignableExpression> variable(int index) { return variables[index]; }
    int readNumberFlagCount() { return readNumberFlags.count(); }
    bool readNumberFlag(int index) { return readNumberFlags[index]; }
    virtual QString toString();
    virtual void prettyPrint(CodeFormatter *f);

    virtual void traverse(shared_ptr<PrettyPrintable> p, Traverser *tv);

    virtual void traverseChildren(Traverser *tv);

    virtual QString childrenToString();
};


class DrawPixelStmt  :   public GraphicsStatement
{
public:
    shared_ptr<Expression> _x;
    shared_ptr<Expression> _y;
    shared_ptr<Expression> _color;
public:

    DrawPixelStmt(Token _pos,
                  Token _endingpos,
                  shared_ptr<Expression> _x,
                  shared_ptr<Expression> _y,
                  shared_ptr<Expression> _color);
    shared_ptr<Expression> x() { return _x; }
    shared_ptr<Expression> y() { return _y; }
    shared_ptr<Expression> color() { return _color; }
    virtual QString toString();
    virtual void prettyPrint(CodeFormatter *f);

    virtual void traverse(shared_ptr<PrettyPrintable> p, Traverser *tv);

    virtual void traverseChildren(Traverser *tv);

    virtual QString childrenToString();
};


class DrawLineStmt  :   public GraphicsStatement
{
public:
    shared_ptr<Expression> _x1;
    shared_ptr<Expression> _y1;
    shared_ptr<Expression> _x2;
    shared_ptr<Expression> _y2;
    shared_ptr<Expression> _color;
public:

    DrawLineStmt(Token _pos,
                 Token _endingpos,
                 shared_ptr<Expression> _x1,
                 shared_ptr<Expression> _y1,
                 shared_ptr<Expression> _x2,
                 shared_ptr<Expression> _y2,
                 shared_ptr<Expression> _color);
    shared_ptr<Expression> x1() { return _x1; }
    shared_ptr<Expression> y1() { return _y1; }
    shared_ptr<Expression> x2() { return _x2; }
    shared_ptr<Expression> y2() { return _y2; }
    shared_ptr<Expression> color() { return _color; }
    virtual QString toString();
    virtual void prettyPrint(CodeFormatter *f);

    virtual void traverse(shared_ptr<PrettyPrintable> p, Traverser *tv);

    virtual void traverseChildren(Traverser *tv);

    virtual QString childrenToString();
};


class DrawRectStmt  :   public GraphicsStatement
{
public:
    shared_ptr<Expression> _x1;
    shared_ptr<Expression> _y1;
    shared_ptr<Expression> _x2;
    shared_ptr<Expression> _y2;
    shared_ptr<Expression> _color;
    shared_ptr<Expression> _filled;
public:

    DrawRectStmt(Token _pos,
                 Token _endingpos,
                 shared_ptr<Expression> _x1,
                 shared_ptr<Expression> _y1,
                 shared_ptr<Expression> _x2,
                 shared_ptr<Expression> _y2,
                 shared_ptr<Expression> _color,
                 shared_ptr<Expression> _filled);
    shared_ptr<Expression> x1() { return _x1; }
    shared_ptr<Expression> y1() { return _y1; }
    shared_ptr<Expression> x2() { return _x2; }
    shared_ptr<Expression> y2() { return _y2; }
    shared_ptr<Expression> color() { return _color; }
    shared_ptr<Expression> filled() { return _filled; }
    virtual QString toString();
    virtual void prettyPrint(CodeFormatter *f);

    virtual void traverse(shared_ptr<PrettyPrintable> p, Traverser *tv);

    virtual void traverseChildren(Traverser *tv);

    virtual QString childrenToString();
};


class DrawCircleStmt  :   public GraphicsStatement
{
public:
    shared_ptr<Expression> _cx;
    shared_ptr<Expression> _cy;
    shared_ptr<Expression> _radius;
    shared_ptr<Expression> _color;
    shared_ptr<Expression> _filled;
public:

    DrawCircleStmt(Token _pos,
                   Token _endingpos,
                   shared_ptr<Expression> _cx,
                   shared_ptr<Expression> _cy,
                   shared_ptr<Expression> _radius,
                   shared_ptr<Expression> _color,
                   shared_ptr<Expression> _filled);
    shared_ptr<Expression> cx() { return _cx; }
    shared_ptr<Expression> cy() { return _cy; }
    shared_ptr<Expression> radius() { return _radius; }
    shared_ptr<Expression> color() { return _color; }
    shared_ptr<Expression> filled() { return _filled; }
    virtual QString toString();
    virtual void prettyPrint(CodeFormatter *f);

    virtual void traverse(shared_ptr<PrettyPrintable> p, Traverser *tv);

    virtual void traverseChildren(Traverser *tv);

    virtual QString childrenToString();
};


class DrawImageStmt  :   public GraphicsStatement
{
public:
    shared_ptr<Expression> _x;
    shared_ptr<Expression> _y;
    shared_ptr<Expression> _image;
public:

    DrawImageStmt(Token _pos,
                  Token _endingpos,
                  shared_ptr<Expression> _x,
                  shared_ptr<Expression> _y,
                  shared_ptr<Expression> _image);
    shared_ptr<Expression> x() { return _x; }
    shared_ptr<Expression> y() { return _y; }
    shared_ptr<Expression> image() { return _image; }
    virtual QString toString();
    virtual void prettyPrint(CodeFormatter *f);

    virtual void traverse(shared_ptr<PrettyPrintable> p, Traverser *tv);

    virtual void traverseChildren(Traverser *tv);

    virtual QString childrenToString();
};


class DrawSpriteStmt  :   public GraphicsStatement
{
public:
    shared_ptr<Expression> _x;
    shared_ptr<Expression> _y;
    shared_ptr<Expression> _sprite;
public:

    DrawSpriteStmt(Token _pos,
                   Token _endingpos,
                   shared_ptr<Expression> _x,
                   shared_ptr<Expression> _y,
                   shared_ptr<Expression> _sprite);
    shared_ptr<Expression> x() { return _x; }
    shared_ptr<Expression> y() { return _y; }
    shared_ptr<Expression> sprite() { return _sprite; }
    virtual QString toString();
    virtual void prettyPrint(CodeFormatter *f);

    virtual void traverse(shared_ptr<PrettyPrintable> p, Traverser *tv);

    virtual void traverseChildren(Traverser *tv);

    virtual QString childrenToString();
};


class ZoomStmt  :   public GraphicsStatement
{
public:
    shared_ptr<Expression> _x1;
    shared_ptr<Expression> _y1;
    shared_ptr<Expression> _x2;
    shared_ptr<Expression> _y2;
public:

    ZoomStmt(Token _pos,
             Token _endingpos,
             shared_ptr<Expression> _x1,
             shared_ptr<Expression> _y1,
             shared_ptr<Expression> _x2,
             shared_ptr<Expression> _y2);
    shared_ptr<Expression> x1() { return _x1; }
    shared_ptr<Expression> y1() { return _y1; }
    shared_ptr<Expression> x2() { return _x2; }
    shared_ptr<Expression> y2() { return _y2; }
    virtual QString toString();
    virtual void prettyPrint(CodeFormatter *f);

    virtual void traverse(shared_ptr<PrettyPrintable> p, Traverser *tv);

    virtual void traverseChildren(Traverser *tv);

    virtual QString childrenToString();
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


class EventStatement  :   public Statement
{
public:
    EventType _type;
    shared_ptr<Identifier> _handler;
public:

    EventStatement(Token _pos,
                   Token _endingpos,
                   EventType _type,
                   shared_ptr<Identifier> _handler);
    EventType type() { return _type; }
    shared_ptr<Identifier> handler() { return _handler; }
    virtual QString toString();
    virtual void prettyPrint(CodeFormatter *f);

    virtual void traverse(shared_ptr<PrettyPrintable> p, Traverser *tv);

    virtual void traverseChildren(Traverser *tv);

    virtual QString childrenToString();
};


class SendStmt  :   public ChannelCommunicationStmt
{
public:
    shared_ptr<Expression> _value;
    shared_ptr<Expression> _channel;
    bool _signal;
public:

    SendStmt(Token _pos,
             Token _endingpos,
             shared_ptr<Expression> _value,
             shared_ptr<Expression> _channel,
             bool _signal);
    shared_ptr<Expression> value() { return _value; }
    shared_ptr<Expression> channel() { return _channel; }
    bool signal() { return _signal; }
    virtual QString toString();
    virtual void prettyPrint(CodeFormatter *f);

    virtual void traverse(shared_ptr<PrettyPrintable> p, Traverser *tv);

    virtual void traverseChildren(Traverser *tv);

    virtual QString childrenToString();
};


class ReceiveStmt  :   public ChannelCommunicationStmt
{
public:
    shared_ptr<AssignableExpression> _value;
    shared_ptr<Expression> _channel;
    bool _signal;
public:

    ReceiveStmt(Token _pos,
                Token _endingpos,
                shared_ptr<AssignableExpression> _value,
                shared_ptr<Expression> _channel,
                bool _signal);
    shared_ptr<AssignableExpression> value() { return _value; }
    shared_ptr<Expression> channel() { return _channel; }
    bool signal() { return _signal; }
    virtual QString toString();
    virtual void prettyPrint(CodeFormatter *f);

    virtual void traverse(shared_ptr<PrettyPrintable> p, Traverser *tv);

    virtual void traverseChildren(Traverser *tv);

    virtual QString childrenToString();
};


class SelectStmt  :   public Statement
{
public:
    QVector<shared_ptr<ChannelCommunicationStmt> > conditions;
    QVector<shared_ptr<Statement> > actions;
public:

    SelectStmt(Token _pos,
               Token _endingpos,
               QVector<shared_ptr<ChannelCommunicationStmt> > conditions,
               QVector<shared_ptr<Statement> > actions);
    int conditionCount() { return conditions.count(); }
    shared_ptr<ChannelCommunicationStmt> condition(int index) { return conditions[index]; }
    int actionCount() { return actions.count(); }
    shared_ptr<Statement> action(int index) { return actions[index]; }
    virtual QString toString();
    virtual void prettyPrint(CodeFormatter *f);

    virtual void traverse(shared_ptr<PrettyPrintable> p, Traverser *tv);

    virtual void traverseChildren(Traverser *tv);

    virtual QString childrenToString();
};


class BlockStmt  :   public Statement
{
public:
    QVector<shared_ptr<Statement> > _statements;
public:

    BlockStmt(Token _pos,
              Token _endingpos,
              QVector<shared_ptr<Statement> > _statements);
    int statementCount() { return _statements.count(); }
    shared_ptr<Statement> statement(int index) { return _statements[index]; }
    QVector<shared_ptr<Statement> > statements() { return _statements; }
    virtual QString toString();
    virtual void prettyPrint(CodeFormatter *f);

    virtual void traverse(shared_ptr<PrettyPrintable> p, Traverser *tv);

    virtual void traverseChildren(Traverser *tv);

    virtual QString childrenToString();
};


class InvokationStmt  :   public Statement
{
public:
    shared_ptr<Expression> _expression;
public:

    InvokationStmt(Token _pos,
                   Token _endingpos,
                   shared_ptr<Expression> _expression);
    shared_ptr<Expression> expression() { return _expression; }
    virtual QString toString();
    virtual void prettyPrint(CodeFormatter *f);

    virtual void traverse(shared_ptr<PrettyPrintable> p, Traverser *tv);

    virtual void traverseChildren(Traverser *tv);

    virtual QString childrenToString();
};


class BinaryOperation  :   public Expression
{
public:
    QString _operator_;
    shared_ptr<Expression> _operand1;
    shared_ptr<Expression> _operand2;
public:

    BinaryOperation(Token _pos,
                    Token _endingpos,
                    QString _operator_,
                    shared_ptr<Expression> _operand1,
                    shared_ptr<Expression> _operand2);
    QString operator_() { return _operator_; }
    shared_ptr<Expression> operand1() { return _operand1; }
    shared_ptr<Expression> operand2() { return _operand2; }
    virtual QString toString();
    virtual void prettyPrint(CodeFormatter *f);

    virtual void traverse(shared_ptr<PrettyPrintable> p, Traverser *tv);

    virtual void traverseChildren(Traverser *tv);

    virtual QString childrenToString();
};


class UnaryOperation  :   public Expression
{
public:
    QString _operator_;
    shared_ptr<Expression> _operand;
public:

    UnaryOperation(Token _pos,
                   Token _endingpos,
                   QString _operator_,
                   shared_ptr<Expression> _operand);
    QString operator_() { return _operator_; }
    shared_ptr<Expression> operand() { return _operand; }
    virtual QString toString();
    virtual void prettyPrint(CodeFormatter *f);

    virtual void traverse(shared_ptr<PrettyPrintable> p, Traverser *tv);

    virtual void traverseChildren(Traverser *tv);

    virtual QString childrenToString();
};


class IsaOperation  :   public Expression
{
public:
    shared_ptr<Expression> _expression;
    shared_ptr<Identifier> _type;
public:

    IsaOperation(Token _pos,
                 Token _endingpos,
                 shared_ptr<Expression> _expression,
                 shared_ptr<Identifier> _type);
    shared_ptr<Expression> expression() { return _expression; }
    shared_ptr<Identifier> type() { return _type; }
    virtual QString toString();
    virtual void prettyPrint(CodeFormatter *f);

    virtual void traverse(shared_ptr<PrettyPrintable> p, Traverser *tv);

    virtual void traverseChildren(Traverser *tv);

    virtual QString childrenToString();
};


class MatchOperation  :   public Expression
{
public:
    shared_ptr<Expression> _expression;
    shared_ptr<Pattern> _pattern;
public:

    MatchOperation(Token _pos,
                   Token _endingpos,
                   shared_ptr<Expression> _expression,
                   shared_ptr<Pattern> _pattern);
    shared_ptr<Expression> expression() { return _expression; }
    shared_ptr<Pattern> pattern() { return _pattern; }
    virtual QString toString();
    virtual void prettyPrint(CodeFormatter *f);

    virtual void traverse(shared_ptr<PrettyPrintable> p, Traverser *tv);

    virtual void traverseChildren(Traverser *tv);

    virtual QString childrenToString();
};


class Identifier  :   public KalimatAst
{
public:
    QString _name;
public:

    Identifier(Token _pos,
               Token _endingpos,
               QString _name);
    QString name() { return _name; }
    virtual QString toString();
    virtual void prettyPrint(CodeFormatter *f);

    virtual void traverse(shared_ptr<PrettyPrintable> p, Traverser *tv);

    virtual void traverseChildren(Traverser *tv);

    virtual QString childrenToString();
    Identifier(Token _pos,
               QString _name);
};


class VarAccess  :   public AssignableExpression
{
public:
    shared_ptr<Identifier> _name;
public:

    VarAccess(Token _pos,
              Token _endingpos,
              shared_ptr<Identifier> _name);
    shared_ptr<Identifier> name() { return _name; }
    virtual QString toString();
    virtual void prettyPrint(CodeFormatter *f);

    virtual void traverse(shared_ptr<PrettyPrintable> p, Traverser *tv);

    virtual void traverseChildren(Traverser *tv);

    virtual QString childrenToString();
    VarAccess(shared_ptr<Identifier> _name);
};


class NumLiteral  :   public SimpleLiteral
{
public:
    long _lValue;
    double _dValue;
    bool _longNotDouble;
    bool _valueRecognized;
public:

    long lValue() { return _lValue; }
    double dValue() { return _dValue; }
    bool longNotDouble() { return _longNotDouble; }
    bool valueRecognized() { return _valueRecognized; }
    virtual QString toString();
    virtual void prettyPrint(CodeFormatter *f);

    virtual void traverse(shared_ptr<PrettyPrintable> p, Traverser *tv);

    virtual void traverseChildren(Traverser *tv);

    virtual QString childrenToString();
    NumLiteral(Token pos, QString lexeme);
    NumLiteral(Token pos, int value);
    QString repr();};


class StrLiteral  :   public SimpleLiteral
{
public:
    QString _value;
public:

    StrLiteral(Token _pos,
               Token _endingpos,
               QString _value);
    QString value() { return _value; }
    virtual QString toString();
    virtual void prettyPrint(CodeFormatter *f);

    virtual void traverse(shared_ptr<PrettyPrintable> p, Traverser *tv);

    virtual void traverseChildren(Traverser *tv);

    virtual QString childrenToString();
    QString repr();
    StrLiteral(Token _pos,
               QString _value);
};


class NullLiteral  :   public SimpleLiteral
{
public:
public:

    NullLiteral(Token _pos,
                Token _endingpos);
    virtual QString toString();
    virtual void prettyPrint(CodeFormatter *f);

    virtual void traverse(shared_ptr<PrettyPrintable> p, Traverser *tv);

    virtual void traverseChildren(Traverser *tv);

    virtual QString childrenToString();
    QString repr();
    NullLiteral(Token _pos);};


class BoolLiteral  :   public SimpleLiteral
{
public:
    bool _value;
public:

    BoolLiteral(Token _pos,
                Token _endingpos,
                bool _value);
    bool value() { return _value; }
    virtual QString toString();
    virtual void prettyPrint(CodeFormatter *f);

    virtual void traverse(shared_ptr<PrettyPrintable> p, Traverser *tv);

    virtual void traverseChildren(Traverser *tv);

    virtual QString childrenToString();
    QString repr();
    BoolLiteral(Token _pos,
                bool _value);
};


class ArrayLiteral  :   public Literal
{
public:
    QVector<shared_ptr<Expression> > _data;
public:

    ArrayLiteral(Token _pos,
                 Token _endingpos,
                 QVector<shared_ptr<Expression> > _data);
    int dataCount() { return _data.count(); }
    shared_ptr<Expression> data(int index) { return _data[index]; }
    QVector<shared_ptr<Expression> > getData() { return _data; }
    virtual QString toString();
    virtual void prettyPrint(CodeFormatter *f);

    virtual void traverse(shared_ptr<PrettyPrintable> p, Traverser *tv);

    virtual void traverseChildren(Traverser *tv);

    virtual QString childrenToString();
};


class MapLiteral  :   public Literal
{
public:
    QVector<shared_ptr<Expression> > _data;
public:

    MapLiteral(Token _pos,
               Token _endingpos,
               QVector<shared_ptr<Expression> > _data);
    int dataCount() { return _data.count(); }
    shared_ptr<Expression> data(int index) { return _data[index]; }
    QVector<shared_ptr<Expression> > getData() { return _data; }
    virtual QString toString();
    virtual void prettyPrint(CodeFormatter *f);

    virtual void traverse(shared_ptr<PrettyPrintable> p, Traverser *tv);

    virtual void traverseChildren(Traverser *tv);

    virtual QString childrenToString();
};


class IInvokation  :   public Expression
{
public:
public:

    IInvokation(Token _pos,
                Token _endingpos);
    virtual QString toString()=0;
    virtual void prettyPrint(CodeFormatter *f)=0;

    virtual void traverse(shared_ptr<PrettyPrintable> p, Traverser *tv)=0;

    virtual void traverseChildren(Traverser *tv);

    virtual QString childrenToString();
};


class Invokation  :   public IInvokation
{
public:
    shared_ptr<Identifier> _functor;
    QVector<shared_ptr<Expression> > arguments;
public:

    Invokation(Token _pos,
               Token _endingpos,
               shared_ptr<Identifier> _functor,
               QVector<shared_ptr<Expression> > arguments);
    shared_ptr<Identifier> functor() { return _functor; }
    int argumentCount() { return arguments.count(); }
    shared_ptr<Expression> argument(int index) { return arguments[index]; }
    virtual QString toString();
    virtual void prettyPrint(CodeFormatter *f);

    virtual void traverse(shared_ptr<PrettyPrintable> p, Traverser *tv);

    virtual void traverseChildren(Traverser *tv);

    virtual QString childrenToString();
};


class MethodInvokation  :   public IInvokation
{
public:
    shared_ptr<Expression> _receiver;
    shared_ptr<Identifier> _methodSelector;
    QVector<shared_ptr<Expression> > arguments;
public:

    MethodInvokation(Token _pos,
                     Token _endingpos,
                     shared_ptr<Expression> _receiver,
                     shared_ptr<Identifier> _methodSelector,
                     QVector<shared_ptr<Expression> > arguments);
    shared_ptr<Expression> receiver() { return _receiver; }
    shared_ptr<Identifier> methodSelector() { return _methodSelector; }
    int argumentCount() { return arguments.count(); }
    shared_ptr<Expression> argument(int index) { return arguments[index]; }
    virtual QString toString();
    virtual void prettyPrint(CodeFormatter *f);

    virtual void traverse(shared_ptr<PrettyPrintable> p, Traverser *tv);

    virtual void traverseChildren(Traverser *tv);

    virtual QString childrenToString();
};


class ForAutocomplete  :   public IInvokation
{
public:
    shared_ptr<Expression> _toBeCompleted;
public:

    ForAutocomplete(Token _pos,
                    Token _endingpos,
                    shared_ptr<Expression> _toBeCompleted);
    shared_ptr<Expression> toBeCompleted() { return _toBeCompleted; }
    virtual QString toString();
    virtual void prettyPrint(CodeFormatter *f);

    virtual void traverse(shared_ptr<PrettyPrintable> p, Traverser *tv);

    virtual void traverseChildren(Traverser *tv);

    virtual QString childrenToString();
};


class TimingExpression  :   public Expression
{
public:
    shared_ptr<Expression> _toTime;
public:

    TimingExpression(Token _pos,
                     Token _endingpos,
                     shared_ptr<Expression> _toTime);
    shared_ptr<Expression> toTime() { return _toTime; }
    virtual QString toString();
    virtual void prettyPrint(CodeFormatter *f);

    virtual void traverse(shared_ptr<PrettyPrintable> p, Traverser *tv);

    virtual void traverseChildren(Traverser *tv);

    virtual QString childrenToString();
};


enum DeclarationType
{
    Proc,
    Function
};


// TheProc, TheFunction...etc
class TheSomething  :   public Expression
{
public:
    QString _name;
    DeclarationType _what;
public:

    TheSomething(Token _pos,
                 Token _endingpos,
                 QString _name,
                 DeclarationType _what);
    QString name() { return _name; }
    DeclarationType what() { return _what; }
    virtual void prettyPrint(CodeFormatter *f);

    virtual void traverse(shared_ptr<PrettyPrintable> p, Traverser *tv);

    virtual void traverseChildren(Traverser *tv);

    virtual QString childrenToString();
    virtual QString toString();};


class Idafa  :   public AssignableExpression
{
public:
    shared_ptr<Identifier> _modaf;
    shared_ptr<Expression> _modaf_elaih;
public:

    Idafa(Token _pos,
          Token _endingpos,
          shared_ptr<Identifier> _modaf,
          shared_ptr<Expression> _modaf_elaih);
    shared_ptr<Identifier> modaf() { return _modaf; }
    shared_ptr<Expression> modaf_elaih() { return _modaf_elaih; }
    virtual QString toString();
    virtual void prettyPrint(CodeFormatter *f);

    virtual void traverse(shared_ptr<PrettyPrintable> p, Traverser *tv);

    virtual void traverseChildren(Traverser *tv);

    virtual QString childrenToString();
};


class ArrayIndex  :   public AssignableExpression
{
public:
    shared_ptr<Expression> _array;
    shared_ptr<Expression> _index;
public:

    ArrayIndex(Token _pos,
               Token _endingpos,
               shared_ptr<Expression> _array,
               shared_ptr<Expression> _index);
    shared_ptr<Expression> array() { return _array; }
    shared_ptr<Expression> index() { return _index; }
    virtual QString toString();
    virtual void prettyPrint(CodeFormatter *f);

    virtual void traverse(shared_ptr<PrettyPrintable> p, Traverser *tv);

    virtual void traverseChildren(Traverser *tv);

    virtual QString childrenToString();
};


class MultiDimensionalArrayIndex  :   public AssignableExpression
{
public:
    shared_ptr<Expression> _array;
    QVector<shared_ptr<Expression> > _indexes;
public:

    MultiDimensionalArrayIndex(Token _pos,
                               Token _endingpos,
                               shared_ptr<Expression> _array,
                               QVector<shared_ptr<Expression> > _indexes);
    shared_ptr<Expression> array() { return _array; }
    int indexCount() { return _indexes.count(); }
    shared_ptr<Expression> index(int index) { return _indexes[index]; }
    QVector<shared_ptr<Expression> > indexes() { return _indexes; }
    virtual QString toString();
    virtual void prettyPrint(CodeFormatter *f);

    virtual void traverse(shared_ptr<PrettyPrintable> p, Traverser *tv);

    virtual void traverseChildren(Traverser *tv);

    virtual QString childrenToString();
};


class ObjectCreation  :   public Expression
{
public:
    shared_ptr<Identifier> _className;
    QVector<shared_ptr<Identifier> > fieldInitNames;
    QVector<shared_ptr<Expression> > fieldInitValues;
public:

    ObjectCreation(Token _pos,
                   Token _endingpos,
                   shared_ptr<Identifier> _className,
                   QVector<shared_ptr<Identifier> > fieldInitNames,
                   QVector<shared_ptr<Expression> > fieldInitValues);
    shared_ptr<Identifier> className() { return _className; }
    int fieldInitNameCount() { return fieldInitNames.count(); }
    shared_ptr<Identifier> fieldInitName(int index) { return fieldInitNames[index]; }
    int fieldInitValueCount() { return fieldInitValues.count(); }
    shared_ptr<Expression> fieldInitValue(int index) { return fieldInitValues[index]; }
    virtual QString toString();
    virtual void prettyPrint(CodeFormatter *f);

    virtual void traverse(shared_ptr<PrettyPrintable> p, Traverser *tv);

    virtual void traverseChildren(Traverser *tv);

    virtual QString childrenToString();
};


class LambdaExpression  :   public Expression
{
public:
    QVector<shared_ptr<FormalParam> > _argList;
    QVector<shared_ptr<Statement> > statements;
    // todo: The hasDoToken flag is a kludge, we should split lambda expressions
    // into one class that encapsulate a simple expression and one (or two) for
    // longer procs and functions.
    bool _hasDoToken;
    QSet<QString > freeVariables;
public:

    LambdaExpression(Token _pos,
                     Token _endingpos,
                     QVector<shared_ptr<FormalParam> > _argList,
                     QVector<shared_ptr<Statement> > statements,
                     bool _hasDoToken);
    int argListCount() { return _argList.count(); }
    shared_ptr<FormalParam> argList(int index) { return _argList[index]; }
    int statementCount() { return statements.count(); }
    shared_ptr<Statement> statement(int index) { return statements[index]; }
    bool hasDoToken() { return _hasDoToken; }
    int freeVariableCount() { return freeVariables.count(); }
    void insertFreeVariable(QString a) { freeVariables.insert(a);}
    bool containsFreeVariable(QString a) { return freeVariables.contains(a);}
    virtual QString toString();
    virtual void prettyPrint(CodeFormatter *f);

    virtual void traverse(shared_ptr<PrettyPrintable> p, Traverser *tv);

    virtual void traverseChildren(Traverser *tv);

    virtual QString childrenToString();
};


class Pattern  :   public KalimatAst
{
public:
public:

    Pattern(Token _pos,
            Token _endingpos);
    virtual QString toString()=0;
    virtual void prettyPrint(CodeFormatter *f)=0;

    virtual void traverse(shared_ptr<PrettyPrintable> p, Traverser *tv)=0;

    virtual void traverseChildren(Traverser *tv);

    virtual QString childrenToString();
};


class SimpleLiteralPattern  :   public Pattern
{
public:
    shared_ptr<SimpleLiteral> _value;
public:

    SimpleLiteralPattern(Token _pos,
                         Token _endingpos,
                         shared_ptr<SimpleLiteral> _value);
    shared_ptr<SimpleLiteral> value() { return _value; }
    virtual QString toString();
    virtual void prettyPrint(CodeFormatter *f);

    virtual void traverse(shared_ptr<PrettyPrintable> p, Traverser *tv);

    virtual void traverseChildren(Traverser *tv);

    virtual QString childrenToString();
};


class VarPattern  :   public Pattern
{
public:
    shared_ptr<VarAccess> _id;
public:

    VarPattern(Token _pos,
               Token _endingpos,
               shared_ptr<VarAccess> _id);
    shared_ptr<VarAccess> id() { return _id; }
    virtual QString toString();
    virtual void prettyPrint(CodeFormatter *f);

    virtual void traverse(shared_ptr<PrettyPrintable> p, Traverser *tv);

    virtual void traverseChildren(Traverser *tv);

    virtual QString childrenToString();
};


class AssignedVarPattern  :   public Pattern
{
public:
    shared_ptr<AssignableExpression> _lv;
public:

    AssignedVarPattern(Token _pos,
                       Token _endingpos,
                       shared_ptr<AssignableExpression> _lv);
    shared_ptr<AssignableExpression> lv() { return _lv; }
    virtual QString toString();
    virtual void prettyPrint(CodeFormatter *f);

    virtual void traverse(shared_ptr<PrettyPrintable> p, Traverser *tv);

    virtual void traverseChildren(Traverser *tv);

    virtual QString childrenToString();
};


class ArrayPattern  :   public Pattern
{
public:
    QVector<shared_ptr<Pattern> > elements;
    bool _fixedLength;
public:

    ArrayPattern(Token _pos,
                 Token _endingpos,
                 QVector<shared_ptr<Pattern> > elements,
                 bool _fixedLength);
    int elementCount() { return elements.count(); }
    shared_ptr<Pattern> element(int index) { return elements[index]; }
    bool fixedLength() { return _fixedLength; }
    virtual QString toString();
    virtual void prettyPrint(CodeFormatter *f);

    virtual void traverse(shared_ptr<PrettyPrintable> p, Traverser *tv);

    virtual void traverseChildren(Traverser *tv);

    virtual QString childrenToString();
};


class ObjPattern  :   public Pattern
{
public:
    shared_ptr<Identifier> _classId;
    QVector<shared_ptr<Identifier> > fieldNames;
    QVector<shared_ptr<Pattern> > fieldPatterns;
public:

    ObjPattern(Token _pos,
               Token _endingpos,
               shared_ptr<Identifier> _classId,
               QVector<shared_ptr<Identifier> > fieldNames,
               QVector<shared_ptr<Pattern> > fieldPatterns);
    shared_ptr<Identifier> classId() { return _classId; }
    int fieldNameCount() { return fieldNames.count(); }
    shared_ptr<Identifier> fieldName(int index) { return fieldNames[index]; }
    int fieldPatternCount() { return fieldPatterns.count(); }
    shared_ptr<Pattern> fieldPattern(int index) { return fieldPatterns[index]; }
    virtual QString toString();
    virtual void prettyPrint(CodeFormatter *f);

    virtual void traverse(shared_ptr<PrettyPrintable> p, Traverser *tv);

    virtual void traverseChildren(Traverser *tv);

    virtual QString childrenToString();
};


class MapPattern  :   public Pattern
{
public:
    QVector<shared_ptr<Expression> > keys;
    QVector<shared_ptr<Pattern> > values;
public:

    MapPattern(Token _pos,
               Token _endingpos,
               QVector<shared_ptr<Expression> > keys,
               QVector<shared_ptr<Pattern> > values);
    int keyCount() { return keys.count(); }
    shared_ptr<Expression> key(int index) { return keys[index]; }
    int valueCount() { return values.count(); }
    shared_ptr<Pattern> value(int index) { return values[index]; }
    virtual QString toString();
    virtual void prettyPrint(CodeFormatter *f);

    virtual void traverse(shared_ptr<PrettyPrintable> p, Traverser *tv);

    virtual void traverseChildren(Traverser *tv);

    virtual QString childrenToString();
};


class TypeExpression  :   public KalimatAst
{
public:
public:

    TypeExpression(Token _pos,
                   Token _endingpos);
    virtual QString toString()=0;
    virtual void prettyPrint(CodeFormatter *f)=0;

    virtual void traverse(shared_ptr<PrettyPrintable> p, Traverser *tv)=0;

    virtual void traverseChildren(Traverser *tv);

    virtual QString childrenToString();
};


class TypeIdentifier  :   public TypeExpression
{
public:
    QString _name;
public:

    TypeIdentifier(Token _pos,
                   Token _endingpos,
                   QString _name);
    QString name() { return _name; }
    virtual QString toString();
    virtual void prettyPrint(CodeFormatter *f);

    virtual void traverse(shared_ptr<PrettyPrintable> p, Traverser *tv);

    virtual void traverseChildren(Traverser *tv);

    virtual QString childrenToString();

    TypeIdentifier(Token _pos,
                   QString _name);
};


class PointerTypeExpression  :   public TypeExpression
{
public:
    shared_ptr<TypeExpression> _pointeeType;
public:

    PointerTypeExpression(Token _pos,
                          Token _endingpos,
                          shared_ptr<TypeExpression> _pointeeType);
    shared_ptr<TypeExpression> pointeeType() { return _pointeeType; }
    virtual QString toString();
    virtual void prettyPrint(CodeFormatter *f);

    virtual void traverse(shared_ptr<PrettyPrintable> p, Traverser *tv);

    virtual void traverseChildren(Traverser *tv);

    virtual QString childrenToString();
};


class FunctionTypeExpression  :   public TypeExpression
{
public:
    shared_ptr<TypeExpression> _retType;
    QVector<shared_ptr<TypeExpression> > argTypes;
public:

    FunctionTypeExpression(Token _pos,
                           Token _endingpos,
                           shared_ptr<TypeExpression> _retType,
                           QVector<shared_ptr<TypeExpression> > argTypes);
    shared_ptr<TypeExpression> retType() { return _retType; }
    int argTypeCount() { return argTypes.count(); }
    shared_ptr<TypeExpression> argType(int index) { return argTypes[index]; }
    virtual QString toString();
    virtual void prettyPrint(CodeFormatter *f);

    virtual void traverse(shared_ptr<PrettyPrintable> p, Traverser *tv);

    virtual void traverseChildren(Traverser *tv);

    virtual QString childrenToString();
};


class Declaration  :   public TopLevel
{
public:
    bool _isPublic;
public:

    Declaration(Token _pos,
                Token _endingpos,
                bool _isPublic);
    bool isPublic() { return _isPublic; }
    virtual QString toString()=0;
    virtual void prettyPrint(CodeFormatter *f)=0;

    virtual void traverse(shared_ptr<PrettyPrintable> p, Traverser *tv)=0;

    virtual void traverseChildren(Traverser *tv);

    virtual QString childrenToString();
};


class FormalParam  :   public PrettyPrintable
{
public:
    shared_ptr<Identifier> _name;
    shared_ptr<TypeExpression> _type;
public:

    FormalParam(shared_ptr<Identifier> _name,
                shared_ptr<TypeExpression> _type);
    FormalParam() {}
    shared_ptr<Identifier> name() { return _name; }
    shared_ptr<TypeExpression> type() { return _type; }
    virtual QString toString();
    virtual void prettyPrint(CodeFormatter *f);

    virtual void traverse(shared_ptr<PrettyPrintable> p, Traverser *tv);

    virtual void traverseChildren(Traverser *tv);

    virtual QString childrenToString();
    FormalParam(shared_ptr<Identifier> name)
        :_name(name)
    {

    }
};


class ProceduralDecl  :   public Declaration,  public IScopeIntroducer
{
public:
    shared_ptr<Identifier> _procName;
    QVector<shared_ptr<FormalParam> > formals;
    shared_ptr<BlockStmt> _body;
public:

    ProceduralDecl(Token _pos,
                   Token _endingpos,
                   bool _isPublic,
                   shared_ptr<Identifier> _procName,
                   QVector<shared_ptr<FormalParam> > formals,
                   shared_ptr<BlockStmt> _body);
    shared_ptr<Identifier> procName() { return _procName; }
    int formalCount() { return formals.count(); }
    shared_ptr<FormalParam> formal(int index) { return formals[index]; }
    shared_ptr<BlockStmt> body() { return _body; }
    void setBody(shared_ptr<BlockStmt> val) { _body = val; }
    virtual QString toString()=0;
    virtual void prettyPrint(CodeFormatter *f)=0;

    virtual void traverse(shared_ptr<PrettyPrintable> p, Traverser *tv)=0;

    virtual void traverseChildren(Traverser *tv);

    virtual QString childrenToString();
    virtual QString getTooltip();
    virtual QVector<shared_ptr<Identifier> > getIntroducedVariables();};


class IProcedure 
{
public:
public:

    IProcedure();
};


class IFunction 
{
public:
public:

    IFunction();
};


class ProcedureDecl  :   public ProceduralDecl,  public IProcedure
{
public:
public:

    ProcedureDecl(Token _pos,
                  Token _endingpos,
                  bool _isPublic,
                  shared_ptr<Identifier> _procName,
                  QVector<shared_ptr<FormalParam> > formals,
                  shared_ptr<BlockStmt> _body);
    virtual QString toString();
    virtual void prettyPrint(CodeFormatter *f);

    virtual void traverse(shared_ptr<PrettyPrintable> p, Traverser *tv);

    virtual void traverseChildren(Traverser *tv);

    virtual QString childrenToString();
};


class FunctionDecl  :   public ProceduralDecl,  public IFunction
{
public:
public:

    FunctionDecl(Token _pos,
                 Token _endingpos,
                 bool _isPublic,
                 shared_ptr<Identifier> _procName,
                 QVector<shared_ptr<FormalParam> > formals,
                 shared_ptr<BlockStmt> _body);
    virtual QString toString();
    virtual void prettyPrint(CodeFormatter *f);

    virtual void traverse(shared_ptr<PrettyPrintable> p, Traverser *tv);

    virtual void traverseChildren(Traverser *tv);

    virtual QString childrenToString();
};


class FFILibraryDecl  :   public Declaration
{
public:
    QString _libName;
    QVector<shared_ptr<Declaration> > decls;
public:

    FFILibraryDecl(Token _pos,
                   Token _endingpos,
                   bool _isPublic,
                   QString _libName,
                   QVector<shared_ptr<Declaration> > decls);
    QString libName() { return _libName; }
    int declCount() { return decls.count(); }
    shared_ptr<Declaration> decl(int index) { return decls[index]; }
    virtual QString toString();
    virtual void prettyPrint(CodeFormatter *f);

    virtual void traverse(shared_ptr<PrettyPrintable> p, Traverser *tv);

    virtual void traverseChildren(Traverser *tv);

    virtual QString childrenToString();
};


class FFIProceduralDecl  :   public Declaration
{
public:
    bool _isFunctionNotProc;
    QString _procName;
    QString _symbol;
    shared_ptr<TypeExpression> _returnType;
    QVector<shared_ptr<TypeExpression> > paramTypes;
public:

    FFIProceduralDecl(Token _pos,
                      Token _endingpos,
                      bool _isPublic,
                      bool _isFunctionNotProc,
                      QString _procName,
                      QString _symbol,
                      shared_ptr<TypeExpression> _returnType,
                      QVector<shared_ptr<TypeExpression> > paramTypes);
    bool isFunctionNotProc() { return _isFunctionNotProc; }
    QString procName() { return _procName; }
    QString symbol() { return _symbol; }
    shared_ptr<TypeExpression> returnType() { return _returnType; }
    int paramTypeCount() { return paramTypes.count(); }
    shared_ptr<TypeExpression> paramType(int index) { return paramTypes[index]; }
    virtual QString toString();
    virtual void prettyPrint(CodeFormatter *f);

    virtual void traverse(shared_ptr<PrettyPrintable> p, Traverser *tv);

    virtual void traverseChildren(Traverser *tv);

    virtual QString childrenToString();
};


class FFIStructDecl  :   public Declaration
{
public:
    shared_ptr<Identifier> _name;
    QVector<shared_ptr<Identifier> > fieldNames;
    QVector<shared_ptr<TypeExpression> > fieldTypes;
    QVector<int > fieldBatches;
public:

    FFIStructDecl(Token _pos,
                  Token _endingpos,
                  bool _isPublic,
                  shared_ptr<Identifier> _name,
                  QVector<shared_ptr<Identifier> > fieldNames,
                  QVector<shared_ptr<TypeExpression> > fieldTypes,
                  QVector<int > fieldBatches);
    shared_ptr<Identifier> name() { return _name; }
    int fieldNameCount() { return fieldNames.count(); }
    shared_ptr<Identifier> fieldName(int index) { return fieldNames[index]; }
    int fieldTypeCount() { return fieldTypes.count(); }
    shared_ptr<TypeExpression> fieldType(int index) { return fieldTypes[index]; }
    int fieldBatcheCount() { return fieldBatches.count(); }
    int fieldBatche(int index) { return fieldBatches[index]; }
    virtual QString toString();
    virtual void prettyPrint(CodeFormatter *f);

    virtual void traverse(shared_ptr<PrettyPrintable> p, Traverser *tv);

    virtual void traverseChildren(Traverser *tv);

    virtual QString childrenToString();
};


class ClassInternalDecl  :   public PrettyPrintable
{
public:
public:

    ClassInternalDecl();
    virtual QString toString()=0;
    virtual void prettyPrint(CodeFormatter *f)=0;

    virtual void traverse(shared_ptr<PrettyPrintable> p, Traverser *tv)=0;

    virtual void traverseChildren(Traverser *tv);

    virtual QString childrenToString();
};


class MethodInfo 
{
public:
    int _arity;
    bool _isFunction;
public:

    MethodInfo(int _arity,
               bool _isFunction);
    int arity() { return _arity; }
    bool isFunction() { return _isFunction; }
    MethodInfo();};


class ConcreteResponseInfo  :   public PrettyPrintable
{
public:
    shared_ptr<Identifier> _name;
    QVector<shared_ptr<FormalParam> > params;
public:

    ConcreteResponseInfo(shared_ptr<Identifier> _name);
    shared_ptr<Identifier> name() { return _name; }
    int paramCount() { return params.count(); }
    shared_ptr<FormalParam> param(int index) { return params[index]; }
    void addParam(shared_ptr<FormalParam> arg) { params.append(arg);}
    virtual QString toString();
    virtual void prettyPrint(CodeFormatter *f);

    virtual void traverse(shared_ptr<PrettyPrintable> p, Traverser *tv);

    virtual void traverseChildren(Traverser *tv);

    virtual QString childrenToString();
};


class Has  :   public ClassInternalDecl
{
public:
    QMap<QString, shared_ptr<TypeExpression> > _fieldMarshallAs;
    QVector<shared_ptr<Identifier> > fields;
public:

    Has();
    int fieldMarshallAsCount() { return _fieldMarshallAs.count(); }
    shared_ptr<TypeExpression> fieldMarshallAs(QString index) { return _fieldMarshallAs[index]; }
    bool containsFieldMarshallAs(QString a) { return _fieldMarshallAs.contains(a);}
    int fieldCount() { return fields.count(); }
    shared_ptr<Identifier> field(int index) { return fields[index]; }
    void addField(shared_ptr<Identifier> arg) { fields.append(arg);}
    virtual QString toString();
    virtual void prettyPrint(CodeFormatter *f);

    virtual void traverse(shared_ptr<PrettyPrintable> p, Traverser *tv);

    virtual void traverseChildren(Traverser *tv);

    virtual QString childrenToString();
};


class RespondsTo  :   public ClassInternalDecl
{
public:
    bool _isFunctions;
    QVector<shared_ptr<ConcreteResponseInfo> > methods;
public:

    RespondsTo(bool _isFunctions);
    bool isFunctions() { return _isFunctions; }
    int methodCount() { return methods.count(); }
    shared_ptr<ConcreteResponseInfo> method(int index) { return methods[index]; }
    void addMethod(shared_ptr<ConcreteResponseInfo> arg) { methods.append(arg);}
    virtual QString toString();
    virtual void prettyPrint(CodeFormatter *f);

    virtual void traverse(shared_ptr<PrettyPrintable> p, Traverser *tv);

    virtual void traverseChildren(Traverser *tv);

    virtual QString childrenToString();
};


class ClassDecl  :   public Declaration
{
public:
    shared_ptr<Identifier> _ancestorName;
    shared_ptr<Identifier> _name;
    QVector<shared_ptr<Identifier> > _fields;
    QMap<QString, MethodInfo > _methodPrototypes;
    QVector<shared_ptr<ClassInternalDecl> > _internalDecls;
    QMap<QString, shared_ptr<TypeExpression> > _fieldMarshallAs;
    QMap<QString, shared_ptr<MethodDecl> > _methods;
    shared_ptr<ClassDecl> _ancestorClass;
public:

    ClassDecl(Token _pos,
              Token _endingpos,
              bool _isPublic,
              shared_ptr<Identifier> _ancestorName,
              shared_ptr<Identifier> _name,
              QVector<shared_ptr<Identifier> > _fields,
              QMap<QString, MethodInfo > _methodPrototypes,
              QVector<shared_ptr<ClassInternalDecl> > _internalDecls,
              QMap<QString, shared_ptr<TypeExpression> > _fieldMarshallAs);
    shared_ptr<Identifier> ancestorName() { return _ancestorName; }
    shared_ptr<Identifier> name() { return _name; }
    int fieldCount() { return _fields.count(); }
    shared_ptr<Identifier> field(int index) { return _fields[index]; }
    int methodPrototypeCount() { return _methodPrototypes.count(); }
    MethodInfo methodPrototype(QString index) { return _methodPrototypes[index]; }
    bool containsMethodPrototype(QString a) { return _methodPrototypes.contains(a);}
    int internalDeclCount() { return _internalDecls.count(); }
    shared_ptr<ClassInternalDecl> internalDecl(int index) { return _internalDecls[index]; }
    int fieldMarshallAsCount() { return _fieldMarshallAs.count(); }
    shared_ptr<TypeExpression> fieldMarshallAs(QString index) { return _fieldMarshallAs[index]; }
    bool containsFieldMarshallAs(QString a) { return _fieldMarshallAs.contains(a);}
    int methodCount() { return _methods.count(); }
    shared_ptr<MethodDecl> method(QString index) { return _methods[index]; }
    void insertMethod(QString a, shared_ptr<MethodDecl>b) { _methods[a]=b;}
    bool containsMethod(QString a) { return _methods.contains(a);}
    shared_ptr<ClassDecl> ancestorClass() { return _ancestorClass; }
    void setAncestorClass(shared_ptr<ClassDecl> val) { _ancestorClass = val; }
    virtual QString toString();
    virtual void prettyPrint(CodeFormatter *f);

    virtual void traverse(shared_ptr<PrettyPrintable> p, Traverser *tv);

    virtual void traverseChildren(Traverser *tv);

    virtual QString childrenToString();
};


class GlobalDecl  :   public Declaration
{
public:
    QString _varName;
public:

    GlobalDecl(Token _pos,
               Token _endingpos,
               bool _isPublic,
               QString _varName);
    QString varName() { return _varName; }
    virtual QString toString();
    virtual void prettyPrint(CodeFormatter *f);

    virtual void traverse(shared_ptr<PrettyPrintable> p, Traverser *tv);

    virtual void traverseChildren(Traverser *tv);

    virtual QString childrenToString();
};


class MethodDecl  :   public ProceduralDecl
{
public:
    shared_ptr<Identifier> _className;
    shared_ptr<Identifier> _receiverName;
    bool _isFunctionNotProcedure;
public:

    MethodDecl(Token _pos,
               Token _endingpos,
               bool _isPublic,
               shared_ptr<Identifier> _procName,
               QVector<shared_ptr<FormalParam> > formals,
               shared_ptr<BlockStmt> _body,
               shared_ptr<Identifier> _className,
               shared_ptr<Identifier> _receiverName,
               bool _isFunctionNotProcedure);
    shared_ptr<Identifier> className() { return _className; }
    shared_ptr<Identifier> receiverName() { return _receiverName; }
    bool isFunctionNotProcedure() { return _isFunctionNotProcedure; }
    virtual QString toString();
    virtual void prettyPrint(CodeFormatter *f);

    virtual void traverse(shared_ptr<PrettyPrintable> p, Traverser *tv);

    virtual void traverseChildren(Traverser *tv);

    virtual QString childrenToString();
    QString getTooltip();};


class PegExpr  :   public KalimatAst
{
public:
public:

    PegExpr(Token _pos,
            Token _endingpos);
    virtual QString toString()=0;
    virtual void prettyPrint(CodeFormatter *f)=0;

    virtual void traverse(shared_ptr<PrettyPrintable> p, Traverser *tv)=0;

    virtual void traverseChildren(Traverser *tv);

    virtual QString childrenToString();
    virtual QSet<QString> getAllAssociatedVars()=0;};


class PegPrimary  :   public PegExpr
{
public:
    shared_ptr<Identifier> _associatedVar;
public:

    PegPrimary(Token _pos,
               Token _endingpos,
               shared_ptr<Identifier> _associatedVar);
    shared_ptr<Identifier> associatedVar() { return _associatedVar; }
    void setAssociatedVar(shared_ptr<Identifier> val) { _associatedVar = val; }
    virtual QString toString()=0;
    virtual void prettyPrint(CodeFormatter *f)=0;

    virtual void traverse(shared_ptr<PrettyPrintable> p, Traverser *tv)=0;

    virtual void traverseChildren(Traverser *tv);

    virtual QString childrenToString();
    virtual QSet<QString> getAllAssociatedVars();};


class PegSequence  :   public PegExpr
{
public:
    QVector<shared_ptr<PegExpr> > elements;
public:

    PegSequence(Token _pos,
                Token _endingpos,
                QVector<shared_ptr<PegExpr> > elements);
    int elementCount() { return elements.count(); }
    shared_ptr<PegExpr> element(int index) { return elements[index]; }
    virtual QString toString();
    virtual void prettyPrint(CodeFormatter *f);

    virtual void traverse(shared_ptr<PrettyPrintable> p, Traverser *tv);

    virtual void traverseChildren(Traverser *tv);

    virtual QString childrenToString();
    virtual QSet<QString> getAllAssociatedVars();};


class PegRuleInvokation  :   public PegPrimary
{
public:
    shared_ptr<Identifier> _ruleName;
public:

    PegRuleInvokation(Token _pos,
                      Token _endingpos,
                      shared_ptr<Identifier> _associatedVar,
                      shared_ptr<Identifier> _ruleName);
    shared_ptr<Identifier> ruleName() { return _ruleName; }
    virtual QString toString();
    virtual void prettyPrint(CodeFormatter *f);

    virtual void traverse(shared_ptr<PrettyPrintable> p, Traverser *tv);

    virtual void traverseChildren(Traverser *tv);

    virtual QString childrenToString();
};


class PegLiteral  :   public PegPrimary
{
public:
    shared_ptr<StrLiteral> _value;
public:

    PegLiteral(Token _pos,
               Token _endingpos,
               shared_ptr<Identifier> _associatedVar,
               shared_ptr<StrLiteral> _value);
    shared_ptr<StrLiteral> value() { return _value; }
    virtual QString toString();
    virtual void prettyPrint(CodeFormatter *f);

    virtual void traverse(shared_ptr<PrettyPrintable> p, Traverser *tv);

    virtual void traverseChildren(Traverser *tv);

    virtual QString childrenToString();
};


class PegCharRange  :   public PegPrimary
{
public:
    shared_ptr<StrLiteral> _value1;
    shared_ptr<StrLiteral> _value2;
public:

    PegCharRange(Token _pos,
                 Token _endingpos,
                 shared_ptr<Identifier> _associatedVar,
                 shared_ptr<StrLiteral> _value1,
                 shared_ptr<StrLiteral> _value2);
    shared_ptr<StrLiteral> value1() { return _value1; }
    shared_ptr<StrLiteral> value2() { return _value2; }
    virtual QString toString();
    virtual void prettyPrint(CodeFormatter *f);

    virtual void traverse(shared_ptr<PrettyPrintable> p, Traverser *tv);

    virtual void traverseChildren(Traverser *tv);

    virtual QString childrenToString();
};


// rep(stmt)
// expr = term:t1 rep(x=t1, "+" term:t2, x=binOp("+", t1,t2)):x
class PegRepetion  :   public PegPrimary
{
public:
    shared_ptr<Identifier> _resultVar;
    shared_ptr<PegExpr> _subExpr;
    shared_ptr<AssignmentStmt> _stepAssignment;
public:

    PegRepetion(Token _pos,
                Token _endingpos,
                shared_ptr<Identifier> _associatedVar,
                shared_ptr<Identifier> _resultVar,
                shared_ptr<PegExpr> _subExpr,
                shared_ptr<AssignmentStmt> _stepAssignment);
    shared_ptr<Identifier> resultVar() { return _resultVar; }
    shared_ptr<PegExpr> subExpr() { return _subExpr; }
    shared_ptr<AssignmentStmt> stepAssignment() { return _stepAssignment; }
    virtual QString toString();
    virtual void prettyPrint(CodeFormatter *f);

    virtual void traverse(shared_ptr<PrettyPrintable> p, Traverser *tv);

    virtual void traverseChildren(Traverser *tv);

    virtual QString childrenToString();
};


class RuleOption  :   public KalimatAst
{
public:
    shared_ptr<PegExpr> _expression;
    shared_ptr<Expression> _resultExpr;
public:

    RuleOption(Token _pos,
               Token _endingpos,
               shared_ptr<PegExpr> _expression,
               shared_ptr<Expression> _resultExpr);
    shared_ptr<PegExpr> expression() { return _expression; }
    shared_ptr<Expression> resultExpr() { return _resultExpr; }
    virtual QString toString();
    virtual void prettyPrint(CodeFormatter *f);

    virtual void traverse(shared_ptr<PrettyPrintable> p, Traverser *tv);

    virtual void traverseChildren(Traverser *tv);

    virtual QString childrenToString();
};


class RuleDecl  :   public KalimatAst
{
public:
    QString _ruleName;
    QVector<shared_ptr<RuleOption> > options;
public:

    RuleDecl(Token _pos,
             Token _endingpos,
             QString _ruleName,
             QVector<shared_ptr<RuleOption> > options);
    QString ruleName() { return _ruleName; }
    int optionCount() { return options.count(); }
    shared_ptr<RuleOption> option(int index) { return options[index]; }
    virtual QString toString();
    virtual void prettyPrint(CodeFormatter *f);

    virtual void traverse(shared_ptr<PrettyPrintable> p, Traverser *tv);

    virtual void traverseChildren(Traverser *tv);

    virtual QString childrenToString();
    virtual QSet<QString> getAllAssociatedVars();};


class RulesDecl  :   public Declaration
{
public:
    shared_ptr<Identifier> _ruleName;
    QVector<shared_ptr<RuleDecl> > _subRules;
public:

    RulesDecl(Token _pos,
              Token _endingpos,
              bool _isPublic,
              shared_ptr<Identifier> _ruleName,
              QVector<shared_ptr<RuleDecl> > _subRules);
    shared_ptr<Identifier> ruleName() { return _ruleName; }
    int subRuleCount() { return _subRules.count(); }
    shared_ptr<RuleDecl> subRule(int index) { return _subRules[index]; }
    virtual QString toString();
    virtual void prettyPrint(CodeFormatter *f);

    virtual void traverse(shared_ptr<PrettyPrintable> p, Traverser *tv);

    virtual void traverseChildren(Traverser *tv);

    virtual QString childrenToString();
    virtual QSet<QString> getAllAssociatedVars();};


#endif
