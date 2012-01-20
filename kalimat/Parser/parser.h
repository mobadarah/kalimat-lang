/**************************************************************************
** The Kalimat programming language
**   Copyright (C) 2010 Mohamed Samy Ali - samy2004@gmail.com
**   This project is released under the Apache License version 2.0
**   as described in the included license.txt file
**************************************************************************/

#ifndef PARSER_H
#define PARSER_H

#include <memory>
using namespace std;

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
    virtual shared_ptr<AST> parseRoot()=0;
public:
    Parser();
    Parser(QString (*tokenFormatter)(int));
    virtual ~Parser();


    virtual void init(QString s, Lexer *lxr);
    virtual void init(QString s, Lexer *lxr, void *tag);
    virtual void init(QString s, Lexer *lxr, void *tag, QString fileName);
    shared_ptr<AST> parse();
    shared_ptr<AST> parse(shared_ptr<AST> (*root)(Parser *p));

    ParserState saveState();
    void restoreState(ParserState);

protected:
    bool eof();
    void initLookAhead();
    bool LA(TokenType tokenId);
    bool LA2(TokenType tokenId, TokenType followingTokenId);
    bool expect(TokenType tokenType);
    bool match(int TokenType);
    void advanceToken();
    Token getPos();
};

#endif // PARSER_H
