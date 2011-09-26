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

#include "../mainwindow.h" // temp todo: remove

KalimatParser::KalimatParser() : Parser(TokenNameFromId)
{
}

KalimatParser::~KalimatParser()
{
}

void KalimatParser::init(QString s, Lexer *lxr, void *tag)
{
    init(s, lxr, tag, "");
}

void KalimatParser::init(QString s, Lexer *lxr, void *tag, QString fileName)
{
    // Here we manually filter out comments from our token stream.
    // The comments are not automatically skipped because automatic
    // skipping happens only for tokens of type TokenNone and
    // we cannot make our token type "TokenNone" for comments since
    // we need them in the lexical analyzer's output for syntax hilighting

    // todo: Find a more efficient way for this
    // todo: This code is very tightly coupled with Parser::init(...)

    Parser::init(s, lxr, tag, fileName);
    QVector<Token> tokens2;
    bool pendingSisterHood = false;
    Token sis;

    for(int i=0; i<tokens.count(); i++)
    {
        if(tokens[i].Type != COMMENT)
        {
            if(pendingSisterHood)
            {
                pendingSisterHood = false;
                tokens[i].sister = new Token(sis);
                /*MainWindow::that->outputMsg(QString(
                        "Token: %1 has sister: %2)")
                        .arg(tokens[i].Lexeme)
                        .arg(sis.Lexeme));*/
            }
            tokens2.append(tokens[i]);
        }
        else
        {
            if(pendingSisterHood)
            {
                // We have 'sis' being the previous comment
                // so we'll merge it with this comment
                sis.Lexeme+= tokens[i].Lexeme;//.mid(2);
            }
            else
            {
                sis = tokens[i];
                pendingSisterHood = true;
            }
        }
    }
    tokens = tokens2;
}

AST *KalimatParser::parseRoot()
{
    if(LA(UNIT))
        return module();
    return program();
}

AST *KalimatParser::program()
{
    QVector<TopLevel *> elements;
    QVector<Statement *> topLevelStatements;
    ProceduralDecl *entryPoint;
    QVector<StrLiteral *> usedModules = usingDirectives();
    QVector<TopLevel *> originalElements;
    while(!eof())
    {
        // Declaration has to be tested first because of possible
        // ambiguity with identifiers:
        // ID GLOBAL => declaration
        // ID ...    => statement
        if(LA_first_declaration())
        {
            TopLevel *decl = declaration();
            elements.append(decl);
            originalElements.append(decl);
        }
        else if(LA_first_statement())
        {
            Statement *stmt = statement();
            if(!eof())
                match(NEWLINE);
            topLevelStatements.append(stmt);
            originalElements.append(stmt);
        }
        else
        {
            throw ParserException(getPos(), "Expected statement or declaration");
        }
        newLines();
    }
    entryPoint = new ProcedureDecl(Token(), Token(),new Identifier(Token(), "%main"), QVector<Identifier *>(), new BlockStmt(Token(),topLevelStatements), true);
    elements.append(entryPoint);
    return new Program(Token(), elements, usedModules, originalElements);
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
            throw ParserException(getPos(), "Modules cannot contain statements");
        }
        else
        {
            throw ParserException(getPos(), "Expected declaration");
        }
        newLines();
    }
    return new Module(Token(), modName, elements, usedModules);
}

bool KalimatParser::LA_first_statement()
{
    return LA(IF) || LA(FORALL) || LA(WHILE) || LA(RETURN_WITH)
            || LA(DELEGATE) || LA(LAUNCH) || LA(LABEL) || LA(GO) || LA(WHEN)
            || LA(SEND) || LA(RECEIVE) || LA(SELECT)
            || LA_first_io_statement() || LA_first_grfx_statement()
            || LA_first_assignment_or_invokation();
}

Statement *KalimatParser::statement()
{
    Token firstToken = lookAhead; // Save it in case it has a 'sister' comment
    Statement *ret = NULL;
    if(LA_first_assignment_or_invokation())
    {
        ParserState state= this->saveState();
        ret = assignmentStmt_or_Invokation(state);
    }
    else if(LA(IF))
    {
        ret = ifStmt();
    }
    else if(LA(FORALL))
    {
        ret = forEachStmt();
    }
    else if(LA(WHILE))
    {
        ret = whileStmt();
    }
    else if(LA(RETURN_WITH))
    {
        ret = returnStmt();
    }
    else if(LA(DELEGATE))
    {
        ret = delegateStmt();
    }
    else if(LA(LAUNCH))
    {
        ret = launchStmt();
    }
    else if(LA(LABEL))
    {
        ret = labelStmt();
    }
    else if(LA(GO))
    {
        ret = gotoStmt();
    }
    else if(LA(WHEN))
    {
        ret = eventHandlerStmt();
    }
    else if(LA_first_io_statement())
    {
        ret = ioStmt();
    }
    else if(LA_first_grfx_statement())
    {
        ret = grfxStatement();
    }
    else if(LA(SEND))
    {
        ret = sendStmt();
    }
    else if(LA(RECEIVE))
    {
        ret = receiveStmt();
    }
    else if(LA(SELECT))
    {
        ret = selectStmt();
    }

    if(ret == NULL)
        throw ParserException(getPos(), "statement not implemented");

    if(firstToken.sister != NULL)
    {
        ret->attachedComments = firstToken.sister->Lexeme;
     /*   MainWindow::that->outputMsg(QString("Statement: %1; has sister: %2")
                                    .arg(ret->toString()).arg(ret->attachedComments));
                                    */
    }
    return ret;
}

bool KalimatParser::LA_first_declaration()
{
    return LA(PROCEDURE) || LA(FUNCTION) || LA(CLASS)
            || LA_first_method_declaration() || LA2(IDENTIFIER, GLOBAL)
            || LA(LIBRARY);
}

Declaration *KalimatParser::declaration()
{
    Token firstToken = lookAhead;
    Declaration *ret = NULL;

    if(LA(PROCEDURE))
    {
        ret = procedureDecl();
    }
    else if(LA(FUNCTION))
    {
        ret = functionDecl();
    }
    else if(LA(CLASS))
    {
        ret = classDecl();
    }
    else if(LA_first_method_declaration())
    {
        ret = methodDecl();
    }
    else if(LA2(IDENTIFIER, GLOBAL))
    {
        ret = globalDecl();
    }
    else if(LA(LIBRARY))
    {
        ret = ffiLibraryDecl();
    }
    if(ret == NULL)
        throw ParserException(getPos(), "Expected a declaration");

    if(firstToken.sister != NULL)
    {
        ret->attachedComments = firstToken.sister->Lexeme;
       /* MainWindow::that->outputMsg(QString("Declaration: %1; has sister: %2")
                                    .arg(ret->toString()).arg(ret->attachedComments));*/
    }
    return ret;
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
            throw ParserException(getPos(), "Left of = must be an assignable expression");
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
    throw ParserException(getPos(), "Expected IDENTIFIER");
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

        QVector<Token> positions;
        QVector<Expression *> conditions;
        QVector<Statement *> statements;

        while(LA2(ELSE, IF))
        {
            match(ELSE);
            Token p2 = lookAhead;
            positions.append(p2);
            match(IF);
            Expression *cond2 = expression();
            conditions.append(cond2);
            match(COLON);
            Statement *otherPart;
            if(newLine)
            {
                match(NEWLINE);
                otherPart = block();
            }
            else
            {
                otherPart = statement();
            }
            statements.append(otherPart);
        }

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
            {
                elsePart = statement();
            }
        }
        if(newLine)
        {
         //  match(NEWLINE);
           match(DONE);
        }
        for(int i=positions.count()-1; i>=0; i--)
        {
            elsePart  = new IfStmt(positions[i], conditions[i], statements[i], elsePart);
        }
        return new IfStmt(ifTok, cond, thenPart, elsePart);
    }
    throw ParserException(getPos(), "Expected IF");
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
    match(RETURN_WITH);
    Expression *retVal = expression();
    return new ReturnStmt(returnTok, retVal);
}

Statement *KalimatParser::delegateStmt()
{
    Token returnTok  = lookAhead;
    match(DELEGATE);
    match(TO);
    Expression *expr = expression();
    IInvokation *invokation = dynamic_cast<IInvokation *>(expr);
    if(invokation !=NULL)
    {
        return new DelegationStmt(returnTok, invokation);
    }
    else
    {
        throw new ParserException(expr->getPos(),
            QString::fromStdWString(L"لا يمكن التوكيل لغير استدعاء إجراء أو دالة") );
    }
}

Statement *KalimatParser::launchStmt()
{
    Token returnTok  = lookAhead;
    match(LAUNCH);
    Expression *expr = expression();
    IInvokation *invokation = dynamic_cast<IInvokation *>(expr);
    if(invokation !=NULL)
    {
        return new LaunchStmt(returnTok, invokation);
    }
    else
    {
        throw new ParserException(expr->getPos(),
            QString::fromStdWString(L"لا يمكن تشغيل ما ليس باستدعاء إجراء") );
    }
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
        throw ParserException("An identifier or number is expected after 'goto'");
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
        QVector<AssignableExpression *> vars;
        QVector<bool> readNums;
        bool readInt = false;
        AssignableExpression *var = NULL;
        if(LA(HASH))
        {
            match(HASH);
            readInt = true;
        }
        Expression *lvalue = expression();
        var = dynamic_cast<AssignableExpression *>(lvalue);
        if(var == NULL)
        {
            throw ParserException(getPos(), "Item in read statement must be an assignable expression");
        }
        vars.append(var);
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
            lvalue = expression();
            var = dynamic_cast<AssignableExpression *>(lvalue);
            if(var == NULL)
            {
                throw ParserException(getPos(), "Item in read statement must be an assignable expression");
            }
            vars.append(var);
            readNums.append(readInt);
        }
        return new ReadStmt(readTok, fileObject, prompt, vars, readNums);
    }
    throw ParserException(getPos(), "Expected PRINT or READ");
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
    throw ParserException(getPos(), "Expected a drawing statement");
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
    Expression *filled = NULL;

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
            filled = expression();
        }
    }
    return new DrawRectStmt(tok, x1, y1, x2, y2, color, filled);
}
Statement *KalimatParser::drawCircleStmt()
{
    Expression *cx, *cy;
    Expression *radius;
    Expression *color = NULL;
    Expression *filled = NULL;

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
            filled = expression();
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

    if(LA(KEY_DOWN))
    {
        match(KEY_DOWN);
        type = KalimatKeyDownEvent;
    }
    else if(LA(KEY_UP))
    {
        match(KEY_UP);
        type = KalimatKeyUpEvent;
    }
    else if(LA(KEY_PRESS))
    {
        match(KEY_DOWN);
        type = KalimatKeyPressEvent;
    }
    else if(LA(MOUSE_BUTTON_DOWN))
    {
        match(MOUSE_BUTTON_DOWN);
        type = KalimatMouseDownEvent;
    }
    else if(LA(MOUSE_BUTTON_UP))
    {
        match(MOUSE_BUTTON_UP);
        type = KalimatMouseUpEvent;
    }
    else if(LA(MOUSE_MOVE))
    {
        match(MOUSE_MOVE);
        type = KalimatMouseMoveEvent;
    }
    else if(LA(COLLISION))
    {
        match(COLLISION);
        type = KalimatSpriteCollisionEvent;
    }
    else
    {
        throw ParserException("Expected KB or MOUSE");
    }
    match(DO);
    proc = identifier();
    return new EventStatement(tok, type, proc);
}

SendStmt *KalimatParser::sendStmt()
{
    bool signal = false;
    Expression *value = NULL;
    Expression *chan = NULL;
    match(SEND);
    if(eof())
        throw ParserException(getPos(), "Expected an expression");
    Token tok = lookAhead;
    if(LA(SIGNAL_))
    {
        match(SIGNAL_);
        signal = true;
    }
    else
        value = expression();
    match(TO);
    chan = expression();
    return new SendStmt(tok, signal, value, chan);
}

ReceiveStmt *KalimatParser::receiveStmt()
{
    bool signal = false;
    AssignableExpression *value = NULL;
    Expression *chan = NULL;
    match(RECEIVE);
    if(eof())
        throw ParserException(getPos(), "Expected an expression");
    Token tok = lookAhead;
    if(LA(SIGNAL_))
    {
        match(SIGNAL_);
        signal = true;
    }
    else
    {
        Expression *expr = primaryExpression();
        value = dynamic_cast<AssignableExpression *>(expr);
        if(value == NULL)
        {
            throw ParserException(getPos(), "'Receive' must take an assignable expression");
        }
    }
    match(FROM);
    chan = expression();
    return new ReceiveStmt(tok, signal, value, chan);
}

Statement *KalimatParser::selectStmt()
{
    Token tok = lookAhead;
    QVector<ChannelCommunicationStmt *> conditions;
    QVector<Statement *> actions;
    bool multilineStatement = false;
    match(SELECT);
    match(COLON);
    match(NEWLINE);

    if(LA(SEND))
    {
        conditions.append(sendStmt());
    }
    else if(LA(RECEIVE))
    {
        conditions.append(receiveStmt());
    }
    else
    {
        throw ParserException(getPos(), "Expected a 'send' or 'receive' operation");
    }
    match(COLON);
    if(LA(NEWLINE))
    {
        multilineStatement = true;
    }
    if(multilineStatement)
        actions.append(block());
    else
        actions.append(statement());

    while(LA(OR))
    {
        match(OR);
        if(LA(SEND))
        {
            conditions.append(sendStmt());
        }
        else if(LA(RECEIVE))
        {
            conditions.append(receiveStmt());
        }
        else
        {
            throw ParserException(getPos(), "Expected a 'send' or 'receive' operation");
        }
        match(COLON);

        if(multilineStatement)
            actions.append(block());
        else
            actions.append(statement());
    }
    match(DONE);
    return new SelectStmt(tok, conditions, actions);
}

BlockStmt *KalimatParser::block()
{
    QVector<Statement *> stmts;
    Token tok = lookAhead;
    newLines();
    while(LA_first_statement())
    {
        stmts.append(statement());
        if(!eof())
            match(NEWLINE);
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

    ProcedureDecl *ret = new ProcedureDecl(tok, Token(), procName, formals, NULL, true);

    varContext.push(ret);
    BlockStmt *body = block();
    varContext.pop();

    expect(END);
    Token endToken = lookAhead;
    match(END);

    ret->_endingToken = endToken;
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
    FunctionDecl *ret = new FunctionDecl(tok, Token(), procName, formals, NULL, true);

    varContext.push(ret);
    BlockStmt *body = block();
    varContext.pop();

    expect(END);
    Token endToken = lookAhead;
    match(END);

    ret->_endingToken = endToken;
    ret->body(body);
    return ret;
}

void KalimatParser::addPropertySetter(Token pos, Identifier *methodName, QVector<Identifier *> formals,
                                      QMap<QString, PropInfo> &propertyInfo)
{
    // Since this is 'responds', the property has to be a setter

    if(!methodName->name.startsWith(QString::fromStdWString(L"حدد.")))
    {
        throw ParserException(pos, QString::fromStdWString(L"الاستجابة التي تكتب الخاصية لابد أن تبدأ بـ 'حدد.'"));
    }
    else if(formals.count() !=1)
    {
        throw ParserException(pos, QString::fromStdWString(L"الاستجابة التي تكتب الخاصية لابد أن تأخذ عاملاً واحداً.'"));
    }
    QString realName = methodName->name.mid(4); // remove حدد.
    if(!propertyInfo.contains(realName))
    {
        propertyInfo[realName] = PropInfo();
        propertyInfo[realName].write = true;
    }
}

void KalimatParser::addPropertyGetter(Token pos, Identifier *methodName, QVector<Identifier *> formals,
                                      QMap<QString, PropInfo> &propertyInfo)
{
    // Since this is 'replies', the property has to be a getter

    if(formals.count() !=0)
    {
        throw ParserException(pos, QString::fromStdWString(L"الرد الذي يكتب الخاصية لابد ألّا يأخذ عواملاً.'"));
    }
    QString realName = methodName->name;
    if(!propertyInfo.contains(realName))
    {
        propertyInfo[realName] = PropInfo();
        propertyInfo[realName].read = true;
    }
}

Declaration *KalimatParser::classDecl()
{
    QVector<Identifier *> fields;
    QMap<QString, MethodInfo> methods;
    QMap<QString, TypeExpression *> fieldMarshallAs;

    match(CLASS);
    Token tok  = lookAhead;
    Identifier *className = identifier();
    Identifier *ancestorName = NULL;
    match(COLON);
    match(NEWLINE);
    newLines();
    QVector<QSharedPointer<ClassInternalDecl> > internalDecls;
    QMap<QString, PropInfo> propertyInfo;

    while(LA(HAS)|| LA(RESPONDS) || LA(REPLIES) || LA(BUILT))
    {
        if(LA(HAS))
        {
            Has *h = new Has();
            match(HAS);
            Identifier *fname = identifier();
            h->add(fname);
            fields.append(fname);
            if(LA(MARSHALLAS))
            {
                match(MARSHALLAS);
                TypeExpression *te = typeExpression();
                fieldMarshallAs[fname->name] = te;
            }
            while(LA(COMMA))
            {
                match(COMMA);
                fname = identifier();
                h->add(fname);
                fields.append(fname);
                if(LA(MARSHALLAS))
                {
                    match(MARSHALLAS);
                    TypeExpression *te = typeExpression();
                    fieldMarshallAs[fname->name] = te;
                }
            }
            match(NEWLINE);
            internalDecls.append(QSharedPointer<ClassInternalDecl>(h));
        }
        if(LA(RESPONDS))
        {
            match(RESPONDS);
            match(UPON);

            Identifier *methodName = identifier();
            QVector<Identifier *> formals = formalParamList();
            RespondsTo *rt = new RespondsTo(false);

            ConcreteResponseInfo *ri = new ConcreteResponseInfo(methodName);
            for(QVector<Identifier *>::const_iterator i=formals.begin(); i!= formals.end(); ++i)
                ri->add(*i);

            rt->add(ri);
            if(LA(PROPERTY))
            {
                Token p = lookAhead;
                match(PROPERTY);
                addPropertySetter(p, methodName,formals,propertyInfo);
            }

            methods[methodName->name] = MethodInfo(formals.count(), false);
            while(LA(COMMA))
            {
                match(COMMA);
                Identifier *methodName = identifier();
                QVector<Identifier *> formals= formalParamList();
                ConcreteResponseInfo *ri = new ConcreteResponseInfo(methodName);
                for(QVector<Identifier *>::const_iterator i=formals.begin(); i!= formals.end(); ++i)
                    ri->add(*i);
                rt->add(ri);
                if(LA(PROPERTY))
                {
                    Token p = lookAhead;
                    match(PROPERTY);
                    addPropertySetter(p, methodName,formals,propertyInfo);
                }
                methods[methodName->name] = MethodInfo(formals.count(), false);
            }
            internalDecls.append(QSharedPointer<ClassInternalDecl>(rt));
            match(NEWLINE);
        }
        if(LA(REPLIES))
        {
            match(REPLIES);
            match(ON);

            Identifier *methodName = identifier();
            QVector<Identifier *> formals = formalParamList();
            RespondsTo *rt = new RespondsTo(true);
            ConcreteResponseInfo *ri = new ConcreteResponseInfo(methodName);
            for(QVector<Identifier *>::const_iterator i=formals.begin(); i!= formals.end(); ++i)
                ri->add(*i);
            rt->add(ri);
            if(LA(PROPERTY))
            {
                Token p = lookAhead;
                match(PROPERTY);
                addPropertyGetter(p, methodName,formals,propertyInfo);
            }

            methods[methodName->name] = MethodInfo(formals.count(), true);
            while(LA(COMMA))
            {
                match(COMMA);
                Identifier *methodName = identifier();
                QVector<Identifier *> formals= formalParamList();
                ConcreteResponseInfo *ri = new ConcreteResponseInfo(methodName);
                for(QVector<Identifier *>::const_iterator i=formals.begin(); i!= formals.end(); ++i)
                    ri->add(*i);
                rt->add(ri);
                if(LA(PROPERTY))
                {
                    Token p = lookAhead;
                    match(PROPERTY);
                    addPropertyGetter(p, methodName,formals,propertyInfo);
                }
                methods[methodName->name] = MethodInfo(formals.count(), true);
            }
            internalDecls.append(QSharedPointer<ClassInternalDecl>(rt));
            match(NEWLINE);
        }
        if(LA(BUILT))
        {
            Token b = lookAhead;
            if(ancestorName != NULL)
                throw ParserException(getPos(), "Class cannot inherit from more than one base class");
            match(BUILT);
            match(ON);
            ancestorName = identifier();

        }

        newLines();
    }
    match(END);
    return new ClassDecl(tok, ancestorName, className, fields, methods, internalDecls, fieldMarshallAs, true);

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
    if(LA(IDENTIFIER))
    {
        methodName = identifier();
    }
    else if(isFunctionNotProcedure && LA(MESSAGE))
    {
        match(MESSAGE);
        match(OTHER);
        methodName = new Identifier(tok, "%nosuchmethod");
    }
    else
    {
        throw ParserException(tok, "Syntax error");
    }

    formals = formalParamList();
    match(COLON);
    match(NEWLINE);

    if(methodName->name == "%nosuchmethod" && formals.count() !=2)
    {
        throw ParserException(tok, QString::fromStdWString(L"الرد على رسالة أخرى لابد أن يأخذ عاملين"));
    }

    MethodDecl *ret = new MethodDecl(tok, Token(), className, receiverName, methodName, formals, NULL, isFunctionNotProcedure);

    varContext.push(ret);
    BlockStmt *body = block();
    varContext.pop();
    ret->body(body);

    expect(END);
    Token endingToken = lookAhead;
    match(END);
    ret->_endingToken = endingToken;
    return ret;

}

Declaration *KalimatParser::ffiLibraryDecl()
{
    Token tok = lookAhead;
    match(LIBRARY);
    FFILibraryDecl *ffiLib = NULL;
    QVector<Declaration *> decls;
    if(LA(STR_LITERAL))
    {
        StrLiteral *libName = new StrLiteral(lookAhead, prepareStringLiteral(lookAhead.Lexeme));
        match(STR_LITERAL);
        match(COLON);
        match(NEWLINE);
        newLines();
        while(LA(FUNCTION) || LA(PROCEDURE) || LA(CLASS))
        {
            if(LA(FUNCTION))
                decls.append(ffiFunctionDecl());
            else if(LA(PROCEDURE))
                decls.append(ffiProcDecl());
            newLines();
        }
        match(END);
        ffiLib = new FFILibraryDecl(tok, libName->value, decls, true);
        return ffiLib;
    }
    else
    {
        throw ParserException(tok, "Expected string literal");
    }
}

FFIProceduralDecl *KalimatParser::ffiFunctionDecl()
{
    QVector<TypeExpression *> argTypes;
    TypeExpression *retType = NULL;
    QString symbol;
    Token pos = lookAhead;
    match(FUNCTION);

    Identifier *fname = identifier();
    symbol = fname->name;
    if(LA(SYMBOL))
    {
        match(SYMBOL);
        if(LA(STR_LITERAL))
        {
            StrLiteral *s = new StrLiteral(lookAhead, prepareStringLiteral(lookAhead.Lexeme));
            symbol = s->value;
            match(STR_LITERAL);
        }
        else
        {
            throw ParserException(pos, "Expected string literal after keyword 'symbol'");
        }
    }

    match(LPAREN);
    if(LA_first_typeExpression())
    {
        argTypes.append(typeExpression());
        while(LA(COMMA))
        {
            match(COMMA);
            argTypes.append(typeExpression());
        }
    }
    match(RPAREN);
    retType = typeExpression();
    return new FFIProceduralDecl(pos, true, retType, argTypes, fname->name, symbol, true);
}

FFIProceduralDecl *KalimatParser::ffiProcDecl()
{
    QVector<TypeExpression *> argTypes;
    TypeExpression *retType = NULL;
    QString symbol;
    Token pos = lookAhead;
    match(PROCEDURE);

    Identifier *fname = identifier();
    symbol = fname->name;
    if(LA(SYMBOL))
    {
        match(SYMBOL);
        if(LA(STR_LITERAL))
        {
            StrLiteral *s = new StrLiteral(lookAhead, lookAhead.Lexeme);
            symbol = s->value;
            match(STR_LITERAL);
        }
        else
        {
            throw ParserException(pos, "Expected string literal after keyword 'symbol'");
        }
    }

    match(LPAREN);
    if(LA_first_typeExpression())
    {
        argTypes.append(typeExpression());
        while(LA(COMMA))
        {
            match(COMMA);
            argTypes.append(typeExpression());
        }
    }
    match(RPAREN);
    retType = NULL;
    return new FFIProceduralDecl(pos, false, retType, argTypes, fname->name, symbol, true);
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

    while(true)
    {
        if(LA(LT) || LA(GT) || LA(EQ) || LA(NE) || LA(LE) || LA(GE))
        {
            Token tok  = lookAhead;
            QString operation = getOperation(lookAhead);
            match(lookAhead.Type);
            Expression * t2 = arithmeticExpression();
            t = new BinaryOperation(tok, operation, t, t2);
        }
        else if(LA(IS))
        {
            Token tok  = lookAhead;
            match(lookAhead.Type);
            Identifier *t2 = identifier();
            t = new IsaOperation(tok, t, t2);
        }
        else
        {
            break;
        }
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
            catch(ParserException ex)
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
    return LA(NUM_LITERAL) ||
            LA(STR_LITERAL) ||
            LA(NOTHING) ||
            LA(C_TRUE) ||
            LA(C_FALSE) ||
            LA(LBRACKET) ||
            LA(LBRACE) ||
            LA(IDENTIFIER) ||
            LA(LPAREN);
                //todo: decide if we still want to keep the field accessor field(obj, id)
                //||  LA(FIELD);
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
    else if(LA(LBRACE))
    {
        Token lbPos = lookAhead;
        match(LBRACE);
        QVector<Expression *> data = comma_separated_pairs();
        match(RBRACE);
        ret = new MapLiteral(lbPos, data);
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
        else if(LA_first_primary_expression() && !LA(LPAREN) && !LA(LBRACKET))
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
    /*
      //todo: decide if we still want to keep the field accessor field(obj, id)
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
    */
    else
    {
        throw ParserException(getPos(), "Expected a literal, identifier, or parenthesized expression");
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
    throw ParserException(getPos(), "Expected Identifier");
}

bool KalimatParser::LA_first_typeExpression()
{
    return LA(IDENTIFIER) || LA(POINTER) || LA(PROCEDURE) || LA(FUNCTION);
}

TypeExpression *KalimatParser::typeExpression()
{
    if(LA(POINTER))
    {
        Token tok = lookAhead;

        match(POINTER);
        match(LPAREN);
        TypeExpression *pointee = typeExpression();
        match(RPAREN);
        return new PointerTypeExpression(tok, pointee);
    }
    else if(LA(IDENTIFIER))
    {
        TypeIdentifier *ret = NULL;
        ret = new TypeIdentifier(lookAhead, lookAhead.Lexeme);
        match(IDENTIFIER);
        return ret;
    }
    else if(LA(PROCEDURE))
    {
        QVector<TypeExpression *> argTypes;
        Token tok = lookAhead;
        match(PROCEDURE);
        match(LPAREN);
        if(LA_first_typeExpression())
        {
            argTypes.append(typeExpression());
            while(LA(COMMA))
            {
                match(COMMA);
                argTypes.append(typeExpression());
            }
        }
        match(RPAREN);
        return new FunctionTypeExpression(tok, NULL, argTypes);
    }
    else if(LA(FUNCTION))
    {
        QVector<TypeExpression *> argTypes;
        Token tok = lookAhead;
        match(FUNCTION);
        match(LPAREN);
        if(LA_first_typeExpression())
        {
            argTypes.append(typeExpression());
            while(LA(COMMA))
            {
                match(COMMA);
                argTypes.append(typeExpression());
            }
        }
        match(RPAREN);
        TypeExpression *retType = typeExpression();
        return new FunctionTypeExpression(tok, retType, argTypes);
    }

    throw ParserException(getPos(), "Expected Type Expression");
}

QVector<Expression *> KalimatParser::comma_separated_expressions()
{
    QVector<Expression *> ret;
    if(LA_first_expression())
    {
        ret.append(expression());
        while(LA(COMMA))
        {
            match(COMMA);
            newLines();
            ret.append(expression());
        }
    }
    return ret;
}

QVector<Expression *> KalimatParser::comma_separated_pairs()
{
    QVector<Expression *> ret;
    if(LA_first_primary_expression())
    {
        ret.append(expression());
        match(ROCKET);
        ret.append(expression());
        while(LA(COMMA))
        {
            match(COMMA);
            newLines();
            ret.append(expression());
            match(ROCKET);
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
            throw ParserException(getPos(), "USING keyword must be followed by a string literal");
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
        throw ParserException(getPos(), "Unknown operator");
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
