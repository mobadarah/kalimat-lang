#include "kalimat_ast_gen.h"
#include "../../../smallvm/utils.h"
KalimatAst::KalimatAst(Token _pos,
                       Token _endingpos):
    AST(),_pos(_pos),
    _endingpos(_endingpos)
{
}


QString KalimatAst::childrenToString()
{
    QString ret;
    QTextStream out(&ret);
    return ret;
}

void KalimatAst::traverseChildren(Traverser *tv)
{
}

TopLevel::TopLevel(Token _pos,
                   Token _endingpos):
    KalimatAst(_pos,_endingpos)
{
}


QString TopLevel::childrenToString()
{
    QString ret;
    QTextStream out(&ret);
    return ret;
}

void TopLevel::traverseChildren(Traverser *tv)
{
}

Statement::Statement(Token _pos,
                     Token _endingpos):
    TopLevel(_pos,_endingpos)
{
}


QString Statement::childrenToString()
{
    QString ret;
    QTextStream out(&ret);
    return ret;
}

void Statement::traverseChildren(Traverser *tv)
{
}

Expression::Expression(Token _pos,
                       Token _endingpos):
    KalimatAst(_pos,_endingpos)
{
}


QString Expression::childrenToString()
{
    QString ret;
    QTextStream out(&ret);
    return ret;
}

void Expression::traverseChildren(Traverser *tv)
{
}

IOStatement::IOStatement(Token _pos,
                         Token _endingpos):
    Statement(_pos,_endingpos)
{
}


QString IOStatement::childrenToString()
{
    QString ret;
    QTextStream out(&ret);
    return ret;
}

void IOStatement::traverseChildren(Traverser *tv)
{
}

GraphicsStatement::GraphicsStatement(Token _pos,
                                     Token _endingpos):
    Statement(_pos,_endingpos)
{
}


QString GraphicsStatement::childrenToString()
{
    QString ret;
    QTextStream out(&ret);
    return ret;
}

void GraphicsStatement::traverseChildren(Traverser *tv)
{
}

ChannelCommunicationStmt::ChannelCommunicationStmt(Token _pos,
                                                   Token _endingpos):
    Statement(_pos,_endingpos)
{
}


QString ChannelCommunicationStmt::childrenToString()
{
    QString ret;
    QTextStream out(&ret);
    return ret;
}

void ChannelCommunicationStmt::traverseChildren(Traverser *tv)
{
}

CompilationUnit::CompilationUnit(Token _pos,
                                 Token _endingpos,
                                 QVector<shared_ptr<StrLiteral> > usedModules):
    KalimatAst(_pos,_endingpos),usedModules(usedModules)
{
}


QString CompilationUnit::childrenToString()
{
    QString ret;
    QTextStream out(&ret);
    out << "[";
    for(int i=0; i<usedModules.count(); i++)
    {
        if(usedModules[i])
        {
            out << usedModules[i]->toString() << ", " ;
        }
    }
    out << "]";
    return ret;
}

void CompilationUnit::traverseChildren(Traverser *tv)
{
    for(int i=0; i<usedModules.count(); ++i)
    {
        if(usedModules[i])
        {
            usedModules[i]->traverse(usedModules[i], tv);
        }

    }

}

Program::Program(Token _pos,
                 Token _endingpos,
                 QVector<shared_ptr<StrLiteral> > usedModules,
                 QVector<shared_ptr<TopLevel> > elements,
                 QVector<shared_ptr<TopLevel> > originalElements):
    CompilationUnit(_pos,_endingpos,usedModules),elements(elements),
    originalElements(originalElements)
{
}

QString Program::toString()
{
    QString ret;
    QTextStream out(&ret);
    out << _ws(L"برنامج(");
    out << CompilationUnit::childrenToString();
    out << ", " << childrenToString();
    out <<")";
    return ret;
}

QString Program::childrenToString()
{
    QString ret;
    QTextStream out(&ret);
    out << "[";
    for(int i=0; i<elements.count(); i++)
    {
        if(elements[i])
        {
            out << elements[i]->toString() << ", " ;
        }
    }
    out << "]";
    return ret;
}

void Program::traverse(shared_ptr<PrettyPrintable> p, Traverser *tv)
{
    tv->visit(p);
    CompilationUnit::traverseChildren(tv);
    this->traverseChildren(tv);
    tv->exit(p);
}
void Program::traverseChildren(Traverser *tv)
{
    for(int i=0; i<elements.count(); ++i)
    {
        if(elements[i])
        {
            elements[i]->traverse(elements[i], tv);
        }

    }

    for(int i=0; i<originalElements.count(); ++i)
    {
        if(originalElements[i])
        {
            originalElements[i]->traverse(originalElements[i], tv);
        }

    }

}

Module::Module(Token _pos,
               Token _endingpos,
               QVector<shared_ptr<StrLiteral> > usedModules,
               shared_ptr<Identifier> _name,
               QVector<shared_ptr<Declaration> > declarations):
    CompilationUnit(_pos,_endingpos,usedModules),_name(_name),
    declarations(declarations)
{
}

QString Module::toString()
{
    QString ret;
    QTextStream out(&ret);
    out << _ws(L"وحدة.برمجية(");
    out << CompilationUnit::childrenToString();
    out << ", " << childrenToString();
    out <<")";
    return ret;
}

QString Module::childrenToString()
{
    QString ret;
    QTextStream out(&ret);
    if(_name)
    {
        out << name()->toString() << ", " ;
    }
    out << "[";
    for(int i=0; i<declarations.count(); i++)
    {
        if(declarations[i])
        {
            out << declarations[i]->toString() << ", " ;
        }
    }
    out << "]";
    return ret;
}

void Module::traverse(shared_ptr<PrettyPrintable> p, Traverser *tv)
{
    tv->visit(p);
    CompilationUnit::traverseChildren(tv);
    this->traverseChildren(tv);
    tv->exit(p);
}
void Module::traverseChildren(Traverser *tv)
{
    if(_name)
    {
        _name->traverse(_name, tv);
    }

    for(int i=0; i<declarations.count(); ++i)
    {
        if(declarations[i])
        {
            declarations[i]->traverse(declarations[i], tv);
        }

    }

}

AssignableExpression::AssignableExpression(Token _pos,
                                           Token _endingpos):
    Expression(_pos,_endingpos)
{
}


QString AssignableExpression::childrenToString()
{
    QString ret;
    QTextStream out(&ret);
    return ret;
}

void AssignableExpression::traverseChildren(Traverser *tv)
{
}

Literal::Literal(Token _pos,
                 Token _endingpos):
    Expression(_pos,_endingpos)
{
}


QString Literal::childrenToString()
{
    QString ret;
    QTextStream out(&ret);
    return ret;
}

void Literal::traverseChildren(Traverser *tv)
{
}

SimpleLiteral::SimpleLiteral(Token _pos,
                             Token _endingpos):
    Literal(_pos,_endingpos)
{
}


QString SimpleLiteral::childrenToString()
{
    QString ret;
    QTextStream out(&ret);
    return ret;
}

void SimpleLiteral::traverseChildren(Traverser *tv)
{
}

AssignmentStmt::AssignmentStmt(Token _pos,
                               Token _endingpos,
                               shared_ptr<AssignableExpression> _variable,
                               shared_ptr<Expression> _value,
                               shared_ptr<TypeExpression> _type):
    Statement(_pos,_endingpos),_variable(_variable),
    _value(_value),
    _type(_type)
{
}

QString AssignmentStmt::toString()
{
    QString ret;
    QTextStream out(&ret);
    out << _ws(L"أمر.تخصيص(");
    out << Statement::childrenToString();
    out << ", " << childrenToString();
    out <<")";
    return ret;
}

QString AssignmentStmt::childrenToString()
{
    QString ret;
    QTextStream out(&ret);
    if(_variable)
    {
        out << variable()->toString() << ", " ;
    }
    if(_value)
    {
        out << value()->toString() << ", " ;
    }
    if(_type)
    {
        out << type()->toString() << ", " ;
    }
    return ret;
}

void AssignmentStmt::traverse(shared_ptr<PrettyPrintable> p, Traverser *tv)
{
    tv->visit(p);
    Statement::traverseChildren(tv);
    this->traverseChildren(tv);
    tv->exit(p);
}
void AssignmentStmt::traverseChildren(Traverser *tv)
{
    if(_variable)
    {
        _variable->traverse(_variable, tv);
    }

    if(_value)
    {
        _value->traverse(_value, tv);
    }

    if(_type)
    {
        _type->traverse(_type, tv);
    }

}

IfStmt::IfStmt(Token _pos,
               Token _endingpos,
               shared_ptr<Expression> _condition,
               shared_ptr<Statement> _thenPart,
               shared_ptr<Statement> _elsePart):
    Statement(_pos,_endingpos),_condition(_condition),
    _thenPart(_thenPart),
    _elsePart(_elsePart)
{
}

QString IfStmt::toString()
{
    QString ret;
    QTextStream out(&ret);
    out << _ws(L"أمر.إذا(");
    out << Statement::childrenToString();
    out << ", " << childrenToString();
    out <<")";
    return ret;
}

QString IfStmt::childrenToString()
{
    QString ret;
    QTextStream out(&ret);
    if(_condition)
    {
        out << condition()->toString() << ", " ;
    }
    if(_thenPart)
    {
        out << thenPart()->toString() << ", " ;
    }
    if(_elsePart)
    {
        out << elsePart()->toString() << ", " ;
    }
    return ret;
}

void IfStmt::traverse(shared_ptr<PrettyPrintable> p, Traverser *tv)
{
    tv->visit(p);
    Statement::traverseChildren(tv);
    this->traverseChildren(tv);
    tv->exit(p);
}
void IfStmt::traverseChildren(Traverser *tv)
{
    if(_condition)
    {
        _condition->traverse(_condition, tv);
    }

    if(_thenPart)
    {
        _thenPart->traverse(_thenPart, tv);
    }

    if(_elsePart)
    {
        _elsePart->traverse(_elsePart, tv);
    }

}

WhileStmt::WhileStmt(Token _pos,
                     Token _endingpos,
                     shared_ptr<Expression> _condition,
                     shared_ptr<Statement> _statement):
    Statement(_pos,_endingpos),_condition(_condition),
    _statement(_statement)
{
}

QString WhileStmt::toString()
{
    QString ret;
    QTextStream out(&ret);
    out << _ws(L"أمر.كرر(");
    out << Statement::childrenToString();
    out << ", " << childrenToString();
    out <<")";
    return ret;
}

QString WhileStmt::childrenToString()
{
    QString ret;
    QTextStream out(&ret);
    if(_condition)
    {
        out << condition()->toString() << ", " ;
    }
    if(_statement)
    {
        out << statement()->toString() << ", " ;
    }
    return ret;
}

void WhileStmt::traverse(shared_ptr<PrettyPrintable> p, Traverser *tv)
{
    tv->visit(p);
    Statement::traverseChildren(tv);
    this->traverseChildren(tv);
    tv->exit(p);
}
void WhileStmt::traverseChildren(Traverser *tv)
{
    if(_condition)
    {
        _condition->traverse(_condition, tv);
    }

    if(_statement)
    {
        _statement->traverse(_statement, tv);
    }

}

ForAllStmt::ForAllStmt(Token _pos,
                       Token _endingpos,
                       shared_ptr<Identifier> _variable,
                       shared_ptr<Expression> _from,
                       shared_ptr<Expression> _to,
                       shared_ptr<Expression> _step,
                       shared_ptr<Statement> _statement,
                       bool _downTo):
    Statement(_pos,_endingpos),_variable(_variable),
    _from(_from),
    _to(_to),
    _step(_step),
    _statement(_statement),
    _downTo(_downTo)
{
}

QString ForAllStmt::toString()
{
    QString ret;
    QTextStream out(&ret);
    out << _ws(L"أمر.لكل(");
    out << Statement::childrenToString();
    out << ", " << childrenToString();
    out <<")";
    return ret;
}

QString ForAllStmt::childrenToString()
{
    QString ret;
    QTextStream out(&ret);
    if(_variable)
    {
        out << variable()->toString() << ", " ;
    }
    if(_from)
    {
        out << from()->toString() << ", " ;
    }
    if(_to)
    {
        out << to()->toString() << ", " ;
    }
    if(_step)
    {
        out << step()->toString() << ", " ;
    }
    if(_statement)
    {
        out << statement()->toString() << ", " ;
    }
    out << _downTo << ", " ;
    return ret;
}

void ForAllStmt::traverse(shared_ptr<PrettyPrintable> p, Traverser *tv)
{
    tv->visit(p);
    Statement::traverseChildren(tv);
    this->traverseChildren(tv);
    tv->exit(p);
}
void ForAllStmt::traverseChildren(Traverser *tv)
{
    if(_variable)
    {
        _variable->traverse(_variable, tv);
    }

    if(_from)
    {
        _from->traverse(_from, tv);
    }

    if(_to)
    {
        _to->traverse(_to, tv);
    }

    if(_step)
    {
        _step->traverse(_step, tv);
    }

    if(_statement)
    {
        _statement->traverse(_statement, tv);
    }

}

ForEachStmt::ForEachStmt(Token _pos,
                         Token _endingpos,
                         shared_ptr<Identifier> _variable,
                         shared_ptr<Expression> _enumerable,
                         shared_ptr<Statement> _statement):
    Statement(_pos,_endingpos),_variable(_variable),
    _enumerable(_enumerable),
    _statement(_statement)
{
}

QString ForEachStmt::toString()
{
    QString ret;
    QTextStream out(&ret);
    out << _ws(L"أمر.لكل.في(");
    out << Statement::childrenToString();
    out << ", " << childrenToString();
    out <<")";
    return ret;
}

QString ForEachStmt::childrenToString()
{
    QString ret;
    QTextStream out(&ret);
    if(_variable)
    {
        out << variable()->toString() << ", " ;
    }
    if(_enumerable)
    {
        out << enumerable()->toString() << ", " ;
    }
    if(_statement)
    {
        out << statement()->toString() << ", " ;
    }
    return ret;
}

void ForEachStmt::traverse(shared_ptr<PrettyPrintable> p, Traverser *tv)
{
    tv->visit(p);
    Statement::traverseChildren(tv);
    this->traverseChildren(tv);
    tv->exit(p);
}
void ForEachStmt::traverseChildren(Traverser *tv)
{
    if(_variable)
    {
        _variable->traverse(_variable, tv);
    }

    if(_enumerable)
    {
        _enumerable->traverse(_enumerable, tv);
    }

    if(_statement)
    {
        _statement->traverse(_statement, tv);
    }

}

ReturnStmt::ReturnStmt(Token _pos,
                       Token _endingpos,
                       shared_ptr<Expression> _returnVal):
    Statement(_pos,_endingpos),_returnVal(_returnVal)
{
}

QString ReturnStmt::toString()
{
    QString ret;
    QTextStream out(&ret);
    out << _ws(L"أمر.ارجع(");
    out << Statement::childrenToString();
    out << ", " << childrenToString();
    out <<")";
    return ret;
}

QString ReturnStmt::childrenToString()
{
    QString ret;
    QTextStream out(&ret);
    if(_returnVal)
    {
        out << returnVal()->toString() << ", " ;
    }
    return ret;
}

void ReturnStmt::traverse(shared_ptr<PrettyPrintable> p, Traverser *tv)
{
    tv->visit(p);
    Statement::traverseChildren(tv);
    this->traverseChildren(tv);
    tv->exit(p);
}
void ReturnStmt::traverseChildren(Traverser *tv)
{
    if(_returnVal)
    {
        _returnVal->traverse(_returnVal, tv);
    }

}

DelegationStmt::DelegationStmt(Token _pos,
                               Token _endingpos,
                               shared_ptr<IInvokation> _invokation):
    Statement(_pos,_endingpos),_invokation(_invokation)
{
}

QString DelegationStmt::toString()
{
    QString ret;
    QTextStream out(&ret);
    out << _ws(L"أمر.وكل(");
    out << Statement::childrenToString();
    out << ", " << childrenToString();
    out <<")";
    return ret;
}

QString DelegationStmt::childrenToString()
{
    QString ret;
    QTextStream out(&ret);
    if(_invokation)
    {
        out << invokation()->toString() << ", " ;
    }
    return ret;
}

void DelegationStmt::traverse(shared_ptr<PrettyPrintable> p, Traverser *tv)
{
    tv->visit(p);
    Statement::traverseChildren(tv);
    this->traverseChildren(tv);
    tv->exit(p);
}
void DelegationStmt::traverseChildren(Traverser *tv)
{
    if(_invokation)
    {
        _invokation->traverse(_invokation, tv);
    }

}

LaunchStmt::LaunchStmt(Token _pos,
                       Token _endingpos,
                       shared_ptr<IInvokation> _invokation):
    Statement(_pos,_endingpos),_invokation(_invokation)
{
}

QString LaunchStmt::toString()
{
    QString ret;
    QTextStream out(&ret);
    out << _ws(L"أمر.شغل(");
    out << Statement::childrenToString();
    out << ", " << childrenToString();
    out <<")";
    return ret;
}

QString LaunchStmt::childrenToString()
{
    QString ret;
    QTextStream out(&ret);
    if(_invokation)
    {
        out << invokation()->toString() << ", " ;
    }
    return ret;
}

void LaunchStmt::traverse(shared_ptr<PrettyPrintable> p, Traverser *tv)
{
    tv->visit(p);
    Statement::traverseChildren(tv);
    this->traverseChildren(tv);
    tv->exit(p);
}
void LaunchStmt::traverseChildren(Traverser *tv)
{
    if(_invokation)
    {
        _invokation->traverse(_invokation, tv);
    }

}

LabelStmt::LabelStmt(Token _pos,
                     Token _endingpos,
                     shared_ptr<Expression> _target):
    Statement(_pos,_endingpos),_target(_target)
{
}

QString LabelStmt::toString()
{
    QString ret;
    QTextStream out(&ret);
    out << _ws(L"أمر.علامة(");
    out << Statement::childrenToString();
    out << ", " << childrenToString();
    out <<")";
    return ret;
}

QString LabelStmt::childrenToString()
{
    QString ret;
    QTextStream out(&ret);
    if(_target)
    {
        out << target()->toString() << ", " ;
    }
    return ret;
}

void LabelStmt::traverse(shared_ptr<PrettyPrintable> p, Traverser *tv)
{
    tv->visit(p);
    Statement::traverseChildren(tv);
    this->traverseChildren(tv);
    tv->exit(p);
}
void LabelStmt::traverseChildren(Traverser *tv)
{
    if(_target)
    {
        _target->traverse(_target, tv);
    }

}

GotoStmt::GotoStmt(Token _pos,
                   Token _endingpos,
                   shared_ptr<Expression> _target):
    Statement(_pos,_endingpos),_target(_target)
{
}

QString GotoStmt::toString()
{
    QString ret;
    QTextStream out(&ret);
    out << _ws(L"أمر.اذهب(");
    out << Statement::childrenToString();
    out << ", " << childrenToString();
    out <<")";
    return ret;
}

QString GotoStmt::childrenToString()
{
    QString ret;
    QTextStream out(&ret);
    if(_target)
    {
        out << target()->toString() << ", " ;
    }
    return ret;
}

void GotoStmt::traverse(shared_ptr<PrettyPrintable> p, Traverser *tv)
{
    tv->visit(p);
    Statement::traverseChildren(tv);
    this->traverseChildren(tv);
    tv->exit(p);
}
void GotoStmt::traverseChildren(Traverser *tv)
{
    if(_target)
    {
        _target->traverse(_target, tv);
    }

}

PrintStmt::PrintStmt(Token _pos,
                     Token _endingpos,
                     shared_ptr<Expression> _fileObject,
                     QVector<shared_ptr<Expression> > args,
                     QVector<shared_ptr<Expression> > widths,
                     bool _printOnSameLine):
    IOStatement(_pos,_endingpos),_fileObject(_fileObject),
    args(args),
    widths(widths),
    _printOnSameLine(_printOnSameLine)
{
}

QString PrintStmt::toString()
{
    QString ret;
    QTextStream out(&ret);
    out << _ws(L"أمر.اطبع(");
    out << IOStatement::childrenToString();
    out << ", " << childrenToString();
    out <<")";
    return ret;
}

QString PrintStmt::childrenToString()
{
    QString ret;
    QTextStream out(&ret);
    if(_fileObject)
    {
        out << fileObject()->toString() << ", " ;
    }
    out << "[";
    for(int i=0; i<args.count(); i++)
    {
        if(args[i])
        {
            out << args[i]->toString() << ", " ;
        }
    }
    out << "]";
    out << "[";
    for(int i=0; i<widths.count(); i++)
    {
        if(widths[i])
        {
            out << widths[i]->toString() << ", " ;
        }
    }
    out << "]";
    out << _printOnSameLine << ", " ;
    return ret;
}

void PrintStmt::traverse(shared_ptr<PrettyPrintable> p, Traverser *tv)
{
    tv->visit(p);
    IOStatement::traverseChildren(tv);
    this->traverseChildren(tv);
    tv->exit(p);
}
void PrintStmt::traverseChildren(Traverser *tv)
{
    if(_fileObject)
    {
        _fileObject->traverse(_fileObject, tv);
    }

    for(int i=0; i<args.count(); ++i)
    {
        if(args[i])
        {
            args[i]->traverse(args[i], tv);
        }

    }

    for(int i=0; i<widths.count(); ++i)
    {
        if(widths[i])
        {
            widths[i]->traverse(widths[i], tv);
        }

    }

}

ReadStmt::ReadStmt(Token _pos,
                   Token _endingpos,
                   shared_ptr<Expression> _fileObject,
                   QString _prompt,
                   QVector<shared_ptr<AssignableExpression> > variables,
                   QVector<bool > readNumberFlags):
    IOStatement(_pos,_endingpos),_fileObject(_fileObject),
    _prompt(_prompt),
    variables(variables),
    readNumberFlags(readNumberFlags)
{
}

QString ReadStmt::toString()
{
    QString ret;
    QTextStream out(&ret);
    out << _ws(L"أمر.لكل(");
    out << IOStatement::childrenToString();
    out << ", " << childrenToString();
    out <<")";
    return ret;
}

QString ReadStmt::childrenToString()
{
    QString ret;
    QTextStream out(&ret);
    if(_fileObject)
    {
        out << fileObject()->toString() << ", " ;
    }
    out << _prompt << ", " ;
    out << "[";
    for(int i=0; i<variables.count(); i++)
    {
        if(variables[i])
        {
            out << variables[i]->toString() << ", " ;
        }
    }
    out << "]";
    out << "[";
    for(int i=0; i<readNumberFlags.count(); i++)
    {
        out << readNumberFlags[i] << ", " ;
    }
    out << "]";
    return ret;
}

void ReadStmt::traverse(shared_ptr<PrettyPrintable> p, Traverser *tv)
{
    tv->visit(p);
    IOStatement::traverseChildren(tv);
    this->traverseChildren(tv);
    tv->exit(p);
}
void ReadStmt::traverseChildren(Traverser *tv)
{
    if(_fileObject)
    {
        _fileObject->traverse(_fileObject, tv);
    }

    for(int i=0; i<variables.count(); ++i)
    {
        if(variables[i])
        {
            variables[i]->traverse(variables[i], tv);
        }

    }

}

DrawPixelStmt::DrawPixelStmt(Token _pos,
                             Token _endingpos,
                             shared_ptr<Expression> _x,
                             shared_ptr<Expression> _y,
                             shared_ptr<Expression> _color):
    GraphicsStatement(_pos,_endingpos),_x(_x),
    _y(_y),
    _color(_color)
{
}

QString DrawPixelStmt::toString()
{
    QString ret;
    QTextStream out(&ret);
    out << _ws(L"أمر.ارسم.نقطة(");
    out << GraphicsStatement::childrenToString();
    out << ", " << childrenToString();
    out <<")";
    return ret;
}

QString DrawPixelStmt::childrenToString()
{
    QString ret;
    QTextStream out(&ret);
    if(_x)
    {
        out << x()->toString() << ", " ;
    }
    if(_y)
    {
        out << y()->toString() << ", " ;
    }
    if(_color)
    {
        out << color()->toString() << ", " ;
    }
    return ret;
}

void DrawPixelStmt::traverse(shared_ptr<PrettyPrintable> p, Traverser *tv)
{
    tv->visit(p);
    GraphicsStatement::traverseChildren(tv);
    this->traverseChildren(tv);
    tv->exit(p);
}
void DrawPixelStmt::traverseChildren(Traverser *tv)
{
    if(_x)
    {
        _x->traverse(_x, tv);
    }

    if(_y)
    {
        _y->traverse(_y, tv);
    }

    if(_color)
    {
        _color->traverse(_color, tv);
    }

}

DrawLineStmt::DrawLineStmt(Token _pos,
                           Token _endingpos,
                           shared_ptr<Expression> _x1,
                           shared_ptr<Expression> _y1,
                           shared_ptr<Expression> _x2,
                           shared_ptr<Expression> _y2,
                           shared_ptr<Expression> _color):
    GraphicsStatement(_pos,_endingpos),_x1(_x1),
    _y1(_y1),
    _x2(_x2),
    _y2(_y2),
    _color(_color)
{
}

QString DrawLineStmt::toString()
{
    QString ret;
    QTextStream out(&ret);
    out << _ws(L"أمر.ارسم.خط(");
    out << GraphicsStatement::childrenToString();
    out << ", " << childrenToString();
    out <<")";
    return ret;
}

QString DrawLineStmt::childrenToString()
{
    QString ret;
    QTextStream out(&ret);
    if(_x1)
    {
        out << x1()->toString() << ", " ;
    }
    if(_y1)
    {
        out << y1()->toString() << ", " ;
    }
    if(_x2)
    {
        out << x2()->toString() << ", " ;
    }
    if(_y2)
    {
        out << y2()->toString() << ", " ;
    }
    if(_color)
    {
        out << color()->toString() << ", " ;
    }
    return ret;
}

void DrawLineStmt::traverse(shared_ptr<PrettyPrintable> p, Traverser *tv)
{
    tv->visit(p);
    GraphicsStatement::traverseChildren(tv);
    this->traverseChildren(tv);
    tv->exit(p);
}
void DrawLineStmt::traverseChildren(Traverser *tv)
{
    if(_x1)
    {
        _x1->traverse(_x1, tv);
    }

    if(_y1)
    {
        _y1->traverse(_y1, tv);
    }

    if(_x2)
    {
        _x2->traverse(_x2, tv);
    }

    if(_y2)
    {
        _y2->traverse(_y2, tv);
    }

    if(_color)
    {
        _color->traverse(_color, tv);
    }

}

DrawRectStmt::DrawRectStmt(Token _pos,
                           Token _endingpos,
                           shared_ptr<Expression> _x1,
                           shared_ptr<Expression> _y1,
                           shared_ptr<Expression> _x2,
                           shared_ptr<Expression> _y2,
                           shared_ptr<Expression> _color,
                           shared_ptr<Expression> _filled):
    GraphicsStatement(_pos,_endingpos),_x1(_x1),
    _y1(_y1),
    _x2(_x2),
    _y2(_y2),
    _color(_color),
    _filled(_filled)
{
}

QString DrawRectStmt::toString()
{
    QString ret;
    QTextStream out(&ret);
    out << _ws(L"أمر.ارسم.مستطيل(");
    out << GraphicsStatement::childrenToString();
    out << ", " << childrenToString();
    out <<")";
    return ret;
}

QString DrawRectStmt::childrenToString()
{
    QString ret;
    QTextStream out(&ret);
    if(_x1)
    {
        out << x1()->toString() << ", " ;
    }
    if(_y1)
    {
        out << y1()->toString() << ", " ;
    }
    if(_x2)
    {
        out << x2()->toString() << ", " ;
    }
    if(_y2)
    {
        out << y2()->toString() << ", " ;
    }
    if(_color)
    {
        out << color()->toString() << ", " ;
    }
    if(_filled)
    {
        out << filled()->toString() << ", " ;
    }
    return ret;
}

void DrawRectStmt::traverse(shared_ptr<PrettyPrintable> p, Traverser *tv)
{
    tv->visit(p);
    GraphicsStatement::traverseChildren(tv);
    this->traverseChildren(tv);
    tv->exit(p);
}
void DrawRectStmt::traverseChildren(Traverser *tv)
{
    if(_x1)
    {
        _x1->traverse(_x1, tv);
    }

    if(_y1)
    {
        _y1->traverse(_y1, tv);
    }

    if(_x2)
    {
        _x2->traverse(_x2, tv);
    }

    if(_y2)
    {
        _y2->traverse(_y2, tv);
    }

    if(_color)
    {
        _color->traverse(_color, tv);
    }

    if(_filled)
    {
        _filled->traverse(_filled, tv);
    }

}

DrawCircleStmt::DrawCircleStmt(Token _pos,
                               Token _endingpos,
                               shared_ptr<Expression> _cx,
                               shared_ptr<Expression> _cy,
                               shared_ptr<Expression> _radius,
                               shared_ptr<Expression> _color,
                               shared_ptr<Expression> _filled):
    GraphicsStatement(_pos,_endingpos),_cx(_cx),
    _cy(_cy),
    _radius(_radius),
    _color(_color),
    _filled(_filled)
{
}

QString DrawCircleStmt::toString()
{
    QString ret;
    QTextStream out(&ret);
    out << _ws(L"أمر.ارسم.دائرة(");
    out << GraphicsStatement::childrenToString();
    out << ", " << childrenToString();
    out <<")";
    return ret;
}

QString DrawCircleStmt::childrenToString()
{
    QString ret;
    QTextStream out(&ret);
    if(_cx)
    {
        out << cx()->toString() << ", " ;
    }
    if(_cy)
    {
        out << cy()->toString() << ", " ;
    }
    if(_radius)
    {
        out << radius()->toString() << ", " ;
    }
    if(_color)
    {
        out << color()->toString() << ", " ;
    }
    if(_filled)
    {
        out << filled()->toString() << ", " ;
    }
    return ret;
}

void DrawCircleStmt::traverse(shared_ptr<PrettyPrintable> p, Traverser *tv)
{
    tv->visit(p);
    GraphicsStatement::traverseChildren(tv);
    this->traverseChildren(tv);
    tv->exit(p);
}
void DrawCircleStmt::traverseChildren(Traverser *tv)
{
    if(_cx)
    {
        _cx->traverse(_cx, tv);
    }

    if(_cy)
    {
        _cy->traverse(_cy, tv);
    }

    if(_radius)
    {
        _radius->traverse(_radius, tv);
    }

    if(_color)
    {
        _color->traverse(_color, tv);
    }

    if(_filled)
    {
        _filled->traverse(_filled, tv);
    }

}

DrawImageStmt::DrawImageStmt(Token _pos,
                             Token _endingpos,
                             shared_ptr<Expression> _x,
                             shared_ptr<Expression> _y,
                             shared_ptr<Expression> _image):
    GraphicsStatement(_pos,_endingpos),_x(_x),
    _y(_y),
    _image(_image)
{
}

QString DrawImageStmt::toString()
{
    QString ret;
    QTextStream out(&ret);
    out << _ws(L"أمر.ارسم.صورة(");
    out << GraphicsStatement::childrenToString();
    out << ", " << childrenToString();
    out <<")";
    return ret;
}

QString DrawImageStmt::childrenToString()
{
    QString ret;
    QTextStream out(&ret);
    if(_x)
    {
        out << x()->toString() << ", " ;
    }
    if(_y)
    {
        out << y()->toString() << ", " ;
    }
    if(_image)
    {
        out << image()->toString() << ", " ;
    }
    return ret;
}

void DrawImageStmt::traverse(shared_ptr<PrettyPrintable> p, Traverser *tv)
{
    tv->visit(p);
    GraphicsStatement::traverseChildren(tv);
    this->traverseChildren(tv);
    tv->exit(p);
}
void DrawImageStmt::traverseChildren(Traverser *tv)
{
    if(_x)
    {
        _x->traverse(_x, tv);
    }

    if(_y)
    {
        _y->traverse(_y, tv);
    }

    if(_image)
    {
        _image->traverse(_image, tv);
    }

}

DrawSpriteStmt::DrawSpriteStmt(Token _pos,
                               Token _endingpos,
                               shared_ptr<Expression> _x,
                               shared_ptr<Expression> _y,
                               shared_ptr<Expression> _sprite):
    GraphicsStatement(_pos,_endingpos),_x(_x),
    _y(_y),
    _sprite(_sprite)
{
}

QString DrawSpriteStmt::toString()
{
    QString ret;
    QTextStream out(&ret);
    out << _ws(L"أمر.ارسم.طيف(");
    out << GraphicsStatement::childrenToString();
    out << ", " << childrenToString();
    out <<")";
    return ret;
}

QString DrawSpriteStmt::childrenToString()
{
    QString ret;
    QTextStream out(&ret);
    if(_x)
    {
        out << x()->toString() << ", " ;
    }
    if(_y)
    {
        out << y()->toString() << ", " ;
    }
    if(_sprite)
    {
        out << sprite()->toString() << ", " ;
    }
    return ret;
}

void DrawSpriteStmt::traverse(shared_ptr<PrettyPrintable> p, Traverser *tv)
{
    tv->visit(p);
    GraphicsStatement::traverseChildren(tv);
    this->traverseChildren(tv);
    tv->exit(p);
}
void DrawSpriteStmt::traverseChildren(Traverser *tv)
{
    if(_x)
    {
        _x->traverse(_x, tv);
    }

    if(_y)
    {
        _y->traverse(_y, tv);
    }

    if(_sprite)
    {
        _sprite->traverse(_sprite, tv);
    }

}

ZoomStmt::ZoomStmt(Token _pos,
                   Token _endingpos,
                   shared_ptr<Expression> _x1,
                   shared_ptr<Expression> _y1,
                   shared_ptr<Expression> _x2,
                   shared_ptr<Expression> _y2):
    GraphicsStatement(_pos,_endingpos),_x1(_x1),
    _y1(_y1),
    _x2(_x2),
    _y2(_y2)
{
}

QString ZoomStmt::toString()
{
    QString ret;
    QTextStream out(&ret);
    out << _ws(L"أمر.زووم(");
    out << GraphicsStatement::childrenToString();
    out << ", " << childrenToString();
    out <<")";
    return ret;
}

QString ZoomStmt::childrenToString()
{
    QString ret;
    QTextStream out(&ret);
    if(_x1)
    {
        out << x1()->toString() << ", " ;
    }
    if(_y1)
    {
        out << y1()->toString() << ", " ;
    }
    if(_x2)
    {
        out << x2()->toString() << ", " ;
    }
    if(_y2)
    {
        out << y2()->toString() << ", " ;
    }
    return ret;
}

void ZoomStmt::traverse(shared_ptr<PrettyPrintable> p, Traverser *tv)
{
    tv->visit(p);
    GraphicsStatement::traverseChildren(tv);
    this->traverseChildren(tv);
    tv->exit(p);
}
void ZoomStmt::traverseChildren(Traverser *tv)
{
    if(_x1)
    {
        _x1->traverse(_x1, tv);
    }

    if(_y1)
    {
        _y1->traverse(_y1, tv);
    }

    if(_x2)
    {
        _x2->traverse(_x2, tv);
    }

    if(_y2)
    {
        _y2->traverse(_y2, tv);
    }

}


EventStatement::EventStatement(Token _pos,
                               Token _endingpos,
                               EventType _type,
                               shared_ptr<Identifier> _handler):
    Statement(_pos,_endingpos),_type(_type),
    _handler(_handler)
{
}

QString EventStatement::toString()
{
    QString ret;
    QTextStream out(&ret);
    out << _ws(L"أمر.عند.حادثة(");
    out << Statement::childrenToString();
    out << ", " << childrenToString();
    out <<")";
    return ret;
}

QString EventStatement::childrenToString()
{
    QString ret;
    QTextStream out(&ret);
    out << _type << ", " ;
    if(_handler)
    {
        out << handler()->toString() << ", " ;
    }
    return ret;
}

void EventStatement::traverse(shared_ptr<PrettyPrintable> p, Traverser *tv)
{
    tv->visit(p);
    Statement::traverseChildren(tv);
    this->traverseChildren(tv);
    tv->exit(p);
}
void EventStatement::traverseChildren(Traverser *tv)
{
    if(_handler)
    {
        _handler->traverse(_handler, tv);
    }

}

SendStmt::SendStmt(Token _pos,
                   Token _endingpos,
                   shared_ptr<Expression> _value,
                   shared_ptr<Expression> _channel,
                   bool _signal):
    ChannelCommunicationStmt(_pos,_endingpos),_value(_value),
    _channel(_channel),
    _signal(_signal)
{
}

QString SendStmt::toString()
{
    QString ret;
    QTextStream out(&ret);
    out << _ws(L"أمر.ارسل(");
    out << ChannelCommunicationStmt::childrenToString();
    out << ", " << childrenToString();
    out <<")";
    return ret;
}

QString SendStmt::childrenToString()
{
    QString ret;
    QTextStream out(&ret);
    if(_value)
    {
        out << value()->toString() << ", " ;
    }
    if(_channel)
    {
        out << channel()->toString() << ", " ;
    }
    out << _signal << ", " ;
    return ret;
}

void SendStmt::traverse(shared_ptr<PrettyPrintable> p, Traverser *tv)
{
    tv->visit(p);
    ChannelCommunicationStmt::traverseChildren(tv);
    this->traverseChildren(tv);
    tv->exit(p);
}
void SendStmt::traverseChildren(Traverser *tv)
{
    if(_value)
    {
        _value->traverse(_value, tv);
    }

    if(_channel)
    {
        _channel->traverse(_channel, tv);
    }

}

ReceiveStmt::ReceiveStmt(Token _pos,
                         Token _endingpos,
                         shared_ptr<AssignableExpression> _value,
                         shared_ptr<Expression> _channel,
                         bool _signal):
    ChannelCommunicationStmt(_pos,_endingpos),_value(_value),
    _channel(_channel),
    _signal(_signal)
{
}

QString ReceiveStmt::toString()
{
    QString ret;
    QTextStream out(&ret);
    out << _ws(L"أمر.تسلم(");
    out << ChannelCommunicationStmt::childrenToString();
    out << ", " << childrenToString();
    out <<")";
    return ret;
}

QString ReceiveStmt::childrenToString()
{
    QString ret;
    QTextStream out(&ret);
    if(_value)
    {
        out << value()->toString() << ", " ;
    }
    if(_channel)
    {
        out << channel()->toString() << ", " ;
    }
    out << _signal << ", " ;
    return ret;
}

void ReceiveStmt::traverse(shared_ptr<PrettyPrintable> p, Traverser *tv)
{
    tv->visit(p);
    ChannelCommunicationStmt::traverseChildren(tv);
    this->traverseChildren(tv);
    tv->exit(p);
}
void ReceiveStmt::traverseChildren(Traverser *tv)
{
    if(_value)
    {
        _value->traverse(_value, tv);
    }

    if(_channel)
    {
        _channel->traverse(_channel, tv);
    }

}

SelectStmt::SelectStmt(Token _pos,
                       Token _endingpos,
                       QVector<shared_ptr<ChannelCommunicationStmt> > conditions,
                       QVector<shared_ptr<Statement> > actions):
    Statement(_pos,_endingpos),conditions(conditions),
    actions(actions)
{
}

QString SelectStmt::toString()
{
    QString ret;
    QTextStream out(&ret);
    out << _ws(L"أمر.تخير(");
    out << Statement::childrenToString();
    out << ", " << childrenToString();
    out <<")";
    return ret;
}

QString SelectStmt::childrenToString()
{
    QString ret;
    QTextStream out(&ret);
    out << "[";
    for(int i=0; i<conditions.count(); i++)
    {
        if(conditions[i])
        {
            out << conditions[i]->toString() << ", " ;
        }
    }
    out << "]";
    out << "[";
    for(int i=0; i<actions.count(); i++)
    {
        if(actions[i])
        {
            out << actions[i]->toString() << ", " ;
        }
    }
    out << "]";
    return ret;
}

void SelectStmt::traverse(shared_ptr<PrettyPrintable> p, Traverser *tv)
{
    tv->visit(p);
    Statement::traverseChildren(tv);
    this->traverseChildren(tv);
    tv->exit(p);
}
void SelectStmt::traverseChildren(Traverser *tv)
{
    for(int i=0; i<conditions.count(); ++i)
    {
        if(conditions[i])
        {
            conditions[i]->traverse(conditions[i], tv);
        }

    }

    for(int i=0; i<actions.count(); ++i)
    {
        if(actions[i])
        {
            actions[i]->traverse(actions[i], tv);
        }

    }

}

BlockStmt::BlockStmt(Token _pos,
                     Token _endingpos,
                     QVector<shared_ptr<Statement> > _statements):
    Statement(_pos,_endingpos),_statements(_statements)
{
}

QString BlockStmt::toString()
{
    QString ret;
    QTextStream out(&ret);
    out << _ws(L"كتلة.أوامر(");
    out << Statement::childrenToString();
    out << ", " << childrenToString();
    out <<")";
    return ret;
}

QString BlockStmt::childrenToString()
{
    QString ret;
    QTextStream out(&ret);
    out << "[";
    for(int i=0; i<_statements.count(); i++)
    {
        if(_statements[i])
        {
            out << _statements[i]->toString() << ", " ;
        }
    }
    out << "]";
    return ret;
}

void BlockStmt::traverse(shared_ptr<PrettyPrintable> p, Traverser *tv)
{
    tv->visit(p);
    Statement::traverseChildren(tv);
    this->traverseChildren(tv);
    tv->exit(p);
}
void BlockStmt::traverseChildren(Traverser *tv)
{
    for(int i=0; i<_statements.count(); ++i)
    {
        if(_statements[i])
        {
            _statements[i]->traverse(_statements[i], tv);
        }

    }

}

InvokationStmt::InvokationStmt(Token _pos,
                               Token _endingpos,
                               shared_ptr<Expression> _expression):
    Statement(_pos,_endingpos),_expression(_expression)
{
}

QString InvokationStmt::toString()
{
    QString ret;
    QTextStream out(&ret);
    out << _ws(L"أمر.استدعائي(");
    out << Statement::childrenToString();
    out << ", " << childrenToString();
    out <<")";
    return ret;
}

QString InvokationStmt::childrenToString()
{
    QString ret;
    QTextStream out(&ret);
    if(_expression)
    {
        out << expression()->toString() << ", " ;
    }
    return ret;
}

void InvokationStmt::traverse(shared_ptr<PrettyPrintable> p, Traverser *tv)
{
    tv->visit(p);
    Statement::traverseChildren(tv);
    this->traverseChildren(tv);
    tv->exit(p);
}
void InvokationStmt::traverseChildren(Traverser *tv)
{
    if(_expression)
    {
        _expression->traverse(_expression, tv);
    }

}

BinaryOperation::BinaryOperation(Token _pos,
                                 Token _endingpos,
                                 QString _operator_,
                                 shared_ptr<Expression> _operand1,
                                 shared_ptr<Expression> _operand2):
    Expression(_pos,_endingpos),_operator_(_operator_),
    _operand1(_operand1),
    _operand2(_operand2)
{
}

QString BinaryOperation::toString()
{
    QString ret;
    QTextStream out(&ret);
    out << _ws(L"تعبير.عملية.ثنائية(");
    out << Expression::childrenToString();
    out << ", " << childrenToString();
    out <<")";
    return ret;
}

QString BinaryOperation::childrenToString()
{
    QString ret;
    QTextStream out(&ret);
    out << _operator_ << ", " ;
    if(_operand1)
    {
        out << operand1()->toString() << ", " ;
    }
    if(_operand2)
    {
        out << operand2()->toString() << ", " ;
    }
    return ret;
}

void BinaryOperation::traverse(shared_ptr<PrettyPrintable> p, Traverser *tv)
{
    tv->visit(p);
    Expression::traverseChildren(tv);
    this->traverseChildren(tv);
    tv->exit(p);
}
void BinaryOperation::traverseChildren(Traverser *tv)
{
    if(_operand1)
    {
        _operand1->traverse(_operand1, tv);
    }

    if(_operand2)
    {
        _operand2->traverse(_operand2, tv);
    }

}

UnaryOperation::UnaryOperation(Token _pos,
                               Token _endingpos,
                               QString _operator_,
                               shared_ptr<Expression> _operand):
    Expression(_pos,_endingpos),_operator_(_operator_),
    _operand(_operand)
{
}

QString UnaryOperation::toString()
{
    QString ret;
    QTextStream out(&ret);
    out << _ws(L"أمر.عملية.أحادية(");
    out << Expression::childrenToString();
    out << ", " << childrenToString();
    out <<")";
    return ret;
}

QString UnaryOperation::childrenToString()
{
    QString ret;
    QTextStream out(&ret);
    out << _operator_ << ", " ;
    if(_operand)
    {
        out << operand()->toString() << ", " ;
    }
    return ret;
}

void UnaryOperation::traverse(shared_ptr<PrettyPrintable> p, Traverser *tv)
{
    tv->visit(p);
    Expression::traverseChildren(tv);
    this->traverseChildren(tv);
    tv->exit(p);
}
void UnaryOperation::traverseChildren(Traverser *tv)
{
    if(_operand)
    {
        _operand->traverse(_operand, tv);
    }

}

IsaOperation::IsaOperation(Token _pos,
                           Token _endingpos,
                           shared_ptr<Expression> _expression,
                           shared_ptr<Identifier> _type):
    Expression(_pos,_endingpos),_expression(_expression),
    _type(_type)
{
}

QString IsaOperation::toString()
{
    QString ret;
    QTextStream out(&ret);
    out << _ws(L"تعبير.هو(");
    out << Expression::childrenToString();
    out << ", " << childrenToString();
    out <<")";
    return ret;
}

QString IsaOperation::childrenToString()
{
    QString ret;
    QTextStream out(&ret);
    if(_expression)
    {
        out << expression()->toString() << ", " ;
    }
    if(_type)
    {
        out << type()->toString() << ", " ;
    }
    return ret;
}

void IsaOperation::traverse(shared_ptr<PrettyPrintable> p, Traverser *tv)
{
    tv->visit(p);
    Expression::traverseChildren(tv);
    this->traverseChildren(tv);
    tv->exit(p);
}
void IsaOperation::traverseChildren(Traverser *tv)
{
    if(_expression)
    {
        _expression->traverse(_expression, tv);
    }

    if(_type)
    {
        _type->traverse(_type, tv);
    }

}

MatchOperation::MatchOperation(Token _pos,
                               Token _endingpos,
                               shared_ptr<Expression> _expression,
                               shared_ptr<Pattern> _pattern):
    Expression(_pos,_endingpos),_expression(_expression),
    _pattern(_pattern)
{
}

QString MatchOperation::toString()
{
    QString ret;
    QTextStream out(&ret);
    out << _ws(L"أمر.لكل(");
    out << Expression::childrenToString();
    out << ", " << childrenToString();
    out <<")";
    return ret;
}

QString MatchOperation::childrenToString()
{
    QString ret;
    QTextStream out(&ret);
    if(_expression)
    {
        out << expression()->toString() << ", " ;
    }
    if(_pattern)
    {
        out << pattern()->toString() << ", " ;
    }
    return ret;
}

void MatchOperation::traverse(shared_ptr<PrettyPrintable> p, Traverser *tv)
{
    tv->visit(p);
    Expression::traverseChildren(tv);
    this->traverseChildren(tv);
    tv->exit(p);
}
void MatchOperation::traverseChildren(Traverser *tv)
{
    if(_expression)
    {
        _expression->traverse(_expression, tv);
    }

    if(_pattern)
    {
        _pattern->traverse(_pattern, tv);
    }

}

Identifier::Identifier(Token _pos,
                       Token _endingpos,
                       QString _name):
    KalimatAst(_pos,_endingpos),_name(_name)
{
}

QString Identifier::toString()
{
    QString ret;
    QTextStream out(&ret);
    out << _ws(L"معرف(");
    out << KalimatAst::childrenToString();
    out << ", " << childrenToString();
    out <<")";
    return ret;
}

QString Identifier::childrenToString()
{
    QString ret;
    QTextStream out(&ret);
    out << _name << ", " ;
    return ret;
}

void Identifier::traverse(shared_ptr<PrettyPrintable> p, Traverser *tv)
{
    tv->visit(p);
    KalimatAst::traverseChildren(tv);
    this->traverseChildren(tv);
    tv->exit(p);
}
void Identifier::traverseChildren(Traverser *tv)
{
}

VarAccess::VarAccess(Token _pos,
                     Token _endingpos,
                     shared_ptr<Identifier> _name):
    AssignableExpression(_pos,_endingpos),_name(_name)
{
}

QString VarAccess::toString()
{
    QString ret;
    QTextStream out(&ret);
    out << _ws(L"وصول.لمتغير(");
    out << AssignableExpression::childrenToString();
    out << ", " << childrenToString();
    out <<")";
    return ret;
}

QString VarAccess::childrenToString()
{
    QString ret;
    QTextStream out(&ret);
    if(_name)
    {
        out << name()->toString() << ", " ;
    }
    return ret;
}

void VarAccess::traverse(shared_ptr<PrettyPrintable> p, Traverser *tv)
{
    tv->visit(p);
    AssignableExpression::traverseChildren(tv);
    this->traverseChildren(tv);
    tv->exit(p);
}
void VarAccess::traverseChildren(Traverser *tv)
{
    if(_name)
    {
        _name->traverse(_name, tv);
    }

}


QString NumLiteral::toString()
{
    QString ret;
    QTextStream out(&ret);
    out << _ws(L"تعبير.عددي.حرفي(");
    out << SimpleLiteral::childrenToString();
    out << ", " << childrenToString();
    out <<")";
    return ret;
}

QString NumLiteral::childrenToString()
{
    QString ret;
    QTextStream out(&ret);
    out << _lValue << ", " ;
    out << _dValue << ", " ;
    out << _longNotDouble << ", " ;
    out << _valueRecognized << ", " ;
    return ret;
}

void NumLiteral::traverse(shared_ptr<PrettyPrintable> p, Traverser *tv)
{
    tv->visit(p);
    SimpleLiteral::traverseChildren(tv);
    this->traverseChildren(tv);
    tv->exit(p);
}
void NumLiteral::traverseChildren(Traverser *tv)
{
}

StrLiteral::StrLiteral(Token _pos,
                       Token _endingpos,
                       QString _value):
    SimpleLiteral(_pos,_endingpos),_value(_value)
{
}

QString StrLiteral::toString()
{
    QString ret;
    QTextStream out(&ret);
    out << _ws(L"تعبير.نصي.حرفي(");
    out << SimpleLiteral::childrenToString();
    out << ", " << childrenToString();
    out <<")";
    return ret;
}

QString StrLiteral::childrenToString()
{
    QString ret;
    QTextStream out(&ret);
    out << _value << ", " ;
    return ret;
}

void StrLiteral::traverse(shared_ptr<PrettyPrintable> p, Traverser *tv)
{
    tv->visit(p);
    SimpleLiteral::traverseChildren(tv);
    this->traverseChildren(tv);
    tv->exit(p);
}
void StrLiteral::traverseChildren(Traverser *tv)
{
}

NullLiteral::NullLiteral(Token _pos,
                         Token _endingpos):
    SimpleLiteral(_pos,_endingpos)
{
}

QString NullLiteral::toString()
{
    QString ret;
    QTextStream out(&ret);
    out << _ws(L"لاشيء(");
    out << SimpleLiteral::childrenToString();
    out << ", " << childrenToString();
    out <<")";
    return ret;
}

QString NullLiteral::childrenToString()
{
    QString ret;
    QTextStream out(&ret);
    return ret;
}

void NullLiteral::traverse(shared_ptr<PrettyPrintable> p, Traverser *tv)
{
    tv->visit(p);
    SimpleLiteral::traverseChildren(tv);
    this->traverseChildren(tv);
    tv->exit(p);
}
void NullLiteral::traverseChildren(Traverser *tv)
{
}

BoolLiteral::BoolLiteral(Token _pos,
                         Token _endingpos,
                         bool _value):
    SimpleLiteral(_pos,_endingpos),_value(_value)
{
}

QString BoolLiteral::toString()
{
    QString ret;
    QTextStream out(&ret);
    out << _ws(L"تعبير.منطقي.حرفي(");
    out << SimpleLiteral::childrenToString();
    out << ", " << childrenToString();
    out <<")";
    return ret;
}

QString BoolLiteral::childrenToString()
{
    QString ret;
    QTextStream out(&ret);
    out << _value << ", " ;
    return ret;
}

void BoolLiteral::traverse(shared_ptr<PrettyPrintable> p, Traverser *tv)
{
    tv->visit(p);
    SimpleLiteral::traverseChildren(tv);
    this->traverseChildren(tv);
    tv->exit(p);
}
void BoolLiteral::traverseChildren(Traverser *tv)
{
}

ArrayLiteral::ArrayLiteral(Token _pos,
                           Token _endingpos,
                           QVector<shared_ptr<Expression> > _data):
    Literal(_pos,_endingpos),_data(_data)
{
}

QString ArrayLiteral::toString()
{
    QString ret;
    QTextStream out(&ret);
    out << _ws(L"أمر.لكل(");
    out << Literal::childrenToString();
    out << ", " << childrenToString();
    out <<")";
    return ret;
}

QString ArrayLiteral::childrenToString()
{
    QString ret;
    QTextStream out(&ret);
    out << "[";
    for(int i=0; i<_data.count(); i++)
    {
        if(_data[i])
        {
            out << _data[i]->toString() << ", " ;
        }
    }
    out << "]";
    return ret;
}

void ArrayLiteral::traverse(shared_ptr<PrettyPrintable> p, Traverser *tv)
{
    tv->visit(p);
    Literal::traverseChildren(tv);
    this->traverseChildren(tv);
    tv->exit(p);
}
void ArrayLiteral::traverseChildren(Traverser *tv)
{
    for(int i=0; i<_data.count(); ++i)
    {
        if(_data[i])
        {
            _data[i]->traverse(_data[i], tv);
        }

    }

}

MapLiteral::MapLiteral(Token _pos,
                       Token _endingpos,
                       QVector<shared_ptr<Expression> > _data):
    Literal(_pos,_endingpos),_data(_data)
{
}

QString MapLiteral::toString()
{
    QString ret;
    QTextStream out(&ret);
    out << _ws(L"أمر.لكل(");
    out << Literal::childrenToString();
    out << ", " << childrenToString();
    out <<")";
    return ret;
}

QString MapLiteral::childrenToString()
{
    QString ret;
    QTextStream out(&ret);
    out << "[";
    for(int i=0; i<_data.count(); i++)
    {
        if(_data[i])
        {
            out << _data[i]->toString() << ", " ;
        }
    }
    out << "]";
    return ret;
}

void MapLiteral::traverse(shared_ptr<PrettyPrintable> p, Traverser *tv)
{
    tv->visit(p);
    Literal::traverseChildren(tv);
    this->traverseChildren(tv);
    tv->exit(p);
}
void MapLiteral::traverseChildren(Traverser *tv)
{
    for(int i=0; i<_data.count(); ++i)
    {
        if(_data[i])
        {
            _data[i]->traverse(_data[i], tv);
        }

    }

}

IInvokation::IInvokation(Token _pos,
                         Token _endingpos):
    Expression(_pos,_endingpos)
{
}


QString IInvokation::childrenToString()
{
    QString ret;
    QTextStream out(&ret);
    return ret;
}

void IInvokation::traverseChildren(Traverser *tv)
{
}

Invokation::Invokation(Token _pos,
                       Token _endingpos,
                       shared_ptr<Identifier> _functor,
                       QVector<shared_ptr<Expression> > arguments):
    IInvokation(_pos,_endingpos),_functor(_functor),
    arguments(arguments)
{
}

QString Invokation::toString()
{
    QString ret;
    QTextStream out(&ret);
    out << _ws(L"أمر.لكل(");
    out << IInvokation::childrenToString();
    out << ", " << childrenToString();
    out <<")";
    return ret;
}

QString Invokation::childrenToString()
{
    QString ret;
    QTextStream out(&ret);
    if(_functor)
    {
        out << functor()->toString() << ", " ;
    }
    out << "[";
    for(int i=0; i<arguments.count(); i++)
    {
        if(arguments[i])
        {
            out << arguments[i]->toString() << ", " ;
        }
    }
    out << "]";
    return ret;
}

void Invokation::traverse(shared_ptr<PrettyPrintable> p, Traverser *tv)
{
    tv->visit(p);
    IInvokation::traverseChildren(tv);
    this->traverseChildren(tv);
    tv->exit(p);
}
void Invokation::traverseChildren(Traverser *tv)
{
    if(_functor)
    {
        _functor->traverse(_functor, tv);
    }

    for(int i=0; i<arguments.count(); ++i)
    {
        if(arguments[i])
        {
            arguments[i]->traverse(arguments[i], tv);
        }

    }

}

MethodInvokation::MethodInvokation(Token _pos,
                                   Token _endingpos,
                                   shared_ptr<Expression> _receiver,
                                   shared_ptr<Identifier> _methodSelector,
                                   QVector<shared_ptr<Expression> > arguments):
    IInvokation(_pos,_endingpos),_receiver(_receiver),
    _methodSelector(_methodSelector),
    arguments(arguments)
{
}

QString MethodInvokation::toString()
{
    QString ret;
    QTextStream out(&ret);
    out << _ws(L"إرسال.رسالة(");
    out << IInvokation::childrenToString();
    out << ", " << childrenToString();
    out <<")";
    return ret;
}

QString MethodInvokation::childrenToString()
{
    QString ret;
    QTextStream out(&ret);
    if(_receiver)
    {
        out << receiver()->toString() << ", " ;
    }
    if(_methodSelector)
    {
        out << methodSelector()->toString() << ", " ;
    }
    out << "[";
    for(int i=0; i<arguments.count(); i++)
    {
        if(arguments[i])
        {
            out << arguments[i]->toString() << ", " ;
        }
    }
    out << "]";
    return ret;
}

void MethodInvokation::traverse(shared_ptr<PrettyPrintable> p, Traverser *tv)
{
    tv->visit(p);
    IInvokation::traverseChildren(tv);
    this->traverseChildren(tv);
    tv->exit(p);
}
void MethodInvokation::traverseChildren(Traverser *tv)
{
    if(_receiver)
    {
        _receiver->traverse(_receiver, tv);
    }

    if(_methodSelector)
    {
        _methodSelector->traverse(_methodSelector, tv);
    }

    for(int i=0; i<arguments.count(); ++i)
    {
        if(arguments[i])
        {
            arguments[i]->traverse(arguments[i], tv);
        }

    }

}

ForAutocomplete::ForAutocomplete(Token _pos,
                                 Token _endingpos,
                                 shared_ptr<Expression> _toBeCompleted):
    IInvokation(_pos,_endingpos),_toBeCompleted(_toBeCompleted)
{
}

QString ForAutocomplete::toString()
{
    QString ret;
    QTextStream out(&ret);
    out << _ws(L"تعبير.للتكملة.الآلية(");
    out << IInvokation::childrenToString();
    out << ", " << childrenToString();
    out <<")";
    return ret;
}

QString ForAutocomplete::childrenToString()
{
    QString ret;
    QTextStream out(&ret);
    if(_toBeCompleted)
    {
        out << toBeCompleted()->toString() << ", " ;
    }
    return ret;
}

void ForAutocomplete::traverse(shared_ptr<PrettyPrintable> p, Traverser *tv)
{
    tv->visit(p);
    IInvokation::traverseChildren(tv);
    this->traverseChildren(tv);
    tv->exit(p);
}
void ForAutocomplete::traverseChildren(Traverser *tv)
{
    if(_toBeCompleted)
    {
        _toBeCompleted->traverse(_toBeCompleted, tv);
    }

}

TimingExpression::TimingExpression(Token _pos,
                                   Token _endingpos,
                                   shared_ptr<Expression> _toTime):
    Expression(_pos,_endingpos),_toTime(_toTime)
{
}

QString TimingExpression::toString()
{
    QString ret;
    QTextStream out(&ret);
    out << _ws(L"أمر.لكل(");
    out << Expression::childrenToString();
    out << ", " << childrenToString();
    out <<")";
    return ret;
}

QString TimingExpression::childrenToString()
{
    QString ret;
    QTextStream out(&ret);
    if(_toTime)
    {
        out << toTime()->toString() << ", " ;
    }
    return ret;
}

void TimingExpression::traverse(shared_ptr<PrettyPrintable> p, Traverser *tv)
{
    tv->visit(p);
    Expression::traverseChildren(tv);
    this->traverseChildren(tv);
    tv->exit(p);
}
void TimingExpression::traverseChildren(Traverser *tv)
{
    if(_toTime)
    {
        _toTime->traverse(_toTime, tv);
    }

}


TheSomething::TheSomething(Token _pos,
                           Token _endingpos,
                           QString _name,
                           DeclarationType _what):
    Expression(_pos,_endingpos),_name(_name),
    _what(_what)
{
}


QString TheSomething::childrenToString()
{
    QString ret;
    QTextStream out(&ret);
    out << _name << ", " ;
    out << _what << ", " ;
    return ret;
}

void TheSomething::traverse(shared_ptr<PrettyPrintable> p, Traverser *tv)
{
    tv->visit(p);
    Expression::traverseChildren(tv);
    this->traverseChildren(tv);
    tv->exit(p);
}
void TheSomething::traverseChildren(Traverser *tv)
{
}

Idafa::Idafa(Token _pos,
             Token _endingpos,
             shared_ptr<Identifier> _modaf,
             shared_ptr<Expression> _modaf_elaih):
    AssignableExpression(_pos,_endingpos),_modaf(_modaf),
    _modaf_elaih(_modaf_elaih)
{
}

QString Idafa::toString()
{
    QString ret;
    QTextStream out(&ret);
    out << _ws(L"أمر.لكل(");
    out << AssignableExpression::childrenToString();
    out << ", " << childrenToString();
    out <<")";
    return ret;
}

QString Idafa::childrenToString()
{
    QString ret;
    QTextStream out(&ret);
    if(_modaf)
    {
        out << modaf()->toString() << ", " ;
    }
    if(_modaf_elaih)
    {
        out << modaf_elaih()->toString() << ", " ;
    }
    return ret;
}

void Idafa::traverse(shared_ptr<PrettyPrintable> p, Traverser *tv)
{
    tv->visit(p);
    AssignableExpression::traverseChildren(tv);
    this->traverseChildren(tv);
    tv->exit(p);
}
void Idafa::traverseChildren(Traverser *tv)
{
    if(_modaf)
    {
        _modaf->traverse(_modaf, tv);
    }

    if(_modaf_elaih)
    {
        _modaf_elaih->traverse(_modaf_elaih, tv);
    }

}

ArrayIndex::ArrayIndex(Token _pos,
                       Token _endingpos,
                       shared_ptr<Expression> _array,
                       shared_ptr<Expression> _index):
    AssignableExpression(_pos,_endingpos),_array(_array),
    _index(_index)
{
}

QString ArrayIndex::toString()
{
    QString ret;
    QTextStream out(&ret);
    out << _ws(L"أمر.لكل(");
    out << AssignableExpression::childrenToString();
    out << ", " << childrenToString();
    out <<")";
    return ret;
}

QString ArrayIndex::childrenToString()
{
    QString ret;
    QTextStream out(&ret);
    if(_array)
    {
        out << array()->toString() << ", " ;
    }
    if(_index)
    {
        out << index()->toString() << ", " ;
    }
    return ret;
}

void ArrayIndex::traverse(shared_ptr<PrettyPrintable> p, Traverser *tv)
{
    tv->visit(p);
    AssignableExpression::traverseChildren(tv);
    this->traverseChildren(tv);
    tv->exit(p);
}
void ArrayIndex::traverseChildren(Traverser *tv)
{
    if(_array)
    {
        _array->traverse(_array, tv);
    }

    if(_index)
    {
        _index->traverse(_index, tv);
    }

}

MultiDimensionalArrayIndex::MultiDimensionalArrayIndex(Token _pos,
                                                       Token _endingpos,
                                                       shared_ptr<Expression> _array,
                                                       QVector<shared_ptr<Expression> > _indexes):
    AssignableExpression(_pos,_endingpos),_array(_array),
    _indexes(_indexes)
{
}

QString MultiDimensionalArrayIndex::toString()
{
    QString ret;
    QTextStream out(&ret);
    out << _ws(L"أمر.لكل(");
    out << AssignableExpression::childrenToString();
    out << ", " << childrenToString();
    out <<")";
    return ret;
}

QString MultiDimensionalArrayIndex::childrenToString()
{
    QString ret;
    QTextStream out(&ret);
    if(_array)
    {
        out << array()->toString() << ", " ;
    }
    out << "[";
    for(int i=0; i<_indexes.count(); i++)
    {
        if(_indexes[i])
        {
            out << _indexes[i]->toString() << ", " ;
        }
    }
    out << "]";
    return ret;
}

void MultiDimensionalArrayIndex::traverse(shared_ptr<PrettyPrintable> p, Traverser *tv)
{
    tv->visit(p);
    AssignableExpression::traverseChildren(tv);
    this->traverseChildren(tv);
    tv->exit(p);
}
void MultiDimensionalArrayIndex::traverseChildren(Traverser *tv)
{
    if(_array)
    {
        _array->traverse(_array, tv);
    }

    for(int i=0; i<_indexes.count(); ++i)
    {
        if(_indexes[i])
        {
            _indexes[i]->traverse(_indexes[i], tv);
        }

    }

}

ObjectCreation::ObjectCreation(Token _pos,
                               Token _endingpos,
                               shared_ptr<Identifier> _className,
                               QVector<shared_ptr<Identifier> > fieldInitNames,
                               QVector<shared_ptr<Expression> > fieldInitValues):
    Expression(_pos,_endingpos),_className(_className),
    fieldInitNames(fieldInitNames),
    fieldInitValues(fieldInitValues)
{
}

QString ObjectCreation::toString()
{
    QString ret;
    QTextStream out(&ret);
    out << _ws(L"أمر.لكل(");
    out << Expression::childrenToString();
    out << ", " << childrenToString();
    out <<")";
    return ret;
}

QString ObjectCreation::childrenToString()
{
    QString ret;
    QTextStream out(&ret);
    if(_className)
    {
        out << className()->toString() << ", " ;
    }
    out << "[";
    for(int i=0; i<fieldInitNames.count(); i++)
    {
        if(fieldInitNames[i])
        {
            out << fieldInitNames[i]->toString() << ", " ;
        }
    }
    out << "]";
    out << "[";
    for(int i=0; i<fieldInitValues.count(); i++)
    {
        if(fieldInitValues[i])
        {
            out << fieldInitValues[i]->toString() << ", " ;
        }
    }
    out << "]";
    return ret;
}

void ObjectCreation::traverse(shared_ptr<PrettyPrintable> p, Traverser *tv)
{
    tv->visit(p);
    Expression::traverseChildren(tv);
    this->traverseChildren(tv);
    tv->exit(p);
}
void ObjectCreation::traverseChildren(Traverser *tv)
{
    if(_className)
    {
        _className->traverse(_className, tv);
    }

    for(int i=0; i<fieldInitNames.count(); ++i)
    {
        if(fieldInitNames[i])
        {
            fieldInitNames[i]->traverse(fieldInitNames[i], tv);
        }

    }

    for(int i=0; i<fieldInitValues.count(); ++i)
    {
        if(fieldInitValues[i])
        {
            fieldInitValues[i]->traverse(fieldInitValues[i], tv);
        }

    }

}

LambdaExpression::LambdaExpression(Token _pos,
                                   Token _endingpos,
                                   QVector<shared_ptr<FormalParam> > _argList,
                                   QVector<shared_ptr<Statement> > statements,
                                   bool _hasDoToken):
    Expression(_pos,_endingpos),_argList(_argList),
    statements(statements),
    _hasDoToken(_hasDoToken)
{
}

QString LambdaExpression::toString()
{
    QString ret;
    QTextStream out(&ret);
    out << _ws(L"تعبير.لامدا(");
    out << Expression::childrenToString();
    out << ", " << childrenToString();
    out <<")";
    return ret;
}

QString LambdaExpression::childrenToString()
{
    QString ret;
    QTextStream out(&ret);
    out << "[";
    for(int i=0; i<_argList.count(); i++)
    {
        if(_argList[i])
        {
            out << _argList[i]->toString() << ", " ;
        }
    }
    out << "]";
    out << "[";
    for(int i=0; i<statements.count(); i++)
    {
        if(statements[i])
        {
            out << statements[i]->toString() << ", " ;
        }
    }
    out << "]";
    out << _hasDoToken << ", " ;
    out << "{";
    for(QSet<QString >::const_iterator i=freeVariables.begin(); i!=freeVariables.end(); ++i)
    {
        out << *i << ", " ;
    }
    out << "}";
    return ret;
}

void LambdaExpression::traverse(shared_ptr<PrettyPrintable> p, Traverser *tv)
{
    tv->visit(p);
    Expression::traverseChildren(tv);
    this->traverseChildren(tv);
    tv->exit(p);
}
void LambdaExpression::traverseChildren(Traverser *tv)
{
    for(int i=0; i<_argList.count(); ++i)
    {
        if(_argList[i])
        {
            _argList[i]->traverse(_argList[i], tv);
        }

    }

    for(int i=0; i<statements.count(); ++i)
    {
        if(statements[i])
        {
            statements[i]->traverse(statements[i], tv);
        }

    }

}

Pattern::Pattern(Token _pos,
                 Token _endingpos):
    KalimatAst(_pos,_endingpos)
{
}


QString Pattern::childrenToString()
{
    QString ret;
    QTextStream out(&ret);
    return ret;
}

void Pattern::traverseChildren(Traverser *tv)
{
}

SimpleLiteralPattern::SimpleLiteralPattern(Token _pos,
                                           Token _endingpos,
                                           shared_ptr<SimpleLiteral> _value):
    Pattern(_pos,_endingpos),_value(_value)
{
}

QString SimpleLiteralPattern::toString()
{
    QString ret;
    QTextStream out(&ret);
    out << _ws(L"نمط.بسيط(");
    out << Pattern::childrenToString();
    out << ", " << childrenToString();
    out <<")";
    return ret;
}

QString SimpleLiteralPattern::childrenToString()
{
    QString ret;
    QTextStream out(&ret);
    if(_value)
    {
        out << value()->toString() << ", " ;
    }
    return ret;
}

void SimpleLiteralPattern::traverse(shared_ptr<PrettyPrintable> p, Traverser *tv)
{
    tv->visit(p);
    Pattern::traverseChildren(tv);
    this->traverseChildren(tv);
    tv->exit(p);
}
void SimpleLiteralPattern::traverseChildren(Traverser *tv)
{
    if(_value)
    {
        _value->traverse(_value, tv);
    }

}

VarPattern::VarPattern(Token _pos,
                       Token _endingpos,
                       shared_ptr<VarAccess> _id):
    Pattern(_pos,_endingpos),_id(_id)
{
}

QString VarPattern::toString()
{
    QString ret;
    QTextStream out(&ret);
    out << _ws(L"نمط.متغير(");
    out << Pattern::childrenToString();
    out << ", " << childrenToString();
    out <<")";
    return ret;
}

QString VarPattern::childrenToString()
{
    QString ret;
    QTextStream out(&ret);
    if(_id)
    {
        out << id()->toString() << ", " ;
    }
    return ret;
}

void VarPattern::traverse(shared_ptr<PrettyPrintable> p, Traverser *tv)
{
    tv->visit(p);
    Pattern::traverseChildren(tv);
    this->traverseChildren(tv);
    tv->exit(p);
}
void VarPattern::traverseChildren(Traverser *tv)
{
    if(_id)
    {
        _id->traverse(_id, tv);
    }

}

AssignedVarPattern::AssignedVarPattern(Token _pos,
                                       Token _endingpos,
                                       shared_ptr<AssignableExpression> _lv):
    Pattern(_pos,_endingpos),_lv(_lv)
{
}

QString AssignedVarPattern::toString()
{
    QString ret;
    QTextStream out(&ret);
    out << _ws(L"نمط.متغير.مخصص.له(");
    out << Pattern::childrenToString();
    out << ", " << childrenToString();
    out <<")";
    return ret;
}

QString AssignedVarPattern::childrenToString()
{
    QString ret;
    QTextStream out(&ret);
    if(_lv)
    {
        out << lv()->toString() << ", " ;
    }
    return ret;
}

void AssignedVarPattern::traverse(shared_ptr<PrettyPrintable> p, Traverser *tv)
{
    tv->visit(p);
    Pattern::traverseChildren(tv);
    this->traverseChildren(tv);
    tv->exit(p);
}
void AssignedVarPattern::traverseChildren(Traverser *tv)
{
    if(_lv)
    {
        _lv->traverse(_lv, tv);
    }

}

ArrayPattern::ArrayPattern(Token _pos,
                           Token _endingpos,
                           QVector<shared_ptr<Pattern> > elements,
                           bool _fixedLength):
    Pattern(_pos,_endingpos),elements(elements),
    _fixedLength(_fixedLength)
{
}

QString ArrayPattern::toString()
{
    QString ret;
    QTextStream out(&ret);
    out << _ws(L"نمط.مصفوفة(");
    out << Pattern::childrenToString();
    out << ", " << childrenToString();
    out <<")";
    return ret;
}

QString ArrayPattern::childrenToString()
{
    QString ret;
    QTextStream out(&ret);
    out << "[";
    for(int i=0; i<elements.count(); i++)
    {
        if(elements[i])
        {
            out << elements[i]->toString() << ", " ;
        }
    }
    out << "]";
    out << _fixedLength << ", " ;
    return ret;
}

void ArrayPattern::traverse(shared_ptr<PrettyPrintable> p, Traverser *tv)
{
    tv->visit(p);
    Pattern::traverseChildren(tv);
    this->traverseChildren(tv);
    tv->exit(p);
}
void ArrayPattern::traverseChildren(Traverser *tv)
{
    for(int i=0; i<elements.count(); ++i)
    {
        if(elements[i])
        {
            elements[i]->traverse(elements[i], tv);
        }

    }

}

ObjPattern::ObjPattern(Token _pos,
                       Token _endingpos,
                       shared_ptr<Identifier> _classId,
                       QVector<shared_ptr<Identifier> > fieldNames,
                       QVector<shared_ptr<Pattern> > fieldPatterns):
    Pattern(_pos,_endingpos),_classId(_classId),
    fieldNames(fieldNames),
    fieldPatterns(fieldPatterns)
{
}

QString ObjPattern::toString()
{
    QString ret;
    QTextStream out(&ret);
    out << _ws(L"نمط.كائن(");
    out << Pattern::childrenToString();
    out << ", " << childrenToString();
    out <<")";
    return ret;
}

QString ObjPattern::childrenToString()
{
    QString ret;
    QTextStream out(&ret);
    if(_classId)
    {
        out << classId()->toString() << ", " ;
    }
    out << "[";
    for(int i=0; i<fieldNames.count(); i++)
    {
        if(fieldNames[i])
        {
            out << fieldNames[i]->toString() << ", " ;
        }
    }
    out << "]";
    out << "[";
    for(int i=0; i<fieldPatterns.count(); i++)
    {
        if(fieldPatterns[i])
        {
            out << fieldPatterns[i]->toString() << ", " ;
        }
    }
    out << "]";
    return ret;
}

void ObjPattern::traverse(shared_ptr<PrettyPrintable> p, Traverser *tv)
{
    tv->visit(p);
    Pattern::traverseChildren(tv);
    this->traverseChildren(tv);
    tv->exit(p);
}
void ObjPattern::traverseChildren(Traverser *tv)
{
    if(_classId)
    {
        _classId->traverse(_classId, tv);
    }

    for(int i=0; i<fieldNames.count(); ++i)
    {
        if(fieldNames[i])
        {
            fieldNames[i]->traverse(fieldNames[i], tv);
        }

    }

    for(int i=0; i<fieldPatterns.count(); ++i)
    {
        if(fieldPatterns[i])
        {
            fieldPatterns[i]->traverse(fieldPatterns[i], tv);
        }

    }

}

MapPattern::MapPattern(Token _pos,
                       Token _endingpos,
                       QVector<shared_ptr<Expression> > keys,
                       QVector<shared_ptr<Pattern> > values):
    Pattern(_pos,_endingpos),keys(keys),
    values(values)
{
}

QString MapPattern::toString()
{
    QString ret;
    QTextStream out(&ret);
    out << _ws(L"نمط.قاموس(");
    out << Pattern::childrenToString();
    out << ", " << childrenToString();
    out <<")";
    return ret;
}

QString MapPattern::childrenToString()
{
    QString ret;
    QTextStream out(&ret);
    out << "[";
    for(int i=0; i<keys.count(); i++)
    {
        if(keys[i])
        {
            out << keys[i]->toString() << ", " ;
        }
    }
    out << "]";
    out << "[";
    for(int i=0; i<values.count(); i++)
    {
        if(values[i])
        {
            out << values[i]->toString() << ", " ;
        }
    }
    out << "]";
    return ret;
}

void MapPattern::traverse(shared_ptr<PrettyPrintable> p, Traverser *tv)
{
    tv->visit(p);
    Pattern::traverseChildren(tv);
    this->traverseChildren(tv);
    tv->exit(p);
}
void MapPattern::traverseChildren(Traverser *tv)
{
    for(int i=0; i<keys.count(); ++i)
    {
        if(keys[i])
        {
            keys[i]->traverse(keys[i], tv);
        }

    }

    for(int i=0; i<values.count(); ++i)
    {
        if(values[i])
        {
            values[i]->traverse(values[i], tv);
        }

    }

}

TypeExpression::TypeExpression(Token _pos,
                               Token _endingpos):
    KalimatAst(_pos,_endingpos)
{
}


QString TypeExpression::childrenToString()
{
    QString ret;
    QTextStream out(&ret);
    return ret;
}

void TypeExpression::traverseChildren(Traverser *tv)
{
}

TypeIdentifier::TypeIdentifier(Token _pos,
                               Token _endingpos,
                               QString _name):
    TypeExpression(_pos,_endingpos),_name(_name)
{
}

QString TypeIdentifier::toString()
{
    QString ret;
    QTextStream out(&ret);
    out << _ws(L"معرف.نوع(");
    out << TypeExpression::childrenToString();
    out << ", " << childrenToString();
    out <<")";
    return ret;
}

QString TypeIdentifier::childrenToString()
{
    QString ret;
    QTextStream out(&ret);
    out << _name << ", " ;
    return ret;
}

void TypeIdentifier::traverse(shared_ptr<PrettyPrintable> p, Traverser *tv)
{
    tv->visit(p);
    TypeExpression::traverseChildren(tv);
    this->traverseChildren(tv);
    tv->exit(p);
}
void TypeIdentifier::traverseChildren(Traverser *tv)
{
}

PointerTypeExpression::PointerTypeExpression(Token _pos,
                                             Token _endingpos,
                                             shared_ptr<TypeExpression> _pointeeType):
    TypeExpression(_pos,_endingpos),_pointeeType(_pointeeType)
{
}

QString PointerTypeExpression::toString()
{
    QString ret;
    QTextStream out(&ret);
    out << _ws(L"معرف.نوع.مشير(");
    out << TypeExpression::childrenToString();
    out << ", " << childrenToString();
    out <<")";
    return ret;
}

QString PointerTypeExpression::childrenToString()
{
    QString ret;
    QTextStream out(&ret);
    if(_pointeeType)
    {
        out << pointeeType()->toString() << ", " ;
    }
    return ret;
}

void PointerTypeExpression::traverse(shared_ptr<PrettyPrintable> p, Traverser *tv)
{
    tv->visit(p);
    TypeExpression::traverseChildren(tv);
    this->traverseChildren(tv);
    tv->exit(p);
}
void PointerTypeExpression::traverseChildren(Traverser *tv)
{
    if(_pointeeType)
    {
        _pointeeType->traverse(_pointeeType, tv);
    }

}

FunctionTypeExpression::FunctionTypeExpression(Token _pos,
                                               Token _endingpos,
                                               shared_ptr<TypeExpression> _retType,
                                               QVector<shared_ptr<TypeExpression> > argTypes):
    TypeExpression(_pos,_endingpos),_retType(_retType),
    argTypes(argTypes)
{
}

QString FunctionTypeExpression::toString()
{
    QString ret;
    QTextStream out(&ret);
    out << _ws(L"تعبير.نوع.إجراء(");
    out << TypeExpression::childrenToString();
    out << ", " << childrenToString();
    out <<")";
    return ret;
}

QString FunctionTypeExpression::childrenToString()
{
    QString ret;
    QTextStream out(&ret);
    if(_retType)
    {
        out << retType()->toString() << ", " ;
    }
    out << "[";
    for(int i=0; i<argTypes.count(); i++)
    {
        if(argTypes[i])
        {
            out << argTypes[i]->toString() << ", " ;
        }
    }
    out << "]";
    return ret;
}

void FunctionTypeExpression::traverse(shared_ptr<PrettyPrintable> p, Traverser *tv)
{
    tv->visit(p);
    TypeExpression::traverseChildren(tv);
    this->traverseChildren(tv);
    tv->exit(p);
}
void FunctionTypeExpression::traverseChildren(Traverser *tv)
{
    if(_retType)
    {
        _retType->traverse(_retType, tv);
    }

    for(int i=0; i<argTypes.count(); ++i)
    {
        if(argTypes[i])
        {
            argTypes[i]->traverse(argTypes[i], tv);
        }

    }

}

Declaration::Declaration(Token _pos,
                         Token _endingpos,
                         bool _isPublic):
    TopLevel(_pos,_endingpos),_isPublic(_isPublic)
{
}


QString Declaration::childrenToString()
{
    QString ret;
    QTextStream out(&ret);
    out << _isPublic << ", " ;
    return ret;
}

void Declaration::traverseChildren(Traverser *tv)
{
}

FormalParam::FormalParam(shared_ptr<Identifier> _name,
                         shared_ptr<TypeExpression> _type):
    PrettyPrintable(),_name(_name),
    _type(_type)
{
}

QString FormalParam::toString()
{
    QString ret;
    QTextStream out(&ret);
    out << _ws(L"عامل.رسمي(");
    out << ", " << childrenToString();
    out <<")";
    return ret;
}

QString FormalParam::childrenToString()
{
    QString ret;
    QTextStream out(&ret);
    if(_name)
    {
        out << name()->toString() << ", " ;
    }
    if(_type)
    {
        out << type()->toString() << ", " ;
    }
    return ret;
}

void FormalParam::traverse(shared_ptr<PrettyPrintable> p, Traverser *tv)
{
    tv->visit(p);
    this->traverseChildren(tv);
    tv->exit(p);
}
void FormalParam::traverseChildren(Traverser *tv)
{
    if(_name)
    {
        _name->traverse(_name, tv);
    }

    if(_type)
    {
        _type->traverse(_type, tv);
    }

}

ProceduralDecl::ProceduralDecl(Token _pos,
                               Token _endingpos,
                               bool _isPublic,
                               shared_ptr<Identifier> _procName,
                               QVector<shared_ptr<FormalParam> > formals,
                               shared_ptr<BlockStmt> _body):
    Declaration(_pos,_endingpos,_isPublic),
    IScopeIntroducer(),_procName(_procName),
    formals(formals),
    _body(_body)
{
}


QString ProceduralDecl::childrenToString()
{
    QString ret;
    QTextStream out(&ret);
    if(_procName)
    {
        out << procName()->toString() << ", " ;
    }
    out << "[";
    for(int i=0; i<formals.count(); i++)
    {
        if(formals[i])
        {
            out << formals[i]->toString() << ", " ;
        }
    }
    out << "]";
    if(_body)
    {
        out << body()->toString() << ", " ;
    }
    return ret;
}

void ProceduralDecl::traverseChildren(Traverser *tv)
{
    if(_procName)
    {
        _procName->traverse(_procName, tv);
    }

    for(int i=0; i<formals.count(); ++i)
    {
        if(formals[i])
        {
            formals[i]->traverse(formals[i], tv);
        }

    }

    if(_body)
    {
        _body->traverse(_body, tv);
    }

}

IProcedure::IProcedure()

{
}




IFunction::IFunction()

{
}




ProcedureDecl::ProcedureDecl(Token _pos,
                             Token _endingpos,
                             bool _isPublic,
                             shared_ptr<Identifier> _procName,
                             QVector<shared_ptr<FormalParam> > formals,
                             shared_ptr<BlockStmt> _body):
    ProceduralDecl(_pos,_endingpos,_isPublic,_procName,formals,_body),
    IProcedure()
{
}

QString ProcedureDecl::toString()
{
    QString ret;
    QTextStream out(&ret);
    out << _ws(L"إعلان.إجراء(");
    out << ProceduralDecl::childrenToString();
    out <<", ";
    out << ", " << childrenToString();
    out <<")";
    return ret;
}

QString ProcedureDecl::childrenToString()
{
    QString ret;
    QTextStream out(&ret);
    return ret;
}

void ProcedureDecl::traverse(shared_ptr<PrettyPrintable> p, Traverser *tv)
{
    tv->visit(p);
    ProceduralDecl::traverseChildren(tv);
    this->traverseChildren(tv);
    tv->exit(p);
}
void ProcedureDecl::traverseChildren(Traverser *tv)
{
}

FunctionDecl::FunctionDecl(Token _pos,
                           Token _endingpos,
                           bool _isPublic,
                           shared_ptr<Identifier> _procName,
                           QVector<shared_ptr<FormalParam> > formals,
                           shared_ptr<BlockStmt> _body):
    ProceduralDecl(_pos,_endingpos,_isPublic,_procName,formals,_body),
    IFunction()
{
}

QString FunctionDecl::toString()
{
    QString ret;
    QTextStream out(&ret);
    out << _ws(L"إعلان.دالة(");
    out << ProceduralDecl::childrenToString();
    out <<", ";
    out << ", " << childrenToString();
    out <<")";
    return ret;
}

QString FunctionDecl::childrenToString()
{
    QString ret;
    QTextStream out(&ret);
    return ret;
}

void FunctionDecl::traverse(shared_ptr<PrettyPrintable> p, Traverser *tv)
{
    tv->visit(p);
    ProceduralDecl::traverseChildren(tv);
    this->traverseChildren(tv);
    tv->exit(p);
}
void FunctionDecl::traverseChildren(Traverser *tv)
{
}

FFILibraryDecl::FFILibraryDecl(Token _pos,
                               Token _endingpos,
                               bool _isPublic,
                               QString _libName,
                               QVector<shared_ptr<Declaration> > decls):
    Declaration(_pos,_endingpos,_isPublic),_libName(_libName),
    decls(decls)
{
}

QString FFILibraryDecl::toString()
{
    QString ret;
    QTextStream out(&ret);
    out << _ws(L"إعلان.مكتبة.خارجية(");
    out << Declaration::childrenToString();
    out << ", " << childrenToString();
    out <<")";
    return ret;
}

QString FFILibraryDecl::childrenToString()
{
    QString ret;
    QTextStream out(&ret);
    out << _libName << ", " ;
    out << "[";
    for(int i=0; i<decls.count(); i++)
    {
        if(decls[i])
        {
            out << decls[i]->toString() << ", " ;
        }
    }
    out << "]";
    return ret;
}

void FFILibraryDecl::traverse(shared_ptr<PrettyPrintable> p, Traverser *tv)
{
    tv->visit(p);
    Declaration::traverseChildren(tv);
    this->traverseChildren(tv);
    tv->exit(p);
}
void FFILibraryDecl::traverseChildren(Traverser *tv)
{
    for(int i=0; i<decls.count(); ++i)
    {
        if(decls[i])
        {
            decls[i]->traverse(decls[i], tv);
        }

    }

}

FFIProceduralDecl::FFIProceduralDecl(Token _pos,
                                     Token _endingpos,
                                     bool _isPublic,
                                     bool _isFunctionNotProc,
                                     QString _procName,
                                     QString _symbol,
                                     shared_ptr<TypeExpression> _returnType,
                                     QVector<shared_ptr<TypeExpression> > paramTypes):
    Declaration(_pos,_endingpos,_isPublic),_isFunctionNotProc(_isFunctionNotProc),
    _procName(_procName),
    _symbol(_symbol),
    _returnType(_returnType),
    paramTypes(paramTypes)
{
}

QString FFIProceduralDecl::toString()
{
    QString ret;
    QTextStream out(&ret);
    out << _ws(L"إعلان.إجراء.أو.دالة.خارجية(");
    out << Declaration::childrenToString();
    out << ", " << childrenToString();
    out <<")";
    return ret;
}

QString FFIProceduralDecl::childrenToString()
{
    QString ret;
    QTextStream out(&ret);
    out << _isFunctionNotProc << ", " ;
    out << _procName << ", " ;
    out << _symbol << ", " ;
    if(_returnType)
    {
        out << returnType()->toString() << ", " ;
    }
    out << "[";
    for(int i=0; i<paramTypes.count(); i++)
    {
        if(paramTypes[i])
        {
            out << paramTypes[i]->toString() << ", " ;
        }
    }
    out << "]";
    return ret;
}

void FFIProceduralDecl::traverse(shared_ptr<PrettyPrintable> p, Traverser *tv)
{
    tv->visit(p);
    Declaration::traverseChildren(tv);
    this->traverseChildren(tv);
    tv->exit(p);
}
void FFIProceduralDecl::traverseChildren(Traverser *tv)
{
    if(_returnType)
    {
        _returnType->traverse(_returnType, tv);
    }

    for(int i=0; i<paramTypes.count(); ++i)
    {
        if(paramTypes[i])
        {
            paramTypes[i]->traverse(paramTypes[i], tv);
        }

    }

}

FFIStructDecl::FFIStructDecl(Token _pos,
                             Token _endingpos,
                             bool _isPublic,
                             shared_ptr<Identifier> _name,
                             QVector<shared_ptr<Identifier> > fieldNames,
                             QVector<shared_ptr<TypeExpression> > fieldTypes,
                             QVector<int > fieldBatches):
    Declaration(_pos,_endingpos,_isPublic),_name(_name),
    fieldNames(fieldNames),
    fieldTypes(fieldTypes),
    fieldBatches(fieldBatches)
{
}

QString FFIStructDecl::toString()
{
    QString ret;
    QTextStream out(&ret);
    out << _ws(L"إعلان.تركيب.خارجي(");
    out << Declaration::childrenToString();
    out << ", " << childrenToString();
    out <<")";
    return ret;
}

QString FFIStructDecl::childrenToString()
{
    QString ret;
    QTextStream out(&ret);
    if(_name)
    {
        out << name()->toString() << ", " ;
    }
    out << "[";
    for(int i=0; i<fieldNames.count(); i++)
    {
        if(fieldNames[i])
        {
            out << fieldNames[i]->toString() << ", " ;
        }
    }
    out << "]";
    out << "[";
    for(int i=0; i<fieldTypes.count(); i++)
    {
        if(fieldTypes[i])
        {
            out << fieldTypes[i]->toString() << ", " ;
        }
    }
    out << "]";
    out << "[";
    for(int i=0; i<fieldBatches.count(); i++)
    {
        out << fieldBatches[i] << ", " ;
    }
    out << "]";
    return ret;
}

void FFIStructDecl::traverse(shared_ptr<PrettyPrintable> p, Traverser *tv)
{
    tv->visit(p);
    Declaration::traverseChildren(tv);
    this->traverseChildren(tv);
    tv->exit(p);
}
void FFIStructDecl::traverseChildren(Traverser *tv)
{
    if(_name)
    {
        _name->traverse(_name, tv);
    }

    for(int i=0; i<fieldNames.count(); ++i)
    {
        if(fieldNames[i])
        {
            fieldNames[i]->traverse(fieldNames[i], tv);
        }

    }

    for(int i=0; i<fieldTypes.count(); ++i)
    {
        if(fieldTypes[i])
        {
            fieldTypes[i]->traverse(fieldTypes[i], tv);
        }

    }

}

ClassInternalDecl::ClassInternalDecl(): 
    PrettyPrintable()
{
}


QString ClassInternalDecl::childrenToString()
{
    QString ret;
    QTextStream out(&ret);
    return ret;
}

void ClassInternalDecl::traverseChildren(Traverser *tv)
{
}

MethodInfo::MethodInfo(int _arity,
                       bool _isFunction):
    _arity(_arity),
    _isFunction(_isFunction)
{
}




ConcreteResponseInfo::ConcreteResponseInfo(shared_ptr<Identifier> _name): 
    PrettyPrintable(),_name(_name)
{
}

QString ConcreteResponseInfo::toString()
{
    QString ret;
    QTextStream out(&ret);
    out << _ws(L"معلومات.تفاصيل.الاستجابة(");
    out << ", " << childrenToString();
    out <<")";
    return ret;
}

QString ConcreteResponseInfo::childrenToString()
{
    QString ret;
    QTextStream out(&ret);
    if(_name)
    {
        out << name()->toString() << ", " ;
    }
    out << "[";
    for(int i=0; i<params.count(); i++)
    {
        if(params[i])
        {
            out << params[i]->toString() << ", " ;
        }
    }
    out << "]";
    return ret;
}

void ConcreteResponseInfo::traverse(shared_ptr<PrettyPrintable> p, Traverser *tv)
{
    tv->visit(p);
    this->traverseChildren(tv);
    tv->exit(p);
}
void ConcreteResponseInfo::traverseChildren(Traverser *tv)
{
    if(_name)
    {
        _name->traverse(_name, tv);
    }

    for(int i=0; i<params.count(); ++i)
    {
        if(params[i])
        {
            params[i]->traverse(params[i], tv);
        }

    }

}

Has::Has(): 
    ClassInternalDecl()
{
}

QString Has::toString()
{
    QString ret;
    QTextStream out(&ret);
    out << _ws(L"مال(");
    out << ClassInternalDecl::childrenToString();
    out << ", " << childrenToString();
    out <<")";
    return ret;
}

QString Has::childrenToString()
{
    QString ret;
    QTextStream out(&ret);
    out << "{";
    for(QMap<QString, shared_ptr<TypeExpression>  >::const_iterator i=_fieldMarshallAs.begin(); i!=_fieldMarshallAs.end(); ++i)
    {
        out << i.key();
        if(i.value())
        {
            out << i.value()->toString() << ", " ;
        }
    }
    out << "}";
    out << "[";
    for(int i=0; i<fields.count(); i++)
    {
        if(fields[i])
        {
            out << fields[i]->toString() << ", " ;
        }
    }
    out << "]";
    return ret;
}

void Has::traverse(shared_ptr<PrettyPrintable> p, Traverser *tv)
{
    tv->visit(p);
    this->traverseChildren(tv);
    tv->exit(p);
}
void Has::traverseChildren(Traverser *tv)
{
    for(int i=0; i<fields.count(); ++i)
    {
        if(fields[i])
        {
            fields[i]->traverse(fields[i], tv);
        }

    }

}

RespondsTo::RespondsTo(bool _isFunctions): 
    ClassInternalDecl(),_isFunctions(_isFunctions)
{
}

QString RespondsTo::toString()
{
    QString ret;
    QTextStream out(&ret);
    out << _ws(L"استجابات(");
    out << ClassInternalDecl::childrenToString();
    out << ", " << childrenToString();
    out <<")";
    return ret;
}

QString RespondsTo::childrenToString()
{
    QString ret;
    QTextStream out(&ret);
    out << _isFunctions << ", " ;
    out << "[";
    for(int i=0; i<methods.count(); i++)
    {
        if(methods[i])
        {
            out << methods[i]->toString() << ", " ;
        }
    }
    out << "]";
    return ret;
}

void RespondsTo::traverse(shared_ptr<PrettyPrintable> p, Traverser *tv)
{
    tv->visit(p);
    this->traverseChildren(tv);
    tv->exit(p);
}
void RespondsTo::traverseChildren(Traverser *tv)
{
    for(int i=0; i<methods.count(); ++i)
    {
        if(methods[i])
        {
            methods[i]->traverse(methods[i], tv);
        }

    }

}

ClassDecl::ClassDecl(Token _pos,
                     Token _endingpos,
                     bool _isPublic,
                     shared_ptr<Identifier> _ancestorName,
                     shared_ptr<Identifier> _name,
                     QVector<shared_ptr<Identifier> > _fields,
                     QMap<QString, MethodInfo > _methodPrototypes,
                     QVector<shared_ptr<ClassInternalDecl> > _internalDecls,
                     QMap<QString, shared_ptr<TypeExpression> > _fieldMarshallAs):
    Declaration(_pos,_endingpos,_isPublic),_ancestorName(_ancestorName),
    _name(_name),
    _fields(_fields),
    _methodPrototypes(_methodPrototypes),
    _internalDecls(_internalDecls),
    _fieldMarshallAs(_fieldMarshallAs)
{
}

QString ClassDecl::toString()
{
    QString ret;
    QTextStream out(&ret);
    out << _ws(L"تعريف.فصيلة(");
    out << Declaration::childrenToString();
    out << ", " << childrenToString();
    out <<")";
    return ret;
}

QString ClassDecl::childrenToString()
{
    QString ret;
    QTextStream out(&ret);
    if(_ancestorName)
    {
        out << ancestorName()->toString() << ", " ;
    }
    if(_name)
    {
        out << name()->toString() << ", " ;
    }
    out << "[";
    for(int i=0; i<_fields.count(); i++)
    {
        if(_fields[i])
        {
            out << _fields[i]->toString() << ", " ;
        }
    }
    out << "]";
    out << "{";
    for(QMap<QString, shared_ptr<TypeExpression>  >::const_iterator i=_fieldMarshallAs.begin(); i!=_fieldMarshallAs.end(); ++i)
    {
        out << i.key();
        if(i.value())
        {
            out << i.value()->toString() << ", " ;
        }
    }
    out << "}";
    out << "{";
    for(QMap<QString, shared_ptr<MethodDecl>  >::const_iterator i=_methods.begin(); i!=_methods.end(); ++i)
    {
        out << i.key();
        if(i.value())
        {
            out << i.value()->toString() << ", " ;
        }
    }
    out << "}";
    if(_ancestorClass)
    {
        out << ancestorClass()->toString() << ", " ;
    }
    return ret;
}

void ClassDecl::traverse(shared_ptr<PrettyPrintable> p, Traverser *tv)
{
    tv->visit(p);
    Declaration::traverseChildren(tv);
    this->traverseChildren(tv);
    tv->exit(p);
}
void ClassDecl::traverseChildren(Traverser *tv)
{
    if(_ancestorName)
    {
        _ancestorName->traverse(_ancestorName, tv);
    }

    if(_name)
    {
        _name->traverse(_name, tv);
    }

    for(int i=0; i<_fields.count(); ++i)
    {
        if(_fields[i])
        {
            _fields[i]->traverse(_fields[i], tv);
        }

    }

    if(_ancestorClass)
    {
        _ancestorClass->traverse(_ancestorClass, tv);
    }

}

GlobalDecl::GlobalDecl(Token _pos,
                       Token _endingpos,
                       bool _isPublic,
                       QString _varName):
    Declaration(_pos,_endingpos,_isPublic),_varName(_varName)
{
}

QString GlobalDecl::toString()
{
    QString ret;
    QTextStream out(&ret);
    out << _ws(L"تعريف.متغير.مشترك(");
    out << Declaration::childrenToString();
    out << ", " << childrenToString();
    out <<")";
    return ret;
}

QString GlobalDecl::childrenToString()
{
    QString ret;
    QTextStream out(&ret);
    out << _varName << ", " ;
    return ret;
}

void GlobalDecl::traverse(shared_ptr<PrettyPrintable> p, Traverser *tv)
{
    tv->visit(p);
    Declaration::traverseChildren(tv);
    this->traverseChildren(tv);
    tv->exit(p);
}
void GlobalDecl::traverseChildren(Traverser *tv)
{
}

MethodDecl::MethodDecl(Token _pos,
                       Token _endingpos,
                       bool _isPublic,
                       shared_ptr<Identifier> _procName,
                       QVector<shared_ptr<FormalParam> > formals,
                       shared_ptr<BlockStmt> _body,
                       shared_ptr<Identifier> _className,
                       shared_ptr<Identifier> _receiverName,
                       bool _isFunctionNotProcedure):
    ProceduralDecl(_pos,_endingpos,_isPublic,_procName,formals,_body),_className(_className),
    _receiverName(_receiverName),
    _isFunctionNotProcedure(_isFunctionNotProcedure)
{
    this->formals.prepend(
                shared_ptr<FormalParam>(
                    new FormalParam(receiverName(),
                                    shared_ptr<TypeIdentifier>
                                    (new TypeIdentifier
                                     (className()->getPos(), className()->name())
                                     )
                                    )));
}

QString MethodDecl::toString()
{
    QString ret;
    QTextStream out(&ret);
    out << _ws(L"تعريف.وسيلة(");
    out << ProceduralDecl::childrenToString();
    out << ", " << childrenToString();
    out <<")";
    return ret;
}

QString MethodDecl::childrenToString()
{
    QString ret;
    QTextStream out(&ret);
    if(_className)
    {
        out << className()->toString() << ", " ;
    }
    if(_receiverName)
    {
        out << receiverName()->toString() << ", " ;
    }
    out << _isFunctionNotProcedure << ", " ;
    return ret;
}

void MethodDecl::traverse(shared_ptr<PrettyPrintable> p, Traverser *tv)
{
    tv->visit(p);
    ProceduralDecl::traverseChildren(tv);
    this->traverseChildren(tv);
    tv->exit(p);
}
void MethodDecl::traverseChildren(Traverser *tv)
{
    if(_className)
    {
        _className->traverse(_className, tv);
    }

    if(_receiverName)
    {
        _receiverName->traverse(_receiverName, tv);
    }

}

PegExpr::PegExpr(Token _pos,
                 Token _endingpos):
    KalimatAst(_pos,_endingpos)
{
}


QString PegExpr::childrenToString()
{
    QString ret;
    QTextStream out(&ret);
    return ret;
}

void PegExpr::traverseChildren(Traverser *tv)
{
}

PegPrimary::PegPrimary(Token _pos,
                       Token _endingpos,
                       shared_ptr<Identifier> _associatedVar):
    PegExpr(_pos,_endingpos),_associatedVar(_associatedVar)
{
}


QString PegPrimary::childrenToString()
{
    QString ret;
    QTextStream out(&ret);
    if(_associatedVar)
    {
        out << associatedVar()->toString() << ", " ;
    }
    return ret;
}

void PegPrimary::traverseChildren(Traverser *tv)
{
    if(_associatedVar)
    {
        _associatedVar->traverse(_associatedVar, tv);
    }

}

PegSequence::PegSequence(Token _pos,
                         Token _endingpos,
                         QVector<shared_ptr<PegExpr> > elements):
    PegExpr(_pos,_endingpos),elements(elements)
{
}

QString PegSequence::toString()
{
    QString ret;
    QTextStream out(&ret);
    out << _ws(L"تعبير.نحوي.تسلسلي(");
    out << PegExpr::childrenToString();
    out << ", " << childrenToString();
    out <<")";
    return ret;
}

QString PegSequence::childrenToString()
{
    QString ret;
    QTextStream out(&ret);
    out << "[";
    for(int i=0; i<elements.count(); i++)
    {
        if(elements[i])
        {
            out << elements[i]->toString() << ", " ;
        }
    }
    out << "]";
    return ret;
}

void PegSequence::traverse(shared_ptr<PrettyPrintable> p, Traverser *tv)
{
    tv->visit(p);
    PegExpr::traverseChildren(tv);
    this->traverseChildren(tv);
    tv->exit(p);
}
void PegSequence::traverseChildren(Traverser *tv)
{
    for(int i=0; i<elements.count(); ++i)
    {
        if(elements[i])
        {
            elements[i]->traverse(elements[i], tv);
        }

    }

}

PegRuleInvokation::PegRuleInvokation(Token _pos,
                                     Token _endingpos,
                                     shared_ptr<Identifier> _associatedVar,
                                     shared_ptr<Identifier> _ruleName):
    PegPrimary(_pos,_endingpos,_associatedVar),_ruleName(_ruleName)
{
}

QString PegRuleInvokation::toString()
{
    QString ret;
    QTextStream out(&ret);
    out << _ws(L"تعبير.نحوي.استدعاء.قاعدة(");
    out << PegPrimary::childrenToString();
    out << ", " << childrenToString();
    out <<")";
    return ret;
}

QString PegRuleInvokation::childrenToString()
{
    QString ret;
    QTextStream out(&ret);
    if(_ruleName)
    {
        out << ruleName()->toString() << ", " ;
    }
    return ret;
}

void PegRuleInvokation::traverse(shared_ptr<PrettyPrintable> p, Traverser *tv)
{
    tv->visit(p);
    PegPrimary::traverseChildren(tv);
    this->traverseChildren(tv);
    tv->exit(p);
}
void PegRuleInvokation::traverseChildren(Traverser *tv)
{
    if(_ruleName)
    {
        _ruleName->traverse(_ruleName, tv);
    }

}

PegLiteral::PegLiteral(Token _pos,
                       Token _endingpos,
                       shared_ptr<Identifier> _associatedVar,
                       shared_ptr<StrLiteral> _value):
    PegPrimary(_pos,_endingpos,_associatedVar),_value(_value)
{
}

QString PegLiteral::toString()
{
    QString ret;
    QTextStream out(&ret);
    out << _ws(L"تعبير.نحوي.حرفي(");
    out << PegPrimary::childrenToString();
    out << ", " << childrenToString();
    out <<")";
    return ret;
}

QString PegLiteral::childrenToString()
{
    QString ret;
    QTextStream out(&ret);
    if(_value)
    {
        out << value()->toString() << ", " ;
    }
    return ret;
}

void PegLiteral::traverse(shared_ptr<PrettyPrintable> p, Traverser *tv)
{
    tv->visit(p);
    PegPrimary::traverseChildren(tv);
    this->traverseChildren(tv);
    tv->exit(p);
}
void PegLiteral::traverseChildren(Traverser *tv)
{
    if(_value)
    {
        _value->traverse(_value, tv);
    }

}

PegCharRange::PegCharRange(Token _pos,
                           Token _endingpos,
                           shared_ptr<Identifier> _associatedVar,
                           shared_ptr<StrLiteral> _value1,
                           shared_ptr<StrLiteral> _value2):
    PegPrimary(_pos,_endingpos,_associatedVar),_value1(_value1),
    _value2(_value2)
{
}

QString PegCharRange::toString()
{
    QString ret;
    QTextStream out(&ret);
    out << _ws(L"تعبير.نحوي.نطاق.حروف(");
    out << PegPrimary::childrenToString();
    out << ", " << childrenToString();
    out <<")";
    return ret;
}

QString PegCharRange::childrenToString()
{
    QString ret;
    QTextStream out(&ret);
    if(_value1)
    {
        out << value1()->toString() << ", " ;
    }
    if(_value2)
    {
        out << value2()->toString() << ", " ;
    }
    return ret;
}

void PegCharRange::traverse(shared_ptr<PrettyPrintable> p, Traverser *tv)
{
    tv->visit(p);
    PegPrimary::traverseChildren(tv);
    this->traverseChildren(tv);
    tv->exit(p);
}
void PegCharRange::traverseChildren(Traverser *tv)
{
    if(_value1)
    {
        _value1->traverse(_value1, tv);
    }

    if(_value2)
    {
        _value2->traverse(_value2, tv);
    }

}

PegRepetion::PegRepetion(Token _pos,
                         Token _endingpos,
                         shared_ptr<Identifier> _associatedVar,
                         shared_ptr<Identifier> _resultVar,
                         shared_ptr<PegExpr> _subExpr,
                         shared_ptr<AssignmentStmt> _stepAssignment):
    PegPrimary(_pos,_endingpos,_associatedVar),_resultVar(_resultVar),
    _subExpr(_subExpr),
    _stepAssignment(_stepAssignment)
{
}

QString PegRepetion::toString()
{
    QString ret;
    QTextStream out(&ret);
    out << _ws(L"تعبير.نحوي.تكراري(");
    out << PegPrimary::childrenToString();
    out << ", " << childrenToString();
    out <<")";
    return ret;
}

QString PegRepetion::childrenToString()
{
    QString ret;
    QTextStream out(&ret);
    if(_resultVar)
    {
        out << resultVar()->toString() << ", " ;
    }
    if(_subExpr)
    {
        out << subExpr()->toString() << ", " ;
    }
    if(_stepAssignment)
    {
        out << stepAssignment()->toString() << ", " ;
    }
    return ret;
}

void PegRepetion::traverse(shared_ptr<PrettyPrintable> p, Traverser *tv)
{
    tv->visit(p);
    PegPrimary::traverseChildren(tv);
    this->traverseChildren(tv);
    tv->exit(p);
}
void PegRepetion::traverseChildren(Traverser *tv)
{
    if(_resultVar)
    {
        _resultVar->traverse(_resultVar, tv);
    }

    if(_subExpr)
    {
        _subExpr->traverse(_subExpr, tv);
    }

    if(_stepAssignment)
    {
        _stepAssignment->traverse(_stepAssignment, tv);
    }

}

RuleOption::RuleOption(Token _pos,
                       Token _endingpos,
                       shared_ptr<PegExpr> _expression,
                       shared_ptr<Expression> _resultExpr):
    KalimatAst(_pos,_endingpos),_expression(_expression),
    _resultExpr(_resultExpr)
{
}

QString RuleOption::toString()
{
    QString ret;
    QTextStream out(&ret);
    out << _ws(L"اختيار.في.قاعدة(");
    out << KalimatAst::childrenToString();
    out << ", " << childrenToString();
    out <<")";
    return ret;
}

QString RuleOption::childrenToString()
{
    QString ret;
    QTextStream out(&ret);
    if(_expression)
    {
        out << expression()->toString() << ", " ;
    }
    if(_resultExpr)
    {
        out << resultExpr()->toString() << ", " ;
    }
    return ret;
}

void RuleOption::traverse(shared_ptr<PrettyPrintable> p, Traverser *tv)
{
    tv->visit(p);
    KalimatAst::traverseChildren(tv);
    this->traverseChildren(tv);
    tv->exit(p);
}
void RuleOption::traverseChildren(Traverser *tv)
{
    if(_expression)
    {
        _expression->traverse(_expression, tv);
    }

    if(_resultExpr)
    {
        _resultExpr->traverse(_resultExpr, tv);
    }

}

RuleDecl::RuleDecl(Token _pos,
                   Token _endingpos,
                   QString _ruleName,
                   QVector<shared_ptr<RuleOption> > options):
    KalimatAst(_pos,_endingpos),_ruleName(_ruleName),
    options(options)
{
}

QString RuleDecl::toString()
{
    QString ret;
    QTextStream out(&ret);
    out << _ws(L"إعلان.قاعدة(");
    out << KalimatAst::childrenToString();
    out << ", " << childrenToString();
    out <<")";
    return ret;
}

QString RuleDecl::childrenToString()
{
    QString ret;
    QTextStream out(&ret);
    out << _ruleName << ", " ;
    out << "[";
    for(int i=0; i<options.count(); i++)
    {
        if(options[i])
        {
            out << options[i]->toString() << ", " ;
        }
    }
    out << "]";
    return ret;
}

void RuleDecl::traverse(shared_ptr<PrettyPrintable> p, Traverser *tv)
{
    tv->visit(p);
    KalimatAst::traverseChildren(tv);
    this->traverseChildren(tv);
    tv->exit(p);
}
void RuleDecl::traverseChildren(Traverser *tv)
{
    for(int i=0; i<options.count(); ++i)
    {
        if(options[i])
        {
            options[i]->traverse(options[i], tv);
        }

    }

}

RulesDecl::RulesDecl(Token _pos,
                     Token _endingpos,
                     bool _isPublic,
                     shared_ptr<Identifier> _ruleName,
                     QVector<shared_ptr<RuleDecl> > _subRules):
    Declaration(_pos,_endingpos,_isPublic),_ruleName(_ruleName),
    _subRules(_subRules)
{
}

QString RulesDecl::toString()
{
    QString ret;
    QTextStream out(&ret);
    out << _ws(L"إعلان.قواعد(");
    out << Declaration::childrenToString();
    out << ", " << childrenToString();
    out <<")";
    return ret;
}

QString RulesDecl::childrenToString()
{
    QString ret;
    QTextStream out(&ret);
    if(_ruleName)
    {
        out << ruleName()->toString() << ", " ;
    }
    out << "[";
    for(int i=0; i<_subRules.count(); i++)
    {
        if(_subRules[i])
        {
            out << _subRules[i]->toString() << ", " ;
        }
    }
    out << "]";
    return ret;
}

void RulesDecl::traverse(shared_ptr<PrettyPrintable> p, Traverser *tv)
{
    tv->visit(p);
    Declaration::traverseChildren(tv);
    this->traverseChildren(tv);
    tv->exit(p);
}
void RulesDecl::traverseChildren(Traverser *tv)
{
    if(_ruleName)
    {
        _ruleName->traverse(_ruleName, tv);
    }

    for(int i=0; i<_subRules.count(); ++i)
    {
        if(_subRules[i])
        {
            _subRules[i]->traverse(_subRules[i], tv);
        }

    }

}

