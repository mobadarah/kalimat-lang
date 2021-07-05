/**************************************************************************
**   The Kalimat programming language
**   Copyright (C) 2010 Mohamed Samy Ali - samy2004@gmail.com
**   This project is released under the Apache License version 2.0
**   as described in the included license.txt file
**************************************************************************/

#include <QString>
#include "token.h"

QTextStream & operator <<(QTextStream &str, const Token & t)
{
    return str << t.Lexeme;
}

Token::Token(QString lexeme, TokenType type)
{
    Lexeme = lexeme;
    Type = type;
    tag = NULL;
    sister = NULL;
}
Token::Token()
{
    Lexeme = "";
    Type = TokenInvalid;
    tag = NULL;
    sister = NULL;
}
Token::Token(const Token &other)
{
    Lexeme = other.Lexeme;
    Type = other.Type;
    Line = other.Line;
    Column = other.Column;
    Pos = other.Pos;
    tag = other.tag;
    sister = other.sister;
}
Token &Token::operator = (const Token &other)
{
    Lexeme = other.Lexeme;
    Type = other.Type;
    Line = other.Line;
    Column = other.Column;
    Pos = other.Pos;
    tag = other.tag;
    sister = other.sister;

    return *this;
}
bool Token::operator ==(Token &t2)
{
    return Type==t2.Type && Lexeme == t2.Lexeme && Line == t2.Line && Column == t2.Column && Pos == t2.Pos
            && tag == t2.tag && sister == t2.sister;
}
bool Token::operator !=(Token &t2)
{
    return Type!=t2.Type || Lexeme != t2.Lexeme  || Line != t2.Line || Column != t2.Column || Pos != t2.Pos
            || tag != t2.tag || sister != t2.sister;
}
bool Token::Is(QString lexeme)
{
    return Lexeme == lexeme;
}
bool Token::Is(TokenType type)
{
    return Type == type;
}
