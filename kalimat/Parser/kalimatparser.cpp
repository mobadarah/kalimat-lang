/**************************************************************************
**   The Kalimat programming language
**   Copyright (C) 2010 Mohamed Samy Ali - samy2004@gmail.com
**   This project is released under the Apache License version 2.0
**   as described in the included license.txt file
**************************************************************************/

#include "parser_incl.h"
#include "../Lexer/kalimatlexer.h"
#include "kalimatast.h"
#include "kalimatparser.h"


KalimatParser::KalimatParser() : Parser(TokenNameFromId)
{
}
KalimatParser::~KalimatParser()
{
}
void KalimatParser::init(QString s, Lexer *lxr)
{
    // Here we manually filter out comments from our token stream.
    // The comments are not automatically skipped because automatic
    // skipping happens only for tokens of type TokenNone and
    // we cannot make our token type "TokenNone" for comments since
    // we need them in the lexical analyzer's output for syntax hilighting

    // todo: Find a more efficient way for this
    // todo: This code is very tightly coupled with Parser::init(...)

    Parser::init(s, lxr);
    QVector<Token> tokens2;
    for(int i=0; i<tokens.count(); i++)
    {
        if(tokens[i].Type != COMMENT)
            tokens2.append(tokens[i]);
    }
    tokens = tokens2;
}

AST *KalimatParser::parseRoot()
{
    return program();
}
AST *KalimatParser::program()
{
    QVector<TopLevel *> elements;
    QVector<Statement *> topLevelStatements;
    ProceduralDecl *entryPoint;
    QVector<StrLiteral *> usedModules = usingDirectives();
    while(!eof())
    {
        // Declaration has to be tested first because of possible
        // ambiguity with identifiers:
        // ID GLOBAL => declaration
        // ID ...    => statement
        if(LA_first_declaration())
        {
            elements.append(declaration());
        }
        else if(LA_first_statement())
        {
            topLevelStatements.append(statement());
        }
        else
        {
            throw new ParserException(getPos(), "Expected statement or declaration");
        }
        newLines();
    }
    entryPoint = new ProcedureDecl(Token(),new Identifier(Token(), "%main"), QVector<Identifier *>(), new BlockStmt(Token(),topLevelStatements), true);
    elements.append(entryPoint);
    return new Program(Token(),elements, usedModules);
}

AST *KalimatParser::module()
{
    QVector<Declaration *> elements;

    newLines();
    match(UNIT);
    Identifier *modName = identifier();
    match(NEWLINE);
    newLines();
    QVector<StrLiteral *> usedModules = usingDirectives();

    while(!eof())
    {
        if(LA_first_declaration())
        {
            elements.append(declaration());
        }
        else if(LA_first_statement())
        {
            throw new ParserException(getPos(), "Modules cannot contain statements");
        }
        else
        {
            throw new ParserException(getPos(), "Expected declaration");
        }
        newLines();
    }
    return new Module(Token(), modName, elements, usedModules);
}


bool KalimatParser::LA_first_statement()
{
    return LA(IF) || LA(FORALL) || LA(WHILE) || LA(RETURN) || LA(LABEL) || LA(GO) || LA(WHEN) || LA_first_io_statement() || LA_first_grfx_statement()
            || LA_first_assignment_or_invokation();
}

Statement *KalimatParser::statement()
{
    if(LA_first_assignment_or_invokation())
    {
        ParserState state= this->saveState();
        return assignmentStmt_or_Invokation(state);
    }
    if(LA(IF))
    {
        return ifStmt();
    }
    if(LA(FORALL))
    {
        return forEachStmt();
    }
    if(LA(WHILE))
    {
        return whileStmt();
    }
    if(LA(RETURN))
    {
        return returnStmt();
    }
    if(LA(LABEL))
    {
        return labelStmt();
    }
    if(LA(GO))
    {
        return gotoStmt();
    }
    if(LA(WHEN))
    {
        return eventHandlerStmt();
    }
    if(LA_first_io_statement())
    {
        return ioStmt();
    }
    if(LA_first_grfx_statement())
    {
        return grfxStatement();
    }
    throw new ParserException(getPos(), "statement not implemented");
}

bool KalimatParser::LA_first_declaration()
{
    return LA(PROCEDURE) || LA(FUNCTION) || LA(CLASS) || LA_first_method_declaration() || LA2(IDENTIFIER, GLOBAL) ;
}
Declaration *KalimatParser::declaration()
{
    if(LA(PROCEDURE))
    {
        return procedureDecl();
    }
    if(LA(FUNCTION))
    {
        return functionDecl();
    }
    if(LA(CLASS))
    {
        return classDecl();
    }
    if(LA_first_method_declaration())
    {
        return methodDecl();
    }
    if(LA2(IDENTIFIER, GLOBAL))
    {
        return globalDecl();
    }
    throw new ParserException(getPos(), "Expected a declaration");
}
bool KalimatParser::LA_first_assignment_or_invokation()
{
    return LA_first_primary_expression();
}
Statement *KalimatParser::assignmentStmt_or_Invokation(ParserState s)
{
    Expression *first = primaryExpression();
    if(LA(EQ))
    {
        AssignableExpression *id = dynamic_cast<AssignableExpression *>(first);
        if(id == NULL)
        {
            throw new ParserException(getPos(), "Left of = must be an assignable expression");
        }
        Token eqToken = lookAhead;
        match(EQ);
        Expression *value = expression();
        return new AssignmentStmt(eqToken, id, value);
    }
    else
    {
        IInvokation *invokation = dynamic_cast<IInvokation *>(first);
        if(invokation !=NULL)
        {
            return new InvokationStmt(invokation->getPos(), invokation);
        }
    }
    /*else if(LA(LPAREN))
    {
        restoreState(s);
        Expression *invokation = expression();
        return new InvokationStmt(invokation->getPos(), invokation);
    }
    else if(LA(COLON))
    {
        restoreState(s);
        Expression *invokation = expression();
        return new InvokationStmt(invokation->getPos(), invokation);
    }*/
    throw new ParserException(getPos(), "Expected IDENTIFIER");
}

Statement *KalimatParser::ifStmt()
{
    Statement *thenPart;
    Statement *elsePart = NULL;
    if(LA(IF))
    {
        bool newLine = false;
        Token ifTok = lookAhead;
        match(IF);
        Expression *cond = expression();
        match(COLON);
        if(LA(NEWLINE))
        {
            match(NEWLINE);
            newLine = true;
        }

        if(newLine)
        {
            thenPart = block();
            //match(NEWLINE);
        }
        else
            thenPart = statement();

        if(LA(ELSE))
        {
            match(ELSE);
            match(COLON);
            if(newLine)
                match(NEWLINE);
            if(newLine)
            {
                elsePart = block();
           //     match(NEWLINE);
            }
            else
                elsePart = statement();
        }
        if(newLine)
        {
         //  match(NEWLINE);
           match(DONE);
        }
        return new IfStmt(ifTok, cond, thenPart, elsePart);
    }
    throw new ParserException(getPos(), "Expected IF");
}
Statement *KalimatParser::forEachStmt()
{
    bool multiLineStmt = false;
    Statement *theStmt = NULL;
    Token forAllTok = lookAhead;
    match(FORALL);
    Identifier *id = identifier();
    match(FROM);
    Expression *from = expression();
    match(TO);
    Expression *to = expression();
    match(COLON);
    if(LA(NEWLINE))
    {
        match(NEWLINE);
        multiLineStmt = true;
    }
    if(multiLineStmt)
    {
        theStmt = block();
    }
    else
    {
        theStmt = statement();
    }
    match(CONTINUE);
    return new ForAllStmt(forAllTok, id, from, to, theStmt);

}
Statement *KalimatParser::whileStmt()
{
    bool multiLineStmt = false;
    Statement *theStmt = NULL;
    Token whileTok = lookAhead;
    match(WHILE);
    Expression *cond = expression();
    match(COLON);
    if(LA(NEWLINE))
    {
        match(NEWLINE);
        multiLineStmt = true;
    }
    if(multiLineStmt)
    {
        theStmt = block();
    }
    else
    {
        theStmt = statement();
    }
    match(CONTINUE);
    return new WhileStmt(whileTok, cond, theStmt);
}
Statement *KalimatParser::returnStmt()
{
    Token returnTok  = lookAhead;
    match(RETURN);
    match(WITH);
    Expression *retVal = expression();
    return new ReturnStmt(returnTok, retVal);
}

Statement *KalimatParser::labelStmt()
{
    Token pos = lookAhead;
    match(LABEL);
    Expression *target = expression();
    return new LabelStmt(pos, target);
}

Statement *KalimatParser::gotoStmt()
{
    Token pos = lookAhead;
    Expression *target = NULL;
    match(GO);
    match(TO);
    bool targetIsNum = false;
    if(LA(NUM_LITERAL))
    {
        targetIsNum = true;
        target = new NumLiteral(lookAhead, lookAhead.Lexeme);
        match(NUM_LITERAL);
    }
    else if(LA(IDENTIFIER))
    {
        targetIsNum = false;
        target = identifier();
    }
    else
    {
        throw new ParserException("An identifier or number is expected after 'goto'");
    }
    return new GotoStmt(pos, targetIsNum, target);
}

bool KalimatParser::LA_first_io_statement()
{
    return LA(PRINT) || LA(READ);
}
Statement *KalimatParser::ioStmt()
{
    if(LA(PRINT))
    {
        bool printOnSameLine = false;
        QVector<Expression *> args;
        QVector<Expression *> widths;
        Expression *fileObject = NULL;
        Token printTok  = lookAhead;
        match(PRINT);
        if(LA(IN))
        {
            match(IN);
            fileObject = expression();
            match(COLON);
        }
        if(LA(USING_WIDTH))
        {
            match(USING_WIDTH);
            widths.append(expression());
        }
        else
        {
            widths.append(NULL);
        }
        args.append(expression());
        while(LA(COMMA))
        {
            match(COMMA);
            if(LA(ELLIPSIS))
            {
                match(ELLIPSIS);
                printOnSameLine = true;
                goto officialEndOfPrintStmt;
            }
            if(LA(USING_WIDTH))
            {
                match(USING_WIDTH);
                widths.append(expression());
            }
            else
            {
                widths.append(NULL);
            }
            args.append(expression());
        }
        if(LA(ELLIPSIS))
        {
            match(ELLIPSIS);
            printOnSameLine = true;
        }
        officialEndOfPrintStmt:
        return new PrintStmt(printTok, fileObject, args, widths, printOnSameLine);
    }
    if(LA(READ))
    {
        QString prompt = "";
        Token readTok  = lookAhead;
        Expression *fileObject = NULL;
        match(READ);
        if(LA(FROM))
        {
            match(FROM);
            fileObject = expression();
            match(COLON);
        }
        if(LA(STR_LITERAL))
        {
            prompt = prepareStringLiteral(lookAhead.Lexeme);
            match(STR_LITERAL);
            match(COMMA);
        }
        QVector<Identifier *> vars;
        QVector<bool> readNums;
        bool readInt = false;
        if(LA(HASH))
        {
            match(HASH);
            readInt = true;
        }
        vars.append(identifier());
        readNums.append(readInt);
        while(LA(COMMA))
        {
            match(COMMA);
            if(LA(HASH))
            {
                match(HASH);
                readInt = true;
            }
            else
            {
                readInt = false;
            }
            vars.append(identifier());
            readNums.append(readInt);
        }
        return new ReadStmt(readTok, fileObject, prompt, vars, readNums);
    }
    throw new ParserException(getPos(), "Expected PRINT or READ");
}
bool KalimatParser::LA_first_grfx_statement()
{
    return LA(DRAW_PIXEL) || LA(DRAW_LINE) || LA(DRAW_RECT) || LA(DRAW_CIRCLE) || LA(DRAW_SPRITE) || LA(ZOOM);
}
Statement *KalimatParser::grfxStatement()
{
    if(LA(DRAW_PIXEL))
        return drawPixelStmt();
    if(LA(DRAW_LINE))
        return drawLineStmt();
    if(LA(DRAW_RECT))
        return drawRectStmt();
    if(LA(DRAW_CIRCLE))
        return drawCircleStmt();
    if(LA(DRAW_SPRITE))
        return drawSpriteStmt();
    if(LA(ZOOM))
        return zoomStmt();
    throw new ParserException(getPos(), "Expected a drawing statement");
}
Statement *KalimatParser::drawPixelStmt()
{
    Expression *x, *y;
    Expression *color = NULL;
    Token tok  = lookAhead;
    match(DRAW_PIXEL);
    match(LPAREN);
    x = expression();
    match(COMMA);
    y = expression();
    match(RPAREN);
    if(LA(COMMA))
    {
        match(COMMA);
        color = expression();
    }
    return new DrawPixelStmt(tok, x,y, color);
}
Statement *KalimatParser::drawLineStmt()
{
    Expression *x1 = NULL, *y1 = NULL;
    Expression *x2, *y2;
    Expression *color = NULL;

    Token tok  = lookAhead;
    match(DRAW_LINE);

    if(LA(LPAREN))
    {
        match(LPAREN);
        x1 = expression();
        match(COMMA);
        y1 = expression();
        match(RPAREN);
    }

    match(SUB_OP);
    match(LPAREN);
    x2 = expression();
    match(COMMA);
    y2 = expression();
    match(RPAREN);

    if(LA(COMMA))
    {
        match(COMMA);
        color = expression();
    }
    return new DrawLineStmt(tok, x1, y1, x2, y2, color);
}
Statement *KalimatParser::drawRectStmt()
{
    Expression *x1 = NULL, *y1 = NULL;
    Expression *x2, *y2;
    Expression *color = NULL;
    bool filled = false;

    Token tok  = lookAhead;
    match(DRAW_RECT);
    if(LA(LPAREN))
    {
        match(LPAREN);
        x1 = expression();
        match(COMMA);
        y1 = expression();
        match(RPAREN);
    }

    match(SUB_OP);
    match(LPAREN);
    x2 = expression();
    match(COMMA);
    y2 = expression();
    match(RPAREN);

    if(LA(COMMA))
    {
        match(COMMA);
        if(!LA(COMMA))
        {
            color = expression();
        }
        if(LA(COMMA))
        {
            match(COMMA);
            expression();
            filled = true;
        }
    }
    return new DrawRectStmt(tok, x1, y1, x2, y2, color, filled);
}
Statement *KalimatParser::drawCircleStmt()
{
    Expression *cx, *cy;
    Expression *radius;
    Expression *color = NULL;
    bool filled = false;

    Token tok  = lookAhead;
    match(DRAW_CIRCLE);
    match(LPAREN);
    cx = expression();
    match(COMMA);
    cy = expression();
    match(RPAREN);
    match(COMMA);
    radius = expression();

    if(LA(COMMA))
    {
        match(COMMA);
        if(!LA(COMMA))
        {
            color = expression();
        }
        if(LA(COMMA))
        {
            match(COMMA);
            expression();
            filled = true;
        }
    }
    return new DrawCircleStmt(tok, cx, cy, radius, color, filled);
}
Statement *KalimatParser::drawSpriteStmt()
{
    Expression *x, *y;
    Expression *number;

    Token tok  = lookAhead;
    match(DRAW_SPRITE);
    number = expression();
    match(IN);

    match(LPAREN);
    x = expression();
    match(COMMA);
    y = expression();
    match(RPAREN);

    return new DrawSpriteStmt(tok, x, y, number);
}
Statement *KalimatParser::zoomStmt()
{

    Expression *x1 = NULL, *y1 = NULL;
    Expression *x2, *y2;

    Token tok  = lookAhead;
    match(ZOOM);
    //if(LA(LPAREN))
    {
        match(LPAREN);
        x1 = expression();
        match(COMMA);
        y1 = expression();
        match(RPAREN);
    }

    match(SUB_OP);
    match(LPAREN);
    x2 = expression();
    match(COMMA);
    y2 = expression();
    match(RPAREN);

    return new ZoomStmt(tok, x1, y1, x2, y2);
}
Statement *KalimatParser::eventHandlerStmt()
{
    EventType type;
    Identifier *proc = NULL;
    Token tok  = lookAhead;
    match(WHEN);
    match(EVENT);

    if(LA(KB))
    {
        match(KB);
        type = KalimatKbEvent;
    }
    else if(LA(MOUSE))
    {
        match(MOUSE);
        type = KalimatMouseEvent;
    }
    else if(LA(COLLISION))
    {
        match(COLLISION);
        type = KalimatSpriteCollisionEvent;
    }
    else
    {
        throw new ParserException("Expected KB or MOUSE");
    }
    match(DO);
    proc = identifier();
    return new EventStatement(tok, type, proc);
}

BlockStmt *KalimatParser::block()
{
    QVector<Statement *> stmts;
    Token tok = lookAhead;
    newLines();
    while(LA_first_statement())
    {
        stmts.append(statement());
        newLines();
    }
    return new BlockStmt(tok, stmts);
}

QVector<Identifier *> KalimatParser::formalParamList()
{
    QVector<Identifier *> formals;

    match(LPAREN);
    if(LA(IDENTIFIER))
    {
      formals.append(identifier());
      while(LA(COMMA))
      {
          match(COMMA);
          formals.append(identifier());
      }
    }
    match(RPAREN);
    return formals;
}

Declaration *KalimatParser::procedureDecl()
{

    match(PROCEDURE);
    Token tok  = lookAhead;
    Identifier *procName = identifier();
    QVector<Identifier *> formals = formalParamList();

    match(COLON);
    match(NEWLINE);
    ProcedureDecl *ret = new ProcedureDecl(tok, procName, formals, NULL, true);

    varContext.push(ret);
    BlockStmt *body = block();
    varContext.pop();

    match(END);
    ret->body(body);
    return ret;
}
Declaration *KalimatParser::functionDecl()
{
    match(FUNCTION);
    Token tok  = lookAhead;
    Identifier *procName = identifier();
    QVector<Identifier *> formals=formalParamList();
    match(COLON);
    match(NEWLINE);
    FunctionDecl *ret = new FunctionDecl(tok, procName, formals, NULL, true);

    varContext.push(ret);
    BlockStmt *body = block();
    varContext.pop();
    match(END);

    ret->body(body);
    return ret;
}
Declaration *KalimatParser::classDecl()
{
    QVector<Identifier *> fields;
    QMap<QString, MethodInfo> methods;

    match(CLASS);
    Token tok  = lookAhead;
    Identifier *className = identifier();
    Identifier *ancestorName = NULL;
    match(COLON);
    match(NEWLINE);
    newLines();
    while(LA(HAS)|| LA(RESPONDS) || LA(REPLIES) || LA(BUILT))
    {
        if(LA(HAS))
        {
            match(HAS);
            fields.append(identifier());
            while(LA(COMMA))
            {
                match(COMMA);
                fields.append(identifier());
            }
            match(NEWLINE);
        }
        if(LA(RESPONDS))
        {
            match(RESPONDS);
            match(UPON);

            Identifier *methodName = identifier();
            QVector<Identifier *> formals = formalParamList();

            methods[methodName->name] = MethodInfo(formals.count(), false);
            while(LA(COMMA))
            {
                match(COMMA);
                Identifier *methodName = identifier();
                QVector<Identifier *> formals= formalParamList();
                methods[methodName->name] = MethodInfo(formals.count(), false);
            }
            match(NEWLINE);
        }
        if(LA(REPLIES))
        {
            match(REPLIES);
            match(ON);

            Identifier *methodName = identifier();
            QVector<Identifier *> formals = formalParamList();

            methods[methodName->name] = MethodInfo(formals.count(), true);
            while(LA(COMMA))
            {
                match(COMMA);
                Identifier *methodName = identifier();
                QVector<Identifier *> formals= formalParamList();
                methods[methodName->name] = MethodInfo(formals.count(), false);
            }
            match(NEWLINE);
        }
        if(LA(BUILT))
        {
            Token b = lookAhead;
            if(ancestorName != NULL)
                throw new ParserException(getPos(), "Class cannot inherit from more than one base class");
            match(BUILT);
            match(ON);
            ancestorName = identifier();

        }

        newLines();
    }
    match(END);
    return new ClassDecl(tok, ancestorName, className, fields, methods, true);

}
Declaration *KalimatParser::globalDecl()
{
    Identifier *var = NULL;
    Token tok  = lookAhead;
    var = identifier();
    match(GLOBAL);
    return new GlobalDecl(tok, var->name, true);
}
bool KalimatParser::LA_first_method_declaration()
{
    return LA(RESPONSEOF) || LA(REPLYOF);
}

Declaration *KalimatParser::methodDecl()
{
    bool isFunctionNotProcedure;
    Identifier *className;
    Identifier *receiverName;
    Identifier *methodName;

    QVector<Identifier *> formals;
    if(LA(RESPONSEOF))
    {
        isFunctionNotProcedure = false;
        match(RESPONSEOF);
        className = identifier();
        receiverName = identifier();
        match(UPON);
    }
    else if(LA(REPLYOF))
    {
        isFunctionNotProcedure = true;
        match(REPLYOF);
        className = identifier();
        receiverName = identifier();
        match(ON);
    }
    Token tok  = lookAhead;
    methodName = identifier();
    formals = formalParamList();
    match(COLON);
    match(NEWLINE);

    MethodDecl *ret = new MethodDecl(tok, className, receiverName, methodName, formals, NULL, isFunctionNotProcedure);

    varContext.push(ret);
    BlockStmt *body = block();
    varContext.pop();
    ret->body(body);
    match(END);
    return ret;

}
bool KalimatParser::LA_first_expression()
{
    return LA_first_primary_expression()
            || LA(NOT) || LA(ADD_OP) || LA(SUB_OP);
}

Expression *KalimatParser::expression()
{
    return andOrExpression();
}

Expression *KalimatParser::andOrExpression()
{
    Expression *t = notExpression();
    while(LA(AND) || LA(OR))
    {
        QString operation = getOperation(lookAhead);
        Token tok  = lookAhead;
        match(lookAhead.Type);
        Expression* t2 = notExpression();
        t = new BinaryOperation(tok, operation, t, t2);
    }
    return t;
}

Expression *KalimatParser::notExpression()
{
    bool _not = false;
    Token tok  = lookAhead;
    if(LA(NOT))
    {
        match(NOT);
        _not = true;
    }
    Expression *t = comparisonExpression();
    if(_not)
    {
        t = new UnaryOperation(tok, "not", t);
        while(LA(ANDNOT))
        {
            tok = lookAhead;
            match(ANDNOT);
            Expression *t2 = comparisonExpression();
            t = new BinaryOperation(tok, "and", t, new UnaryOperation(tok, "not", t2));
        }
    }

    return t;
}

Expression *KalimatParser::comparisonExpression()
{
    Expression *t = arithmeticExpression();

    while(LA(LT) || LA(GT) || LA(EQ) || LA(NE) || LA(LE) || LA(GE))
    {
        Token tok  = lookAhead;
        QString operation = getOperation(lookAhead);
        match(lookAhead.Type);
        Expression * t2 = arithmeticExpression();
        t = new BinaryOperation(tok, operation, t, t2);
    }
    return t;
}

Expression *KalimatParser::arithmeticExpression()
{
    Expression *t = multiplicativeExpression();

    while(LA(ADD_OP) || LA(SUB_OP))
    {
        Token tok  = lookAhead;
        QString operation = getOperation(lookAhead);
        match(lookAhead.Type);
        Expression * t2 = multiplicativeExpression();
        t = new BinaryOperation(tok, operation, t, t2);
    }
    return t;
}

Expression *KalimatParser::multiplicativeExpression()
{
    Expression *t = positiveOrNegativeExpression();
    while(LA(MUL_OP) || LA(DIV_OP))
    {
        Token tok  = lookAhead;
        QString operation = getOperation(lookAhead);
        match(lookAhead.Type);
        Expression * t2 = positiveOrNegativeExpression();
        t = new BinaryOperation(tok, operation, t, t2);
    }
    return t;
}

Expression *KalimatParser::positiveOrNegativeExpression()
{
    bool pos = false;
    bool neg = false;
    Token tok  = lookAhead;
    if(LA(ADD_OP))
    {
        match(ADD_OP);
        pos = true;
    }
    else if(LA(SUB_OP))
    {
        match(SUB_OP);
        neg = true;
    }

    Expression *t = primaryExpression();
    if(pos)
        t = new UnaryOperation(tok, "pos", t);
    else if(neg)
        t = new UnaryOperation(tok, "neg", t);

    return t;
}

Expression *KalimatParser::primaryExpression()
{
    Expression *ret = primaryExpressionNonInvokation();
    Token tok;
    while(LA(LPAREN) || LA(COLON) || LA(LBRACKET))
    {
        if(LA(LPAREN))
        {
            QVector<Expression *> args;
            tok = lookAhead;
            match(LPAREN);
            if(!LA(RPAREN))
            {
                args.append(expression());
                while(LA(COMMA))
                {
                    match(COMMA);
                    args.append(expression());
                }
            }
            match(RPAREN);
            ret = new Invokation(tok, ret, args);
        }
        if(LA(COLON))
        {
            ParserState s = saveState();
            try
            {
                match(COLON);
                tok = lookAhead;
                Identifier *methodName = identifier();
                QVector<Expression *> args;
                match(LPAREN);
                if(!LA(RPAREN))
                {
                    args.append(expression());
                    while(LA(COMMA))
                    {
                        match(COMMA);
                        args.append(expression());
                    }
                }
                match(RPAREN);
                ret = new MethodInvokation(tok, ret, methodName, args);
            }
            catch(ParserException *ex)
            {
                restoreState(s);
                break;
            }
        }
        if(LA(LBRACKET))
        {
            bool multiDim = false;
            tok = lookAhead;
            match(LBRACKET);
            Expression *index = expression();
            if(LA(COMMA))
            {
                multiDim = true;
                QVector<Expression *> indexes;
                indexes.append(index);

                match(COMMA);
                indexes.append(expression());

                while(LA(COMMA))
                {
                    match(COMMA);
                    indexes.append(expression());
                }
                ret = new MultiDimensionalArrayIndex(tok, ret, indexes);

            }
            match(RBRACKET);
            if(!multiDim)
            {
                ret = new ArrayIndex(tok, ret, index);
            }
        }

    }
    return ret;
}
bool KalimatParser::LA_first_primary_expression()
{
    return LA(NUM_LITERAL) || LA(NOTHING) || LA(STR_LITERAL) || LA(IDENTIFIER) || LA(LPAREN) || LA(FIELD);
}

Expression *KalimatParser::primaryExpressionNonInvokation()
{
    Expression *ret = NULL;
    if(LA(NUM_LITERAL))
    {
        ret = new NumLiteral(lookAhead, lookAhead.Lexeme);
        match(NUM_LITERAL); // will throw if wrong lookAhead
    }
    else if(LA(STR_LITERAL))
    {
        ret = new StrLiteral(lookAhead, prepareStringLiteral(lookAhead.Lexeme));
        match(STR_LITERAL); // will throw if wrong lookAhead
    }
    else if(LA(NOTHING))
    {
        ret = new NullLiteral(lookAhead);
        match(NOTHING);
    }
    else if(LA(C_TRUE))
    {
        ret = new BoolLiteral(lookAhead, true);
        match(lookAhead.Type);
    }
    else if(LA(C_FALSE))
    {
        ret = new BoolLiteral(lookAhead, false);
        match(lookAhead.Type);
    }
    else if(LA(LBRACKET))
    {
        Token lbPos = lookAhead;
        match(LBRACKET);
        QVector<Expression *> data = comma_separated_expressions();
        match(RBRACKET);
        ret = new ArrayLiteral(lbPos, data);
    }
    else if(LA(IDENTIFIER))
    {
        Token tok = lookAhead;
        Identifier *id = identifier();
        ret = id;
        if(LA(NEW))
        {
            Token newTok = lookAhead;
            match(NEW);
            ret = new ObjectCreation(newTok, id);
        }
        else if(LA(DOLLAR))
        {
            match(DOLLAR);
            Expression *modaf_elaih = primaryExpression();
            ret = new Idafa(id->getPos(), id, modaf_elaih);
        }
        else if(LA_first_primary_expression() && !LA(LPAREN))
        {
            ParserState s = saveState();
            Expression *modaf_elaih = primaryExpression();
            ret = new Idafa(id->getPos(), id, modaf_elaih);
        }

    }
    else if(LA(LPAREN))
    {
        match(LPAREN);
        ret = expression();
        match(RPAREN);
    }
    else if(LA(FIELD))
    {
        match(FIELD);
        match(LPAREN);
        Identifier *modaf = identifier();
        match(COMMA);
        Expression *modaf_elaih = expression();
        match(RPAREN);
        ret = new Idafa(modaf->getPos(), modaf, modaf_elaih);
    }
    else
    {
        throw new ParserException(getPos(), "Expected a literal, identifier, or parenthesized expression");
    }
    return ret;
}
Identifier *KalimatParser::identifier()
{
    Identifier *ret = NULL;
    if(LA(IDENTIFIER))
    {
        ret = new Identifier(lookAhead, lookAhead.Lexeme);
        match(IDENTIFIER);
        if(!varContext.empty())
            varContext.top()->addReference(ret);
        return ret;
    }
    throw new ParserException(getPos(), "Expected Identifier");
}
QVector<Expression *> KalimatParser::comma_separated_expressions()
{
    QVector<Expression *> ret;
    if(LA_first_primary_expression())
    {
        ret.append(expression());
        while(LA(COMMA))
        {
            match(COMMA);
            ret.append(expression());
        }
    }
    return ret;
}
QVector<StrLiteral *> KalimatParser::usingDirectives()
{
    QVector<StrLiteral *> usedModules;

    newLines();
    while(LA(USING))
    {
        match(USING);
        if(LA(STR_LITERAL))
        {
            StrLiteral *moduleName =  new StrLiteral(lookAhead, prepareStringLiteral(lookAhead.Lexeme));
            usedModules.append(moduleName);
            match(STR_LITERAL);
            match(NEWLINE);
            newLines();
        }
        else
        {
            throw new ParserException(getPos(), "USING keyword must be followed by a string literal");
        }
    }
    return usedModules;
}

QString KalimatParser::getOperation(Token token)
{
    // Currently we will use the return value
    // from here as in instruction in SmallVM
    // to make sure they match
    switch(token.Type)
    {
    case ADD_OP:
        return "add";
    case SUB_OP:
        return "sub";
    case MUL_OP:
        return "mul";
    case DIV_OP:
        return "div";
    case AND:
        return "and";
    case OR:
        return "or";
    case NOT:
        return "not";
    case LT:
        return "lt";
    case GT:
        return "gt";
    case LE:
        return "le";
    case GE:
        return "ge";
    case EQ:
        return "eq";
    case NE:
        return "ne";
    default:
        throw new ParserException(getPos(), "Unknown operator");
    }
}

QString KalimatParser::prepareStringLiteral(QString str)
{
    return str.mid(1, str.length()-2).replace("\"\"","\"");
}
void KalimatParser::newLines()
{
    while(LA(NEWLINE))
        match(NEWLINE);
}
