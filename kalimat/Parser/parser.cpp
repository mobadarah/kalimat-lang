/**************************************************************************
**   The Kalimat programming language
**   Copyright (C) 2010 Mohamed Samy Ali - samy2004@gmail.com
**   This project is released under the Apache License version 2.0
**   as described in the included license.txt file
**************************************************************************/

#include "parser_incl.h"

Parser::Parser()
{
    curToken = -1;
    tokenFormatter = NULL;
}
Parser::Parser(QString(*tokenFormatter)(int))
{
    curToken = -1;
    this->tokenFormatter = tokenFormatter;
}

Parser::~Parser()
{

}
void Parser::init(QString s, Lexer *lxr)
{
    init(s, lxr, NULL);
}

void Parser::init(QString s, Lexer *lxr, void *tag)
{
    lxr->init(s, tag);
    lxr->tokenize();
    tokens = lxr->getTokens();
    curToken = -1;
}
bool Parser::LA(TokenType tokenId)
{
    if(eof())
        return false;
    return lookAhead.Type == tokenId;
}
bool Parser::LA2(TokenType tokenId, TokenType followingTokenId)
{
    // Make sure we have at least 2 more tokens
    if(curToken+1 == tokens.size())
        return false;

    return lookAhead.Type == tokenId && tokens[curToken+1].Type == followingTokenId;
}
bool Parser::eof()
{
    return curToken == tokens.size() ;
}
bool Parser::match(TokenType tokenType)
{
    QString tokenId;
    if(tokenFormatter == NULL)
        tokenId = QString("%1").arg(tokenType);
    else
        tokenId = tokenFormatter(tokenType);

    if(eof())
        throw ParserException(QString("EOF reached while expecting token:%1").arg(tokenId));
    if(lookAhead.Type == tokenType)
    {
        if(!eof())
            advanceToken();
        return true;
    }
    if(true)
    {
        throw ParserException(getPos(), QString("Exprected token of type:%1").arg(tokenId));
    }
}
void Parser::advanceToken()
{
    // advanceToken does notihng if at EOF instead of signaling an error.
    // It is the caller's responsibility to check for EOF before matching lookAhead.

    curToken++;
    if(curToken < tokens.size())
    {
        lookAhead = tokens[curToken];
    }
}
void Parser::initLookAhead()
{
    if (tokens.size() == 0)
    {
        curToken = 0;
    }
    else
    {
        curToken = -1;
        advanceToken();
    }
}
Token Parser::getPos()
{
    if(curToken >=0 && curToken< tokens.size())
    {
        Token t = tokens[curToken];
        return t;
    }
    else if(tokens.size()!=0)
    {
        // if we're at EOF and there's a last token, return it
        return tokens[tokens.size()-1];
    }
    else
    {
        // Otherwise input is empty, return a token at position number 0
        Token t("", TokenInvalid);
        t.Pos = 0;
        t.Line = t.Column = 0;
        return t;
    }
}

AST *Parser::parse()
{
    initLookAhead();
    AST *ret = parseRoot();
    if(!eof())
    {
        throw ParserException
                (QString("Parser did not consume all input, stopped at token #%1 : %2")
                 .arg(curToken)
                 .arg(lookAhead.Lexeme));
    }
    return ret;
}
AST *Parser::parse(AST *(*root)(Parser *p))
{
    initLookAhead();
    AST *ret = root(this);
    if(!eof())
    {
        throw ParserException
                (QString("Parser did not consume all input, stopped at token #%1 : %2")
                 .arg(curToken)
                 .arg(lookAhead.Lexeme));
    }
    return ret;
}

ParserState Parser::saveState()
{
    return ParserState(curToken, lookAhead, tokens, tokenFormatter);
}
void Parser::restoreState(ParserState s)
{
    this->curToken = s.curToken;
    this->lookAhead = s.lookAhead;
    this->tokens = s.tokens;
    this->tokenFormatter = s.tokenFormatter;
}

ParserState::ParserState(int curToken, Token lookAhead, QVector<Token>tokens, QString(*tokenFormatter)(int))
{
    this->curToken = curToken;
    this->lookAhead = lookAhead;
    this->tokens = tokens;
    this->tokenFormatter = tokenFormatter;
}

ParserException::ParserException(QString msg)
{
    message = msg;
    hasPosInfo = false;
}
ParserException::ParserException(Token pos, QString msg)
{
    if(pos.Type == TokenInvalid)
        hasPosInfo = false;
    else
        hasPosInfo = true;
    this->pos = pos;
    message = QString("(%1,%2, pos=%3): %4").arg(pos.Line).arg(pos.Column).arg(pos.Pos).arg(msg);
}

