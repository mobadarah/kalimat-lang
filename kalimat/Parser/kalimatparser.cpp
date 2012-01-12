/**************************************************************************
**   The Kalimat programming language
**   Copyright (C) 2010 Mohamed Samy Ali - samy2004@gmail.com
**   This project is released under the Apache License version 2.0
**   as described in the included license.txt file
**************************************************************************/

#include "parser_incl.h"
#include "../Lexer/kalimatlexer.h"
#include "kalimatast/kalimatast_incl.h"
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

shared_ptr<AST> KalimatParser::parseRoot()
{
    if(LA(UNIT))
        return module();
    return program();
}

shared_ptr<AST> KalimatParser::program()
{
    QVector<shared_ptr<TopLevel> > elements;
    QVector<shared_ptr<Statement> > topLevelStatements;
    shared_ptr<ProceduralDecl> entryPoint;
    QVector<shared_ptr<StrLiteral > > usedModules = usingDirectives();
    QVector<shared_ptr<TopLevel > > originalElements;
    while(!eof())
    {
        // Declaration has to be tested first because of possible
        // ambiguity with identifiers:
        // ID GLOBAL => declaration
        // ID ...    => statement
        if(LA_first_declaration())
        {
            shared_ptr<TopLevel> decl = declaration();
            elements.append(decl);
            originalElements.append(decl);
        }
        else if(LA_first_statement())
        {
            shared_ptr<Statement> stmt = statement();
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
    entryPoint = shared_ptr<ProceduralDecl>(new ProcedureDecl(Token(),
                                                                  Token(),
                                                                  shared_ptr<Identifier>(new Identifier(Token(), "%main")),
                                                                  QVector<shared_ptr<Identifier> >(),
                                                                  shared_ptr<BlockStmt>(new BlockStmt(Token(),topLevelStatements)),
                                                                  true));
    elements.append(entryPoint);
    return shared_ptr<AST>(new Program(Token(), elements, usedModules, originalElements));
}

shared_ptr<AST> KalimatParser::module()
{
    QVector<shared_ptr<Declaration> > elements;

    newLines();
    match(UNIT);
    shared_ptr<Identifier> modName = identifier();
    match(NEWLINE);
    newLines();
    QVector<shared_ptr<StrLiteral > > usedModules = usingDirectives();

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
    return shared_ptr<AST>(new Module(Token(), modName, elements, usedModules));
}

bool KalimatParser::LA_first_statement()
{
    return LA(IF) || LA(FORALL) || LA(WHILE) || LA(RETURN_WITH)
            || LA(DELEGATE) || LA(LAUNCH) || LA(LABEL) || LA(GO) || LA(WHEN)
            || LA(SEND) || LA(RECEIVE) || LA(SELECT)
            || LA_first_io_statement() || LA_first_grfx_statement()
            || LA_first_assignment_or_invokation();
}

shared_ptr<Statement> KalimatParser::statement()
{
    Token firstToken = lookAhead; // Save it in case it has a 'sister' comment
    shared_ptr<Statement> ret;
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
        ret.get()->attachedComments = firstToken.sister->Lexeme;
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
            || LA(LIBRARY) || LA(RULES);
}

shared_ptr<Declaration> KalimatParser::declaration()
{
    Token firstToken = lookAhead;
    shared_ptr<Declaration> ret;

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
    else if(LA(RULES))
    {
        ret = rulesDecl();
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
shared_ptr<Statement> KalimatParser::assignmentStmt_or_Invokation(ParserState s)
{
    shared_ptr<Expression> first = primaryExpression();
    if(LA(EQ))
    {

        shared_ptr<AssignableExpression> id = dynamic_pointer_cast<AssignableExpression>(first);
        if(id == NULL)
        {
            throw ParserException(getPos(), "Left of = must be an assignable expression");
        }
        Token eqToken = lookAhead;
        match(EQ);
        shared_ptr<Expression> value = expression();
        return shared_ptr<Statement>(new AssignmentStmt(eqToken, id, value));
    }
    else
    {
        shared_ptr<IInvokation> invokation = dynamic_pointer_cast<IInvokation>(first);
        if(invokation != NULL)
        {
            return shared_ptr<Statement>(new InvokationStmt(invokation->getPos(), invokation));
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

shared_ptr<Statement> KalimatParser::ifStmt()
{
    shared_ptr<Statement> thenPart;
    shared_ptr<Statement> elsePart;
    if(LA(IF))
    {
        bool newLine = false;
        Token ifTok = lookAhead;
        match(IF);
        shared_ptr<Expression> cond = expression();
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
        QVector<shared_ptr<Expression> > conditions;
        QVector<shared_ptr<Statement> > statements;

        while(LA2(ELSE, IF))
        {
            match(ELSE);
            Token p2 = lookAhead;
            positions.append(p2);
            match(IF);
            shared_ptr<Expression> cond2 = expression();
            conditions.append(cond2);
            match(COLON);
            shared_ptr<Statement> otherPart;
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
            elsePart  = shared_ptr<Statement>(new IfStmt(positions[i], conditions[i], statements[i], elsePart));
        }
        return shared_ptr<Statement>(new IfStmt(ifTok, cond, thenPart, elsePart));
    }
    throw ParserException(getPos(), "Expected IF");
}
shared_ptr<Statement> KalimatParser::forEachStmt()
{
    bool multiLineStmt = false;
    shared_ptr<Statement> theStmt;
    Token forAllTok = lookAhead;
    match(FORALL);
    shared_ptr<Identifier> id = identifier();
    match(FROM);
    shared_ptr<Expression> from = expression();
    match(TO);
    shared_ptr<Expression> to = expression();
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
    return shared_ptr<Statement>(new ForAllStmt(forAllTok, id, from, to, theStmt));
}

shared_ptr<Statement> KalimatParser::whileStmt()
{
    bool multiLineStmt = false;
    shared_ptr<Statement> theStmt;
    Token whileTok = lookAhead;
    match(WHILE);
    shared_ptr<Expression> cond = expression();
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
    return shared_ptr<Statement>(new WhileStmt(whileTok, cond, theStmt));
}

shared_ptr<Statement> KalimatParser::returnStmt()
{
    Token returnTok  = lookAhead;
    match(RETURN_WITH);
    shared_ptr<Expression> retVal = expression();
    return shared_ptr<Statement>(new ReturnStmt(returnTok, retVal));
}

shared_ptr<Statement> KalimatParser::delegateStmt()
{
    Token returnTok  = lookAhead;
    match(DELEGATE);
    match(TO);
    shared_ptr<Expression> expr = expression();
    shared_ptr<IInvokation> invokation = dynamic_pointer_cast<IInvokation>(expr);
    if(invokation != NULL)
    {
        return shared_ptr<Statement>(new DelegationStmt(returnTok, invokation));
    }
    else
    {
        throw ParserException(expr->getPos(),
            QString::fromStdWString(L"لا يمكن التوكيل لغير استدعاء إجراء أو دالة") );
    }
}

shared_ptr<Statement> KalimatParser::launchStmt()
{
    Token returnTok  = lookAhead;
    match(LAUNCH);
    shared_ptr<Expression> expr = expression();
    shared_ptr<IInvokation> invokation = dynamic_pointer_cast<IInvokation>(expr);
    if(invokation !=NULL)
    {
        return shared_ptr<Statement>(new LaunchStmt(returnTok, invokation));
    }
    else
    {
        throw ParserException(expr->getPos(),
            QString::fromStdWString(L"لا يمكن تشغيل ما ليس باستدعاء إجراء") );
    }
}

shared_ptr<Statement> KalimatParser::labelStmt()
{
    Token pos = lookAhead;
    match(LABEL);
    shared_ptr<Expression> target = expression();
    return shared_ptr<Statement>(new LabelStmt(pos, target));
}

shared_ptr<Statement> KalimatParser::gotoStmt()
{
    Token pos = lookAhead;
    shared_ptr<Expression> target;
    match(GO);
    match(TO);
    bool targetIsNum = false;
    if(LA(NUM_LITERAL))
    {
        targetIsNum = true;
        target = shared_ptr<Expression>(new NumLiteral(lookAhead, lookAhead.Lexeme));
        match(NUM_LITERAL);
    }
    else if(LA(IDENTIFIER))
    {
        targetIsNum = false;
        target = identifier();
    }
    else
    {
        throw ParserException(pos, "An identifier or number is expected after 'goto'");
    }
    return shared_ptr<Statement>(new GotoStmt(pos, targetIsNum, target));
}

bool KalimatParser::LA_first_io_statement()
{
    return LA(PRINT) || LA(READ);
}

shared_ptr<Statement> KalimatParser::ioStmt()
{
    if(LA(PRINT))
    {
        bool printOnSameLine = false;
        QVector<shared_ptr<Expression> > args;
        QVector<shared_ptr<Expression> > widths;
        shared_ptr<Expression> fileObject;
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
            widths.append(shared_ptr<Expression>());
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
                widths.append(shared_ptr<Expression>());
            }
            args.append(expression());
        }
        if(LA(ELLIPSIS))
        {
            match(ELLIPSIS);
            printOnSameLine = true;
        }
        officialEndOfPrintStmt:
        return shared_ptr<Statement>(new PrintStmt(printTok, fileObject, args, widths, printOnSameLine));
    }
    if(LA(READ))
    {
        QString prompt = "";
        Token readTok  = lookAhead;
        shared_ptr<Expression> fileObject;
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
        QVector<shared_ptr<AssignableExpression> > vars;
        QVector<bool> readNums;
        bool readInt = false;
        shared_ptr<AssignableExpression> var;
        if(LA(HASH))
        {
            match(HASH);
            readInt = true;
        }
        shared_ptr<Expression> lvalue = expression();
        var = dynamic_pointer_cast<AssignableExpression>(lvalue);
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
            var = dynamic_pointer_cast<AssignableExpression>(lvalue);
            if(var == NULL)
            {
                throw ParserException(getPos(), "Item in read statement must be an assignable expression");
            }
            vars.append(var);
            readNums.append(readInt);
        }
        return shared_ptr<Statement>(new ReadStmt(readTok, fileObject, prompt, vars, readNums));
    }
    throw ParserException(getPos(), "Expected PRINT or READ");
}

bool KalimatParser::LA_first_grfx_statement()
{
    return LA(DRAW_PIXEL) || LA(DRAW_LINE) ||
            LA(DRAW_RECT) || LA(DRAW_CIRCLE) ||
            LA(DRAW_IMAGE) || LA(DRAW_SPRITE) || LA(ZOOM);
}

shared_ptr<Statement> KalimatParser::grfxStatement()
{
    if(LA(DRAW_PIXEL))
        return drawPixelStmt();
    if(LA(DRAW_LINE))
        return drawLineStmt();
    if(LA(DRAW_RECT))
        return drawRectStmt();
    if(LA(DRAW_CIRCLE))
        return drawCircleStmt();
    if(LA(DRAW_IMAGE))
        return drawImageStmt();
    if(LA(DRAW_SPRITE))
        return drawSpriteStmt();
    if(LA(ZOOM))
        return zoomStmt();
    throw ParserException(getPos(), "Expected a drawing statement");
}
shared_ptr<Statement> KalimatParser::drawPixelStmt()
{
    shared_ptr<Expression> x, y;
    shared_ptr<Expression> color;
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
    return shared_ptr<Statement>(new DrawPixelStmt(tok, x,y, color));
}

shared_ptr<Statement> KalimatParser::drawLineStmt()
{
    shared_ptr<Expression> x1, y1;
    shared_ptr<Expression> x2, y2;
    shared_ptr<Expression> color;

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
    return shared_ptr<Statement>(new DrawLineStmt(tok, x1, y1, x2, y2, color));
}

shared_ptr<Statement> KalimatParser::drawRectStmt()
{
    shared_ptr<Expression> x1, y1;
    shared_ptr<Expression> x2, y2;
    shared_ptr<Expression> color;
    shared_ptr<Expression> filled;

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
    return shared_ptr<Statement>(new DrawRectStmt(tok, x1, y1, x2, y2, color, filled));
}

shared_ptr<Statement> KalimatParser::drawCircleStmt()
{
    shared_ptr<Expression> cx, cy;
    shared_ptr<Expression> radius;
    shared_ptr<Expression> color;
    shared_ptr<Expression> filled;

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
    return shared_ptr<Statement>(new DrawCircleStmt(tok, cx, cy, radius, color, filled));
}

shared_ptr<Statement> KalimatParser::drawImageStmt()
{
    shared_ptr<Expression> x, y;
    shared_ptr<Expression> number;

    Token tok  = lookAhead;
    match(DRAW_IMAGE);
    number = expression();
    match(IN);

    match(LPAREN);
    x = expression();
    match(COMMA);
    y = expression();
    match(RPAREN);

    return shared_ptr<Statement>(new DrawImageStmt(tok, x, y, number));
}

shared_ptr<Statement> KalimatParser::drawSpriteStmt()
{
    shared_ptr<Expression> x, y;
    shared_ptr<Expression> number;

    Token tok  = lookAhead;
    match(DRAW_SPRITE);
    number = expression();
    match(IN);

    match(LPAREN);
    x = expression();
    match(COMMA);
    y = expression();
    match(RPAREN);

    return shared_ptr<Statement>(new DrawSpriteStmt(tok, x, y, number));
}

shared_ptr<Statement> KalimatParser::zoomStmt()
{

    shared_ptr<Expression> x1, y1;
    shared_ptr<Expression> x2, y2;

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

    return shared_ptr<Statement>(new ZoomStmt(tok, x1, y1, x2, y2));
}

shared_ptr<Statement> KalimatParser::eventHandlerStmt()
{
    EventType type;
    shared_ptr<Identifier> proc;
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
    return shared_ptr<Statement>(new EventStatement(tok, type, proc));
}

shared_ptr<SendStmt> KalimatParser::sendStmt()
{
    bool signal = false;
    shared_ptr<Expression> value;
    shared_ptr<Expression> chan;
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
    return shared_ptr<SendStmt>(new SendStmt(tok, signal, value, chan));
}

shared_ptr<ReceiveStmt> KalimatParser::receiveStmt()
{
    bool signal = false;
    shared_ptr<AssignableExpression> value;
    shared_ptr<Expression> chan;
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
        shared_ptr<Expression> expr = primaryExpression();
        value = dynamic_pointer_cast<AssignableExpression>(expr);
        if(value == NULL)
        {
            throw ParserException(getPos(), "'Receive' must take an assignable expression");
        }
    }
    match(FROM);
    chan = expression();
    return shared_ptr<ReceiveStmt>(new ReceiveStmt(tok, signal, value, chan));
}

shared_ptr<Statement> KalimatParser::selectStmt()
{
    Token tok = lookAhead;
    QVector<shared_ptr<ChannelCommunicationStmt> > conditions;
    QVector<shared_ptr<Statement> > actions;
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
    return shared_ptr<Statement>(new SelectStmt(tok, conditions, actions));
}

shared_ptr<BlockStmt> KalimatParser::block()
{
    QVector<shared_ptr<Statement> > stmts;
    Token tok = lookAhead;
    newLines();
    while(LA_first_statement())
    {
        stmts.append(statement());
        if(!eof())
            match(NEWLINE);
        newLines();
    }
    return shared_ptr<BlockStmt>(new BlockStmt(tok, stmts));
}

QVector<shared_ptr<Identifier> > KalimatParser::formalParamList()
{
    QVector<shared_ptr<Identifier> > formals;

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

shared_ptr<Declaration> KalimatParser::procedureDecl()
{
    match(PROCEDURE);
    Token tok  = lookAhead;
    shared_ptr<Identifier> procName = identifier();
    QVector<shared_ptr<Identifier> > formals = formalParamList();

    match(COLON);
    match(NEWLINE);

    shared_ptr<ProcedureDecl> ret(new ProcedureDecl(tok,
                                                          Token(),
                                                          procName,
                                                          formals,
                                                          shared_ptr<BlockStmt>(),
                                                          true));

    varContext.push(ret);
    shared_ptr<BlockStmt> body = block();
    varContext.pop();

    expect(END);
    Token endToken = lookAhead;
    match(END);

    ret->_endingToken = endToken;
    ret->body(body);
    return ret;
}

shared_ptr<Declaration> KalimatParser::functionDecl()
{
    match(FUNCTION);
    Token tok  = lookAhead;
    shared_ptr<Identifier> procName = identifier();
    QVector<shared_ptr<Identifier> > formals = formalParamList();
    match(COLON);
    match(NEWLINE);
    shared_ptr<FunctionDecl> ret(new FunctionDecl(tok, Token(), procName, formals, shared_ptr<BlockStmt>(), true));

    varContext.push(ret);
    shared_ptr<BlockStmt> body = block();
    varContext.pop();

    expect(END);
    Token endToken = lookAhead;
    match(END);

    ret->_endingToken = endToken;
    ret->body(body);
    return ret;
}

void KalimatParser::addPropertySetter(Token pos,
                                       shared_ptr<Identifier> methodName,
                                       QVector<shared_ptr<Identifier> > formals,
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

void KalimatParser::addPropertyGetter(Token pos,
                                      shared_ptr<Identifier> methodName,
                                      QVector<shared_ptr<Identifier> > formals,
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

shared_ptr<Declaration> KalimatParser::classDecl()
{
    QVector<shared_ptr<Identifier> > fields;
    QMap<QString, MethodInfo> methods;
    QMap<QString, shared_ptr<TypeExpression> > fieldMarshallAs;

    match(CLASS);
    Token tok  = lookAhead;
    shared_ptr<Identifier> className = identifier();
    shared_ptr<Identifier> ancestorName;
    match(COLON);
    match(NEWLINE);
    newLines();
    QVector<shared_ptr<ClassInternalDecl> > internalDecls;
    QMap<QString, PropInfo> propertyInfo;

    while(LA(HAS)|| LA(RESPONDS) || LA(REPLIES) || LA(BUILT))
    {
        if(LA(HAS))
        {
            shared_ptr<Has> h(new Has());
            match(HAS);
            shared_ptr<Identifier> fname = identifier();
            h->add(fname);
            fields.append(fname);
            if(LA(MARSHALLAS))
            {
                match(MARSHALLAS);
                shared_ptr<TypeExpression> te = typeExpression();
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
                    shared_ptr<TypeExpression> te = typeExpression();
                    fieldMarshallAs[fname->name] = te;
                }
            }
            match(NEWLINE);
            internalDecls.append(h);
        }
        if(LA(RESPONDS))
        {
            match(RESPONDS);
            match(UPON);

            shared_ptr<Identifier> methodName = identifier();
            QVector<shared_ptr<Identifier> > formals = formalParamList();
            shared_ptr<RespondsTo> rt(new RespondsTo(false));

            shared_ptr<ConcreteResponseInfo> ri(new ConcreteResponseInfo(methodName));
            for(QVector<shared_ptr<Identifier> >::const_iterator i=formals.begin(); i!= formals.end(); ++i)
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
                shared_ptr<Identifier> methodName = identifier();
                QVector<shared_ptr<Identifier> > formals= formalParamList();
                shared_ptr<ConcreteResponseInfo> ri(new ConcreteResponseInfo(methodName));
                for(QVector<shared_ptr<Identifier> >::const_iterator i=formals.begin(); i!= formals.end(); ++i)
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
            internalDecls.append(rt);
            match(NEWLINE);
        }
        if(LA(REPLIES))
        {
            match(REPLIES);
            match(ON);

            shared_ptr<Identifier> methodName = identifier();
            QVector<shared_ptr<Identifier> > formals = formalParamList();
            shared_ptr<RespondsTo> rt(new RespondsTo(true));
            shared_ptr<ConcreteResponseInfo> ri(new ConcreteResponseInfo(methodName));
            for(QVector<shared_ptr<Identifier> >::const_iterator i=formals.begin(); i!= formals.end(); ++i)
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
                shared_ptr<Identifier> methodName = identifier();
                QVector<shared_ptr<Identifier> > formals= formalParamList();
                shared_ptr<ConcreteResponseInfo> ri(new ConcreteResponseInfo(methodName));
                for(QVector<shared_ptr<Identifier> >::const_iterator i=formals.begin(); i!= formals.end(); ++i)
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
            internalDecls.append(rt);
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
    return shared_ptr<Declaration>(new ClassDecl(tok, ancestorName, className, fields, methods, internalDecls, fieldMarshallAs, true));
}

shared_ptr<Declaration> KalimatParser::globalDecl()
{
    shared_ptr<Identifier> var;
    Token tok  = lookAhead;
    var = identifier();
    match(GLOBAL);
    return shared_ptr<Declaration>(new GlobalDecl(tok, var->name, true));
}

bool KalimatParser::LA_first_method_declaration()
{
    return LA(RESPONSEOF) || LA(REPLYOF);
}

shared_ptr<Declaration> KalimatParser::methodDecl()
{
    bool isFunctionNotProcedure;
    shared_ptr<Identifier> className;
    shared_ptr<Identifier> receiverName;
    shared_ptr<Identifier> methodName;

    QVector<shared_ptr<Identifier> > formals;
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
        methodName = shared_ptr<Identifier>(new Identifier(tok, "%nosuchmethod"));
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

    shared_ptr<MethodDecl> ret(new MethodDecl(tok, Token(), className, receiverName,
                                     methodName, formals, shared_ptr<BlockStmt>(),
                                     isFunctionNotProcedure));

    varContext.push(ret);
    shared_ptr<BlockStmt> body = block();
    varContext.pop();
    ret->body(body);

    expect(END);
    Token endingToken = lookAhead;
    match(END);
    ret->_endingToken = endingToken;
    return ret;
}

shared_ptr<Declaration> KalimatParser::ffiLibraryDecl()
{
    Token tok = lookAhead;
    match(LIBRARY);
    shared_ptr<FFILibraryDecl> ffiLib;
    QVector<shared_ptr<Declaration> > decls;
    if(LA(STR_LITERAL))
    {
        shared_ptr<StrLiteral>  libName(new StrLiteral(lookAhead, prepareStringLiteral(lookAhead.Lexeme)));
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
        ffiLib = shared_ptr<FFILibraryDecl>(new FFILibraryDecl(tok, libName->value, decls, true));
        return ffiLib;
    }
    else
    {
        throw ParserException(tok, "Expected string literal");
    }
}

shared_ptr<Declaration> KalimatParser::rulesDecl()
{
    match(RULES);
    shared_ptr<Identifier> ruleName = identifier();
    match(COLON);
    match(NEWLINE);
    newLines();
    while(LA(IDENTIFIER))
    {
        shared_ptr<RuleDecl> rd = ruleDecl();
        newLines();
    }
    match(END);
}

shared_ptr<RuleDecl> KalimatParser::ruleDecl()
{
    QVector<shared_ptr<RuleOption> > options;
    shared_ptr<Identifier> name = identifier();

    match(EQ);
    shared_ptr<PegExpr> expr = pegExpr();
    shared_ptr<Expression> resultExpr;
    if(LA(ROCKET))
    {
        match(ROCKET);
        resultExpr = expression();
    }
    match(NEWLINE);
    newLines();
    options.append(shared_ptr<RuleOption>(new RuleOption(expr, resultExpr)));
    while(LA(OR))
    {
        expr = pegExpr();
        if(LA(ROCKET))
        {
            match(ROCKET);
            resultExpr = expression();
        }
        options.append(shared_ptr<RuleOption>(new RuleOption(expr, resultExpr)));
        match(NEWLINE);
        newLines();
    }
    return shared_ptr<RuleDecl>(new RuleDecl(name->name, options));
}

shared_ptr<PegExpr> KalimatParser::pegExpr()
{
    identifier();
}

shared_ptr<FFIProceduralDecl> KalimatParser::ffiFunctionDecl()
{
    QVector<shared_ptr<TypeExpression> > argTypes;
    shared_ptr<TypeExpression> retType;
    QString symbol;
    Token pos = lookAhead;
    match(FUNCTION);

    shared_ptr<Identifier> fname = identifier();
    symbol = fname->name;
    if(LA(SYMBOL))
    {
        match(SYMBOL);
        if(LA(STR_LITERAL))
        {
            shared_ptr<StrLiteral> s(new StrLiteral(lookAhead, prepareStringLiteral(lookAhead.Lexeme)));
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
    return shared_ptr<FFIProceduralDecl>(new FFIProceduralDecl(pos, true, retType, argTypes, fname->name, symbol, true));
}

shared_ptr<FFIProceduralDecl> KalimatParser::ffiProcDecl()
{
    QVector<shared_ptr<TypeExpression> > argTypes;
    shared_ptr<TypeExpression> retType;
    QString symbol;
    Token pos = lookAhead;
    match(PROCEDURE);

    shared_ptr<Identifier> fname = identifier();
    symbol = fname->name;
    if(LA(SYMBOL))
    {
        match(SYMBOL);
        if(LA(STR_LITERAL))
        {
            shared_ptr<StrLiteral> s(new StrLiteral(lookAhead, lookAhead.Lexeme));
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
    return shared_ptr<FFIProceduralDecl>(new FFIProceduralDecl(pos, false, retType, argTypes, fname->name, symbol, true));
}

bool KalimatParser::LA_first_expression()
{
    return LA_first_primary_expression()
            || LA(NOT) || LA(ADD_OP) || LA(SUB_OP);
}

shared_ptr<Expression> KalimatParser::expression()
{
    return andOrExpression();
}

shared_ptr<Expression> KalimatParser::andOrExpression()
{
    shared_ptr<Expression> t = notExpression();
    while(LA(AND) || LA(OR))
    {
        QString operation = getOperation(lookAhead);
        Token tok  = lookAhead;
        match(lookAhead.Type);
        shared_ptr<Expression> t2 = notExpression();
        t = shared_ptr<Expression>(new BinaryOperation(tok, operation, t, t2));
    }
    return t;
}

shared_ptr<Expression> KalimatParser::notExpression()
{
    bool _not = false;
    Token tok  = lookAhead;
    if(LA(NOT))
    {
        match(NOT);
        _not = true;
    }
    shared_ptr<Expression> t = comparisonExpression();
    if(_not)
    {
        t = shared_ptr<Expression>(new UnaryOperation(tok, "not", t));
        while(LA(ANDNOT))
        {
            tok = lookAhead;
            match(ANDNOT);
            shared_ptr<Expression> t2 = comparisonExpression();
            t = shared_ptr<Expression>(new BinaryOperation(tok,
                                                               "and",
                                                               t,
                                                               shared_ptr<Expression>(new UnaryOperation(tok, "not", t2))));
        }
    }
    return t;
}

shared_ptr<Expression> KalimatParser::comparisonExpression()
{
    shared_ptr<Expression> t = arithmeticExpression();

    while(true)
    {
        if(LA(LT) || LA(GT) || LA(EQ) || LA(NE) || LA(LE) || LA(GE))
        {
            Token tok  = lookAhead;
            QString operation = getOperation(lookAhead);
            match(lookAhead.Type);
            shared_ptr<Expression> t2 = arithmeticExpression();
            t = shared_ptr<Expression>(new BinaryOperation(tok, operation, t, t2));
        }
        else if(LA(IS))
        {
            Token tok  = lookAhead;
            match(lookAhead.Type);
            shared_ptr<Identifier> t2 = identifier();
            t = shared_ptr<Expression>(new IsaOperation(tok, t, t2));
        }
        else if(LA(MATCHES))
        {
            Token tok  = lookAhead;
            match(lookAhead.Type);
            shared_ptr<Pattern> t2 = pattern();
            t = shared_ptr<Expression>(new MatchOperation(tok, t, t2));
        }
        else
        {
            break;
        }
    }
    return t;
}

bool KalimatParser::LA_first_pattern()
{
    return LA(IDENTIFIER) || LA(QUESTION) || LA(LBRACKET)
            || LA(LBRACE) || LA_first_simple_literal();
}

shared_ptr<Pattern> KalimatParser::pattern()
{
    if(LA_first_simple_literal())
        return simpleLiteralPattern();
    if(LA(IDENTIFIER))
        return varOrObjPattern();
    if(LA(IS))
        return varOrObjPattern();
    if(LA(QUESTION))
        return assignedVarPattern();
    if(LA(LBRACKET))
        return arrayPattern();
    if(LA(LBRACE))
        return mapPattern();
}

shared_ptr<Pattern> KalimatParser::simpleLiteralPattern()
{
    shared_ptr<SimpleLiteral> sl = simpleLiteral();
    return shared_ptr<Pattern>(new SimpleLiteralPattern(sl->getPos(), sl));
}

shared_ptr<Pattern> KalimatParser::assignedVarPattern()
{
    match(QUESTION);
    shared_ptr<Expression> expr = expression();

    shared_ptr<AssignableExpression> id = dynamic_pointer_cast<AssignableExpression>(expr);
    if(id == NULL)
    {
        throw ParserException(getPos(), "After ? must be an assignable expression");
    }
    return shared_ptr<Pattern>(new AssignedVarPattern(id->getPos(), id));
}

shared_ptr<Pattern> KalimatParser::varOrObjPattern()
{
    bool mustBeObj = false;
    QVector<shared_ptr<Identifier> > fnames;
    QVector<shared_ptr<Pattern> > fpatterns;

    if(LA(ANYOF))
    {
        match(ANYOF);
        mustBeObj = true;
    }

    shared_ptr<Identifier> id = identifier();
    if(LA(HAS))
    {
        mustBeObj = true;
        match(HAS);

        fnames.append(identifier());
        match(EQ);
        fpatterns.append(pattern());
        while(LA(COMMA))
        {
            match(COMMA);
            newLines();
            fnames.append(identifier());
            match(EQ);
            fpatterns.append(pattern());
        }
    }
    if(mustBeObj)
    {
        return shared_ptr<Pattern>(new ObjPattern(id->getPos(), id, fnames, fpatterns));
    }
    else
    {
        return shared_ptr<Pattern>(new VarPattern(id->getPos(), id));
    }
}

shared_ptr<Pattern> KalimatParser::arrayPattern()
{
    bool fixedLen = true;
    QVector<shared_ptr<Pattern> > patterns;
    Token pos = lookAhead;
    match(LBRACKET);
    if(LA_first_pattern())
    {
        patterns.append(pattern());
        while(LA(COMMA))
        {
            match(COMMA);
            newLines();
            if(LA(ELLIPSIS))
            {
                match(ELLIPSIS);
                fixedLen = false;
                break;
            }
            else
            {
                patterns.append(pattern());
            }
        }
    }
    match(RBRACKET);
    return shared_ptr<Pattern>(new ArrayPattern(pos, patterns));
}

shared_ptr<Pattern> KalimatParser::mapPattern()
{
    QVector<shared_ptr<Expression> > keys;
    QVector<shared_ptr<Pattern> > patterns;
    Token pos = lookAhead;
    match(LBRACE);
    if(LA_first_expression())
    {
        keys.append(expression());
        match(ROCKET);
        patterns.append(pattern());
        while(LA(COMMA))
        {
            match(COMMA);
            newLines();
            keys.append(expression());
            match(ROCKET);
            patterns.append(pattern());
        }
    }
    match(RBRACE);
    return shared_ptr<Pattern>(new MapPattern(pos, keys, patterns));
}

shared_ptr<Expression> KalimatParser::arithmeticExpression()
{
    shared_ptr<Expression> t = multiplicativeExpression();

    while(LA(ADD_OP) || LA(SUB_OP))
    {
        Token tok  = lookAhead;
        QString operation = getOperation(lookAhead);
        match(lookAhead.Type);
        shared_ptr<Expression> t2 = multiplicativeExpression();
        t = shared_ptr<Expression>(new BinaryOperation(tok, operation, t, t2));
    }
    return t;
}

shared_ptr<Expression> KalimatParser::multiplicativeExpression()
{
    shared_ptr<Expression> t = positiveOrNegativeExpression();
    while(LA(MUL_OP) || LA(DIV_OP))
    {
        Token tok  = lookAhead;
        QString operation = getOperation(lookAhead);
        match(lookAhead.Type);
        shared_ptr<Expression> t2 = positiveOrNegativeExpression();
        t = shared_ptr<Expression>(new BinaryOperation(tok, operation, t, t2));
    }
    return t;
}

shared_ptr<Expression> KalimatParser::positiveOrNegativeExpression()
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

    shared_ptr<Expression> t = primaryExpression();
    if(pos)
        t = shared_ptr<Expression>(new UnaryOperation(tok, "pos", t));
    else if(neg)
        t = shared_ptr<Expression>(new UnaryOperation(tok, "neg", t));

    return t;
}

shared_ptr<Expression> KalimatParser::primaryExpression()
{
    shared_ptr<Expression> ret = primaryExpressionNonInvokation();
    Token tok;
    while(LA(LPAREN) || LA(COLON) || LA(LBRACKET))
    {
        if(LA(LPAREN))
        {
            QVector<shared_ptr<Expression> > args;
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
            ret = shared_ptr<Expression>(new Invokation(tok, ret, args));
        }
        if(LA(COLON))
        {
            ParserState s = saveState();
            try
            {
                match(COLON);
                tok = lookAhead;
                shared_ptr<Identifier> methodName = identifier();
                QVector<shared_ptr<Expression> > args;
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
                ret = shared_ptr<Expression>(new MethodInvokation(tok, ret, methodName, args));
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
            shared_ptr<Expression> index = expression();
            if(LA(COMMA))
            {
                multiDim = true;
                QVector<shared_ptr<Expression> > indexes;
                indexes.append(index);

                match(COMMA);
                indexes.append(expression());

                while(LA(COMMA))
                {
                    match(COMMA);
                    indexes.append(expression());
                }
                ret = shared_ptr<Expression>(new MultiDimensionalArrayIndex(tok, ret, indexes));

            }
            match(RBRACKET);
            if(!multiDim)
            {
                ret = shared_ptr<Expression>(new ArrayIndex(tok, ret, index));
            }
        }
    }
    return ret;
}

bool KalimatParser::LA_first_simple_literal()
{
    return LA(NUM_LITERAL) ||
                LA(STR_LITERAL) ||
                LA(NOTHING) ||
                LA(C_TRUE) ||
                LA(C_FALSE);
}

bool KalimatParser::LA_first_primary_expression()
{
    return LA_first_simple_literal() ||
            LA(LBRACKET) ||
            LA(LBRACE) ||
            LA(IDENTIFIER) ||
            LA(TIMING) ||
            LA(LPAREN);
                //todo: decide if we still want to keep the field accessor field(obj, id)
                //||  LA(FIELD);
}

shared_ptr<SimpleLiteral> KalimatParser::simpleLiteral()
{
    shared_ptr<SimpleLiteral> ret;
    if(LA(NUM_LITERAL))
    {
        ret = shared_ptr<SimpleLiteral>(new NumLiteral(lookAhead, lookAhead.Lexeme));
        match(NUM_LITERAL); // will throw if wrong lookAhead
    }
    else if(LA(STR_LITERAL))
    {
        ret = shared_ptr<SimpleLiteral>(new StrLiteral(lookAhead, prepareStringLiteral(lookAhead.Lexeme)));
        match(STR_LITERAL); // will throw if wrong lookAhead
    }
    else if(LA(NOTHING))
    {
        ret = shared_ptr<SimpleLiteral>(new NullLiteral(lookAhead));
        match(NOTHING);
    }
    else if(LA(C_TRUE))
    {
        ret = shared_ptr<SimpleLiteral>(new BoolLiteral(lookAhead, true));
        match(lookAhead.Type);
    }
    else if(LA(C_FALSE))
    {
        ret = shared_ptr<SimpleLiteral>(new BoolLiteral(lookAhead, false));
        match(lookAhead.Type);
    }
    else
    {
        throw ParserException(getPos(), "Expected a simple literal");
    }
    return ret;
}

shared_ptr<Expression> KalimatParser::primaryExpressionNonInvokation()
{
    shared_ptr<Expression> ret;
    if(LA_first_simple_literal())
    {
        return simpleLiteral();
    }
    else if(LA(LBRACKET))
    {
        Token lbPos = lookAhead;
        match(LBRACKET);
        QVector<shared_ptr<Expression> > data = comma_separated_expressions();
        match(RBRACKET);
        ret = shared_ptr<Expression>(new ArrayLiteral(lbPos, data));
    }
    else if(LA(LBRACE))
    {
        Token lbPos = lookAhead;
        match(LBRACE);
        QVector<shared_ptr<Expression> > data = comma_separated_pairs();
        match(RBRACE);
        ret = shared_ptr<Expression>(new MapLiteral(lbPos, data));
    }
    else if(LA(IDENTIFIER))
    {
        Token tok = lookAhead;
        shared_ptr<Identifier> id = identifier();
        ret = id;
        if(LA(NEW))
        {
            Token newTok = lookAhead;
            match(NEW);
            ret = shared_ptr<Expression>(new ObjectCreation(newTok, id));
        }
        else if(LA(DOLLAR))
        {
            match(DOLLAR);
            shared_ptr<Expression> modaf_elaih = primaryExpression();
            ret = shared_ptr<Expression>(new Idafa(id->getPos(), id, modaf_elaih));
        }
        else if(LA_first_primary_expression() && !LA(LPAREN) && !LA(LBRACKET))
        {
            ParserState s = saveState();
            shared_ptr<Expression> modaf_elaih = primaryExpression();
            ret = shared_ptr<Expression>(new Idafa(id->getPos(), id, modaf_elaih));
        }

    }
    else if(LA(TIMING))
    {
        Token pos = lookAhead;
        match(TIMING);
        match(LPAREN);
        shared_ptr<Expression> toTime = expression();
        match(RPAREN);
        ret = shared_ptr<Expression>(new TimingExpression(pos, toTime));
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

shared_ptr<Identifier> KalimatParser::identifier()
{
    shared_ptr<Identifier> ret;
    if(LA(IDENTIFIER))
    {
        ret = shared_ptr<Identifier>(new Identifier(lookAhead, lookAhead.Lexeme));
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

shared_ptr<TypeExpression> KalimatParser::typeExpression()
{
    if(LA(POINTER))
    {
        Token tok = lookAhead;

        match(POINTER);
        match(LPAREN);
        shared_ptr<TypeExpression> pointee = typeExpression();
        match(RPAREN);
        return shared_ptr<TypeExpression>(new PointerTypeExpression(tok, pointee));
    }
    else if(LA(IDENTIFIER))
    {
        shared_ptr<TypeIdentifier> ret(new TypeIdentifier(lookAhead, lookAhead.Lexeme));
        match(IDENTIFIER);
        return ret;
    }
    else if(LA(PROCEDURE))
    {
        QVector<shared_ptr<TypeExpression> > argTypes;
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
        return shared_ptr<TypeExpression>(new FunctionTypeExpression(tok,
                                                                         shared_ptr<TypeExpression>(),
                                                                         argTypes));
    }
    else if(LA(FUNCTION))
    {
        QVector<shared_ptr<TypeExpression> > argTypes;
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
        shared_ptr<TypeExpression> retType = typeExpression();
        return shared_ptr<TypeExpression>(new FunctionTypeExpression(tok, retType, argTypes));
    }

    throw ParserException(getPos(), "Expected Type Expression");
}

QVector<shared_ptr<Expression> > KalimatParser::comma_separated_expressions()
{
    QVector<shared_ptr<Expression> > ret;
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

QVector<shared_ptr<Expression> > KalimatParser::comma_separated_pairs()
{
    QVector<shared_ptr<Expression> > ret;
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

QVector<shared_ptr<StrLiteral> > KalimatParser::usingDirectives()
{
    QVector<shared_ptr<StrLiteral> > usedModules;

    newLines();
    while(LA(USING))
    {
        match(USING);
        if(LA(STR_LITERAL))
        {
            shared_ptr<StrLiteral> moduleName(new StrLiteral(lookAhead, prepareStringLiteral(lookAhead.Lexeme)));
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
