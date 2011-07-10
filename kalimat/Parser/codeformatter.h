#ifndef CODEFORMATTER_H
#define CODEFORMATTER_H

#include <QStringList>
#include <QColor>

class CodeFormatter
{
public:
    virtual void printRaw(QString str) = 0;
    virtual void print(QString code) = 0;
    virtual void println(QString code) = 0;
    virtual void printColored(QString code, QColor clr) = 0;
    virtual void rawNl() = 0;
    virtual void printKw(QString code) = 0;
    virtual void printKwExpression(QString code) = 0;

    virtual void print(QStdWString code) = 0;
    virtual void println(QStdWString code) = 0;
    virtual void printColored(QStdWString code, QColor clr) = 0;

    virtual void printKw(QStdWString code) = 0;
    virtual void printKwExpression(QStdWString code) = 0;

    virtual void indent() = 0;
    virtual void deindent() = 0;

    virtual void space() = 0;
    virtual void comma() = 0;
    virtual void colon() = 0;
    virtual void nl() = 0;
    virtual void blankLine()=0;
    virtual void openParen() = 0;
    virtual void closeParen() = 0;

    virtual void openBracket() = 0;
    virtual void closeBracket() = 0;

    virtual void openBrace() = 0;
    virtual void closeBrace() = 0;

};

class CommonCodeFormatter : public CodeFormatter
{
public:
    virtual void println(QString code) { print(code); nl(); }

    virtual void print(QStdWString code) { print(QString::fromStdWString(code)); }
    virtual void println(QStdWString code) { println(QString::fromStdWString(code)); }
    virtual void printColored(QStdWString code, QColor clr) { printColored(QString::fromStdWString(code), clr); }
    virtual void printKw(QStdWString code) { printKw(QString::fromStdWString(code)); }
    virtual void printKwExpression(QStdWString code) { printKwExpression(QString::fromStdWString(code)); }

    virtual void space()
    {
        print(" ");
    }
    virtual void comma() { print(QString::fromStdWString(L"، ")); }
    virtual void colon() { print(":"); }
    virtual void openParen() { print("("); }
    virtual void closeParen() { print(")"); }

    virtual void openBracket() { print("["); }
    virtual void closeBracket() { print("]"); }

    virtual void openBrace() { print("{"); }
    virtual void closeBrace() { print("}"); }

    virtual void print(QString code) = 0;
    virtual void printColored(QString code, QColor clr) = 0;
    virtual void printKw(QString code) = 0;
    virtual void printKwExpression(QString code) = 0;
    virtual void indent() = 0;
    virtual void deindent() = 0;
    virtual void nl() = 0;
    virtual void blankLine()=0;
};

class IndentingCodeFormatter : public CommonCodeFormatter
{
    bool atLineStart;
    int indentLevel;
    QString lastPrinted;
public:
    IndentingCodeFormatter();
    virtual void print(QString code);
    virtual void indent();
    virtual void deindent();
    virtual void nl();
    virtual void blankLine();
    virtual void space();
};

class SimpleCodeFormatter : public IndentingCodeFormatter
{
    QStringList o;
public:
    void printRaw(QString str) { o.append(str); }
    void rawNl() { o.append("\n");}
    void printColored(QString code, QColor clr) {print(code);}
    void printKw(QString code)
    {
        print(code); space();
    }
    void printKwExpression(QString code)
    {
        print(code);
    }

    QString getOutput() { return o.join("");}
};

#endif // CODEFORMATTER_H
