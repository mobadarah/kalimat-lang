#ifndef FORMATMAKER_H
#define FORMATMAKER_H

template<typename V> QString vector_toString(V vector)
{
    QStringList lst;
    for(int i=0; i<vector.count(); i++)
    {
        AST *ast = vector[i].get();
        lst.append(ast->toString());
    }
    return lst.join(", ");
}

/*
// We will implement those closures manually
// since mingw has problems when we support C++0x
typedef function<void(CodeFormatter *)> FormatMaker;

FormatMaker parens(FormatMaker f)
{
    return [&f](CodeFormatter *cf)
    {
    cf->openParen();
    f(cf);
    cf->closeParen();
    };
}

FormatMaker ast(AST *a)
{
    return [&a](CodeFormatter *cf) { a->prettyPrint(cf); };
}

FormatMaker commaSep(FormatMaker *f1, FormatMaker *f2)
{
    return [&f1,&f2](CodeFormatter *cf)
    {
    f1(cf);
    cf->comma();
    f2(cf);
    };
}
*/
class FormatMaker{ public: virtual void run(CodeFormatter *f) = 0; };
class parens : public FormatMaker
{
    FormatMaker *f;
public:
    parens(FormatMaker *_f) { f = _f;}
    void run(CodeFormatter *cf)
    {
        cf->openParen();
        f->run(cf);
        cf->closeParen();
    }
};

class brackets : public FormatMaker
{
    FormatMaker *f;
public:
    brackets(FormatMaker *_f) { f = _f;}
    void run(CodeFormatter *cf)
    {
        cf->openBracket();
        f->run(cf);
        cf->closeBracket();
    }
};

class braces : public FormatMaker
{
    FormatMaker *f;
public:
    braces(FormatMaker *_f) { f = _f;}
    void run(CodeFormatter *cf)
    {
        cf->openBrace();
        f->run(cf);
        cf->closeBrace();
    }
};

class ast : public FormatMaker
{
    PrettyPrintable *a;
public:
    ast(PrettyPrintable *_a) { a = _a;}
    void run(CodeFormatter *cf)  { a->prettyPrint(cf); }
};

class commaSep: public FormatMaker
{
    QVector<FormatMaker *> fs;
public:
    commaSep(FormatMaker *_f1, FormatMaker *_f2) { fs.append(_f1);fs.append(_f2);}
    commaSep(QVector<FormatMaker *> _fs) { fs = _fs;}
    void run(CodeFormatter *cf)
    {
        for(int i=0; i<fs.count(); i++)
        {
            fs[i]->run(cf);
            if(i+1<fs.count())
                cf->comma();
        }
    }
};

class commaSepPairs: public FormatMaker
{
    QVector<FormatMaker *> fs;
    QString abSeparator;
public:
    commaSepPairs(FormatMaker *_f1, FormatMaker *_f2, QString _abSep)
    {
        fs.append(_f1);
        fs.append(_f2);
        abSeparator = _abSep;
    }
    commaSepPairs(QVector<FormatMaker *> _fs, QString _abSep)
    {
        fs = _fs;
        abSeparator = _abSep;
    }
    void run(CodeFormatter *cf)
    {
        for(int i=0; i<fs.count(); i+=2)
        {
            fs[i]->run(cf);
            cf->print(abSeparator);
            fs[i+1]->run(cf);
            if(i+2<fs.count())
                cf->comma();
        }
    }
};

class spaceSep: public FormatMaker
{
    QVector<FormatMaker *> fs;
public:
    spaceSep(FormatMaker *_f1, FormatMaker *_f2) { fs.append(_f1);fs.append(_f2);}
    spaceSep(QVector<FormatMaker *> _fs) { fs = _fs;}
    void run(CodeFormatter *cf)
    {
        for(int i=0; i<fs.count(); i++)
        {
            fs[i]->run(cf);
            if(i+1<fs.count())
                cf->space();
        }
    }
};

class PrintFmt : public FormatMaker
{
    Expression *width;
    Expression *expression;
public:
    PrintFmt(Expression *_expression, Expression *_width) { expression = _expression; width = _width; }
    void run(CodeFormatter *f)
    {
        if(width)
        {
            f->printKw(L"بعرض");
            parens(&ast(width)).run(f);
            f->space();
        }
        expression->prettyPrint(f);
    }
};

class ReadFmt : public FormatMaker
{
    AssignableExpression *var;
    bool readNumber;
public:
    ReadFmt(AssignableExpression *_var, bool _readNumber) { var = _var; readNumber = _readNumber; }
    void run(CodeFormatter *f)
    {
        if(readNumber)
        {
            f->print("#");
        }
        var->prettyPrint(f);
    }
};

// TODO: This leaks!
template<typename V>
QVector<FormatMaker *> mapFmt(QVector<shared_ptr<V> >v, int from=0)
{
    QVector<FormatMaker *> ret;
    for(int i=from; i<v.count(); i++)
        ret.append(new ast(v[i].get()));
    return ret;
}

// TODO: This leaks!
template<typename V>
QVector<FormatMaker *> mapFmt(QVector<V> v, int from=0)
{
    QVector<FormatMaker *> ret;
    for(int i=from; i<v.count(); i++)
        ret.append(new ast(v[i]));
    return ret;
}

//todo: This leaks
QVector<FormatMaker *> mapPrint(QVector<shared_ptr<Expression > > args, QVector<shared_ptr<Expression > > widths);

//todo: This leaks
QVector<FormatMaker *> mapRead(QVector<shared_ptr<AssignableExpression> > _variables, QVector<bool> readNumberFlags);

#endif // FORMATMAKER_H
