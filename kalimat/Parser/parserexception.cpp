#include "../Lexer/token.h"
#include "parserexception.h"

ParserException::ParserException(QString msg)
{
    message = msg;
    hasPosInfo = false;
    ErrType = -1;
}
ParserException::ParserException(Token pos, QString msg)
{
    if(pos.Type == TokenInvalid)
        hasPosInfo = false;
    else
        hasPosInfo = true;
    this->pos = pos;
    message = QString("(%1,%2, pos=%3): %4").arg(pos.Line).arg(pos.Column).arg(pos.Pos).arg(msg);
    ErrType = -1;
}

ParserException::ParserException(Token pos, int type)
{
    if(pos.Type == TokenInvalid)
        hasPosInfo = false;
    else
        hasPosInfo = true;
    this->pos = pos;
    ErrType = type;
}
