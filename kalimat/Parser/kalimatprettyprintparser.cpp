#include "parser_incl.h"
#include "../Lexer/kalimatlexer.h"

#include "kalimatprettyprintparser.h"

KalimatPrettyprintParser::KalimatPrettyprintParser()
{
}

KalimatPrettyprintParser::~KalimatPrettyprintParser()
{

}

void KalimatPrettyprintParser::init(QString s, Lexer *lxr, void *tag)
{
    init(s, lxr, tag, "");
}

void KalimatPrettyprintParser::init(QString s, Lexer *lxr, void *tag, QString fileName)
{
    Parser::init(s, lxr, tag, fileName);
}

shared_ptr<AST> KalimatPrettyprintParser::parseRoot()
{
    BlockPP *ret = new BlockPP();
    QStack<BlockPP *> levels;
    QStack<QString> expectedBlockEnders;
    levels.push(ret);
    QVector<LinePP *> lines = parseLines();
    QStringList all;
    for(int i=0; i<lines.count(); i++)
        all.append(QString("%1: %2").arg(i).arg(lines[i]->toString()));
    QString lns = all.join("\n");
    for(int i=0; i<lines.count(); i++)
    {
        QString ender;
        if(levels.empty())
            throw ParserException("weird indentation");
        LinePP *line = lines[i];

        if(is_indentation_starter(line, ender))
        {
            line->blockfollows = true;
            ret->append(line);
            BlockPP *indent = new BlockPP();
            levels.push(indent);
            ret->append(indent);
            ret = indent;
            expectedBlockEnders.push(ender);
        }
        else if(is_indentation_ender(line))
        {
            if(expectedBlockEnders.empty() || (!match_ender(line, expectedBlockEnders.top())))
            {
                //throw ParserException("Non-matching block delimiters");
                ret->append(line);
            }
            else
            {
                expectedBlockEnders.pop();
                levels.pop();
                ret = levels.top();
                line->afterblock = true;
                ret->append(line);
            }
        }
        else if(is_indentation_ender_and_starter(line))
        {
            levels.pop();
            ret = levels.top();
            ret->append(line);

            BlockPP *indent = new BlockPP();
            levels.push(indent);
            ret->append(indent);
            ret = indent;
        }
        else
        {
            ret->append(line);
        }
    }

    while(levels.count() > 1)
    {
        levels.pop();
        ret = levels.top();
    }

    return shared_ptr<BlockPP>(ret);
}

QVector<LinePP *>KalimatPrettyprintParser::parseLines()
{
    QVector<LinePP *> ret;
    LinePP *line = new LinePP();
    while(true)
    {
        if(eof())
        {
            if(line->code.count() != 0)
                ret.append(line);
            break;
        }
        else if(lookAhead.Is(NEWLINE))
        {
            if(line->code.count() != 0)
                ret.append(line);
            line = new LinePP();
        }
        else if(lookAhead.Is(COMMENT))
        {
            line->append(new TokPP(lookAhead));
            ret.append(line);
            line = new LinePP();
        }
        else
        {
            line->append(new TokPP(lookAhead));
        }
        match(lookAhead.Type);
    }
    return ret;
}

bool KalimatPrettyprintParser::is_indentation_starter(LinePP *line, QString &ender)
{
    int libraryDecl[] = { LIBRARY, STR_LITERAL, COLON };
    int classDecl[] = { CLASS, IDENTIFIER, COLON };
    int ifStmtStart[] = { IF }, ifStmtEnd[] = { COLON };
    int forStmtStart[] = { FORALL }, forStmtEnd[] = { COLON };
    int whileStmtStart[] = { WHILE }, whileStmtEnd[] = { COLON };
    int selectStmtStart[] = { SELECT }, selectStmtEnd[] = { COLON };
    int procDeclStart[] = { PROCEDURE, IDENTIFIER, LPAREN }, procDeclEnd[] = { RPAREN, COLON };
    int funcDeclStart[] = { FUNCTION, IDENTIFIER, LPAREN }, funcDeclEnd[] = { RPAREN, COLON };
    int responseDeclStart[] = { RESPONSEOF, IDENTIFIER, IDENTIFIER }, responseDeclEnd[] = { RPAREN, COLON };
    int replyDeclStart[] = { REPLYOF, IDENTIFIER, IDENTIFIER }, replyDeclEnd[] = { RPAREN, COLON };

    if(line->tokensEqual(classDecl, 3) ||
       line->tokensEqual(libraryDecl, 3) ||
       line->tokensBeginEnd(procDeclStart, procDeclEnd, 3, 2) ||
       line->tokensBeginEnd(funcDeclStart, funcDeclEnd, 3, 2) ||
       line->tokensBeginEnd(responseDeclStart, responseDeclEnd, 3, 2) ||
       line->tokensBeginEnd(replyDeclStart, replyDeclEnd, 3, 2)
       )
    {
        ender = QString::fromStdWString(L"نهاية");
        return true;
    }
    else if(line->tokensBeginEnd(ifStmtStart, ifStmtEnd, 1,1))
    {
        ender = QString::fromStdWString(L"تم");
        return true;
    }
    else if(line->tokensBeginEnd(forStmtStart, forStmtEnd, 1,1) ||
            line->tokensBeginEnd(whileStmtStart, whileStmtEnd, 1,1))
    {
        ender = QString::fromStdWString(L"تابع");
        return true;
    }
    else if(line->tokensBeginEnd(selectStmtStart, selectStmtEnd,1,1))
    {
        ender = QString::fromStdWString(L"تم");
        return true;
    }
    return false;
}

bool KalimatPrettyprintParser::is_indentation_ender(LinePP *line)
{
    QVector<Token> toks = line->toTokens();
    if(toks.count() != 1)
        return false;
    Token token = toks[0];
    return token.Is(CONTINUE) || token.Is(END) || token.Is(DONE);
}

bool KalimatPrettyprintParser::is_indentation_ender_and_starter(LinePP *line)
{
    int elsePart[] = { ELSE, COLON };
    int elseIfStart[] = { ELSE, IF }, elseIfEnd[] = { COLON };

    int sendBegin[] = {SEND }, sendEnd[] = { COLON };
    int recvBegin[] = {RECEIVE}, recvEnd[] = { COLON };
    int orSendBegin[] = { OR, SEND }, orSendEnd[] = { COLON };
    int orRecvBegin[] = {OR, RECEIVE}, orRecvEnd[] = { COLON };

    if(line->tokensEqual(elsePart, 2) ||
       line->tokensBeginEnd(elseIfStart, elseIfEnd, 2, 1))
    {
        return true;
    }

    if(line->tokensBeginEnd(sendBegin, sendEnd, 1, 1) ||
       line->tokensBeginEnd(recvBegin, recvEnd, 1, 1) ||
       line->tokensBeginEnd(orSendBegin, orSendEnd, 2, 1) ||
       line->tokensBeginEnd(orRecvBegin, orRecvEnd, 2, 1))
    {
        return true;
    }

    return false;
}

bool KalimatPrettyprintParser::match_ender(LinePP *line, QString ender)
{
    QVector<Token> toks = line->toTokens();
    if(toks.count() != 1)
        return false;
    Token token = toks[0];
    return token.Lexeme == ender;
}

bool LinePP::tokensEqual(int ids[], int count)
{
    QVector<Token> toks = toTokens();
    if(toks.count() != count)
        return false;
    for(int i=0; i<count; i++)
    {
        if(toks[i].Type != ids[i])
            return false;
    }
    return true;
}

bool LinePP::tokensBeginEnd(int ids1[], int ids2[], int count1, int count2)
{
    QVector<Token> toks = toTokens();
    if(toks.count()< count1 || toks.count()< count2)
        return false;
    for(int i=0; i<count1; i++)
    {
        if(toks[i].Type != ids1[i])
            return false;
    }

    for(int i=count2-1, j = toks.count() - 1; i>=0; i--,j--)
    {
        if(toks[j].Type != ids2[i])
            return false;
    }
    return true;
}

QVector<Token> LinePP::toTokens()
{
    QVector<Token> ret;
    for(int i=0; i<code.count(); i++)
    {
        ret.append(((TokPP *) code[i])->token);
    }
    return ret;
}

QString TokPP::toString()
{
    return token.Lexeme;
}

QString LinePP::toString()
{
    QStringList ret;
    for(int i=0; i<code.count(); i++)
        ret.append(code[i]->toString());
    return ret.join("");
}

QString BlockPP::toString()
{
    QStringList ret;
    for(int i=0; i<subs.count(); i++)
        ret.append(subs[i]->toString());
    return ret.join("\n");
}

bool isOperator(TokenType t)
{
    bool ret = false;
    switch(t)
    {
    case ADD_OP:
    case SUB_OP:
    case MUL_OP:
    case DIV_OP:
    case AND:
    case OR:
    case NOT:
    case LT:
    case GT:
    case LE:
    case GE:
    case EQ:
    case NE:
        ret = true;
    }
    return ret;
}

bool isKw(TokenType t)
{
    return t <= KEYWORD_CUTOFF;
}

void TokPP::prettyPrint(CodeFormatter *cf)
{
    if(isOperator(token.Type))
    {
        cf->space();
    }

    if(isKw(token.Type))
    {
        cf->printKw(token.Lexeme);
    }
    else if(token.Type == IDENTIFIER)
    {
        cf->print(token.Lexeme);
        cf->space();
    }
    else if(token.Type == NUM_LITERAL)
    {
        cf->print(token.Lexeme);
        cf->space();
    }
    else if(token.Type == STR_LITERAL)
    {
        cf->printColored(token.Lexeme, QColor::fromCmyk(0, 255, 0,0));
    }
    else if(token.Type == COMMENT)
    {
        cf->print(token.Lexeme.replace("\n", "").trimmed());
    }
    else if(token.Type == COMMA)
    {
        cf->comma();
    }
    else
    {
        cf->print(token.Lexeme);
    }

    if(isOperator(token.Type))
    {
        cf->space();
    }
}

void LinePP::prettyPrint(CodeFormatter *cf)
{
    TokPP *tok;
    for(int i=0; i<code.count(); i++)
    {
        code[i]->prettyPrint(cf);
        if(i+1<code.count())
        {
            tok = dynamic_cast<TokPP *>(code[i+1]);
            if(tok)
            {
                if(isKw(tok->token.Type))
                {
                    cf->space();
                }
            }
        }
    }
    cf->nl();
}

void BlockPP::prettyPrint(CodeFormatter *cf)
{
    bool afterComment = false;
    static int comment[] = {COMMENT};
    for(int i=0; i<subs.count(); i++)
    {
        KPPAST *pp = subs[i];
        BlockPP *subBlk = dynamic_cast<BlockPP *>(pp);
        if(subBlk != NULL)
        {
            cf->indent();
            subBlk->prettyPrint(cf);
            cf->deindent();
        }
        else
        {
            LinePP *line = dynamic_cast<LinePP *>(subs[i]);
            if(line == NULL)
            {
                throw ParserException("weird Pretty print element");
            }
            if(line->blockfollows && i>0 &&!afterComment)
            {
                cf->blankLine();
            }

            line->prettyPrint(cf);
            if(line->tokensEqual(comment, 1))
                afterComment = true;
            else
                afterComment = false;

            if(line->afterblock && (i+1 < subs.count()))
            {
                cf->blankLine();
            }
        }
    }
}
