#ifndef PATTERN_H
#define PATTERN_H

#include "kalimatast.h"
#include "expression.h"

class Pattern : public KalimatAst
{
    Q_OBJECT
    ASTImpl _astImpl;
public:
    Pattern(Token pos);
    Token getPos() { return _astImpl.getPos();}
};

class SimpleLiteralPattern : public Pattern
{
    Q_OBJECT
    shared_ptr<SimpleLiteral> _value;
public:
    SimpleLiteralPattern(Token pos, shared_ptr<SimpleLiteral> value)
        : Pattern(pos),_value(value) {}
    shared_ptr<SimpleLiteral> value() { return _value; }
    QString toString() { return value()->toString(); }
    void prettyPrint(CodeFormatter *formatter)
    {
        value()->prettyPrint(formatter);
    }
};

class VarPattern : public Pattern
{
    Q_OBJECT
    shared_ptr<Identifier> _id;
public:
    VarPattern(Token pos, shared_ptr<Identifier> id) : Pattern(pos),_id(id) {}
    shared_ptr<Identifier> id() { return _id; }
    QString toString();
    void prettyPrint(CodeFormatter *formatter);
};

class AssignedVarPattern : public Pattern
{
    Q_OBJECT
    shared_ptr<AssignableExpression> _lv;
public:
    AssignedVarPattern(Token pos, shared_ptr<AssignableExpression> lv)
        : Pattern(pos),_lv(lv)
    {
    }
    shared_ptr<AssignableExpression> lv() { return _lv; }
    QString toString() { return QString("? %1").arg(lv()->toString()); }
    void prettyPrint(CodeFormatter *formatter)
    {
        formatter->print("? ");
        lv()->prettyPrint(formatter);
    }
};

class ArrayPattern : public Pattern
{
    Q_OBJECT
    QVector<shared_ptr<Pattern> > _elements;
public:
    bool fixedLength;
public:
    ArrayPattern(Token pos, QVector<shared_ptr<Pattern> > elements);
    int elementCount() { return _elements.count(); }
    shared_ptr<Pattern> element(int i) { return _elements[i]; }
    QString toString();
    void prettyPrint(CodeFormatter *formatter);
};

class ObjPattern : public Pattern
{
    Q_OBJECT
    shared_ptr<Identifier> _classId;
    QVector<shared_ptr<Identifier> > _fieldNames;
    QVector<shared_ptr<Pattern> > _fieldPatterns;
public:
    ObjPattern(Token pos,
               shared_ptr<Identifier> classId,
               QVector<shared_ptr<Identifier> > fieldNames,
               QVector<shared_ptr<Pattern> > fieldPatterns);
    shared_ptr<Identifier> classId() { return _classId; }
    int fieldCount() { return _fieldNames.count(); }
    shared_ptr<Identifier> fieldName(int i) { return _fieldNames[i]; }
    shared_ptr<Identifier> fieldNamePtr(int i) { return _fieldNames[i]; }
    shared_ptr<Pattern> fieldPattern(int i) { return _fieldPatterns[i]; }
    QString toString();
    void prettyPrint(CodeFormatter *formatter);
};

class MapPattern : public Pattern
{
    Q_OBJECT
    QVector<shared_ptr<Expression> > _keys;
    QVector<shared_ptr<Pattern> > _values;
public:
    MapPattern(Token pos, QVector<shared_ptr<Expression> > keys, QVector<shared_ptr<Pattern> > values);
    int pairCount() { return _keys.count(); }
    shared_ptr<Expression> key(int i) { return _keys[i]; }
    shared_ptr<Pattern> value(int i) { return _values[i]; }
    QString toString();
    void prettyPrint(CodeFormatter *formatter);
};

#endif // PATTERN_H
