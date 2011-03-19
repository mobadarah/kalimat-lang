#ifndef KALIMATPRETTYPRINTPARSER_H
#define KALIMATPRETTYPRINTPARSER_H

class KPPAST : public AST
{
public:
    KPPAST() : AST(Token()) {}
    virtual void prettyPrint(CodeFormatter *) = 0;
    virtual QString toString()=0;
};

class TokPP : public KPPAST
{
public:
    Token token;
public:
    TokPP(Token token) { this->token = token; }
    void prettyPrint(CodeFormatter *);
    QString toString();
};

class SpacePP : public TokPP
{
public:
    SpacePP() : TokPP(Token(" ", WHITESPACE)) {}
};

// Not a block of statements, rather
// block means here 'same indentation level'
class BlockPP : public KPPAST
{
public:
    QVector<KPPAST *> subs;
public:
    BlockPP() {}
    BlockPP(QVector<KPPAST *> subs) { this->subs = subs; }
    void append(KPPAST *ast) { subs.append(ast); }
    void prettyPrint(CodeFormatter *);
    QString toString();
};

class LinePP : public KPPAST
{
public:
    QVector<KPPAST *> code;
    bool blockfollows;
    bool afterblock;
public:
    LinePP(QVector<KPPAST *> code) { this->code = code; blockfollows = false; afterblock = false;}
    LinePP() { blockfollows = false; afterblock = false;}
    void append(KPPAST *ast) { code.append(ast); }
    bool tokensEqual(int ids[], int count);
    bool tokensBeginEnd(int ids1[], int ids2[], int count1, int count2);
    QVector<Token> toTokens();
    void prettyPrint(CodeFormatter *);
    QString toString();
};

class KalimatPrettyprintParser : public Parser
{
public:
    KalimatPrettyprintParser();
    ~KalimatPrettyprintParser();
    void init(QString s, Lexer *lxr, void *tag);
    void init(QString s, Lexer *lxr, void *tag, QString fileName);

    AST *parseRoot();
private:
    QVector<LinePP *> parseLines();
    bool is_indentation_starter(LinePP *line, QString &ender);
    bool is_indentation_ender(LinePP *line);
    bool is_indentation_ender_and_starter(LinePP *line);
    bool match_ender(LinePP *line, QString ender);
};

#endif // KALIMATPRETTYPRINTPARSER_H
