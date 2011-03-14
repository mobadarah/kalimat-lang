/**************************************************************************
** The Kalimat programming language
**   Copyright (C) 2010 Mohamed Samy Ali - samy2004@gmail.com
**   This project is released under the Apache License version 2.0
**   as described in the included license.txt file
**************************************************************************/

#ifndef PARSER_H
#define PARSER_H

struct ParserState
                           {
    int curToken;
    QVector<Token> tokens;
    Token lookAhead;

    QString (*tokenFormatter)(int);
    ParserState(int curToken, Token lookAhead, QVector<Token> tokens, QString (*tokenFormatter)(int));
};

class Parser
{
    int curToken;

    QString (*tokenFormatter)(int);

protected:
    QVector<Token> tokens;
    Token lookAhead;
    virtual AST *parseRoot()=0;
public:
    Parser();
    Parser(QString (*tokenFormatter)(int));
    virtual ~Parser();


    virtual void init(QString s, Lexer *lxr);
    virtual void init(QString s, Lexer *lxr, void *tag);
    virtual void init(QString s, Lexer *lxr, void *tag, QString fileName);
    AST *parse();
    AST *parse(AST *(*root)(Parser *p));

    ParserState saveState();
    void restoreState(ParserState);

protected:
    bool eof();
    void initLookAhead();
    bool LA(TokenType tokenId);
    bool LA2(TokenType tokenId, TokenType followingTokenId);
    bool match(int TokenType);
    void advanceToken();
    Token getPos();
};

class ParserException
{
public:
    Token pos;
    bool hasPosInfo;
    QString message;
public:

    ParserException(QString msg);
    ParserException(Token pos, QString msg);
};
class ParserBacktrackingException
{

};

#endif // PARSER_H
