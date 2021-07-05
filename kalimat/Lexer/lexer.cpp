/**************************************************************************
**   The Kalimat programming language
**   Copyright (C) 2010 Mohamed Samy Ali - samy2004@gmail.com
**   This project is released under the Apache License version 2.0
**   as described in the included license.txt file
**************************************************************************/

#include "lexer_incl.h"

Lexer::Lexer()
{

}
Lexer::Lexer(StateMachine sm)
{
    stateMachine = sm;
}

void Lexer::init(QString s, void *tag, QString fileName)
{
    this->tokenTag = tag;
    state = 0; // The start state is always assumed = 0
    buffer.init(s);
    acceptedTokens.clear();
    this->fileName = fileName;
}

void Lexer::init(QString s, void *tag)
{
    init(s, tag, "");
}

void Lexer::init(QString s)
{
    init(s, NULL);
}

QVector<Token> Lexer::getTokens()
{
    return acceptedTokens;
}

void Lexer::tokenize(bool ignoreUnexpectedChar)
{
    QChar c;
    State nextState;
    Action *action;
    bool accepted;
    TokenType acceptedType;
    bool readEof = false;
    while(true)
    {
        if(buffer.eof())
            readEof = true; // schedule terminating after next loop iteration

        if(stateMachine.Transition(state, buffer, nextState, accepted, acceptedType, action))
        {
            state= nextState;
            if(action != NULL)
                (*action)(buffer);
        }
        else
        {
            if(!ignoreUnexpectedChar)
            {
                QString curChar;
                if(buffer.eof())
                    curChar = "<EOF>";
                else
                    curChar = QString("%1").arg(buffer.read());
                throw UnexpectedCharException(curChar, stateMachine.GetPossibleTransitions(state), buffer.GetLine(), buffer.GetColumn(), buffer.GetPos(), state, fileName);
            }
            else
            {
                int l,c,p;
                if(!buffer.eof())
                {
                    buffer.read();
                    buffer.accept(l,c,p);
                }
                state = 0;
            }
        }
        if(accepted)
        {
            Token t = accept(acceptedType);
            if(t.Type != TokenNone)
                acceptedTokens.append(t);
        }
        if(readEof)
        {
            // We cannot exit the loop whenever readEof is true, because
            // an action might have retracted the buffer to before the EOF position
            // therefore we also test if we are still at the EOF position before breaking
            // the loop
            if(buffer.eof())
                break;
            else
                readEof = false;
        }
    }
    // Now we've reached end of input. Are we at an 'accept at EOF' state?
    /*
    if(stateMachine.StateHasFlag(state, StateAcceptAtEof))
    {
        Token t = accept();
        acceptedTokens.append(t);
    }
    else
    {
        throw UnexpectedEndOfFileException(fileName);
    }
    */
}
/*
void Trace(Lexer *lxr)
{
        cout <<lxr->_source<<endl;
        for(int i=0; i<=lxr->_end; i++)
        {
                if(i==lxr->_start && (lxr->_start==lxr->_end))
                        cout<<"!";
                if(i==lxr->_start)
                        cout <<"^";
                else if (i==lxr->_end)
                        cout <<"$";
                else
                        cout<<" ";
        }
        cout <<endl;

}
*/
Token Lexer::accept(TokenType type)
{
    /*
    if(stateMachine.IsStateGoBack(state))
    {
        buffer.retract(1);
    }
    */
    Token ret;
    int l,c, p;
    QString lexeme = buffer.accept(l, c, p);
    ret.Line =l;
    ret.Column = c;
    ret.Pos = p;
    ret.tag = tokenTag;

    ret.Lexeme = lexeme;
    ret.Type = type;
    state = 0;
    return ret;
}
UnexpectedCharException::UnexpectedCharException(QString _gotThis, QVector<Predicate *> possibleTransitions, int _line, int _column, int _pos, int _state, QString fileName)
{
    _transitions = possibleTransitions;
    gotThis = _gotThis;
    if(gotThis =="\n")
        gotThis = "\\n";
    line = _line;
    column = _column;
    pos = _pos;
    state = _state;
    this->fileName = fileName;
}
QString UnexpectedCharException::buildMessage()
{
    return QString("Unexpected char:'%1'',%2(%3,%4, %5) state=%6, char code =%7").arg(gotThis)
            .arg(fileName!=""?QString(":%1").arg(fileName):"").arg(line).arg(column).arg(pos).arg(state).arg(gotThis[0].unicode());
}
