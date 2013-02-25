/**************************************************************************
**   The Kalimat programming language
**   Copyright (C) 2010 Mohamed Samy Ali - samy2004@gmail.com
**   This project is released under the Apache License version 2.0
**   as described in the included license.txt file
**************************************************************************/

#include "kalimatast_incl.h"
#include "../../../smallvm/utils.h"
using namespace std;

uint qHash(shared_ptr<Identifier> id)
{
    return qHash(id->name());
}

MethodInfo::MethodInfo()
{

}

Identifier::Identifier(Token _pos, QString _name)
    : KalimatAst(_pos,_pos), _name(_name)
{

}

VarAccess::VarAccess(shared_ptr<Identifier> _name)
    : AssignableExpression(_name->getPos(),_name->getEndingPos()),_name(_name)
{

}

StrLiteral::StrLiteral(Token _pos, QString _value)
    :SimpleLiteral(_pos,_pos),_value(_value)
{

}

NullLiteral::NullLiteral(Token _pos)
    :SimpleLiteral(_pos,_pos)
{

}

BoolLiteral::BoolLiteral(Token _pos, bool _value)
    :SimpleLiteral(_pos, _pos),_value(_value)
{

}

TypeIdentifier::TypeIdentifier(Token _pos,
               QString _name)
    : TypeExpression(_pos, _pos),_name(_name)
{

}


QVector<shared_ptr<Identifier> > ForAllStmt::getIntroducedVariables()
{
    QVector<shared_ptr<Identifier> > ret;
    ret.append(variable());
    return ret;
}

QVector<shared_ptr<Identifier> > ForEachStmt::getIntroducedVariables()
{
    QVector<shared_ptr<Identifier> > ret;
    ret.append(variable());
    return ret;
}


NumLiteral::NumLiteral(Token pos ,QString lexeme)
    : SimpleLiteral(pos, pos)
{
    bool ok;
    QLocale loc(QLocale::Arabic, QLocale::Egypt);
    _longNotDouble = true;
    _lValue= loc.toLongLong(lexeme, &ok, 10);
    if(!ok)
        _lValue = lexeme.toLong(&ok, 10);

    if(!ok)
    {
        _longNotDouble = false;
        _dValue = loc.toDouble(lexeme, &ok);
    }
    if(!ok)
        _dValue = lexeme.toDouble(&ok);

    _valueRecognized = ok;
}

NumLiteral::NumLiteral(Token pos ,int value) :
    SimpleLiteral(pos, pos)
{
    _valueRecognized = true;
    this->_lValue = value;
    _longNotDouble = true;
}

QString NumLiteral::repr()
{
    return QString("%1").arg(longNotDouble()? lValue(): dValue());
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
    return strLiteralRepr(this->value());
}

QString NullLiteral::repr()
{
    return _ws(L"لاشيء");
}

QString BoolLiteral::repr()
{
    return value()? QString::fromWCharArray(L"صحيح") : QString::fromWCharArray(L"خطأ");
}

QString DeclarationTypeToKeyword(DeclarationType type)
{
    switch(type)
    {
    case Proc:
        return _ws(L"الإجراء");
    case Function:
        return _ws(L"الدالة");
    }
    return "";
}

QString TheSomething::toString()
{
    return QString("%1(%2)").arg(DeclarationTypeToKeyword(what())).arg(name());
}

QString ProceduralDecl::getTooltip()
{
    QStringList argz;
    for(int i=0; i<formalCount(); i++)
        argz.append(formal(i)->toString());
    return QString("%1(%2)")
            .arg(procName()->name())
            .arg(argz.join(", "));
}

QVector<shared_ptr<Identifier> > ProceduralDecl::getIntroducedVariables()
{
    QVector<shared_ptr<Identifier> > ret;
    for(auto i=this->formals.begin(); i!=formals.end(); ++i)
    {
        ret.append((*i)->name());
    }
    return ret;
}


QSet<QString> RuleDecl::getAllAssociatedVars()
{
    QSet<QString> ret;
    for(int i=0; i<this->options.count(); i++)
        appendAll(ret, options[i]->expression()->getAllAssociatedVars());
    return ret;
}

QSet<QString> RulesDecl::getAllAssociatedVars()
{
    QSet<QString> ret;
    for(int i=0; i<this->subRuleCount(); i++)
        appendAll(ret, subRule(i)->getAllAssociatedVars());
    return ret;
}

QSet<QString> PegSequence::getAllAssociatedVars()
{
    QSet<QString> ret;
    for(int i=0; i<elementCount(); i++)
    {
        appendAll(ret, element(i)->getAllAssociatedVars());
    }
    return ret;
}

QSet<QString> PegPrimary::getAllAssociatedVars()
{
    QSet<QString> ret;
    if(associatedVar())
    {
        ret.insert(associatedVar()->name());
    }
    return ret;
}




QString MethodDecl::getTooltip()
{
    QStringList argz;
    for(int i=1; i<formalCount(); i++)
        argz.append(formal(i)->toString());
    return QString("%1(%2)")
            .arg(procName()->name())
            .arg(argz.join(", "));
}

// Pretty printing
void Program::prettyPrint(CodeFormatter *f)
{
    for(int i=0; i<this->originalElementCount(); i++)
    {
        shared_ptr<TopLevel> el = this->originalElement(i);
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

    for(int i=0; i<declarationCount(); i++)
    {
        declaration(i)->prettyPrint(f);
        f->nl();
        f->nl();
    }
}

void AssignmentStmt::prettyPrint(CodeFormatter *f)
{
    variable()->prettyPrint(f);
    f->space();
    if(type())
    {
        f->printKw(L"هو");
        type()->prettyPrint(f);
        f->space();
    }
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
    f->printKw(L"كرر مادام");
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

void ForEachStmt::prettyPrint(CodeFormatter *f)
{
    f->printKw(L"لكل");
    this->variable()->prettyPrint(f);
    f->space();
    f->printKw(L"في");
    this->enumerable()->prettyPrint(f);
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
    commaSep(mapPrint(this->args, this->widths)).run(f);
    if(this->printOnSameLine())
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
    if(this->prompt() !="")
    {
        f->print(strLiteralRepr(prompt()));
        f->comma();
    }

    commaSep(mapRead(this->variables, this->readNumberFlags)).run(f);
}

void DrawPixelStmt::prettyPrint(CodeFormatter *f)
{
    f->printKw(L"ارسم.نقطة");
    ast a(this->x()), b(this->y());
    commaSep c(&a, &b);
    parens(&c).run(f);
    if(this->color() !=NULL)
    {
        f->comma();
        color()->prettyPrint(f);
    }
}

void DrawLineStmt::prettyPrint(CodeFormatter *f)
{
    f->printKw(L"ارسم.خط");
    ast a(this->x1()), b(this->y1());
    commaSep c(&a, &b);

    parens(&c).run(f);
    f->print("-");

    ast a2(this->x2()), b2(this->y2());
    commaSep c2(&a2, &b2);
    parens(&c2).run(f);
    if(this->color() !=NULL)
    {
        f->comma();
        color()->prettyPrint(f);
    }
}

void DrawRectStmt::prettyPrint(CodeFormatter *f)
{
    f->printKw(L"ارسم.مستطيل");
    ast a1(this->x1()), b1(this->y1()), a2(this->x2()), b2(this->y2());
    commaSep c1(&a1, &b1), c2(&a2, &b2);
    parens(&c1).run(f);
    f->print("-");
    parens(&c2).run(f);

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
    ast a(this->cx()), b(this->cy());
    commaSep c(&a, &b);
    parens(&c).run(f);
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
    ast a(this->x()),b(this->y());
    commaSep c(&a, &b);
    parens(&c).run(f);
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
    ast a(this->x()),b(this->y());
    commaSep c(&a, &b);
    parens(&c).run(f);
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
    return L"";
}

void EventStatement::prettyPrint(CodeFormatter *f)
{
    f->printKw(L"عند حادثة");
    f->printKw(translateEventType(this->type()));
    f->printKw(L"نفذ");
    this->handler()->prettyPrint(f);
}

void SendStmt::prettyPrint(CodeFormatter *f)
{
    f->printKw(L"ارسل");
    if(signal())
        f->printKw(L"إشارة");
    else
        value()->prettyPrint(f);
    f->printKw(L"إلى");
    this->channel()->prettyPrint(f);
}

void ReceiveStmt::prettyPrint(CodeFormatter *f)
{
    f->printKw(L"تسلم");
    if(signal())
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
    for(int i=0; i<conditionCount(); i++)
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

void SimpleLiteralPattern::prettyPrint(CodeFormatter *formatter)
{
   value()->prettyPrint(formatter);
}
void VarPattern::prettyPrint(CodeFormatter *f)
{
    id()->prettyPrint(f);
}

void AssignedVarPattern::prettyPrint(CodeFormatter *formatter)
{
    formatter->print("? ");
    lv()->prettyPrint(formatter);
}

void ArrayPattern::prettyPrint(CodeFormatter *f)
{
    //todo: ArrayPattern::prettyPrint() by not copying to new vector
    QVector<shared_ptr<PrettyPrintable> > data;
    for(int i=0; i<elementCount(); i++)
        data.append(element(i));

    commaSep c(mapFmt(this->elements));
    brackets(&c).run(f);
}

void ObjPattern::prettyPrint(CodeFormatter *f)
{
    this->classId()->prettyPrint(f);
    f->space();
    f->printKw(_ws(L"له"));
    QVector<shared_ptr<PrettyPrintable> > data;
    //todo: optimize ObjPattern::prettyPrint() by not copying to new vector
    for(int i=0; i<this->fieldNameCount(); i++)
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
    for(int i=0; i<this->keyCount(); i++)
    {
        data.append(key(i));
        data.append(value(i));
    }
    commaSepPairs c(mapFmt(data), "=>");
    braces(&c).run(f);
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
    f->print(translateOperator(this->operator_()));
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
    f->print(translateOperator(this->operator_()));
    f->space();
    this->operand()->prettyPrint(f);
}

void Identifier::prettyPrint(CodeFormatter *f)
{
    f->print(name());
}

void VarAccess::prettyPrint(CodeFormatter *f)
{
    name()->prettyPrint(f);
}

void TypeIdentifier::prettyPrint(CodeFormatter *f)
{
    f->print(name());
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
    commaSep c(mapFmt(this->_data));
    brackets(&c).run(f);
}

void MapLiteral::prettyPrint(CodeFormatter *f)
{
    commaSepPairs c(mapFmt(this->_data), "=>");
    braces(&c).run(f);
}

void Invokation::prettyPrint(CodeFormatter *f)
{
    this->functor()->prettyPrint(f);
    commaSep c(mapFmt(this->arguments));
    parens(&c).run(f);
}

void MethodInvokation::prettyPrint(CodeFormatter *f)
{
    this->receiver()->prettyPrint(f);
    f->colon();
    f->space();
    this->methodSelector()->prettyPrint(f);
    commaSep c(mapFmt(this->arguments));
    parens(&c).run(f);
}

void ForAutocomplete::prettyPrint(CodeFormatter *f)
{
    toBeCompleted()->prettyPrint(f);
}

void TimingExpression::prettyPrint(CodeFormatter *f)
{
    f->printKw(L"توقيت");
    f->openParen();
    this->toTime()->prettyPrint(f);
    f->closeParen();
}

void TheSomething::prettyPrint(CodeFormatter *f)
{
    f->printKw(DeclarationTypeToKeyword(what()));
    f->openParen();
    f->print(name());
    f->closeParen();
}

void Idafa::prettyPrint(CodeFormatter *f)
{
    ast a(modaf()), b(modaf_elaih());
    spaceSep(&a, &b).run(f);
}

void ArrayIndex::prettyPrint(CodeFormatter *f)
{
    this->array()->prettyPrint(f);
    ast a(this->index());
    brackets(&a).run(f);
}

void MultiDimensionalArrayIndex::prettyPrint(CodeFormatter *f)
{
    this->array()->prettyPrint(f);
    commaSep c(mapFmt(this->_indexes));
    brackets(&c).run(f);
}

void ObjectCreation::prettyPrint(CodeFormatter *f)
{
    className()->prettyPrint(f);
    f->space();
    f->printKw(L"جديد");
}

void LambdaExpression::prettyPrint(CodeFormatter *f)
{
    f->print(L"λ");
    commaSep(mapFmt(_argList)).run(f);
    f->print(":");
    if(!hasDoToken())
    {
        shared_ptr<ReturnStmt> rs = dynamic_pointer_cast<ReturnStmt>(statements[0]);
        rs->returnVal()->prettyPrint(f);
    }
    else
    {
        semiColonSep(mapFmt(statements)).run(f);
    }
}

void FormalParam::prettyPrint(CodeFormatter *f)
{
    name()->prettyPrint(f);
    f->space();
    f->printKw(L"هو");
    type()->prettyPrint(f);
}

void ProcedureDecl::prettyPrint(CodeFormatter *f)
{
    f->printKw(L"إجراء");
    procName()->prettyPrint(f);

    commaSep c(mapFmt(this->formals));
    parens(&c).run(f);

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

    commaSep c(mapFmt(this->formals));
    parens(&c).run(f);

    f->colon();
    f->nl();
    this->body()->prettyPrint(f);
    f->printKw(L"نهاية");
    f->nl(); // for extra neatness, add an empty line after definitions
}

void FFILibraryDecl::prettyPrint(CodeFormatter *f)
{
    f->printKw(L"مكتبة");
    f->print(QString("\"%1\"").arg(libName()));
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
    if(this->isFunctionNotProc())
        f->printKw(L"دالة");
    else
        f->printKw("إجراء");

    f->print(this->procName());
    f->space();
    if(symbol() != procName())
    {
        f->printKw(L"برمز");
        f->print(QString("\"%1\"").arg(symbol()));
    }
    f->openParen();
    for(int i=0; i<paramTypeCount(); i++)
    {
        paramType(i)->prettyPrint(f);
        if(i < paramTypeCount() -1)
            f->comma();
    }
    f->closeParen();
    if(isFunctionNotProc())
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

void RuleOption::prettyPrint(CodeFormatter *f)
{
    expression()->prettyPrint(f);
    if(resultExpr())
    {
        f->space();
        f->print("=>");
        f->space();
        resultExpr()->prettyPrint(f);
    }
}

void RuleDecl::prettyPrint(CodeFormatter *f)
{
    f->print(ruleName());
    f->space();
    f->print("=");
    f->space();
    for(int j=0; j<options.count(); j++)
    {
        if(j>0)
        {
            f->printKw(L"أو");
        }
        shared_ptr<RuleOption> ro = options[j];
        ro->prettyPrint(f);
        f->nl();
    }
}

void RulesDecl::prettyPrint(CodeFormatter *f)
{
    f->printKw(L"قواعد");
    this->ruleName()->prettyPrint(f);
    f->colon();
    f->nl();
    f->indent();
    for(int i=0; i<subRuleCount(); i++)
    {
        shared_ptr<RuleDecl> r = subRule(i);
        r->prettyPrint(f);
        f->nl();
        if(i+1 < subRuleCount())
            f->nl();
    }
    f->deindent();
    f->printKw(L"نهاية");
    f->nl(); // for extra neatness, add an empty line after definitions
}

void PegRepetion::prettyPrint(CodeFormatter *f)
{

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

void PegCharRange::prettyPrint(CodeFormatter *f)
{
    f->printKw(L"من");
    this->value1()->prettyPrint(f);
    f->space();
    f->printKw(L"إلى");
    this->value2()->prettyPrint(f);
    f->space();
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
    if(isFunctions())
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
    this->name()->prettyPrint(f);
    f->space();

    commaSep c(mapFmt(params));
    parens(&c).run(f);
}

void ClassDecl::prettyPrint(CodeFormatter *f)
{
    f->printKw(L"فصيلة");
    this->name()->prettyPrint(f);
    f->colon();
    f->nl();
    f->indent();
    if(ancestorName())
    {
        f->printKw(L"مبني على");
        ancestorName()->prettyPrint(f);
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
    f->print(varName());
    f->space();
    f->printKw(L"مشترك");
}

void MethodDecl::prettyPrint(CodeFormatter *f)
{
    if(this->isFunctionNotProcedure())
        f->printKw(L"رد");
    else
        f->printKw(L"استجابة");

    this->className()->prettyPrint(f);
    f->space();
    this->receiverName()->prettyPrint(f);
    f->space();

    if(this->isFunctionNotProcedure())
        f->printKw(L"على");
    else
        f->printKw(L"ل:");

    procName()->prettyPrint(f);

    commaSep c(mapFmt(this->formals, 1));
    parens(&c).run(f);

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

