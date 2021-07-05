#ifndef PARSEREXCEPTION_H
#define PARSEREXCEPTION_H

#include <QString>

class ParserException
{
public:
    Token pos;
    bool hasPosInfo;
    QString message;
    QString fileName;
    int ErrType;
public:

    ParserException(QString fileName, QString msg);
    ParserException(QString fileName, Token pos, QString msg);
    ParserException(QString fileName, Token pos, int type);
    bool hasType() { return ErrType != -1;}
    int errType() { return ErrType;}
};

class ParserBacktrackingException
{

};

#endif // PARSEREXCEPTION_H
