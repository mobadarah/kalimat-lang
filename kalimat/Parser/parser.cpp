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
    atStartOfFile = false;
    lineStart = false;
    withRecovery = false;
    tag = NULL;
}
Parser::Parser(QString(*tokenFormatter)(int))
{
    curToken = -1;
    this->tokenFormatter = tokenFormatter;
    atStartOfFile = false;
    lineStart = false;
    withRecovery = false;
    tag = NULL;
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
    try
    {
        lxr->init(s, tag);
        lxr->tokenize(withRecovery);
    }
    catch(UnexpectedEndOfFileException)
    {
        if(!withRecovery)
            throw;
    }
    fileName = "";
    tokens = lxr->getTokens();
    curToken = -1;
    atStartOfFile = false;
    lineStart = false;
    this->tag = tag;
}

void Parser::init(QString s, Lexer *lxr, void *tag, QString fileName)
{
    try
    {
        lxr->init(s, tag, fileName);
        lxr->tokenize(withRecovery);
    }
    catch(UnexpectedEndOfFileException)
    {
        if(!withRecovery)
            throw;
    }

    tokens = lxr->getTokens();
    curToken = -1;
    atStartOfFile = false;
    lineStart = false;
    this->tag = tag;
    this->fileName = fileName;
}

bool Parser::LA(TokenType tokenId)
{
    if(eof())
        return false;
    return lookAhead.Type == tokenId;
}

bool Parser::LA(QSet<TokenType> tokenIds)
{
    foreach(TokenType t, tokenIds)
    {
        if(LA(t))
            return true;
    }

    return false;
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

bool Parser::expect(TokenType tokenType)
{
    QString tokenId;
    if(tokenFormatter == NULL)
        tokenId = QString("%1").arg(tokenType);
    else
        tokenId = tokenFormatter(tokenType);

    if(eof())
        throw ParserException(fileName, QString("EOF reached while expecting token:%1").arg(tokenId));
    if(lookAhead.Type == tokenType)
    {
        return true;
    }
    else
    {
        throw ParserException(fileName, getPos(), QString("Exprected token of type:%1").arg(tokenId));
    }
}

bool Parser::match(TokenType tokenType)
{
    QString tokenId;
    if(tokenFormatter == NULL)
        tokenId = QString("%1").arg(tokenType);
    else
        tokenId = tokenFormatter(tokenType);

    if(eof())
        throw ParserException(fileName, getPos(), QString("EOF reached while expecting token:%1").arg(tokenId));
    if(lookAhead.Type == tokenType)
    {
        if(!eof())
            advanceToken();
        return true;
    }
    if(true)
    {
        throw ParserException(fileName, getPos(), QString("Expected token of type:%1, found %2")
                              .arg(tokenId)
                              .arg(tokenFormatter(lookAhead.Type)));
    }
}

void Parser::advanceToken()
{
    // advanceToken does notihng if at EOF instead of signaling an error.
    // It is the caller's responsibility to check for EOF before matching lookAhead.

    curToken++;
    if(curToken < tokens.size())
    {
        if(lookAhead.Lexeme != "\n")
        {
            lineStart = false;
        }
        else
        {
            lineStart = true;
        }

        lookAhead = tokens[curToken];
    }
    if(curToken == 0)
    {
        atStartOfFile = true;
        lineStart = true;
    }
    else
    {
        atStartOfFile = false;
        if(curToken > 0)
        previousToken = tokens[curToken -1];
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
        // we make the type 'TokenNone' instead of 'TokenInvalid'
        // so that the error handler can point to a position in the file
        Token t("", TokenNone);
        t.Pos = 0;
        t.Line = t.Column = 0;
        t.tag = this->tag;

        return t;
    }
}

shared_ptr<AST> Parser::parse()
{
    initLookAhead();
    shared_ptr<AST> ret = parseRoot();
    if(!eof())
    {
        throw ParserException
                (fileName, QString("Parser did not consume all input, stopped at token #%1 : %2")
                 .arg(curToken)
                 .arg(lookAhead.Lexeme));
    }
    return ret;
}

shared_ptr<AST> Parser::parse(shared_ptr<AST> (*root)(Parser *p))
{
    initLookAhead();
    shared_ptr<AST> ret = root(this);
    if(!eof())
    {
        throw ParserException
                (fileName, QString("Parser did not consume all input, stopped at token #%1 : %2")
                 .arg(curToken)
                 .arg(lookAhead.Lexeme));
    }
    return ret;
}

ParserState Parser::saveState()
{
    return ParserState(curToken, lookAhead, previousToken, tokens, tokenFormatter);
}
void Parser::restoreState(ParserState s)
{
    this->curToken = s.curToken;
    this->lookAhead = s.lookAhead;
    this->previousToken = previousToken;
    this->tokens = s.tokens;
    this->tokenFormatter = s.tokenFormatter;
}

ParserState::ParserState(int curToken, Token lookAhead, Token previousToken, QVector<Token>tokens, QString(*tokenFormatter)(int))
{
    this->curToken = curToken;
    this->lookAhead = lookAhead;
    this->previousToken = previousToken;
    this->tokens = tokens;
    this->tokenFormatter = tokenFormatter;
}
