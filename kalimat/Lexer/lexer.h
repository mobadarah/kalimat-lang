/**************************************************************************
** The Kalimat programming language
**   Copyright (C) 2010 Mohamed Samy Ali - samy2004@gmail.com
**   This project is released under the Apache License version 2.0
**   as described in the included license.txt file
**************************************************************************/

#ifndef LEXER_H
#define LEXER_H

class Lexer {

    int state;
    QVector<Token> acceptedTokens;
    void *tokenTag;
public:
    Lexer();
    Lexer(StateMachine sm);
    void init(QString s);
    void init(QString s, void *tag);
    void tokenize(); // runs the state machine and keeps adding accepted tokens to a vector
    QVector<Token> getTokens(); // returns all accepted tokens
protected:
    StateMachine stateMachine;
    Buffer buffer;
    virtual Token accept(TokenType);
};
class UnexpectedEndOfFileException
{
};

class UnexpectedCharException
{
    QVector<Predicate *> _transitions;
    QString gotThis;
    int line, column;
    int state;
public:
    UnexpectedCharException(QString _gotThis, QVector<Predicate *> possibleTransitions, int line, int column, int state);
    QString buildMessage();
};


#endif // LEXER_H
