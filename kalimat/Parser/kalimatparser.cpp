/**************************************************************************
**   The Kalimat programming language
**   Copyright (C) 2010 Mohamed Samy Ali - samy2004@gmail.com
**   This project is released under the Apache License version 2.0
**   as described in the included license.txt file
**************************************************************************/

#include "parser_incl.h"
#include "../Lexer/kalimatlexer.h"
#include "KalimatAst/kalimatast_incl.h"
#include "kalimatparser.h"

#include "../mainwindow.h" // temp todo: remove

#define _ws(str) QString::fromStdWString(str)

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
    //#parser-recovery
recover:
    while(!eof())
    {
        // Declaration has to be tested first because of possible
        // ambiguity with identifiers:
        // ID GLOBAL => declaration
        // ID ...    => statement
        try
        {

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
                throw ParserException(fileName, getPos(), ExpectedStatementOrDeclaration);
            }
            newLines();
        }
        catch(ParserException ex)
        {
recoveryLogic:
            if(!withRecovery)
                throw;
            bool lineStart = atStartOfFile;
            while(true)
            {
                if(eof())
                    break;
                if(lineStart && (LA_first_statement()|| LA_first_declaration()))
                {
                    goto recover;
                }
                lineStart = lookAhead.Is(NEWLINE);
                match(lookAhead.Type);
            }
        }
    }
    entryPoint = shared_ptr<ProceduralDecl>(new ProcedureDecl(Token(),
                                                              true,
                                                              Token(),
                                                              shared_ptr<Identifier>(new Identifier(Token(), "%main")),
                                                              QVector<shared_ptr<FormalParam> >(),
                                                              shared_ptr<BlockStmt>(new BlockStmt(Token(),topLevelStatements))));
    elements.append(entryPoint);
    return shared_ptr<AST>(new Program(Token(), usedModules, elements, originalElements));
}

shared_ptr<AST> KalimatParser::module()
{
    QVector<shared_ptr<Declaration> > elements;
    QVector<shared_ptr<StrLiteral > > usedModules;
    newLines();
    match(UNIT);
    shared_ptr<Identifier> modName = identifier();
    if(!LA(NEWLINE))
        goto emptyModule;

    match(NEWLINE);
    newLines();
    usedModules = usingDirectives();

    //#parser-recovery
recover:

    while(!eof())
    {
        try
        {
            if(LA_first_declaration())
            {
                elements.append(declaration());
            }
            else if(LA_first_statement())
            {
                throw ParserException(fileName, getPos(), ModulesCannotContainStatements);
            }
            else
            {
                throw ParserException(fileName, getPos(), ExpectedDeclaration);
            }
            newLines();
        }
        catch(ParserException ex)
        {
recoveryLogic:
            if(!withRecovery)
                throw;
            bool lineStart = atStartOfFile;
            while(true)
            {
                if(eof())
                    break;
                if(lineStart && (LA_first_declaration()))
                {
                    goto recover;
                }
                lineStart = lookAhead.Is(NEWLINE);
                match(lookAhead.Type);
            }
        }
    }
emptyModule:
    return shared_ptr<AST>(new Module(Token(), usedModules, modName, elements));
}

bool KalimatParser::LA_first_statement()
{
    return LA(IF) || LA(FORALL) || LA(REPEAT) || LA(RETURN_WITH)
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
    else if(LA(REPEAT))
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
        throw ParserException(fileName, getPos(), StatementNotImplemented);

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
        throw ParserException(fileName, getPos(), ExpectedDeclaration);

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
    shared_ptr<TypeExpression> type;
    if(LA(IS))
    {
        match(IS);
        shared_ptr<Identifier> id2 = dynamic_pointer_cast<Identifier>(first);
        if(!id2)
        {
            throw ParserException(fileName, getPos(), DeclaringTypeInAssignmentMustBeForVariable);
        }
        type = typeExpression();
        if(!LA(EQ))
        {
            throw ParserException(fileName, getPos(), MeaninglessTypeDeclaration);
        }
    }
    if(LA(EQ))
    {

        shared_ptr<AssignableExpression> id = dynamic_pointer_cast<AssignableExpression>(first);
        if(id == NULL)
        {
            throw ParserException(fileName, getPos(), LeftOfAssignmentMustBeAssignableExpression);
        }
        if(LA(IS))
        {
            match(IS);
            shared_ptr<Identifier> id2 = dynamic_pointer_cast<Identifier>(id);
            if(!id2)
            {
                throw ParserException(fileName, getPos(), DeclaringTypeInAssignmentMustBeForVariable);
            }
            type = typeExpression();

        }
        Token eqToken = lookAhead;
        match(EQ);
        shared_ptr<Expression> value = expression();
        return shared_ptr<Statement>(new AssignmentStmt(eqToken, id, value, type));
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
    throw ParserException(fileName, getPos(), ExpectedIdentifier);
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
            //todo:slow
            QSet<TokenType> terms;
            terms.insert(ELSE);
            terms.insert(DONE);
            thenPart = block(terms);
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

                //todo:slow
                QSet<TokenType> terms;
                terms.insert(ELSE);
                terms.insert(DONE);
                otherPart = block(terms);
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
                //todo:slow
                QSet<TokenType> terms;
                terms.insert(DONE);
                elsePart = block(terms);
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
    throw ParserException(fileName, getPos(), "Expected IF");
}
shared_ptr<Statement> KalimatParser::forEachStmt()
{
    bool multiLineStmt = false;
    bool downTo = false;
    shared_ptr<Statement> theStmt;
    shared_ptr<Expression> theStep;
    Token forAllTok = lookAhead;
    match(FORALL);
    shared_ptr<Identifier> id = identifier();
    match(FROM);
    shared_ptr<Expression> from = expression();
    if(LA(DOWNTO))
    {
        match(DOWNTO);
        downTo = true;
    }
    match(TO);
    shared_ptr<Expression> to = expression();

    if(LA(STEP))
    {
        match(STEP);
        theStep = expression();
    }
    else
    {
        theStep = shared_ptr<Expression>(new NumLiteral(to->getPos(),
                                                        downTo?-1:1));
    }
    match(COLON);
    if(LA(NEWLINE))
    {
        match(NEWLINE);
        multiLineStmt = true;
    }
    if(multiLineStmt)
    {
        //todo:slow
        QSet<TokenType> terms;
        terms.insert(CONTINUE);
        theStmt = block(terms);
    }
    else
    {
        theStmt = statement();
    }
    match(CONTINUE);
    return shared_ptr<Statement>(new ForAllStmt(forAllTok,
                                                id,
                                                from,
                                                to,
                                                theStep,
                                                theStmt,
                                                downTo));
}

shared_ptr<Statement> KalimatParser::whileStmt()
{
    bool multiLineStmt = false;
    shared_ptr<Statement> theStmt;
    shared_ptr<Expression> cond;
    Token repeatTok = lookAhead;

    match(REPEAT);

    if(LA(WHILE))
    {
        match(WHILE);
        cond = expression();
    }
    else
    {
        cond = shared_ptr<Expression>(new BoolLiteral(repeatTok, true));
    }

    match(COLON);
    if(LA(NEWLINE))
    {
        match(NEWLINE);
        multiLineStmt = true;
    }
    if(multiLineStmt)
    {
        //todo:slow
        QSet<TokenType> terms;
        terms.insert(CONTINUE);
        theStmt = block(terms);
    }
    else
    {
        theStmt = statement();
    }
    match(CONTINUE);
    return shared_ptr<Statement>(new WhileStmt(repeatTok, cond, theStmt));
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
        throw ParserException(fileName, expr->getPos(),CanDelegateOnlyToInvokation);
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
        throw ParserException(fileName, expr->getPos(),
                              CanOnlyLaunchProcedureInvokation);
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
    if(LA_first_expression())
    {
        target = expression();
    }
    else
    {
        throw ParserException(fileName, pos, ExpressionExpectedAfterGoto);
    }
    return shared_ptr<Statement>(new GotoStmt(pos, target));
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
            throw ParserException(fileName, getPos(), "Item in read statement must be an assignable expression");
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
                throw ParserException(fileName, getPos(), "Item in read statement must be an assignable expression");
            }
            vars.append(var);
            readNums.append(readInt);
        }
        return shared_ptr<Statement>(new ReadStmt(readTok, fileObject, prompt, vars, readNums));
    }
    throw ParserException(fileName, getPos(), "Expected PRINT or READ");
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
    throw ParserException(fileName, getPos(), ExpectedDrawingStatement);
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
        throw ParserException(fileName, "Expected KB or MOUSE");
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
        throw ParserException(fileName, getPos(), ExpectedExpression);
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
    return shared_ptr<SendStmt>(new SendStmt(tok, value, chan, signal));
}

shared_ptr<ReceiveStmt> KalimatParser::receiveStmt()
{
    bool signal = false;
    shared_ptr<AssignableExpression> value;
    shared_ptr<Expression> chan;
    match(RECEIVE);
    if(eof())
        throw ParserException(fileName, getPos(), ExpectedExpression);
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
            throw ParserException(fileName, getPos(), "'Receive' must take an assignable expression");
        }
    }
    match(FROM);
    chan = expression();
    return shared_ptr<ReceiveStmt>(new ReceiveStmt(tok, value, chan, signal));
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
        throw ParserException(fileName, getPos(), ExpectedSendOrReceiveOperation);
    }
    match(COLON);
    if(LA(NEWLINE))
    {
        multilineStatement = true;
    }
    if(multilineStatement)
    {
        //todo:slow
        QSet<TokenType> terms;
        terms.insert(OR);
        terms.insert(DONE);
        actions.append(block(terms));
    }
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
            throw ParserException(fileName, getPos(), ExpectedSendOrReceiveOperation);
        }
        match(COLON);

        if(multilineStatement)
        {
            //todo:slow
            QSet<TokenType> terms;
            terms.insert(DONE);
            actions.append(block(terms));
        }
        else
            actions.append(statement());
    }
    match(DONE);
    return shared_ptr<Statement>(new SelectStmt(tok, conditions, actions));
}

shared_ptr<BlockStmt> KalimatParser::block(QSet<TokenType> blockTerminators)
{
    QVector<shared_ptr<Statement> > stmts;
    Token tok = lookAhead;
    newLines();
    //#parser-recovery
recover:
    while(LA_first_statement())
    {
        try
        {
            stmts.append(statement());
            if(!eof())
                match(NEWLINE);
            newLines();
        }
        catch(ParserException ex)
        {
recoveryLogic:
            if(!withRecovery)
                throw;
            bool lineStart = atStartOfFile;
            while(true)
            {
                if(eof())
                    break;
                if(LA(blockTerminators))
                    break;
                if(lineStart && (LA_first_statement()))
                {
                    goto recover;
                }
                if(LA(blockTerminators))
                    break;
                lineStart = lookAhead.Is(NEWLINE);
                match(lookAhead.Type);
            }
        }
    }

    return shared_ptr<BlockStmt>(new BlockStmt(tok, stmts));
}

QVector<shared_ptr<FormalParam> > KalimatParser::formalParamList()
{
    QVector<shared_ptr<FormalParam> > formals;

    match(LPAREN);
    formals = commaSeparatedFormalParams();
    match(RPAREN);
    return formals;
}

QVector<shared_ptr<FormalParam> > KalimatParser::commaSeparatedFormalParams()
{
    QVector<shared_ptr<FormalParam> > formals;
    if(LA(IDENTIFIER))
    {
        shared_ptr<Identifier> pname = identifier();
        shared_ptr<TypeExpression> ptype;

        if(LA(IS))
        {
            match(IS);
            ptype = typeExpression();
        }
        formals.append(shared_ptr<FormalParam>(new FormalParam(pname, ptype)));
        while(LA(COMMA))
        {
            match(COMMA);
            pname = identifier();
            ptype = shared_ptr<TypeExpression>();
            if(LA(IS))
            {
                match(IS);
                ptype = typeExpression();
            }
            formals.append(shared_ptr<FormalParam>(new FormalParam(pname, ptype)));
        }
    }
    return formals;
}

shared_ptr<Declaration> KalimatParser::procedureDecl()
{
    match(PROCEDURE);
    Token tok  = lookAhead;
    shared_ptr<Identifier> procName = identifier();
    QVector<shared_ptr<FormalParam> > formals = formalParamList();

    match(COLON);
    match(NEWLINE);

    shared_ptr<ProcedureDecl> ret(new ProcedureDecl(tok,
                                                    true,
                                                    Token(),
                                                    procName,
                                                    formals,
                                                    shared_ptr<BlockStmt>()
                                                    ));

    varContext.push(ret);

    //todo:slow
    QSet<TokenType> terms;
    terms.insert(END);
    shared_ptr<BlockStmt> body = block(terms);
    varContext.pop();

    expect(END);
    Token endToken = lookAhead;
    match(END);

    ret->_endingToken = endToken;
    ret->setBody(body);
    return ret;
}

shared_ptr<Declaration> KalimatParser::functionDecl()
{
    match(FUNCTION);
    Token tok  = lookAhead;
    shared_ptr<Identifier> procName = identifier();
    QVector<shared_ptr<FormalParam> > formals = formalParamList();
    match(COLON);
    match(NEWLINE);
    shared_ptr<FunctionDecl> ret(new FunctionDecl(tok, true, Token(), procName, formals, shared_ptr<BlockStmt>()));

    varContext.push(ret);
    //todo:slow
    QSet<TokenType> terms;
    terms.insert(END);
    shared_ptr<BlockStmt> body = block(terms);
    varContext.pop();

    expect(END);
    Token endToken = lookAhead;
    match(END);

    ret->_endingToken = endToken;
    ret->setBody(body);
    return ret;
}

void KalimatParser::addPropertySetter(Token pos,
                                      shared_ptr<Identifier> methodName,
                                      QVector<shared_ptr<FormalParam> > formals,
                                      QMap<QString, PropInfo> &propertyInfo)
{
    // Since this is 'responds', the property has to be a setter

    if(!methodName->name().startsWith(QString::fromStdWString(L"حدد.")))
    {
        throw ParserException(fileName, pos, PropertySetterMustBeginWithSet);
    }
    else if(formals.count() !=1)
    {
        throw ParserException(fileName, pos, PropertySetterTakesOneArgument);
    }
    QString realName = methodName->name().mid(4); // remove حدد. //todo: handle when translating to English version
    if(!propertyInfo.contains(realName))
    {
        propertyInfo[realName] = PropInfo();
        propertyInfo[realName].write = true;
    }
}

void KalimatParser::addPropertyGetter(Token pos,
                                      shared_ptr<Identifier> methodName,
                                      QVector<shared_ptr<FormalParam> > formals,
                                      QMap<QString, PropInfo> &propertyInfo)
{
    // Since this is 'replies', the property has to be a getter

    if(formals.count() !=0)
    {
        throw ParserException(fileName, pos, PropertyGetterTakesNoArguments);
    }
    QString realName = methodName->name();
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
            h->addField(fname);
            fields.append(fname);
            if(LA(MARSHALLAS))
            {
                match(MARSHALLAS);
                shared_ptr<TypeExpression> te = typeExpression();
                fieldMarshallAs[fname->name()] = te;
            }
            while(LA(COMMA))
            {
                match(COMMA);
                fname = identifier();
                h->addField(fname);
                fields.append(fname);
                if(LA(MARSHALLAS))
                {
                    match(MARSHALLAS);
                    shared_ptr<TypeExpression> te = typeExpression();
                    fieldMarshallAs[fname->name()] = te;
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
            QVector<shared_ptr<FormalParam> > formals = formalParamList();
            shared_ptr<RespondsTo> rt(new RespondsTo(false));

            shared_ptr<ConcreteResponseInfo> ri(new ConcreteResponseInfo(methodName));
            for(QVector<shared_ptr<FormalParam> >::const_iterator i=formals.begin(); i!= formals.end(); ++i)
                ri->addParam(*i);

            rt->addMethod(ri);
            if(LA(PROPERTY))
            {
                Token p = lookAhead;
                match(PROPERTY);
                addPropertySetter(p, methodName,formals,propertyInfo);
            }

            methods[methodName->name()] = MethodInfo(formals.count(), false);
            while(LA(COMMA))
            {
                match(COMMA);
                shared_ptr<Identifier> methodName = identifier();
                QVector<shared_ptr<FormalParam> > formals= formalParamList();
                shared_ptr<ConcreteResponseInfo> ri(new ConcreteResponseInfo(methodName));
                for(QVector<shared_ptr<FormalParam> >::const_iterator i=formals.begin(); i!= formals.end(); ++i)
                    ri->addParam(*i);
                rt->addMethod(ri);
                if(LA(PROPERTY))
                {
                    Token p = lookAhead;
                    match(PROPERTY);
                    addPropertySetter(p, methodName,formals,propertyInfo);
                }
                methods[methodName->name()] = MethodInfo(formals.count(), false);
            }
            internalDecls.append(rt);
            match(NEWLINE);
        }
        if(LA(REPLIES))
        {
            match(REPLIES);
            match(ON);

            shared_ptr<Identifier> methodName = identifier();
            QVector<shared_ptr<FormalParam> > formals = formalParamList();
            shared_ptr<RespondsTo> rt(new RespondsTo(true));
            shared_ptr<ConcreteResponseInfo> ri(new ConcreteResponseInfo(methodName));
            for(QVector<shared_ptr<FormalParam> >::const_iterator i=formals.begin(); i!= formals.end(); ++i)
                ri->addParam(*i);
            rt->addMethod(ri);
            if(LA(PROPERTY))
            {
                Token p = lookAhead;
                match(PROPERTY);
                addPropertyGetter(p, methodName,formals,propertyInfo);
            }

            methods[methodName->name()] = MethodInfo(formals.count(), true);
            while(LA(COMMA))
            {
                match(COMMA);
                shared_ptr<Identifier> methodName = identifier();
                QVector<shared_ptr<FormalParam> > formals= formalParamList();
                shared_ptr<ConcreteResponseInfo> ri(new ConcreteResponseInfo(methodName));
                for(QVector<shared_ptr<FormalParam> >::const_iterator i=formals.begin(); i!= formals.end(); ++i)
                    ri->addParam(*i);
                rt->addMethod(ri);
                if(LA(PROPERTY))
                {
                    Token p = lookAhead;
                    match(PROPERTY);
                    addPropertyGetter(p, methodName,formals,propertyInfo);
                }
                methods[methodName->name()] = MethodInfo(formals.count(), true);
            }
            internalDecls.append(rt);
            match(NEWLINE);
        }
        if(LA(BUILT))
        {
            Token b = lookAhead;
            if(ancestorName != NULL)
                throw ParserException(fileName, getPos(), "Class cannot inherit from more than one base class");
            match(BUILT);
            match(ON);
            ancestorName = identifier();
        }
        newLines();
    }
    match(END);
    return shared_ptr<Declaration>(new ClassDecl(tok, true, ancestorName, className, fields, methods, internalDecls, fieldMarshallAs));
}

shared_ptr<Declaration> KalimatParser::globalDecl()
{
    shared_ptr<Identifier> var;
    Token tok  = lookAhead;
    var = identifier();
    match(GLOBAL);
    return shared_ptr<Declaration>(new GlobalDecl(tok, true, var->name()));
}

bool KalimatParser::LA_first_method_declaration()
{
    return LA(RESPONSEOF) || LA(REPLYOF);
}

shared_ptr<Declaration> KalimatParser::methodDecl()
{
    bool isFunctionNotProcedure = false;
    shared_ptr<Identifier> className;
    shared_ptr<Identifier> receiverName;
    shared_ptr<Identifier> methodName;

    QVector<shared_ptr<FormalParam> > formals;
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
        throw ParserException(fileName, tok, "Syntax error");
    }
    formals = formalParamList();
    match(COLON);
    match(NEWLINE);

    if(methodName->name() == "%nosuchmethod" && formals.count() !=2)
    {
        throw ParserException(fileName, tok, NoSuchMethodHandlerTakesTwoArguments);
    }

    shared_ptr<MethodDecl> ret(new MethodDecl(tok, true, Token(), methodName,
                                              formals, shared_ptr<BlockStmt>(),
                                              className, receiverName,
                                              isFunctionNotProcedure));

    varContext.push(ret);
    //todo:slow
    QSet<TokenType> terms;
    terms.insert(END);
    shared_ptr<BlockStmt> body = block(terms);
    varContext.pop();
    ret->setBody(body);

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
        ffiLib = shared_ptr<FFILibraryDecl>(new FFILibraryDecl(tok, true, libName->value(), decls));
        return ffiLib;
    }
    else
    {
        throw ParserException(fileName, tok, ExpectedStringLiteral);
    }
}

shared_ptr<Declaration> KalimatParser::rulesDecl()
{
    QVector<shared_ptr<RuleDecl> > subRules;
    match(RULES);
    shared_ptr<Identifier> ruleName = identifier();
    match(COLON);
    match(NEWLINE);
    newLines();
    while(LA(IDENTIFIER))
    {
        shared_ptr<RuleDecl> rd = ruleDecl();
        subRules.append(rd);
        newLines();
    }
    match(END);
    return shared_ptr<Declaration>
            (new RulesDecl(ruleName->getPos(),
                           true,
                           ruleName,
                           subRules
                           ));
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
    else
    {
        shared_ptr<PegPrimary> theExpr = dynamic_pointer_cast<PegPrimary>(expr);
        if(theExpr)
        {
            if(!theExpr->associatedVar())
            {
                theExpr->setAssociatedVar(
                            shared_ptr<Identifier>(
                                new Identifier(theExpr->getPos(), "%tmpAV")));
            }
            resultExpr = shared_ptr<Expression>(new VarAccess(theExpr->getPos(),
                                                              shared_ptr<Identifier>(new Identifier(theExpr->getPos(),
                                                                                         theExpr->associatedVar()->name()))));
        }
    }
    match(NEWLINE);
    newLines();
    options.append(shared_ptr<RuleOption>(new RuleOption(expr->getPos(), expr, resultExpr)));
    while(LA(OR))
    {
        match(OR);
        expr = pegExpr();
        shared_ptr<Expression> _resultExpr;
        if(LA(ROCKET))
        {
            match(ROCKET);
            _resultExpr = expression();
        }
        else
        {
            shared_ptr<PegPrimary> theExpr = dynamic_pointer_cast<PegPrimary>(expr);
            if(theExpr)
            {
                if(!theExpr->associatedVar())
                {
                    theExpr->setAssociatedVar(
                                shared_ptr<Identifier>(
                                    new Identifier(theExpr->getPos(), "%tmpAV")));
                }
                _resultExpr = shared_ptr<Expression>(new VarAccess(theExpr->getPos(), shared_ptr<Identifier>(new Identifier(theExpr->getPos(),
                                                                    theExpr->associatedVar()->name()))));
            }
        }
        options.append(shared_ptr<RuleOption>(new RuleOption(expr->getPos(), expr, _resultExpr)));
        match(NEWLINE);
        newLines();
    }
    return shared_ptr<RuleDecl>(new RuleDecl(name->getPos(), name->name(), options));
}

shared_ptr<PegExpr> KalimatParser::pegExpr()
{
    shared_ptr<PegExpr> ret = primaryPegExpression();
    if(LA_first_primary_peg_expression())
    {
        QVector<shared_ptr<PegExpr> > seqElements;
        seqElements.append(ret);
        while(LA_first_primary_peg_expression())
        {
            seqElements.append(primaryPegExpression());
        }
        ret = shared_ptr<PegExpr>(new PegSequence(ret->getPos(),
                                                  seqElements));
    }
    return ret;
}

bool KalimatParser::LA_first_primary_peg_expression()
{
    return LA(IDENTIFIER) || LA(STR_LITERAL) || LA(FROM);
}

shared_ptr<PegExpr> KalimatParser::primaryPegExpression()
{
    if(LA(IDENTIFIER))
    {
        shared_ptr<Identifier> ruleName = identifier();
        shared_ptr<Identifier> varName;
        if(LA(COLON))
        {
            match(COLON);
            varName = identifier();
        }
        return shared_ptr<PegExpr>(
                    new PegRuleInvokation(ruleName->getPos(), varName, ruleName));
    }
    else if(LA(STR_LITERAL))
    {
        shared_ptr<StrLiteral> lit =
                dynamic_pointer_cast<StrLiteral>(simpleLiteral());
        shared_ptr<Identifier> varName;
        if(LA(COLON))
        {
            match(COLON);
            varName = identifier();
        }
        return shared_ptr<PegExpr>(
                    new PegLiteral(lit->getPos(), varName, lit));
    }
    else if(LA(FROM))
    {
        shared_ptr<Identifier> varName;
        match(FROM);
        if(!LA(STR_LITERAL))
            throw ParserException(fileName, getPos(), ExpectedStringWithSingleCharacter);
        shared_ptr<StrLiteral> lit1 = dynamic_pointer_cast<StrLiteral>(simpleLiteral());
        if(lit1->value().length() != 1)
            throw ParserException(fileName, lit1->getPos(), ExpectedStringWithSingleCharacter);
        match(TO);
        if(!LA(STR_LITERAL))
            throw ParserException(fileName, getPos(), ExpectedStringWithSingleCharacter);
        shared_ptr<StrLiteral> lit2 = dynamic_pointer_cast<StrLiteral>(simpleLiteral());
        if(lit2->value().length() != 1)
            throw ParserException(fileName, lit2->getPos(), ExpectedStringWithSingleCharacter);

        if(LA(COLON))
        {
            match(COLON);
            varName = identifier();
        }

        return shared_ptr<PegExpr>(
                    new PegCharRange(lit1->getPos(), varName, lit1, lit2));
    }
    else
    {
        throw ParserException(fileName, getPos(), InvalidStartOfPrimaryPegExpression);
    }
}

shared_ptr<FFIProceduralDecl> KalimatParser::ffiFunctionDecl()
{
    QVector<shared_ptr<TypeExpression> > argTypes;
    shared_ptr<TypeExpression> retType;
    QString symbol;
    Token pos = lookAhead;
    match(FUNCTION);

    shared_ptr<Identifier> fname = identifier();
    symbol = fname->name();
    if(LA(SYMBOL))
    {
        match(SYMBOL);
        if(LA(STR_LITERAL))
        {
            shared_ptr<StrLiteral> s(new StrLiteral(lookAhead, prepareStringLiteral(lookAhead.Lexeme)));
            symbol = s->value();
            match(STR_LITERAL);
        }
        else
        {
            throw ParserException(fileName, pos, "Expected string literal after keyword 'symbol'");
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
    return shared_ptr<FFIProceduralDecl>(new FFIProceduralDecl(pos, true, true, fname->name(), symbol, retType, argTypes));
}

shared_ptr<FFIProceduralDecl> KalimatParser::ffiProcDecl()
{
    QVector<shared_ptr<TypeExpression> > argTypes;
    shared_ptr<TypeExpression> retType;
    QString symbol;
    Token pos = lookAhead;
    match(PROCEDURE);

    shared_ptr<Identifier> fname = identifier();
    symbol = fname->name();
    if(LA(SYMBOL))
    {
        match(SYMBOL);
        if(LA(STR_LITERAL))
        {
            shared_ptr<StrLiteral> s(new StrLiteral(lookAhead, prepareStringLiteral(lookAhead.Lexeme)));
            symbol = s->value();
            match(STR_LITERAL);
        }
        else
        {
            throw ParserException(fileName, pos, "Expected string literal after keyword 'symbol'");
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
    return shared_ptr<FFIProceduralDecl>(new FFIProceduralDecl(pos, true, false, fname->name(), symbol, retType, argTypes));
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
    throw ParserException(fileName, getPos(), "Invalid start of pattern");
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
        throw ParserException(fileName, getPos(), "After ? must be an assignable expression");
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
        return shared_ptr<Pattern>(new VarPattern(id->getPos(), shared_ptr<VarAccess>(new VarAccess(id->getPos(), id))));
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
    return shared_ptr<Pattern>(new ArrayPattern(pos, patterns, fixedLen));
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
            // If it's an invokation, that means in an expression fun(arg)
            // fun was not a variable access but an identifier ...
            shared_ptr<VarAccess> dummyVar = dynamic_pointer_cast<VarAccess>(ret);
            if(dummyVar)
            {
                ret = shared_ptr<Expression>(new Invokation(tok, dummyVar->name(), args));
            }
            else
            {
                // otherwise it's a 'ForAutoComplete'
                // todo: what should we do here?
            }

        }
        if(LA(COLON))
        {
            ParserState s = saveState();

            match(COLON);
            tok = lookAhead;
            if(false && withRecovery && !LA2(IDENTIFIER, LPAREN))
            {
                // if we are parsing for autocomplete information
                // and we entered x :
                // we need the identifier x to be in the AST
                // so that it's type be determined by the analyzer
                // in that case we'll make the whole expression
                // some special AST type
                ret = shared_ptr<ForAutocomplete>(new
                                                  ForAutocomplete(ret->getPos(),
                                                                  ret)
                                                  );
            }
            else
            {
                if(!LA2(IDENTIFIER, LPAREN))
                {
                    // We have seen a form <primary-non-invokation> <colon>
                    // if it were followed by 'identifier(' it would've been
                    // a method call, but it isn't; could be an ending of an 'if' condition
                    // so we backtrack
                    {
                        restoreState(s);
                        break;
                    }
                }

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
            LA(THEPROC) ||
            LA(THEFUNCTION) ||
            LA(LAMBDA) ||
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
        throw ParserException(fileName, getPos(), ExpectedSimpleLiteral);
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
        ret = shared_ptr<VarAccess>(new VarAccess(id->getPos(),id));
        if(LA(NEW))
        {
            Token newTok = lookAhead;
            QVector<shared_ptr<Identifier> > fieldInitNames;
            QVector<shared_ptr<Expression> > fieldInitValues;
            match(NEW);

            if(LA(HAS))
            {
                match(HAS);
                fieldInitNames.append(identifier());
                match(EQ);
                fieldInitValues.append(expression());
                while(LA(COMMA))
                {
                    match(COMMA);
                    fieldInitNames.append(identifier());
                    match(EQ);
                    fieldInitValues.append(expression());
                }
            }
            ret = shared_ptr<Expression>(new ObjectCreation(newTok,
                                                            id,
                                                            fieldInitNames,
                                                            fieldInitValues));

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
    else if(LA(THEPROC) || LA(THEFUNCTION))
    {
        DeclarationType what = Proc; // meaningless initial value to silence warning
        Token pos = lookAhead;
        if(pos.Type == THEPROC)
            what = Proc;
        else if(pos.Type == THEFUNCTION)
            what = Function;
        match(lookAhead.Type);
        //match(LPAREN);
        shared_ptr<Identifier> name = identifier();
        //match(RPAREN);
        ret = shared_ptr<Expression>(new TheSomething(pos, name->name(), what));
    }
    else if(LA(LAMBDA))
    {
        match(LAMBDA);
        QVector<shared_ptr<FormalParam> > argList;
        QVector<shared_ptr<Statement> > stmts;
        bool hasDo = false;
        Token pos;
        if(LA(IDENTIFIER))
        {
            argList = commaSeparatedFormalParams();
        }
        if(LA(COLON))
        {
            shared_ptr<Expression> expr;
            match(COLON);
            expr= expression();
            pos = expr->getPos();
            stmts.append(shared_ptr<Statement>(new ReturnStmt(expr->getPos(), expr)));
            hasDo = false;
        }
        else if(LA(DO))
        {
            match(DO);
            match(COLON);
            shared_ptr<Statement> stmt = statement();
            pos = stmt->getPos();
            stmts.append(stmt);
            while (LA(SEMI))
            {
                match(SEMI); newLines();
                stmts.append(statement());
            }
            hasDo = true;
        }
        ret = shared_ptr<Expression>(new LambdaExpression(pos, argList, stmts, hasDo));
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
        throw ParserException(fileName, getPos(), "Expected a literal, identifier, or parenthesized expression");
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
        //if(!varContext.empty())
        //    varContext.top()->addReference(ret);
        return ret;
    }
    throw ParserException(fileName, getPos(), ExpectedIdentifier);
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

    throw ParserException(fileName, getPos(), "Expected Type Expression");
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
            throw ParserException(fileName, getPos(), UsingKeywordMustBeFollowedByStringLiteral);
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
        throw ParserException(fileName, getPos(), "Unknown operator");
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
