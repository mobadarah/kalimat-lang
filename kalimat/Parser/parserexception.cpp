#include "../Lexer/token.h"
#include "parserexception.h"

ParserException::ParserException(QString fileName, QString msg)
{
    this->fileName = fileName;
    message = msg;
    hasPosInfo = false;
    ErrType = -1;
}

ParserException::ParserException(QString fileName, Token pos, QString msg)
{
    this->fileName = fileName;
    if(pos.Type == TokenInvalid)
        hasPosInfo = false;
    else
        hasPosInfo = true;
    this->pos = pos;
    message = QString("%5(%1,%2, pos=%3): %4")
            .arg(pos.Line)
            .arg(pos.Column)
            .arg(pos.Pos)
            .arg(msg)
            .arg((fileName!="")?QFileInfo(fileName).fileName()+":" : "");
    ErrType = -1;
}

ParserException::ParserException(QString fileName, Token pos, int type)
{
    this->fileName = fileName;
    if(pos.Type == TokenInvalid)
        hasPosInfo = false;
    else
        hasPosInfo = true;
    this->pos = pos;
    ErrType = type;
}
