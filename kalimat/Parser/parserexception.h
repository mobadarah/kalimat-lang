#ifndef PARSEREXCEPTION_H
#define PARSEREXCEPTION_H

#include <QString>

class ParserException
{
public:
    Token pos;
    bool hasPosInfo;
    QString message;
    int ErrType;
public:

    ParserException(QString msg);
    ParserException(Token pos, QString msg);
    ParserException(Token pos, int type);
    bool hasType() { return ErrType != -1;}
    int errType() { return ErrType;}
};

class ParserBacktrackingException
{

};

#endif // PARSEREXCEPTION_H
