/**************************************************************************
** The Kalimat programming language
**   Copyright (C) 2010 Mohamed Samy Ali - samy2004@gmail.com
**   This project is released under the Apache License version 2.0
**   as described in the included license.txt file
**************************************************************************/

#ifndef KALIMATAST_H
#define KALIMATAST_H

class Expression;
class Identifier;
class AssignableExpression;
class Declaration;
class MethodDecl;
class NumLiteral;
class StrLiteral;

using namespace std;

class TopLevel : public AST
{
public:
    TopLevel(Token pos);
};

class CompilationUnit: public AST
{
public:
    CompilationUnit(Token pos);
    QVector<QSharedPointer<StrLiteral> > _usedModules;
    int usedModuleCount() { return _usedModules.count();}
    StrLiteral *usedModule(int i ) { return _usedModules[i].data();}
};

class Program : public CompilationUnit
{
public:
    QVector<QSharedPointer<TopLevel > > _elements;

public:
    Program(Token pos, QVector<TopLevel *> program, QVector<StrLiteral *> usedModules);
    int elementCount() { return _elements.count(); }
    TopLevel *element(int i) { return _elements[i].data();}
    void addElement(TopLevel *element) { _elements.append(QSharedPointer<TopLevel>(element));}

    QString toString();
};

class Module : public CompilationUnit
{
public:
    QVector<QSharedPointer<Declaration > > _declarations;
    QScopedPointer<Identifier> _name;
public:
    Module(Token pos, Identifier *name, QVector<Declaration *> module, QVector<StrLiteral *> usedModules);
    int declCount() { return _declarations.count(); }
    Declaration *decl(int i) { return _declarations[i].data();}
    void addDecl(Declaration *decl) { _declarations.append(QSharedPointer<Declaration>(decl));}
    Identifier *name() { return _name.data();}

    QString toString();
};

class IScopeIntroducer
{
    virtual QVector<Identifier *> getIntroducedVariables()=0;
};

class Statement : public TopLevel
{
public:
    Statement(Token pos);
};

class IOStatement : public Statement
{
public:
    IOStatement(Token pos);
};

class GraphicsStatement : public Statement
{
public:
    GraphicsStatement(Token pos);
};

class Declaration: public TopLevel
{
    bool _isPublic;
public:
    Declaration(Token pos, bool isPublic);
    bool isPublic();
};

class AssignmentStmt : public Statement
{
public:
    QScopedPointer <AssignableExpression> _variable;
    QScopedPointer<Expression> _value;
public:
    AssignmentStmt(Token pos, AssignableExpression *variable, Expression *value);
    AssignableExpression *variable() {return _variable.data();}
    Expression *value() {return _value.data();}
    QString toString();
};

class IfStmt : public Statement
{
public:
    QScopedPointer <Expression> _condition;
    QScopedPointer <Statement> _thenPart;
    QScopedPointer <Statement> _elsePart;
public:
    IfStmt(Token pos, Expression *_condition, Statement *_thenPart, Statement *_elsePart);
    Expression *condition() {return _condition.data();}
    Statement *thenPart() { return _thenPart.data();}
    Statement *elsePart() {return _elsePart.data();}
    QString toString();
};

class WhileStmt : public Statement
{
public:
    QScopedPointer<Expression> _condition;
    QScopedPointer<Statement> _statement;

public:
    WhileStmt(Token pos, Expression *_condition, Statement *_statement);
    Expression *condition() {return _condition.data();}
    Statement *statement() { return _statement.data();}
    QString toString();
};

class ForAllStmt : public Statement
{
public:
    QScopedPointer<Identifier> _variable;
    QScopedPointer<Expression> _from;
    QScopedPointer<Expression> _to;
    QScopedPointer<Statement> _statement;

public:
    ForAllStmt(Token pos, Identifier *variable, Expression *from, Expression *to, Statement *statement);
    Identifier *variable() { return _variable.data(); }
    Expression *from() { return _from.data(); }
    Expression *to() { return _to.data(); }
    Statement *statement() { return _statement.data();}
    QString toString();
    virtual QVector<Identifier *> getIntroducedVariables();
};

class ReturnStmt : public Statement
{
public:
    QScopedPointer<Expression> _returnVal;
public:
    ReturnStmt(Token pos, Expression *returnVal);
    Expression *returnVal() { return _returnVal.data(); }
    QString toString();
};
class LabelStmt : public Statement
{
    QSharedPointer<Expression> _target;
public:
    LabelStmt(Token pos, Expression *target);
    Expression *target() { return _target.data(); }
    QString toString();
};

class GotoStmt : public Statement
{
public:
    bool targetIsNumber;
    QSharedPointer<NumLiteral> _numericTarget;
    QSharedPointer<Identifier> _idTarget;
public:
    GotoStmt(Token pos, bool _targetIsNumber, Expression *target);
    Identifier *idTarget() { return _idTarget.data(); }
    NumLiteral *numericTarget() { return _numericTarget.data(); }
    QString toString();
};

class PrintStmt : public IOStatement
{
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
};

class ReadStmt : public IOStatement
{
public:
    QString prompt;
    QSharedPointer<Expression> _fileObject;
    QVector<QSharedPointer<Identifier > > _variables;
    QVector<bool> readNumberFlags;
public:
    ReadStmt(Token pos, Expression *fileObject, QString prompt, QVector<Identifier *> variables, QVector<bool> readNumberFlags);
    int variableCount() { return _variables.count();}
    Identifier *variable(int i) { return _variables[i].data();}
    Expression *fileObject() { return _fileObject.data();}
    QString toString();
};

class DrawPixelStmt : public GraphicsStatement
{
public:
    QScopedPointer<Expression> _x, _y;
    QScopedPointer<Expression> _color;
public:
    DrawPixelStmt(Token pos, Expression *x, Expression *y, Expression *color);
    Expression *x() { return _x.data();}
    Expression *y() { return _y.data();}
    Expression *color() { return _color.data();}
    QString toString();
};

class DrawLineStmt : public GraphicsStatement
{
public:
    QScopedPointer<Expression> _x1, _y1, _x2, _y2;
    QScopedPointer<Expression> _color;
public:
    DrawLineStmt(Token pos, Expression *x1, Expression *y1, Expression *x2, Expression *y2, Expression *color);
    Expression *x1() { return _x1.data();}
    Expression *y1() { return _y1.data();}
    Expression *x2() { return _x2.data();}
    Expression *y2() { return _y2.data();}
    Expression *color() { return _color.data();}
    QString toString();
};

class DrawRectStmt : public GraphicsStatement
{
public:
    QScopedPointer<Expression> _x1, _y1, _x2, _y2;
    QScopedPointer<Expression> _color;
    bool filled;
public:
    DrawRectStmt(Token pos, Expression *x1, Expression *y1, Expression *x2, Expression *y2, Expression *color, bool filled);
    Expression *x1() { return _x1.data();}
    Expression *y1() { return _y1.data();}
    Expression *x2() { return _x2.data();}
    Expression *y2() { return _y2.data();}
    Expression *color() { return _color.data();}
    QString toString();
};

class DrawCircleStmt : public GraphicsStatement
{
public:
    QScopedPointer<Expression> _cx, _cy;
    QScopedPointer<Expression> _radius;
    QScopedPointer<Expression> _color;

    bool filled;
public:
    DrawCircleStmt(Token pos, Expression *cx, Expression *cy, Expression *radius, Expression *color, bool filled);
    Expression *cx() { return _cx.data();}
    Expression *cy() { return _cy.data();}
    Expression *radius() { return _radius.data();}
    Expression *color() { return _color.data();}
    QString toString();
};

class DrawSpriteStmt : public GraphicsStatement
{
public:
    QScopedPointer<Expression> _x, _y;
    QScopedPointer<Expression> _number;

public:
    DrawSpriteStmt(Token pos, Expression *x, Expression *y, Expression *number);
    Expression *x() { return _x.data();}
    Expression *y() { return _y.data();}
    Expression *number() { return _number.data();}
    QString toString();
};

class ZoomStmt : public GraphicsStatement
{
public:
    QScopedPointer<Expression> _x1, _y1, _x2, _y2;
public:
    ZoomStmt(Token pos, Expression *x1, Expression *y1, Expression *x2, Expression *y2);
    Expression *x1() { return _x1.data();}
    Expression *y1() { return _y1.data();}
    Expression *x2() { return _x2.data();}
    Expression *y2() { return _y2.data();}
    QString toString();
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
};

class BlockStmt : public Statement
{
public:
    QVector<QSharedPointer<Statement> > _statements;
public:
    BlockStmt(Token pos, QVector<Statement *> statements);
    int statementCount() { return _statements.count(); }
    Statement * statement(int i) { return _statements[i].data(); }
    QVector<Statement *> getStatements();
    QString toString();
};

class InvokationStmt : public Statement
{
public:
    QScopedPointer<Expression> _expression;
public:
    InvokationStmt(Token pos, Expression *expression);
    Expression *expression() { return _expression.data(); }
    QString toString();
};

class Expression : public AST
{
public:
    Expression(Token pos);
};
class AssignableExpression : public Expression
{
public:
    AssignableExpression(Token pos);
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
};
class Identifier : public AssignableExpression
{
public:
    QString name;
public:
    Identifier(Token pos, QString name);
    QString toString();
};

class NumLiteral : public Expression
{
public:
    long lValue;
    double dValue;
    bool longNotDouble;
    bool valueRecognized;
public:
    NumLiteral(Token pos, QString lexeme);
    QString toString();
};

class StrLiteral : public Expression
{
public:
    QString value;
public:
    StrLiteral(Token pos, QString value);
    QString toString();
};
class NullLiteral : public Expression
{
public:
    NullLiteral(Token pos);
    QString toString();
};
class BoolLiteral : public Expression
{
public:
    bool value;
public:
    BoolLiteral(Token pos, bool value);
    QString toString();
};

class ArrayLiteral : public Expression
{
public:
    QVector<QSharedPointer<Expression > > _data;
public:
    ArrayLiteral(Token pos, QVector<Expression *> data);
    int dataCount() {return _data.count();}
    Expression *data(int i) { return _data[i].data(); }
    QVector<QSharedPointer<Expression> > dataVector() { return _data;}
    QString toString();
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
    QString toString();
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

};

class ObjectCreation : public Expression
{
public:
    QScopedPointer<Identifier> _className;
public:
    ObjectCreation(Token pos, Identifier *className);
    Identifier *className() { return _className.data();}
    QString toString();
};

class ProceduralDecl : public Declaration, public IScopeIntroducer
{
public:
    QScopedPointer<Identifier> _procName;
    QVector<QSharedPointer<Identifier > > _formals;
    QVector<QSharedPointer<Identifier > > _allReferences;
    QScopedPointer<BlockStmt> _body;
public:
    ProceduralDecl(Token pos, Identifier *procName, QVector<Identifier *> formals, BlockStmt *body, bool isPublic);
    Identifier *procName() {return _procName.data();}
    int formalCount() { return _formals.count();}
    Identifier *formal(int i) { return _formals[i].data();}
    Identifier *allReferences(int i) { return _allReferences[i].data();}
    void addReference(Identifier *id) { _allReferences.append(QSharedPointer<Identifier>(id));}
    BlockStmt *body() {return _body.data();}
    void body(BlockStmt *stmt) { _body.reset(stmt);}
    virtual QVector<Identifier *> getIntroducedVariables();
};

class IProcedure
{

};
class IFunction
{

};

class ProcedureDecl : public ProceduralDecl, public IProcedure
{
public:
    ProcedureDecl(Token pos, Identifier *procName, QVector<Identifier *> formals, BlockStmt *body, bool isPublic);
    QString toString();
};
class FunctionDecl : public ProceduralDecl, public IFunction
{
public:
    FunctionDecl(Token pos, Identifier *procName, QVector<Identifier *> formals, BlockStmt *body, bool isPublic);
    QString toString();

};

struct MethodInfo
{
    int arity;
    bool isFunction;
    MethodInfo(int arity, bool isFunction);
    MethodInfo();
};

class ClassDecl : public Declaration
{
public:
    QScopedPointer<Identifier> _name;
    QVector<QSharedPointer<Identifier > > _fields;
    QMap<QString, MethodInfo>  _methodPrototypes;
    QMap<QString, QSharedPointer<MethodDecl> > _methods;
    QScopedPointer<Identifier> _ancestorName;
    QSharedPointer<ClassDecl> _ancestorClass;
public:
    ClassDecl(Token pos,
              Identifier *name,
              QVector<Identifier *> fields,
              QMap<QString,MethodInfo> methodPrototypes,
              bool isPublic);
    ClassDecl(Token pos,
              Identifier *ancestorName,
              Identifier *name,
              QVector<Identifier *> fields,
              QMap<QString,MethodInfo> methodPrototypes,
              bool isPublic);
    Identifier *name() { return _name.data();}
    int fieldCount() { return _fields.count();}
    Identifier *field(int i) { return _fields[i].data();}
    bool containsMethod(QString name);
    bool containsPrototype(QString name);
    int methodCount() { return _methods.count();}
    MethodDecl *method(QString name);
    MethodDecl *method(int i) { return _methods.values().at(i).data();}
    MethodInfo methodPrototype(QString name);
    int prototypeCount() { return _methodPrototypes.count();}
    Identifier *ancestor() { return _ancestorName.data();}
    void setAncestorClass(QSharedPointer<ClassDecl> cd);
    void insertMethod(QString name, QSharedPointer<MethodDecl> m);
    QString toString();
};
class GlobalDecl : public Declaration
{
public:
    QString varName;
public:
    GlobalDecl(Token pos, QString varName, bool isPublic);
    QString toString();
};
class MethodDecl : public ProceduralDecl
{
public:
    QScopedPointer<Identifier> _className;
    QScopedPointer<Identifier> _receiverName;
    bool isFunctionNotProcedure;
public:
    MethodDecl(Token pos, Identifier *className, Identifier *receiverName,
               Identifier *methodName, QVector<Identifier *> formals, BlockStmt *body
               , bool isFunctionNotProcedure);
    Identifier *className() { return _className.data();}
    Identifier *receiverName() { return _receiverName.data();}
    QString toString();
};


template<typename V> QString vector_toString(V vector)
{
    QStringList lst;
    for(int i=0; i<vector.count(); i++)
    {
        AST *ast = vector[i].data();
        lst.append(ast->toString());
    }
    return lst.join(", ");
}

#endif // KALIMATAST_H
