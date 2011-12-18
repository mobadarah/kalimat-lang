#ifndef PATTERN_H
#define PATTERN_H

#include "kalimatast.h"

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
    QSharedPointer<SimpleLiteral> _value;
public:
    SimpleLiteralPattern(Token pos, QSharedPointer<SimpleLiteral> value)
        : Pattern(pos),_value(value) {}
    SimpleLiteral *value() { return _value.data(); }
    QString toString() { return value()->toString(); }
    void prettyPrint(CodeFormatter *formatter)
    {
        value()->prettyPrint(formatter);
    }
};

class VarPattern : public Pattern
{
    Q_OBJECT
    QSharedPointer<Identifier> _id;
public:
    VarPattern(Token pos, QSharedPointer<Identifier> id) : Pattern(pos),_id(id) {}
    Identifier *id() { return _id.data(); }
    QString toString();
    void prettyPrint(CodeFormatter *formatter);
};

class AssignedVarPattern : public Pattern
{
    Q_OBJECT
    QSharedPointer<AssignableExpression> _lv;
public:
    AssignedVarPattern(Token pos, QSharedPointer<AssignableExpression> lv)
        : Pattern(pos),_lv(lv)
    {
    }
    AssignableExpression *lv() { return _lv.data(); }
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
    QVector<QSharedPointer<Pattern> > _elements;
public:
    bool fixedLength;
public:
    ArrayPattern(Token pos, QVector<QSharedPointer<Pattern> > elements);
    int elementCount() { return _elements.count(); }
    Pattern *element(int i) { return _elements[i].data(); }
    QString toString();
    void prettyPrint(CodeFormatter *formatter);
};

class ObjPattern : public Pattern
{
    QSharedPointer<Identifier> _classId;
    QVector<QSharedPointer<Identifier> > _fieldNames;
    QVector<QSharedPointer<Pattern> > _fieldPatterns;
public:
    ObjPattern(Token pos,
               QSharedPointer<Identifier> classId,
               QVector<QSharedPointer<Identifier> > fieldNames,
               QVector<QSharedPointer<Pattern> > fieldPatterns);
    Identifier *classId() { return _classId.data(); }
    int fieldCount() { return _fieldNames.count(); }
    Identifier *fieldName(int i) { return _fieldNames[i].data(); }
    Pattern *fieldPattern(int i) { return _fieldPatterns[i].data(); }
    QString toString();
    void prettyPrint(CodeFormatter *formatter);
};

class MapPattern : public Pattern
{
    QVector<QSharedPointer<Expression> > _keys;
    QVector<QSharedPointer<Pattern> > _values;
public:
    MapPattern(Token pos, QVector<QSharedPointer<Expression> > keys, QVector<QSharedPointer<Pattern> > values);
    int pairCount() { return _keys.count(); }
    Expression *key(int i) { return _keys[i].data(); }
    Pattern *value(int i) { return _values[i].data(); }
    QString toString();
    void prettyPrint(CodeFormatter *formatter);
};

#endif // PATTERN_H
