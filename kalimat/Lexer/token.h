/**************************************************************************
** The Kalimat programming language
**   Copyright (C) 2010 Mohamed Samy Ali - samy2004@gmail.com
**   This project is released under the Apache License version 2.0
**   as described in the included license.txt file
**************************************************************************/

#ifndef TOKEN_H
#define TOKEN_H
#include <QtCore>
typedef int TokenType;

// Tokens of type 'TokenNone' are skipped during lexical analysis.
const TokenType TokenNone = -1;

// Invalid tokens are returned by functions that return tokens
// when they shouldn't, for example Parser.getPos() at the end of file
const TokenType TokenInvalid = -2;

// Tokens of type 'WithNext' are skipped when returning the tokenized program, but
// attached to the 'sister' field in the following token in the token stream
// the token is lost if the is no following token.
const TokenType TokenWithNext = -3;

// 'WithPrev' has similar behavior to 'WithNext'
const TokenType TokenWithPrev = -4;

class Token
{
public:
    QString Lexeme;
    TokenType Type;
    int Line, Column, Pos;
    void *tag;
    Token *sister;
public:
        Token(QString lexeme, TokenType type);
        Token(Token const &other);
        Token();
        bool operator ==(Token &t2);
        bool operator !=(Token &t2);
        Token &operator = (const Token &other);
        bool Is(QString lexeme);
        bool Is(TokenType type);
};
Q_DECLARE_METATYPE(Token);
Q_DECLARE_METATYPE(QVector<Token>);
#endif // TOKEN_H
