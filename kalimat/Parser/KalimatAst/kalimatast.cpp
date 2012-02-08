/**************************************************************************
**   The Kalimat programming language
**   Copyright (C) 2010 Mohamed Samy Ali - samy2004@gmail.com
**   This project is released under the Apache License version 2.0
**   as described in the included license.txt file
**************************************************************************/

#include "kalimatast_incl.h"
#include "../../../smallvm/utils.h"
using namespace std;

CompilationUnit::CompilationUnit(Token pos) : _astImpl(pos)
{

}

Program::Program(Token pos,
                 QVector<shared_ptr<TopLevel> > elements,
                 QVector<shared_ptr<StrLiteral> > usedModules,
                 QVector<shared_ptr<TopLevel> > originalElements)
    : CompilationUnit(pos),
      _elements(elements),
      _originalElements(originalElements)
{
    _usedModules = usedModules;
}

QString Program::toString()
{
    QStringList ret;
    for(int i=0; i<elementCount(); i++)
    {
        ret.append(element(i)->toString());
    }
    return _ws(L"برنامج(").append(ret.join(_ws(L"، "))).append(")");
}

Module::Module(Token pos,
               shared_ptr<Identifier> name,
               QVector<shared_ptr<Declaration> > module,
               QVector<shared_ptr<StrLiteral> > usedModules)
    :CompilationUnit(pos),
      _name(name),
      _declarations(module)
{
    _usedModules = usedModules;
    /*
    for(int i=0; i<module.count(); i++)
        _declarations.append(shared_ptr<Declaration>(module[i]));

    for(int i=0; i<usedModules.count(); i++)
        _usedModules.append(shared_ptr<StrLiteral>(usedModules[i]));
    //*/
}

Module::~Module()
{

}

QString Module::toString()
{
    QStringList ret;
    for(int i=0; i<declCount(); i++)
    {
        ret.append(decl(i)->toString());
    }
    return _ws(L"وحدة(").append(ret.join(_ws(L"، "))).append(")");
}

TopLevel::TopLevel(Token pos)
    :_astImpl(pos)
{

}

Statement::Statement(Token pos): TopLevel(pos)
{

}

Expression::Expression(Token pos) : _astImpl(pos)
{

}

AssignableExpression::AssignableExpression(Token pos) : Expression(pos)
{

}

Declaration::Declaration(Token pos, bool isPublic) : TopLevel(pos)
{
    _isPublic = isPublic;
}

bool Declaration::isPublic()
{
    return _isPublic;
}

IOStatement::IOStatement(Token pos) : Statement(pos)
{
}

GraphicsStatement::GraphicsStatement(Token pos) : Statement(pos)
{

}

AssignmentStmt::AssignmentStmt(Token pos,
                               shared_ptr<AssignableExpression> _variable,
                               shared_ptr<Expression> _value)
    :Statement(pos),
    _variable(_variable),
    _value(_value)
{
}

QString AssignmentStmt::toString()
{
    return _ws(L"=(%1، %2)").arg(variable()->toString(),value()->toString());
}

IfStmt::IfStmt(Token pos,
               shared_ptr<Expression> _condition,
               shared_ptr<Statement> _thenPart,
               shared_ptr<Statement> _elsePart)
    :Statement(pos),
    _condition(_condition),
    _thenPart(_thenPart),
    _elsePart(_elsePart)
{

}

QString IfStmt::toString()
{
    if(elsePart() == NULL)
        return _ws(L"إذا(%1، %2)").arg(condition()->toString())
                                   .arg(thenPart()->toString());
    else
        return _ws(L"إذا(%1، %2، %3)").arg(condition()->toString())
                                      .arg(thenPart()->toString())
                                      .arg(elsePart()->toString());
}

WhileStmt::WhileStmt(Token pos,
                     shared_ptr<Expression> _condition,
                     shared_ptr<Statement> _statement)
    :Statement(pos),
    _condition(_condition),
    _statement(_statement)
{
}

QString WhileStmt::toString()
{

    return _ws(L"طالما(%1، %2)").arg(condition()->toString())
                               .arg(statement()->toString());
}

ForAllStmt::ForAllStmt(Token pos,
                       shared_ptr<Identifier> variable,
                       shared_ptr<Expression> from,
                       shared_ptr<Expression> to,
                       shared_ptr<Statement>  statement)
    :Statement(pos),
    _variable(variable),
    _from(from),
    _to(to),
    _statement(statement)
{
}

QVector<shared_ptr<Identifier> > ForAllStmt::getIntroducedVariables()
{
    QVector<shared_ptr<Identifier> > ret;
    ret.append(variable());
    return ret;
}

QString ForAllStmt::toString()
{

    return _ws(L"لكل(%1، %2، %3، %4)")
            .arg(variable()->toString())
            .arg(from()->toString())
            .arg(to()->toString())
            .arg(statement()->toString());
}

ReturnStmt::ReturnStmt(Token pos ,shared_ptr<Expression> returnVal)
    :Statement(pos),
    _returnVal(returnVal)
{
}

QString ReturnStmt::toString()
{
    return _ws(L"ارجع(%1)").arg(returnVal()->toString());
}

DelegationStmt::DelegationStmt(Token pos, shared_ptr<IInvokation> invokation)
    :Statement(pos),
    _invokation(invokation)
{
}

QString DelegationStmt::toString()
{
    return _ws(L"وكل إلى(%1)").arg(invokation()->toString());
}

LaunchStmt::LaunchStmt(Token pos, shared_ptr<IInvokation> invokation)
    :Statement(pos),
    _invokation(invokation)
{
}

QString LaunchStmt::toString()
{
    return _ws(L"شغل(%1)").arg(invokation()->toString());
}


LabelStmt::LabelStmt(Token pos, shared_ptr<Expression> target)
    : Statement(pos),
      _target(target)
{

}
QString LabelStmt::toString()
{
    return _ws(L"علامة(%1)").arg(target()->toString());
}

GotoStmt::GotoStmt(Token pos,
                   shared_ptr<Expression> target)
    :Statement(pos), _target(target)
{

}

QString GotoStmt::toString()
{
    return _ws(L"اذهب(%1)").arg(target()->toString());
}

PrintStmt::PrintStmt(Token pos,
                     shared_ptr<Expression> fileObject,
                     QVector<shared_ptr<Expression> > args,
                     QVector<shared_ptr<Expression> > widths,
                     bool printOnSameLine)
    :IOStatement(pos),
     _fileObject(fileObject),
     _args(args),
     _widths(widths),
     printOnSameLine(printOnSameLine)
{
    /*
    for(int i= 0; i<args.count();i++)
    {
        this->_args.append(shared_ptr<Expression>(args[i]));
        this->_widths.append(shared_ptr<Expression>(widths[i]));
    }
    this->printOnSameLine = printOnSameLine;
    */
}
QString PrintStmt::toString()
{
    QStringList ret;
    for(int i=0; i<argCount(); i++)
    {
        ret.append(arg(i)->toString());
    }
    return _ws(L"اطبع(").append(ret.join(_ws(L"، "))).append(")");
}

ReadStmt::ReadStmt(Token pos,
                   shared_ptr<Expression> fileObject,
                   QString prompt,
                   const QVector<shared_ptr<AssignableExpression> >&variables,
                   QVector<bool> readNumberFlags)
    :IOStatement(pos),
     _fileObject(fileObject),
      prompt(prompt),
      _variables(variables),
      readNumberFlags(readNumberFlags)
{
    /*
    this->prompt = prompt;
    for(int i=0; i<variables.count(); i++)
        this->_variables.append(shared_ptr<AssignableExpression>(variables[i]));
    this->readNumberFlags = readNumberFlags;
    */
}

QString ReadStmt::toString()
{
    QStringList ret;
    for(int i=0; i<variableCount(); i++)
    {
        ret.append(variable(i)->toString());
    }
    return _ws(L"اقرأ(\"").append(prompt).append("\"،").append(ret.join(_ws(L"، "))).append("}");
}

DrawPixelStmt::DrawPixelStmt(Token pos,
                             shared_ptr<Expression>x,
                             shared_ptr<Expression> y,
                             shared_ptr<Expression> color)
    :GraphicsStatement(pos),
    _x(x),
    _y(y),
    _color(color)
{
}

QString DrawPixelStmt::toString()
{
    return _ws(L"رسم.نقطة(%1، %2، %3)")
            .arg(x()->toString())
            .arg(y()->toString())
            .arg(color()? color()->toString(): "default");
}

DrawLineStmt::DrawLineStmt(Token pos,
                           shared_ptr<Expression> x1,
                           shared_ptr<Expression> y1,
                           shared_ptr<Expression> x2,
                           shared_ptr<Expression> y2,
                           shared_ptr<Expression> color)
        :GraphicsStatement(pos),
         _x1(x1),
         _y1(y1),
         _x2(x2),
         _y2(y2),
         _color(color)
{
}

QString DrawLineStmt::toString()
{
    return _ws(L"رسم.خط([%1، %2]، [%3، %4]، %5)")
            .arg(x1()? x1()->toString(): "current")
            .arg(y1()? y1()->toString(): "current")
            .arg(x2()->toString())
            .arg(y2()->toString())
            .arg(color()? color()->toString(): "default");
}

DrawRectStmt::DrawRectStmt(Token pos,
                           shared_ptr<Expression> x1,
                           shared_ptr<Expression> y1,
                           shared_ptr<Expression> x2,
                           shared_ptr<Expression> y2,
                           shared_ptr<Expression> color,
                           shared_ptr<Expression> filled)
        :GraphicsStatement(pos),
        _x1(x1),
        _y1(y1),
        _x2(x2),
        _y2(y2),
        _color(color),
        _filled(filled)

{
}

QString DrawRectStmt::toString()
{
    return _ws(L"رسم.مستطيل([%1، %2]، [%3، %4],%5، %6)")
            .arg(x1()? x1()->toString() : "current")
            .arg(y1()? y1()->toString() : "current")
            .arg(x2()->toString())
            .arg(y2()->toString())
            .arg(color()? color()->toString(): "default")
            .arg(filled()?filled()->toString(): _ws(L"خطأ"));
}

DrawCircleStmt::DrawCircleStmt(Token pos,
                               shared_ptr<Expression> cx,
                               shared_ptr<Expression> cy,
                               shared_ptr<Expression> radius,
                               shared_ptr<Expression> color,
                               shared_ptr<Expression> filled)
        :GraphicsStatement(pos),
        _cx(cx),
        _cy(cy),
        _radius(radius),
        _color(color),
        _filled(filled)

{
}

QString DrawCircleStmt::toString()
{
    return _ws(L"رسم.دائرة([%1، %2]، %3، %4، %5)")
            .arg(cx()->toString())
            .arg(cy()->toString())
            .arg(radius()->toString())
            .arg(color()? color()->toString(): "default")
            .arg((filled()?filled()->toString(): _ws(L"خطأ")));
}

DrawImageStmt::DrawImageStmt(Token pos,
                               shared_ptr<Expression> x,
                               shared_ptr<Expression> y,
                               shared_ptr<Expression> image)
        :GraphicsStatement(pos),
        _x(x),
        _y(y),
        _image(image)

{
}

QString DrawImageStmt::toString()
{
    return _ws(L"رسم.صورة(%، [%2، %3])")
            .arg(image()->toString())
            .arg(x()->toString())
            .arg(y()->toString());
}


DrawSpriteStmt::DrawSpriteStmt(Token pos,
                               shared_ptr<Expression> x,
                               shared_ptr<Expression> y,
                               shared_ptr<Expression> sprite)
        :GraphicsStatement(pos),
        _x(x),
        _y(y),
        _sprite(sprite)

{
}

QString DrawSpriteStmt::toString()
{
    return _ws(L"رسم.طيف(%، [%2، %3])")
            .arg(sprite()->toString())
            .arg(x()->toString())
            .arg(y()->toString());
}

ZoomStmt::ZoomStmt(Token pos,
                   shared_ptr<Expression> x1,
                   shared_ptr<Expression> y1,
                   shared_ptr<Expression> x2,
                   shared_ptr<Expression> y2)
        :GraphicsStatement(pos),
        _x1(x1),
        _y1(y1),
        _x2(x2),
        _y2(y2)
{
}
QString ZoomStmt::toString()
{
    return _ws(L"زووم([%1,%2],[%3,%4])")
            .arg(x1()->toString())
            .arg(y1()->toString())
            .arg(x2()->toString())
            .arg(y2()->toString());
}

EventStatement::EventStatement(Token pos ,EventType type, shared_ptr<Identifier> handler)
        :Statement(pos),
         type(type),
         _handler(handler)

{
}

QString EventStatement::toString()
{
    return _ws(L"عند(%1، %2)").arg(type).arg(handler()->toString());
}

ChannelCommunicationStmt::ChannelCommunicationStmt(Token pos)
    : Statement(pos)
{

}

SendStmt::SendStmt(Token pos,
                   bool signal,
                   shared_ptr<Expression> value,
                   shared_ptr<Expression> channel)
    : ChannelCommunicationStmt(pos), signal(signal), _value(value), _channel(channel)
{
}

QString SendStmt::toString()
{
    return _ws(L"ارسل(%1، %2)").arg(signal? _ws(L"إشارة") : value()->toString(), channel()->toString());
}

ReceiveStmt::ReceiveStmt(Token pos,
                         bool signal,
                         shared_ptr<AssignableExpression> value,
                         shared_ptr<Expression> channel)
    : ChannelCommunicationStmt(pos), signal(signal), _value(value), _channel(channel)
{
}

QString ReceiveStmt::toString()
{
    return _ws(L"تسلم(%1، %2)").arg(signal? _ws(L"إشارة") : value()->toString(), channel()->toString());
}

SelectStmt::SelectStmt(Token pos,
                       QVector<shared_ptr<ChannelCommunicationStmt> > conditions,
                       QVector<shared_ptr<Statement > > actions)
    : Statement(pos),
      _conditions(conditions),
      _actions(actions)
{
   /*
    for(int i=0; i<conditions.count();i++)
        _conditions.append(shared_ptr<ChannelCommunicationStmt>(conditions[i]));
    for(int i=0; i<actions.count();i++)
        _actions.append(shared_ptr<Statement>(actions[i]));
    */
}

QString SelectStmt::toString()
{
    QStringList condact;
    for(int i=0; i<count(); i++)
    {
        condact.append(QString("%1 => %2")
                       .arg(condition(i)->toString())
                       .arg(action(i)->toString()));
    }
    return _ws(L"تخير({%1})").arg(condact.join(", "));
}

BlockStmt::BlockStmt(Token pos, QVector<shared_ptr<Statement> > statements)
        :Statement(pos),
          _statements(statements)

{
    //for(int i=0; i<statements.count(); i++)
    //    _statements.append(shared_ptr<Statement>(statements[i]));
}

QVector<shared_ptr<Statement> > BlockStmt::getStatements()
{
    return _statements;
}

QString BlockStmt::toString()
{
    QStringList ret;
    for(int i=0; i<statementCount(); i++)
    {
        ret.append(statement(i)->toString());
    }
    return QString("{ ").append(ret.join(_ws(L"، "))).append(" }");
}

InvokationStmt::InvokationStmt(Token pos ,shared_ptr<Expression> expression)
        :Statement(pos),
        _expression(expression)
{
}
QString InvokationStmt::toString()
{
    return expression()->toString();
}

Identifier::Identifier(Token pos ,QString name)
        :AssignableExpression(pos)
{
    this->name = name;
}
QString Identifier::toString()
{
    return name;
}

TypeIdentifier::TypeIdentifier(Token pos ,QString name)
        :TypeExpression(pos)
{
    this->name = name;
}
QString TypeIdentifier::toString()
{
    return name;
}

PointerTypeExpression::PointerTypeExpression(Token pos, shared_ptr<TypeExpression> pointeeType)
    :TypeExpression(pos), _pointeeType(pointeeType)
{
}

QString PointerTypeExpression::toString()
{
    return QString("pointer(%1)").arg(pointeeType()->toString());
}

FunctionTypeExpression::FunctionTypeExpression(Token pos,
                                               shared_ptr<TypeExpression> retType,
                                               QVector<shared_ptr<TypeExpression> > argTypes)
    :TypeExpression(pos),
      _retType(retType),
      _argTypes(argTypes)
{
}

QString FunctionTypeExpression::toString()
{
    QStringList strs;
    for(int i=0; i<argTypeCount(); i++)
        strs.append(argType(i)->toString());
    return QString("func(%1,[%2])").arg(retType()?retType()->toString():"void",strs.join(", "));
}

Pattern::Pattern(Token pos)
    : _astImpl(pos)
{

}

QString VarPattern::toString()
{
    return id()->toString();
}

ArrayPattern::ArrayPattern(Token pos, QVector<shared_ptr<Pattern> > elements)
    : Pattern(pos),
      _elements(elements)
{
}

QString ArrayPattern::toString()
{
    QStringList items;
    for(int i=0; i<elementCount(); i++)
        items.append(element(i)->toString());
    return _ws(L"نمط.مصفوفة([%1 %2])").arg(items.join(", ")).arg(this->fixedLength? "": "...");
}

ObjPattern::ObjPattern(Token pos,
                       shared_ptr<Identifier> classId,
                       QVector<shared_ptr<Identifier> > fieldNames,
                       QVector<shared_ptr<Pattern> > fieldPatterns)
    : Pattern(pos),
      _classId(classId),
      _fieldNames(fieldNames),
      _fieldPatterns(fieldPatterns)
{
}

QString ObjPattern::toString()
{
    QStringList pairs;
    for(int i=0;i<this->fieldCount(); i++)
    {
        pairs.append(QString("%1=%2").arg(fieldName(i)->name).arg(fieldPattern(i)->toString()));
    }
    return QString(_ws(L"نمط.كائني(%1،%2)")).arg(this->classId()->name).arg(pairs.join(", "));
}

MapPattern::MapPattern(Token pos,
                       QVector<shared_ptr<Expression> > keys,
                       QVector<shared_ptr<Pattern> > values)
    : Pattern(pos),
      _keys(keys),
      _values(values)
{
}

QString MapPattern::toString()
{
    QStringList pairs;
    for(int i=0;i<this->pairCount(); i++)
    {
        pairs.append(QString("%1=>%2").arg(key(i)->toString()).arg(value(i)->toString()));
    }
    return QString(_ws(L"نمط.قاموسي(%1)"))
            .arg(pairs.join(", "));
}

BinaryOperation::BinaryOperation(Token pos,
                                 QString op,
                                 shared_ptr<Expression> op1,
                                 shared_ptr<Expression> op2)
    : Expression(pos),
    _operator(op),
    _operand1(op1),
    _operand2(op2)

{
}

QString BinaryOperation::toString()
{
    return _ws(L"عملية(%1، %2، %3)")
            .arg(_operator)
            .arg(operand1()->toString())
            .arg(operand2()->toString());
}

IsaOperation::IsaOperation(Token pos, shared_ptr<Expression> expression, shared_ptr<Identifier> type)
    : Expression(pos),
      _expression(expression),
      _type(type)
{

}

QString IsaOperation::toString()
{
    return _ws(L"%1 هو %2").arg(expression()->toString()).arg(type()->toString());
}

MatchOperation::MatchOperation(Token pos,
                               shared_ptr<Expression> expression,
                               shared_ptr<Pattern> pattern)
    : Expression(pos),
      _expression(expression),
      _pattern(pattern)
{

}

QString MatchOperation::toString()
{
    return _ws(L"%1 ~ %2").arg(expression()->toString()).arg(pattern()->toString());
}

UnaryOperation::UnaryOperation(Token pos ,QString __operator, shared_ptr<Expression> operand)
    : Expression(pos),
    _operator(__operator),
    _operand(operand)
{
}
QString UnaryOperation::toString()
{
    return _ws(L"عملية(%1، %2)")
            .arg(_operator)
            .arg(operand()->toString());
}


NumLiteral::NumLiteral(Token pos ,QString lexeme) : SimpleLiteral(pos)
{
    bool ok;
    QLocale loc(QLocale::Arabic, QLocale::Egypt);
    longNotDouble = true;
    lValue= loc.toLongLong(lexeme, &ok, 10);
    if(!ok)
        lValue = lexeme.toLong(&ok, 10);

    if(!ok)
    {
        longNotDouble = false;
        dValue = loc.toDouble(lexeme, &ok);
    }
    if(!ok)
        dValue = lexeme.toDouble(&ok);

    valueRecognized = ok;

}
QString NumLiteral::toString()
{
    return _ws(L"%1")
            .arg(repr());
}

QString NumLiteral::repr()
{
    return QString("%1").arg(longNotDouble? lValue: dValue);
}

StrLiteral::StrLiteral(Token pos ,QString value) : SimpleLiteral(pos)
{
    this->value = value;
}
QString StrLiteral::toString()
{
    return _ws(L"نص(%1)")
            .arg(repr());
}

QString strLiteralRepr(QString value)
{
    QStringList ret;
    ret.append("\"");
    for(int i=0; i<value.length(); i++)
    {
        QChar c = value[i];
        if(c =='"')
            ret.append("\"\"");
        else
            ret.append(c);
    }
    ret.append("\"");
    return ret.join("");
}

QString StrLiteral::repr()
{
    return strLiteralRepr(this->value);
}

NullLiteral::NullLiteral(Token pos) : SimpleLiteral(pos)
{

}
QString NullLiteral::toString()
{
    return repr();
}

QString NullLiteral::repr()
{
    return _ws(L"لاشيء");
}

BoolLiteral::BoolLiteral(Token pos, bool _value) : SimpleLiteral(pos)
{
    value = _value;
}
QString BoolLiteral::toString()
{
    return repr();
}

QString BoolLiteral::repr()
{
    return value? QString::fromWCharArray(L"صحيح") : QString::fromWCharArray(L"خطأ");
}

ArrayLiteral::ArrayLiteral(Token pos, QVector<shared_ptr<Expression> >data)
    : Literal(pos), _data(data)
{
}

QString ArrayLiteral::toString()
{
    return _ws(L"مصفوفة(%1)").arg(vector_toString(_data));
}

MapLiteral::MapLiteral(Token pos, QVector<shared_ptr<Expression> >data)
    : Literal(pos), _data(data)
{
}

QString MapLiteral::toString()
{
    return _ws(L"قاموس(%1)").arg(vector_toString(_data));
}

IInvokation::IInvokation(Token pos)
    :Expression(pos)
{

}

Invokation::Invokation(Token pos,
                       shared_ptr<Expression> functor,
                       QVector<shared_ptr<Expression> >arguments)
    : IInvokation(pos),
    _functor(functor),
    _arguments(arguments)
{
}

QString Invokation::toString()
{
    return _ws(L"نداء(%1، %2)").arg(functor()->toString()).arg(vector_toString(_arguments));
}

MethodInvokation::MethodInvokation(Token pos,
                                   shared_ptr<Expression> receiver,
                                   shared_ptr<Identifier> methodSelector,
                                   QVector<shared_ptr<Expression> >arguments)
    :IInvokation(pos),
    _receiver(receiver),
    _methodSelector(methodSelector),
    _arguments(arguments)
{
}

QString MethodInvokation::toString()
{
    return _ws(L"نداء.وسيلة(%1، %2، %3)")
            .arg(receiver()->toString())
            .arg(methodSelector()->toString())
            .arg(vector_toString(_arguments));
}

TimingExpression::TimingExpression(Token pos, shared_ptr<Expression> toTime)
    : Expression(pos), _toTime(toTime)
{

}

QString TimingExpression::toString()
{
    return _ws(L"توقيت(%1)")
            .arg(_toTime->toString());
}

Idafa::Idafa(Token pos ,shared_ptr<Identifier> modaf, shared_ptr<Expression> modaf_elaih)
    :AssignableExpression(pos),
    _modaf(modaf),
    _modaf_elaih(modaf_elaih)
{
}

QString Idafa::toString()
{
    return _ws(L"اضافة(%1، %2").arg(modaf()->toString(), modaf_elaih()->toString());
}

ArrayIndex::ArrayIndex(Token pos,
                       shared_ptr<Expression> array,
                       shared_ptr<Expression> index)
    :AssignableExpression(pos),
    _array(array),
    _index(index)
{
}

QString ArrayIndex::toString()
{
    return QString("%1[%2]").arg(array()->toString(), index()->toString());
}

MultiDimensionalArrayIndex::MultiDimensionalArrayIndex(Token pos,
                                                       shared_ptr<Expression> array,
                                                       QVector<shared_ptr<Expression> > indexes)
    :AssignableExpression(pos),
    _array(array),
    _indexes(indexes)
{
}

QString MultiDimensionalArrayIndex::toString()
{
    return QString("%1[%2]").arg(array()->toString(), vector_toString(_indexes));
}

ObjectCreation::ObjectCreation(Token pos ,shared_ptr<Identifier> className)
    :Expression(pos),
    _className(className)
{
}

QString ObjectCreation::toString()
{
    return QString("جديد(%1)").arg(className()->name);
}

ProceduralDecl::ProceduralDecl(Token pos,
                               Token endingToken,
                               shared_ptr<Identifier> procName,
                               QVector<shared_ptr<Identifier> > formals,
                               shared_ptr<BlockStmt> body,
                               bool isPublic)
    :Declaration(pos, isPublic),
    _procName(procName),
    _formals(formals),
    _body(body),
    _endingToken(endingToken)
{
    /*
    for(int i=0; i<formals.count(); i++)
        _formals.append(shared_ptr<Identifier>(formals[i]));
    */
}

QVector<shared_ptr<Identifier> > ProceduralDecl::getIntroducedVariables()
{
    return _formals;
}

ProcedureDecl::ProcedureDecl(Token pos,
                             Token endingToken,
                             shared_ptr<Identifier> procName,
                             QVector<shared_ptr<Identifier> >formals,
                             shared_ptr<BlockStmt> body,
                             bool isPublic)
    :ProceduralDecl(pos, endingToken, procName, formals, body, isPublic)
{

}

QString ProcedureDecl::toString()
{
    QString pname = procName()->toString();
    // todo: Rename the actual 'main' procedure
    if(pname=="%main")
        pname = _ws(L"<البداية>");

    return _ws(L"إجراء(%1، %2، %3)")
            .arg(procName()->toString())
            .arg(vector_toString(_formals))
            .arg(body()->toString());
}

FunctionDecl::FunctionDecl(Token pos,
                           Token endingToken,
                           shared_ptr<Identifier> procName,
                           QVector<shared_ptr<Identifier> > formals,
                           shared_ptr<BlockStmt> body,
                           bool isPublic)
    :ProceduralDecl(pos, endingToken, procName, formals, body, isPublic)
{

}

QString FunctionDecl::toString()
{
    return _ws(L"دالة(%1، %2، %3)")
            .arg(procName()->toString())
            .arg(vector_toString(_formals))
            .arg(body()->toString());
}

MethodInfo::MethodInfo(int arity, bool isFunction)
{
    this->arity = arity;
    this->isFunction = isFunction;
}

// Degenerate constructor, create only to allow usage in QMap<>...etc
MethodInfo::MethodInfo()
{
    arity = -1;
    isFunction = false;
}

FFILibraryDecl::FFILibraryDecl(Token pos, QString libName, QVector<shared_ptr<Declaration> > decls, bool isPublic)
    : Declaration(pos, isPublic), libName(libName),_decls(decls)
{
}

QString FFILibraryDecl::toString()
{
    QStringList strs;
    for(int i=0; i<declCount(); i++)
        strs.append(decl(i)->toString());
    return QString("library(%1, %2)").arg(libName).arg(strs.join(", "));
}

FFIProceduralDecl::FFIProceduralDecl(Token pos,
                                     bool isFunctionNotProc,
                                     shared_ptr<TypeExpression> theReturnType,
                                     QVector<shared_ptr<TypeExpression> > paramTypes,
                                     QString procName,
                                     QString symbol,
                                     bool isPublic)
    : Declaration(pos, isPublic),
      isFunctionNotProc(isFunctionNotProc),
      procName(procName),
      symbol(symbol),
      _returnType(theReturnType),
      _paramTypes(paramTypes)
{
}

QString FFIProceduralDecl::toString()
{
    QStringList str1;
    for(int i=0; i<paramTypeCount(); i++)
        str1.append(paramType(i)->toString());
    return QString ("ffi(%1,%2,%3,[%4])").arg(procName).arg(symbol)
            .arg(returnType()?returnType()->toString():"void").arg(str1.join(", "));
}

FFIStructDecl::FFIStructDecl(Token pos,
                             shared_ptr<Identifier> name,
                             QVector<shared_ptr<Identifier> > fieldNames,
                             QVector<shared_ptr<TypeExpression> > fieldTypes,
                             QVector<int> fieldBatches,
                             bool isPublic)
    : Declaration(pos, isPublic),
      _name(name),
      _fieldNames(fieldNames),
      _fieldTypes(fieldTypes),
      fieldBatches(fieldBatches)
{
}

QString FFIStructDecl::toString()
{
    QStringList strs;
    for(int i=0; i<fieldCount(); i++)
    {
        strs.append(QString("%1:%2").arg(fieldName(i)->toString()).arg(fieldType(i)->toString()));
    }
    return QString("ffiStruct(%1,%2)").arg(name()->toString()).arg(strs.join(", "));
}

RulesDecl::RulesDecl(Token pos,
                     shared_ptr<Identifier> name,
                     QVector<shared_ptr<RuleDecl> > subRules,
                     bool isPublic)
    :Declaration(pos, isPublic),
      _ruleName(name),
      _subRules(subRules)
{

}

QString RulesDecl::toString()
{
    QStringList lst;
    lst.append(_ws(L"قواعد"));
    lst.append("(");
    lst.append(this->name()->toString());
    lst.append(", ");
    for(int i=0; i<subRuleCount(); i++)
    {
        shared_ptr<RuleDecl> r = subRule(i);
        lst.append(r->ruleName);
        lst.append("=[");

        for(int j=0; j<r->options.count(); j++)
        {
            if(j>0)
            {
                lst.append("|");
            }
            shared_ptr<RuleOption> ro = r->options[j];
            lst.append(ro->expression()->toString());
            if(ro->resultExpr())
            {
                lst.append(" => ");
                lst.append(ro->resultExpr()->toString());
            }
            lst.append(", ");
        }
        lst.append("]");
    }
    lst.append(")");
    return lst.join("");
}

QString PegRuleInvokation::toString()
{
    return QString("%1%2")
            .arg(ruleName()->toString())
            .arg(associatedVar()?
                     QString(":%1").arg(associatedVar()->toString())
                   :"");
}

QString PegSequence::toString()
{
    QStringList lst;
    for(int i=0; i<elementCount(); i++)
        lst.append(element(i)->toString());
    return lst.join(" ");
}

ClassDecl::ClassDecl(Token pos,
                     shared_ptr<Identifier> name,
                     QVector<shared_ptr<Identifier > >fields,
                     QMap<QString, MethodInfo> methodPrototypes,
                     QVector<shared_ptr<ClassInternalDecl> > internalDecls,
                     QMap<QString, shared_ptr<TypeExpression> > fieldMarshalAs,
                     bool isPublic)
        :Declaration(pos, isPublic),
        _name(name),
        _fields(fields),
        _methodPrototypes(methodPrototypes),
        _internalDecls(internalDecls),
        _fieldMarshallAs(fieldMarshalAs)
{
}

ClassDecl::ClassDecl(Token pos,
                     shared_ptr<Identifier> ancestorName,
                     shared_ptr<Identifier> name,
                     QVector<shared_ptr<Identifier> >fields,
                     QMap<QString, MethodInfo> methodPrototypes,
                     QVector<shared_ptr<ClassInternalDecl> > internalDecls,
                     QMap<QString, shared_ptr<TypeExpression> > fieldMarshalAs,
                     bool isPublic)
        :Declaration(pos, isPublic),
        _ancestorName(ancestorName),
        _name(name),
        _fields(fields),
        _methodPrototypes(methodPrototypes),
        _fieldMarshallAs(fieldMarshalAs),
        _internalDecls(internalDecls),
        _ancestorClass()
{
}

QString ClassDecl::toString()
{
    QMap<QString, MethodInfo> &m = this->_methodPrototypes;
    QStringList lst;
    for(int i=0; i<m.keys().count(); i++)
    {
        lst.append(QString("%1/%2").arg(m.keys().at(i))
                   .arg(m[m.keys().at(i)].arity));
    }
    QString map = lst.join(",");

    return _ws(L"فصيلة(%1، %2، %3")
            .arg(name()->toString())
            .arg(vector_toString(_fields))
            .arg(map);

}

void ClassDecl::insertMethod(QString name, shared_ptr<MethodDecl> m)
{
    _methods[name] = m;
}

bool ClassDecl::containsMethod(QString name)
{
    if(_methods.contains(name))
        return true;
    if(_ancestorClass && _ancestorClass->containsMethod(name))
        return true;
    return false;
}

shared_ptr<MethodDecl> ClassDecl::method(QString name)
{
    if(_methods.contains(name))
        return _methods[name];
    if(_ancestorClass)
        return _ancestorClass->method(name);
    return shared_ptr<MethodDecl>();
}

bool ClassDecl::containsPrototype(QString name)
{
    if(_methodPrototypes.contains(name))
        return true;
    if(_ancestorClass && _ancestorClass->containsPrototype(name))
        return true;
    return false;
}

MethodInfo ClassDecl::methodPrototype(QString name)
{
    if(_methodPrototypes.contains(name))
        return _methodPrototypes[name];
    return _ancestorClass->methodPrototype(name);
}

void ClassDecl::setAncestorClass(shared_ptr<ClassDecl> cd)
{
    _ancestorClass = cd;
}

GlobalDecl::GlobalDecl(Token pos ,QString varName, bool isPublic)
    :Declaration(pos, isPublic)
{
    this->varName = varName;
}
QString GlobalDecl::toString()
{
    return _ws(L"مشترك(%1)").arg(varName);
}

MethodDecl::MethodDecl(Token pos,
                       Token endingToken,
                       shared_ptr<Identifier> className,
                       shared_ptr<Identifier> receiverName,
                       shared_ptr<Identifier> methodName,
                       QVector<shared_ptr<Identifier> > formals,
                       shared_ptr<BlockStmt> body,
                       bool isFunctionNotProcedure)

       :ProceduralDecl(pos, endingToken, methodName, formals, body, true),
       _className(className),
       _receiverName(receiverName),
       isFunctionNotProcedure(isFunctionNotProcedure)
{
    this->_formals.prepend(receiverName);
}

QString MethodDecl::toString()
{
    return QString("%1(%2,%3,%4)")
            .arg(isFunctionNotProcedure?_ws(L"رد")
                :
                _ws(L"استجابة"))
            .arg(procName()->toString())
            .arg(vector_toString(_formals))
            .arg(body()->toString());
}
// Pretty printing
void Program::prettyPrint(CodeFormatter *f)
{
    for(int i=0; i<_originalElements.count(); i++)
    {
        shared_ptr<TopLevel> el = _originalElements[i];
        // todo: loss of information
        if(el.get()->attachedComments.trimmed() != "")
        {
            //f->print("--");
            f->print(el->attachedComments);
        }
        el->prettyPrint(f);
        f->nl();
    }
}

void Module::prettyPrint(CodeFormatter *f)
{
    f->printKw(L"وحدة");
    this->name()->prettyPrint(f);
    f->nl();

    for(int i=0; i<declCount(); i++)
    {
        decl(i)->prettyPrint(f);
        f->nl();
        f->nl();
    }
}

void AssignmentStmt::prettyPrint(CodeFormatter *f)
{
    variable()->prettyPrint(f);
    f->space();
    f->print("=");
    f->space();
    value()->prettyPrint(f);
}

void IfStmt::prettyPrint(CodeFormatter *f)
{
    f->printKw(L"إذا");
    condition()->prettyPrint(f);
    f->space();
    f->colon();

    shared_ptr<BlockStmt> thnBlk = dynamic_pointer_cast<BlockStmt>(this->thenPart());
    if(thnBlk)
        f->nl();
    this->thenPart()->prettyPrint(f);

    if(this->elsePart())
    {
        shared_ptr<BlockStmt> elsBlk = dynamic_pointer_cast<BlockStmt>(this->elsePart());
        if(elsBlk)
        {
            f->printKw(L"وإلا");
            f->colon();
            f->nl();
            elsBlk->prettyPrint(f);
//            f->nl();
            f->printKw(L"تم");
        }
        else
        {
            f->space();
            f->printKw(L"وإلا");
            shared_ptr<IfStmt> elseItselfIsAnotherIf = dynamic_pointer_cast<IfStmt>(this->elsePart());
            if(!elseItselfIsAnotherIf)
            {
                f->colon();
                f->space();
            }
            this->elsePart()->prettyPrint(f);
        }
    }
    else if(thnBlk)
    {
        f->printKw(L"تم");
        f->nl(); // for extra neatness, add an empty line after block statements
    }
}

void WhileStmt::prettyPrint(CodeFormatter *f)
{
    f->printKw(L"طالما");
    condition()->prettyPrint(f);
    //f->space();
    f->colon();

    shared_ptr<BlockStmt> actionBlk = dynamic_pointer_cast<BlockStmt>(this->statement());
    if(actionBlk)
        f->nl();
    else
        f->space();

    this->statement()->prettyPrint(f);
    if(!actionBlk)
        f->space();
    f->printKw(L"تابع");
    if(actionBlk)
        f->nl(); // for extra neatness, add an empty line after block statements
}

void ForAllStmt::prettyPrint(CodeFormatter *f)
{
    f->printKw(L"لكل");
    this->variable()->prettyPrint(f);
    f->space();
    f->printKw(L"من");
    this->from()->prettyPrint(f);
    f->space();
    f->printKw(L"إلى");
    this->to()->prettyPrint(f);
    //f->space();
    f->colon();

    shared_ptr<BlockStmt> actionBlk = dynamic_pointer_cast<BlockStmt>(this->statement());
    if(actionBlk)
        f->nl();
    else
        f->space();

    this->statement()->prettyPrint(f);
    if(!actionBlk)
        f->space();
    f->printKw(L"تابع");
    if(actionBlk)
        f->nl(); // for extra neatness, add an empty line after block statements
}

void ReturnStmt::prettyPrint(CodeFormatter *f)
{
    f->printKw(L"ارجع ب:");
    f->space();
    this->returnVal()->prettyPrint(f);
}

void DelegationStmt::prettyPrint(CodeFormatter *f)
{
    f->printKw(L"وكل إلى");
    f->space();
    this->invokation()->prettyPrint(f);
}

void LaunchStmt::prettyPrint(CodeFormatter *f)
{
    f->printKw(L"شغل");
    f->space();
    this->invokation()->prettyPrint(f);
}


void LabelStmt::prettyPrint(CodeFormatter *f)
{
    f->printKw(L"علامة");
    f->space();
    this->target()->prettyPrint(f);
}

void GotoStmt::prettyPrint(CodeFormatter *f)
{
    f->printKw(L"اذهب إلى");
    f->space();
    this->target()->prettyPrint(f);
}

void PrintStmt::prettyPrint(CodeFormatter *f)
{
    f->printKw(L"اطبع");
    if(this->fileObject() != NULL)
    {
        f->printKw(L"في");
        this->fileObject()->prettyPrint(f);
        f->colon();
        f->space();
    }
    commaSep(mapPrint(this->_args, this->_widths)).run(f);
    if(this->printOnSameLine)
    {
        f->comma();
        f->print("...");
    }
}

void ReadStmt::prettyPrint(CodeFormatter *f)
{
    f->printKw(L"اقرأ");

    if(this->fileObject() != NULL)
    {
        f->printKw(L"من");
        this->fileObject()->prettyPrint(f);
        f->colon();
        f->space();
    }

    // todo: minor loss of information
    // if user has a command read "", x, y
    // since "empty prompt" and "no prompt" have the same AST representation
    if(this->prompt !="")
    {
        f->print(strLiteralRepr(prompt));
        f->comma();
    }

    commaSep(mapRead(this->_variables, this->readNumberFlags)).run(f);
}

void DrawPixelStmt::prettyPrint(CodeFormatter *f)
{
    f->printKw(L"ارسم.نقطة");
    parens(&commaSep(&ast(this->x()), &ast(this->y()))).run(f);
    if(this->color() !=NULL)
    {
        f->comma();
        color()->prettyPrint(f);
    }
}

void DrawLineStmt::prettyPrint(CodeFormatter *f)
{
    f->printKw(L"ارسم.خط");
    parens(&commaSep(&ast(this->x1()), &ast(this->y1()))).run(f);
    f->print("-");
    parens(&commaSep(&ast(this->x2()), &ast(this->y2()))).run(f);
    if(this->color() !=NULL)
    {
        f->comma();
        color()->prettyPrint(f);
    }
}

void DrawRectStmt::prettyPrint(CodeFormatter *f)
{
    f->printKw(L"ارسم.مستطيل");
    parens(&commaSep(&ast(this->x1()), &ast(this->y1()))).run(f);
    f->print("-");
    parens(&commaSep(&ast(this->x2()), &ast(this->y2()))).run(f);

    if(this->color())
    {
        f->comma();
        color()->prettyPrint(f);
        if(this->filled())
        {
            f->comma();
            filled()->prettyPrint(f);
        }
    }
    else if(this->filled())
    {
        f->comma();
        f->comma();
        filled()->prettyPrint(f);
    }
}

void DrawCircleStmt::prettyPrint(CodeFormatter *f)
{
    f->printKw(L"ارسم.دائرة");
    parens(&commaSep(&ast(this->cx()), &ast(this->cy()))).run(f);
    f->comma();
    radius()->prettyPrint(f);
    if(this->color())
    {
        f->comma();
        color()->prettyPrint(f);
        if(this->filled())
        {
            f->comma();
            filled()->prettyPrint(f);
        }
    }
    else if(this->filled())
    {
        f->comma();
        f->comma();
        filled()->prettyPrint(f);
    }
}

void DrawImageStmt::prettyPrint(CodeFormatter *f)
{
    f->printKw(L"ارسم.صورة");
    this->image()->prettyPrint(f);
    f->space();
    f->printKw(L"في");
    parens(&commaSep(&ast(this->x()), &ast(this->y()))).run(f);
    /*
    f->openParen();
    this->x()->prettyPrint(f);
    f->comma();
    this->y()->prettyPrint(f);
    this->closeParen();
    */
}

void DrawSpriteStmt::prettyPrint(CodeFormatter *f)
{
    f->printKw(L"ارسم.طيف");
    this->sprite()->prettyPrint(f);
    f->space();
    f->printKw(L"في");
    parens(&commaSep(&ast(this->x()), &ast(this->y()))).run(f);
    /*
    f->openParen();
    this->x()->prettyPrint(f);
    f->comma();
    this->y()->prettyPrint(f);
    this->closeParen();
    */
}

void ZoomStmt::prettyPrint(CodeFormatter *f)
{

}

QStdWString translateEventType(EventType t)
{
    switch(t)
    {
    case KalimatKeyDownEvent:
        return L"ضغط.مفتاح";
    case KalimatKeyUpEvent:
        return L"رفع.مفتاح";
    case KalimatKeyPressEvent:
        return L"ادخال.حرف";
    case KalimatMouseUpEvent:
        return L"رفع.زر.ماوس";
    case KalimatMouseDownEvent:
        return L"ضغط.زر.ماوس";
    case KalimatMouseMoveEvent:
        return L"تحريك.ماوس";
    case KalimatSpriteCollisionEvent:
        return L"تصادم";
    }
}

void EventStatement::prettyPrint(CodeFormatter *f)
{
    f->printKw(L"عند حادثة");
    f->printKw(translateEventType(this->type));
    f->printKw(L"نفذ");
    this->handler()->prettyPrint(f);
}

void SendStmt::prettyPrint(CodeFormatter *f)
{
    f->printKw(L"ارسل");
    if(signal)
        f->printKw(L"إشارة");
    else
        value()->prettyPrint(f);
    f->printKw(L"إلى");
    this->channel()->prettyPrint(f);
}

void ReceiveStmt::prettyPrint(CodeFormatter *f)
{
    f->printKw(L"تسلم");
    if(signal)
        f->printKw(L"إشارة");
    else
        value()->prettyPrint(f);
    f->printKw(L"من");
    this->channel()->prettyPrint(f);
}

void SelectStmt::prettyPrint(CodeFormatter *f)
{
    f->printKw(L"تخير");
    f->nl();
    f->indent();
    for(int i=0; i<count(); i++)
    {
        shared_ptr<ChannelCommunicationStmt> cond = condition(i);
        shared_ptr<Statement> act = action(i);
        shared_ptr<BlockStmt> actBlk = dynamic_pointer_cast<BlockStmt>(act);

        if(i>0)
            f->printKw(L"أو");
        cond->prettyPrint(f);
        f->space();
        f->colon();
        if(actBlk)
            f->nl();
        act->prettyPrint(f);
    }
    f->deindent();
    f->print(L"تم");
}

void BlockStmt::prettyPrint(CodeFormatter *f)
{
    f->indent();
    for(int i=0; i<statementCount(); i++)
    {
        if(statement(i)->attachedComments.trimmed() != "")
        {
            f->print("--");
            f->print(statement(i)->attachedComments);
        }
        statement(i)->prettyPrint(f);
        f->nl();
    }
    f->deindent();
}

void InvokationStmt::prettyPrint(CodeFormatter *f)
{
    this->expression()->prettyPrint(f);
}

void VarPattern::prettyPrint(CodeFormatter *f)
{
    id()->prettyPrint(f);
}

void ArrayPattern::prettyPrint(CodeFormatter *f)
{
    //todo: ArrayPattern::prettyPrint() by not copying to new vector
    QVector<shared_ptr<PrettyPrintable> > data;
    for(int i=0; i<elementCount(); i++)
        data.append(element(i));

    brackets(&commaSep(mapFmt(this->_elements))).run(f);
}

void ObjPattern::prettyPrint(CodeFormatter *f)
{
    this->classId()->prettyPrint(f);
    f->space();
    f->printKw(_ws(L"له"));
    QVector<shared_ptr<PrettyPrintable> > data;
    //todo: optimize ObjPattern::prettyPrint() by not copying to new vector
    for(int i=0; i<this->fieldCount(); i++)
    {
        data.append(fieldName(i));
        data.append(fieldPattern(i));
    }
    commaSepPairs(mapFmt(data), "=").run(f);
}

void MapPattern::prettyPrint(CodeFormatter *f)
{
    QVector<shared_ptr<PrettyPrintable> > data;
    //todo: optimize ObjPattern::prettyPrint() by not copying to new vector
    for(int i=0; i<this->pairCount(); i++)
    {
        data.append(key(i));
        data.append(value(i));
    }
    braces(&commaSepPairs(mapFmt(data), "=>")).run(f);
}

QString translateOperator(QString op)
{
    if(op == "add")
    {
        return "+";
    }
    else if(op == "sub")
    {
        return "-";
    }
    else if(op == "mul")
    {
        return _ws(L"×");
    }
    else if(op == "div")
    {
        return _ws(L"÷");
    }
    else if(op == "and")
    {
        return _ws(L"وأيضا");
    }
    else if(op == "or")
    {
        return _ws(L"أو");
    }
    else if(op == "not")
    {
        return _ws(L"ليس");
    }
    else if(op == "lt")
    {
        return "<";
    }
    else if(op == "gt")
    {
        return ">";
    }
    else if(op == "le")
    {
        return "<=";
    }
    else if(op == "ge")
    {
        return ">=";
    }
    else if(op == "eq")
    {
        return "=";
    }
    else if(op == "ne")
    {
        return "<>";
    }
    return "?";
}

void BinaryOperation::prettyPrint(CodeFormatter *f)
{
    this->operand1()->prettyPrint(f);
    f->space();
    f->print(translateOperator(this->_operator));
    f->space();
    this->operand2()->prettyPrint(f);
}

void IsaOperation::prettyPrint(CodeFormatter *f)
{
    this->expression()->prettyPrint(f);
    f->space();
    f->printKw(L"هو");
    this->type()->prettyPrint(f);
}

void MatchOperation::prettyPrint(CodeFormatter *f)
{
    this->expression()->prettyPrint(f);
    f->space();
    f->print(_ws(L"~"));
    f->space();
    this->pattern()->prettyPrint(f);
}

void UnaryOperation::prettyPrint(CodeFormatter *f)
{
    f->print(translateOperator(this->_operator));
    f->space();
    this->operand()->prettyPrint(f);
}

void Identifier::prettyPrint(CodeFormatter *f)
{
    f->print(name);
}

void TypeIdentifier::prettyPrint(CodeFormatter *f)
{
    f->print(name);
}

void PointerTypeExpression::prettyPrint(CodeFormatter *f)
{
    f->printKw(L"مشير");
    f->openParen();
    pointeeType()->prettyPrint(f);
    f->closeParen();
}

void FunctionTypeExpression::prettyPrint(CodeFormatter *f)
{
    if(retType())
        f->printKw(L"دالة");
    else
        f->printKw(L"إجراء");
    f->openParen();
    for(int i=0; i<argTypeCount(); i++)
    {
        argType(i)->prettyPrint(f);
        if(i<argTypeCount()-1)
            f->comma();
    }
    f->closeParen();
}

void NumLiteral::prettyPrint(CodeFormatter *f)
{
    f->print(repr());
}

void StrLiteral::prettyPrint(CodeFormatter *f)
{
    f->print(repr());
}

void NullLiteral::prettyPrint(CodeFormatter *f)
{
    f->print(repr());
}

void BoolLiteral::prettyPrint(CodeFormatter *f)
{
    f->printKwExpression(repr());
}

void ArrayLiteral::prettyPrint(CodeFormatter *f)
{
    brackets(&commaSep(mapFmt(this->_data))).run(f);
}

void MapLiteral::prettyPrint(CodeFormatter *f)
{
    braces(&commaSepPairs(mapFmt(this->_data), "=>")).run(f);
}

void Invokation::prettyPrint(CodeFormatter *f)
{
    this->functor()->prettyPrint(f);
    parens(&commaSep(mapFmt(this->_arguments))).run(f);
}

void MethodInvokation::prettyPrint(CodeFormatter *f)
{
    this->receiver()->prettyPrint(f);
    f->colon();
    f->space();
    this->methodSelector()->prettyPrint(f);
    parens(&commaSep(mapFmt(this->_arguments))).run(f);
}

void TimingExpression::prettyPrint(CodeFormatter *f)
{
    f->printKw(L"توقيت");
    f->openParen();
    this->toTime()->prettyPrint(f);
    f->closeParen();
}

void Idafa::prettyPrint(CodeFormatter *f)
{
    spaceSep(&ast(modaf()), &ast(modaf_elaih())).run(f);
}

void ArrayIndex::prettyPrint(CodeFormatter *f)
{
    this->array()->prettyPrint(f);
    brackets(&ast(this->index())).run(f);
}

void MultiDimensionalArrayIndex::prettyPrint(CodeFormatter *f)
{
    this->array()->prettyPrint(f);
    brackets(&commaSep(mapFmt(this->_indexes))).run(f);
}

void ObjectCreation::prettyPrint(CodeFormatter *f)
{
    className()->prettyPrint(f);
    f->space();
    f->printKw(L"جديد");
}

void ProcedureDecl::prettyPrint(CodeFormatter *f)
{
    f->printKw(L"إجراء");
    procName()->prettyPrint(f);

    parens(&commaSep(mapFmt(this->_formals))).run(f);

    f->colon();
    f->nl();
    this->body()->prettyPrint(f);
    f->printKw(L"نهاية");
    f->nl(); // for extra neatness, add an empty line after definitions
}

void FunctionDecl::prettyPrint(CodeFormatter *f)
{
    f->printKw(L"دالة");
    procName()->prettyPrint(f);

    parens(&commaSep(mapFmt(this->_formals))).run(f);

    f->colon();
    f->nl();
    this->body()->prettyPrint(f);
    f->printKw(L"نهاية");
    f->nl(); // for extra neatness, add an empty line after definitions
}

void FFILibraryDecl::prettyPrint(CodeFormatter *f)
{
    f->printKw(L"مكتبة");
    f->print(QString("\"%1\"").arg(libName));
    f->colon();
    f->nl();
    f->indent();
    for(int i=0; i<declCount(); i++)
        decl(i)->prettyPrint(f);
    f->deindent();
    f->printKw(L"نهاية");
    f->nl(); // for extra neatness, add an empty line after definitions
}

void FFIProceduralDecl::prettyPrint(CodeFormatter *f)
{
    if(this->isFunctionNotProc)
        f->printKw(L"دالة");
    else
        f->printKw("إجراء");

    f->print(this->procName);
    f->space();
    if(symbol != procName)
    {
        f->printKw(L"برمز");
        f->print(QString("\"%1\"").arg(symbol));
    }
    f->openParen();
    for(int i=0; i<paramTypeCount(); i++)
    {
        paramType(i)->prettyPrint(f);
        if(i < paramTypeCount() -1)
            f->comma();
    }
    f->closeParen();
    if(isFunctionNotProc)
    {
        returnType()->prettyPrint(f);
    }

    f->nl();
}

void FFIStructDecl::prettyPrint(CodeFormatter *f)
{
    f->printKw(L"فصيلة");
    this->name()->prettyPrint(f);
    f->colon();
    f->nl();
    f->indent();

    int index = 0;
    for(int i=0; i<fieldBatches.count(); i++)
    {
        f->printKw(L"له");

        for(int j=0; j< fieldBatches[i]; j++)
        {
            fieldName(index)->prettyPrint(f);
            f->space();
            f->printKw(L"هو");
            fieldType(index)->prettyPrint(f);
            index++;
        }
        f->nl();
    }
    f->deindent();
    f->printKw(L"نهاية");
    f->nl(); // for extra neatness, add an empty line after definitions
}

void RulesDecl::prettyPrint(CodeFormatter *f)
{
    f->printKw(L"قواعد");
    this->name()->prettyPrint(f);
    f->colon();
    f->nl();
    f->indent();
    for(int i=0; i<subRuleCount(); i++)
    {
        shared_ptr<RuleDecl> r = subRule(i);
        f->print(r->ruleName);
        f->space();
        f->print("=");
        f->space();
        for(int j=0; j<r->options.count(); j++)
        {
            if(j>0)
            {
                f->printKw(L"أو");
            }
            shared_ptr<RuleOption> ro = r->options[j];
            ro->expression()->prettyPrint(f);
            if(ro->resultExpr())
            {
                f->space();
                f->print("=>");
                f->space();
                ro->resultExpr()->prettyPrint(f);
            }
            f->nl();
        }
        f->nl();
        if(i+1 < subRuleCount())
            f->nl();
    }
    f->deindent();
    f->printKw(L"نهاية");
    f->nl(); // for extra neatness, add an empty line after definitions
}

void PegSequence::prettyPrint(CodeFormatter *f)
{
    spaceSep(mapFmt(this->elements)).run(f);
}

void PegRuleInvokation::prettyPrint(CodeFormatter *f)
{
    this->ruleName()->prettyPrint(f);
    if(this->associatedVar())
    {
        f->print(":");
        this->associatedVar()->prettyPrint(f);
    }
}

void PegLiteral::prettyPrint(CodeFormatter *f)
{
    this->value()->prettyPrint(f);
    if(this->associatedVar())
    {
        f->print(":");
        this->associatedVar()->prettyPrint(f);
    }
}

void Has::prettyPrint(CodeFormatter *f)
{
    f->printKw(L"له");
    commaSep(mapFmt(this->fields)).run(f);
}

void RespondsTo::prettyPrint(CodeFormatter *f)
{
    if(isFunctions)
    {
        f->printKw(L"يرد على");
    }
    else
    {
        f->printKw(L"يستجيب ل:");
    }
    commaSep(mapFmt(methods)).run(f);
}

void ConcreteResponseInfo::prettyPrint(CodeFormatter *f)
{
    this->name->prettyPrint(f);
    f->space();
    parens(&commaSep(mapFmt(params))).run(f);
}

void ClassDecl::prettyPrint(CodeFormatter *f)
{
    f->printKw(L"فصيلة");
    this->name()->prettyPrint(f);
    f->colon();
    f->nl();
    f->indent();
    if(ancestor())
    {
        f->printKw(L"مبني على");
        ancestor()->prettyPrint(f);
        f->nl();
    }
    for(int i=0; i<_internalDecls.count(); i++)
    {
        shared_ptr<ClassInternalDecl> id = _internalDecls[i];
        id->prettyPrint(f);
        f->nl();
    }
    f->deindent();
    f->printKw(L"نهاية");
    f->nl(); // for extra neatness, add an empty line after definitions
}

void GlobalDecl::prettyPrint(CodeFormatter *f)
{
    f->print(varName);
    f->space();
    f->printKw(L"مشترك");
}

void MethodDecl::prettyPrint(CodeFormatter *f)
{
    if(this->isFunctionNotProcedure)
        f->printKw(L"رد");
    else
        f->printKw(L"استجابة");

    this->className()->prettyPrint(f);
    f->space();
    this->receiverName()->prettyPrint(f);
    f->space();

    if(this->isFunctionNotProcedure)
        f->printKw(L"على");
    else
        f->printKw(L"ل:");

    procName()->prettyPrint(f);

    parens(&commaSep(mapFmt(this->_formals, 1))).run(f);

    f->colon();
    f->nl();
    this->body()->prettyPrint(f);
    f->printKw(L"نهاية");
    f->nl(); // for extra neatness, add an empty line after definitions
}

QVector<FormatMaker *> mapPrint(QVector<shared_ptr<Expression > > args, QVector<shared_ptr<Expression > > widths)
{
    QVector<FormatMaker *> ret;
    for(int i=0; i<args.count(); i++)
        ret.append(new PrintFmt(args[i], widths[i]));
    return ret;
}

QVector<FormatMaker *> mapRead(QVector<shared_ptr<AssignableExpression> > variables, QVector<bool> readNumberFlags)
{
    QVector<FormatMaker *> ret;
    for(int i=0; i<variables.count(); i++)
        ret.append(new ReadFmt(variables[i], readNumberFlags[i]));
    return ret;
}

InvokationStmt::~InvokationStmt()
{

}

ReceiveStmt::~ReceiveStmt() {}
SendStmt::~SendStmt() {}
ZoomStmt::~ZoomStmt() {}
DrawImageStmt::~DrawImageStmt() {}
DrawSpriteStmt::~DrawSpriteStmt() {}
DrawCircleStmt::~DrawCircleStmt() {}
DrawRectStmt::~DrawRectStmt() {}
DrawLineStmt::~DrawLineStmt() {}
DrawPixelStmt::~DrawPixelStmt() {}
LaunchStmt::~LaunchStmt() {}
DelegationStmt::~DelegationStmt() {}
ReturnStmt::~ReturnStmt() {}
ForAllStmt::~ForAllStmt() {}
WhileStmt::~WhileStmt() {}
IfStmt::~IfStmt() {}
AssignmentStmt::~AssignmentStmt() {}
MatchOperation::~MatchOperation() {}
