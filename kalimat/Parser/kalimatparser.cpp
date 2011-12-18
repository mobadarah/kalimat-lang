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

QSharedPointer<AST> KalimatParser::parseRoot()
{
    if(LA(UNIT))
        return module();
    return program();
}

QSharedPointer<AST> KalimatParser::program()
{
    QVector<QSharedPointer<TopLevel> > elements;
    QVector<QSharedPointer<Statement> > topLevelStatements;
    QSharedPointer<ProceduralDecl> entryPoint;
    QVector<QSharedPointer<StrLiteral > > usedModules = usingDirectives();
    QVector<QSharedPointer<TopLevel > > originalElements;
    while(!eof())
    {
        // Declaration has to be tested first because of possible
        // ambiguity with identifiers:
        // ID GLOBAL => declaration
        // ID ...    => statement
        if(LA_first_declaration())
        {
            QSharedPointer<TopLevel> decl = declaration();
            elements.append(decl);
            originalElements.append(decl);
        }
        else if(LA_first_statement())
        {
            QSharedPointer<Statement> stmt = statement();
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
    entryPoint = QSharedPointer<ProceduralDecl>(new ProcedureDecl(Token(),
                                                                  Token(),
                                                                  QSharedPointer<Identifier>(new Identifier(Token(), "%main")),
                                                                  QVector<QSharedPointer<Identifier> >(),
                                                                  QSharedPointer<BlockStmt>(new BlockStmt(Token(),topLevelStatements)),
                                                                  true));
    elements.append(entryPoint);
    return QSharedPointer<AST>(new Program(Token(), elements, usedModules, originalElements));
}

QSharedPointer<AST> KalimatParser::module()
{
    QVector<QSharedPointer<Declaration> > elements;

    newLines();
    match(UNIT);
    QSharedPointer<Identifier> modName = identifier();
    match(NEWLINE);
    newLines();
    QVector<QSharedPointer<StrLiteral > > usedModules = usingDirectives();

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
    return QSharedPointer<AST>(new Module(Token(), modName, elements, usedModules));
}

bool KalimatParser::LA_first_statement()
{
    return LA(IF) || LA(FORALL) || LA(WHILE) || LA(RETURN_WITH)
            || LA(DELEGATE) || LA(LAUNCH) || LA(LABEL) || LA(GO) || LA(WHEN)
            || LA(SEND) || LA(RECEIVE) || LA(SELECT)
            || LA_first_io_statement() || LA_first_grfx_statement()
            || LA_first_assignment_or_invokation();
}

QSharedPointer<Statement> KalimatParser::statement()
{
    Token firstToken = lookAhead; // Save it in case it has a 'sister' comment
    QSharedPointer<Statement> ret;
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
        ret.data()->attachedComments = firstToken.sister->Lexeme;
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

QSharedPointer<Declaration> KalimatParser::declaration()
{
    Token firstToken = lookAhead;
    QSharedPointer<Declaration> ret;

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
QSharedPointer<Statement> KalimatParser::assignmentStmt_or_Invokation(ParserState s)
{
    QSharedPointer<Expression> first = primaryExpression();
    if(LA(EQ))
    {
        QSharedPointer<AssignableExpression> id = first.dynamicCast<AssignableExpression>();
        if(id == NULL)
        {
            throw ParserException(getPos(), "Left of = must be an assignable expression");
        }
        Token eqToken = lookAhead;
        match(EQ);
        QSharedPointer<Expression> value = expression();
        return QSharedPointer<Statement>(new AssignmentStmt(eqToken, id, value));
    }
    else
    {
        QSharedPointer<IInvokation> invokation = first.dynamicCast<IInvokation>();
        if(invokation != NULL)
        {
            return QSharedPointer<Statement>(new InvokationStmt(invokation->getPos(), invokation));
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

QSharedPointer<Statement> KalimatParser::ifStmt()
{
    QSharedPointer<Statement> thenPart;
    QSharedPointer<Statement> elsePart;
    if(LA(IF))
    {
        bool newLine = false;
        Token ifTok = lookAhead;
        match(IF);
        QSharedPointer<Expression> cond = expression();
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
        QVector<QSharedPointer<Expression> > conditions;
        QVector<QSharedPointer<Statement> > statements;

        while(LA2(ELSE, IF))
        {
            match(ELSE);
            Token p2 = lookAhead;
            positions.append(p2);
            match(IF);
            QSharedPointer<Expression> cond2 = expression();
            conditions.append(cond2);
            match(COLON);
            QSharedPointer<Statement> otherPart;
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
            elsePart  = QSharedPointer<Statement>(new IfStmt(positions[i], conditions[i], statements[i], elsePart));
        }
        return QSharedPointer<Statement>(new IfStmt(ifTok, cond, thenPart, elsePart));
    }
    throw ParserException(getPos(), "Expected IF");
}
QSharedPointer<Statement> KalimatParser::forEachStmt()
{
    bool multiLineStmt = false;
    QSharedPointer<Statement> theStmt;
    Token forAllTok = lookAhead;
    match(FORALL);
    QSharedPointer<Identifier> id = identifier();
    match(FROM);
    QSharedPointer<Expression> from = expression();
    match(TO);
    QSharedPointer<Expression> to = expression();
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
    return QSharedPointer<Statement>(new ForAllStmt(forAllTok, id, from, to, theStmt));
}

QSharedPointer<Statement> KalimatParser::whileStmt()
{
    bool multiLineStmt = false;
    QSharedPointer<Statement> theStmt;
    Token whileTok = lookAhead;
    match(WHILE);
    QSharedPointer<Expression> cond = expression();
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
    return QSharedPointer<Statement>(new WhileStmt(whileTok, cond, theStmt));
}

QSharedPointer<Statement> KalimatParser::returnStmt()
{
    Token returnTok  = lookAhead;
    match(RETURN_WITH);
    QSharedPointer<Expression> retVal = expression();
    return QSharedPointer<Statement>(new ReturnStmt(returnTok, retVal));
}

QSharedPointer<Statement> KalimatParser::delegateStmt()
{
    Token returnTok  = lookAhead;
    match(DELEGATE);
    match(TO);
    QSharedPointer<Expression> expr = expression();
    QSharedPointer<IInvokation> invokation =expr.dynamicCast<IInvokation>();
    if(invokation != NULL)
    {
        return QSharedPointer<Statement>(new DelegationStmt(returnTok, invokation));
    }
    else
    {
        throw new ParserException(expr->getPos(),
            QString::fromStdWString(L"لا يمكن التوكيل لغير استدعاء إجراء أو دالة") );
    }
}

QSharedPointer<Statement> KalimatParser::launchStmt()
{
    Token returnTok  = lookAhead;
    match(LAUNCH);
    QSharedPointer<Expression> expr = expression();
    QSharedPointer<IInvokation> invokation = expr.dynamicCast<IInvokation>();
    if(invokation !=NULL)
    {
        return QSharedPointer<Statement>(new LaunchStmt(returnTok, invokation));
    }
    else
    {
        throw new ParserException(expr->getPos(),
            QString::fromStdWString(L"لا يمكن تشغيل ما ليس باستدعاء إجراء") );
    }
}

QSharedPointer<Statement> KalimatParser::labelStmt()
{
    Token pos = lookAhead;
    match(LABEL);
    QSharedPointer<Expression> target = expression();
    return QSharedPointer<Statement>(new LabelStmt(pos, target));
}

QSharedPointer<Statement> KalimatParser::gotoStmt()
{
    Token pos = lookAhead;
    QSharedPointer<Expression> target;
    match(GO);
    match(TO);
    bool targetIsNum = false;
    if(LA(NUM_LITERAL))
    {
        targetIsNum = true;
        target = QSharedPointer<Expression>(new NumLiteral(lookAhead, lookAhead.Lexeme));
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
    return QSharedPointer<Statement>(new GotoStmt(pos, targetIsNum, target));
}

bool KalimatParser::LA_first_io_statement()
{
    return LA(PRINT) || LA(READ);
}

QSharedPointer<Statement> KalimatParser::ioStmt()
{
    if(LA(PRINT))
    {
        bool printOnSameLine = false;
        QVector<QSharedPointer<Expression> > args;
        QVector<QSharedPointer<Expression> > widths;
        QSharedPointer<Expression> fileObject;
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
            widths.append(QSharedPointer<Expression>());
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
                widths.append(QSharedPointer<Expression>());
            }
            args.append(expression());
        }
        if(LA(ELLIPSIS))
        {
            match(ELLIPSIS);
            printOnSameLine = true;
        }
        officialEndOfPrintStmt:
        return QSharedPointer<Statement>(new PrintStmt(printTok, fileObject, args, widths, printOnSameLine));
    }
    if(LA(READ))
    {
        QString prompt = "";
        Token readTok  = lookAhead;
        QSharedPointer<Expression> fileObject;
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
        QVector<QSharedPointer<AssignableExpression> > vars;
        QVector<bool> readNums;
        bool readInt = false;
        QSharedPointer<AssignableExpression> var;
        if(LA(HASH))
        {
            match(HASH);
            readInt = true;
        }
        QSharedPointer<Expression> lvalue = expression();
        var = lvalue.dynamicCast<AssignableExpression>();
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
            var = lvalue.dynamicCast<AssignableExpression>();
            if(var == NULL)
            {
                throw ParserException(getPos(), "Item in read statement must be an assignable expression");
            }
            vars.append(var);
            readNums.append(readInt);
        }
        return QSharedPointer<Statement>(new ReadStmt(readTok, fileObject, prompt, vars, readNums));
    }
    throw ParserException(getPos(), "Expected PRINT or READ");
}

bool KalimatParser::LA_first_grfx_statement()
{
    return LA(DRAW_PIXEL) || LA(DRAW_LINE) || LA(DRAW_RECT) || LA(DRAW_CIRCLE) || LA(DRAW_SPRITE) || LA(ZOOM);
}

QSharedPointer<Statement> KalimatParser::grfxStatement()
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
QSharedPointer<Statement> KalimatParser::drawPixelStmt()
{
    QSharedPointer<Expression> x, y;
    QSharedPointer<Expression> color;
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
    return QSharedPointer<Statement>(new DrawPixelStmt(tok, x,y, color));
}

QSharedPointer<Statement> KalimatParser::drawLineStmt()
{
    QSharedPointer<Expression> x1, y1;
    QSharedPointer<Expression> x2, y2;
    QSharedPointer<Expression> color;

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
    return QSharedPointer<Statement>(new DrawLineStmt(tok, x1, y1, x2, y2, color));
}

QSharedPointer<Statement> KalimatParser::drawRectStmt()
{
    QSharedPointer<Expression> x1, y1;
    QSharedPointer<Expression> x2, y2;
    QSharedPointer<Expression> color;
    QSharedPointer<Expression> filled;

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
    return QSharedPointer<Statement>(new DrawRectStmt(tok, x1, y1, x2, y2, color, filled));
}

QSharedPointer<Statement> KalimatParser::drawCircleStmt()
{
    QSharedPointer<Expression> cx, cy;
    QSharedPointer<Expression> radius;
    QSharedPointer<Expression> color;
    QSharedPointer<Expression> filled;

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
    return QSharedPointer<Statement>(new DrawCircleStmt(tok, cx, cy, radius, color, filled));
}

QSharedPointer<Statement> KalimatParser::drawSpriteStmt()
{
    QSharedPointer<Expression> x, y;
    QSharedPointer<Expression> number;

    Token tok  = lookAhead;
    match(DRAW_SPRITE);
    number = expression();
    match(IN);

    match(LPAREN);
    x = expression();
    match(COMMA);
    y = expression();
    match(RPAREN);

    return QSharedPointer<Statement>(new DrawSpriteStmt(tok, x, y, number));
}

QSharedPointer<Statement> KalimatParser::zoomStmt()
{

    QSharedPointer<Expression> x1, y1;
    QSharedPointer<Expression> x2, y2;

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

    return QSharedPointer<Statement>(new ZoomStmt(tok, x1, y1, x2, y2));
}

QSharedPointer<Statement> KalimatParser::eventHandlerStmt()
{
    EventType type;
    QSharedPointer<Identifier> proc;
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
    return QSharedPointer<Statement>(new EventStatement(tok, type, proc));
}

QSharedPointer<SendStmt> KalimatParser::sendStmt()
{
    bool signal = false;
    QSharedPointer<Expression> value;
    QSharedPointer<Expression> chan;
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
    return QSharedPointer<Statement>(new SendStmt(tok, signal, value, chan));
}

QSharedPointer<ReceiveStmt> KalimatParser::receiveStmt()
{
    bool signal = false;
    QSharedPointer<AssignableExpression> value;
    QSharedPointer<Expression> chan;
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
        QSharedPointer<Expression> expr = primaryExpression();
        value = expr.dynamicCast<AssignableExpression>();
        if(value == NULL)
        {
            throw ParserException(getPos(), "'Receive' must take an assignable expression");
        }
    }
    match(FROM);
    chan = expression();
    return QSharedPointer<Statement>(new ReceiveStmt(tok, signal, value, chan));
}

QSharedPointer<Statement> KalimatParser::selectStmt()
{
    Token tok = lookAhead;
    QVector<QSharedPointer<ChannelCommunicationStmt> > conditions;
    QVector<QSharedPointer<Statement> > actions;
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
    return QSharedPointer<Statement>(new SelectStmt(tok, conditions, actions));
}

QSharedPointer<BlockStmt> KalimatParser::block()
{
    QVector<QSharedPointer<Statement> > stmts;
    Token tok = lookAhead;
    newLines();
    while(LA_first_statement())
    {
        stmts.append(statement());
        if(!eof())
            match(NEWLINE);
        newLines();
    }
    return QSharedPointer<Statement>(new BlockStmt(tok, stmts));
}

QVector<QSharedPointer<Identifier> > KalimatParser::formalParamList()
{
    QVector<QSharedPointer<Identifier> > formals;

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

QSharedPointer<Declaration> KalimatParser::procedureDecl()
{

    match(PROCEDURE);
    Token tok  = lookAhead;
    QSharedPointer<Identifier> procName = identifier();
    QVector<QSharedPointer<Identifier> > formals = formalParamList();

    match(COLON);
    match(NEWLINE);

    QSharedPointer<ProcedureDecl> ret(new ProcedureDecl(tok,
                                                          Token(),
                                                          procName,
                                                          formals,
                                                          QSharedPointer<BlockStmt>(),
                                                          true));

    varContext.push(ret);
    QSharedPointer<BlockStmt> body = block();
    varContext.pop();

    expect(END);
    Token endToken = lookAhead;
    match(END);

    ret->_endingToken = endToken;
    ret->body(body);
    return ret;
}

QSharedPointer<Declaration> KalimatParser::functionDecl()
{
    match(FUNCTION);
    Token tok  = lookAhead;
    QSharedPointer<Identifier> procName = identifier();
    QVector<QSharedPointer<Identifier> > formals = formalParamList();
    match(COLON);
    match(NEWLINE);
    QSharedPointer<FunctionDecl> ret(new FunctionDecl(tok, Token(), procName, formals, QSharedPointer<BlockStmt>(), true));

    varContext.push(ret);
    QSharedPointer<BlockStmt> body = block();
    varContext.pop();

    expect(END);
    Token endToken = lookAhead;
    match(END);

    ret->_endingToken = endToken;
    ret->body(body);
    return ret;
}

void KalimatParser::addPropertySetter(Token pos,
                                       QSharedPointer<Identifier> methodName,
                                       QVector<QSharedPointer<Identifier> > formals,
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
                                      QSharedPointer<Identifier> methodName,
                                      QVector<QSharedPointer<Identifier> > formals,
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

QSharedPointer<Declaration> KalimatParser::classDecl()
{
    QVector<QSharedPointer<Identifier> > fields;
    QMap<QString, MethodInfo> methods;
    QMap<QString, QSharedPointer<TypeExpression> > fieldMarshallAs;

    match(CLASS);
    Token tok  = lookAhead;
    QSharedPointer<Identifier> className = identifier();
    QSharedPointer<Identifier> ancestorName;
    match(COLON);
    match(NEWLINE);
    newLines();
    QVector<QSharedPointer<ClassInternalDecl> > internalDecls;
    QMap<QString, PropInfo> propertyInfo;

    while(LA(HAS)|| LA(RESPONDS) || LA(REPLIES) || LA(BUILT))
    {
        if(LA(HAS))
        {
            QSharedPointer<Has> h(new Has());
            match(HAS);
            QSharedPointer<Identifier> fname = identifier();
            h.data()->add(fname);
            fields.append(fname);
            if(LA(MARSHALLAS))
            {
                match(MARSHALLAS);
                QSharedPointer<TypeExpression> te = typeExpression();
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
                    QSharedPointer<TypeExpression> te = typeExpression();
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

            QSharedPointer<Identifier> methodName = identifier();
            QVector<QSharedPointer<Identifier> > formals = formalParamList();
            QSharedPointer<RespondsTo> rt(new RespondsTo(false));

            QSharedPointer<ConcreteResponseInfo> ri(new ConcreteResponseInfo(methodName));
            for(QVector<QSharedPointer<Identifier> >::const_iterator i=formals.begin(); i!= formals.end(); ++i)
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
                QSharedPointer<Identifier> methodName = identifier();
                QVector<QSharedPointer<Identifier> > formals= formalParamList();
                QSharedPointer<ConcreteResponseInfo> ri(new ConcreteResponseInfo(methodName));
                for(QVector<QSharedPointer<Identifier> >::const_iterator i=formals.begin(); i!= formals.end(); ++i)
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

            QSharedPointer<Identifier> methodName = identifier();
            QVector<QSharedPointer<Identifier> > formals = formalParamList();
            QSharedPointer<RespondsTo> rt(new RespondsTo(true));
            QSharedPointer<ConcreteResponseInfo> ri(new ConcreteResponseInfo(methodName));
            for(QVector<QSharedPointer<Identifier> >::const_iterator i=formals.begin(); i!= formals.end(); ++i)
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
                QSharedPointer<Identifier> methodName = identifier();
                QVector<QSharedPointer<Identifier> > formals= formalParamList();
                QSharedPointer<ConcreteResponseInfo> ri(new ConcreteResponseInfo(methodName));
                for(QVector<QSharedPointer<Identifier> >::const_iterator i=formals.begin(); i!= formals.end(); ++i)
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
    return QSharedPointer<Declaration>(new ClassDecl(tok, ancestorName, className, fields, methods, internalDecls, fieldMarshallAs, true));
}

QSharedPointer<Declaration> KalimatParser::globalDecl()
{
    QSharedPointer<Identifier> var;
    Token tok  = lookAhead;
    var = identifier();
    match(GLOBAL);
    return QSharedPointer<Declaration>(new GlobalDecl(tok, var->name, true));
}

bool KalimatParser::LA_first_method_declaration()
{
    return LA(RESPONSEOF) || LA(REPLYOF);
}

QSharedPointer<Declaration> KalimatParser::methodDecl()
{
    bool isFunctionNotProcedure;
    QSharedPointer<Identifier> className;
    QSharedPointer<Identifier> receiverName;
    QSharedPointer<Identifier> methodName;

    QVector<QSharedPointer<Identifier> > formals;
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
        methodName = QSharedPointer<Identifier>(new Identifier(tok, "%nosuchmethod"));
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

    QSharedPointer<MethodDecl> ret(new MethodDecl(tok, Token(), className, receiverName,
                                     methodName, formals, QSharedPointer<BlockStmt>(),
                                     isFunctionNotProcedure));

    varContext.push(ret);
    QSharedPointer<BlockStmt> body = block();
    varContext.pop();
    ret->body(body);

    expect(END);
    Token endingToken = lookAhead;
    match(END);
    ret->_endingToken = endingToken;
    return ret;

}

QSharedPointer<Declaration> KalimatParser::ffiLibraryDecl()
{
    Token tok = lookAhead;
    match(LIBRARY);
    QSharedPointer<FFILibraryDecl> ffiLib;
    QVector<QSharedPointer<Declaration> > decls;
    if(LA(STR_LITERAL))
    {
        QSharedPointer<StrLiteral>  libName(new StrLiteral(lookAhead, prepareStringLiteral(lookAhead.Lexeme)));
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
        ffiLib = QSharedPointer<FFILibraryDecl>(new FFILibraryDecl(tok, libName->value, decls, true));
        return ffiLib;
    }
    else
    {
        throw ParserException(tok, "Expected string literal");
    }
}

QSharedPointer<Declaration> KalimatParser::rulesDecl()
{
    match(RULES);
    QSharedPointer<Identifier> ruleName = identifier();
    match(COLON);
    match(NEWLINE);
    newLines();
    while(LA(IDENTIFIER))
    {
        QSharedPointer<RuleDecl> rd = ruleDecl();
        newLines();
    }
    match(END);
}

QSharedPointer<RuleDecl> KalimatParser::ruleDecl()
{
    QVector<QSharedPointer<RuleOption> > options;
    QSharedPointer<Identifier> name = identifier();

    match(EQ);
    QSharedPointer<PegExpr> expr = pegExpr();
    QSharedPointer<Expression> resultExpr;
    if(LA(ROCKET))
    {
        match(ROCKET);
        resultExpr = expression();
    }
    match(NEWLINE);
    newLines();
    options.append(QSharedPointer<RuleOption>(new RuleOption(expr, resultExpr)));
    while(LA(OR))
    {
        expr = pegExpr();
        if(LA(ROCKET))
        {
            match(ROCKET);
            resultExpr = expression();
        }
        options.append(QSharedPointer<RuleOption>(new RuleOption(expr, resultExpr)));
        match(NEWLINE);
        newLines();
    }
    return QSharedPointer<RuleDecl>(new RuleDecl(name->name, options));
}

QSharedPointer<PegExpr> KalimatParser::pegExpr()
{
    identifier();
}

QSharedPointer<FFIProceduralDecl> KalimatParser::ffiFunctionDecl()
{
    QVector<QSharedPointer<TypeExpression> > argTypes;
    QSharedPointer<TypeExpression> retType;
    QString symbol;
    Token pos = lookAhead;
    match(FUNCTION);

    QSharedPointer<Identifier> fname = identifier();
    symbol = fname->name;
    if(LA(SYMBOL))
    {
        match(SYMBOL);
        if(LA(STR_LITERAL))
        {
            QSharedPointer<StrLiteral> s(new StrLiteral(lookAhead, prepareStringLiteral(lookAhead.Lexeme)));
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
    return QSharedPointer<FFIProceduralDecl>(new FFIProceduralDecl(pos, true, retType, argTypes, fname->name, symbol, true));
}

QSharedPointer<FFIProceduralDecl> KalimatParser::ffiProcDecl()
{
    QVector<QSharedPointer<TypeExpression> > argTypes;
    QSharedPointer<TypeExpression> retType;
    QString symbol;
    Token pos = lookAhead;
    match(PROCEDURE);

    QSharedPointer<Identifier> fname = identifier();
    symbol = fname->name;
    if(LA(SYMBOL))
    {
        match(SYMBOL);
        if(LA(STR_LITERAL))
        {
            QSharedPointer<StrLiteral> s(new StrLiteral(lookAhead, lookAhead.Lexeme));
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
    return QSharedPointer<FFIProceduralDecl>(new FFIProceduralDecl(pos, false, retType, argTypes, fname->name, symbol, true));
}

bool KalimatParser::LA_first_expression()
{
    return LA_first_primary_expression()
            || LA(NOT) || LA(ADD_OP) || LA(SUB_OP);
}

QSharedPointer<Expression> KalimatParser::expression()
{
    return andOrExpression();
}

QSharedPointer<Expression> KalimatParser::andOrExpression()
{
    QSharedPointer<Expression> t = notExpression();
    while(LA(AND) || LA(OR))
    {
        QString operation = getOperation(lookAhead);
        Token tok  = lookAhead;
        match(lookAhead.Type);
        QSharedPointer<Expression> t2 = notExpression();
        t = QSharedPointer<Expression>(new BinaryOperation(tok, operation, t, t2));
    }
    return t;
}

QSharedPointer<Expression> KalimatParser::notExpression()
{
    bool _not = false;
    Token tok  = lookAhead;
    if(LA(NOT))
    {
        match(NOT);
        _not = true;
    }
    QSharedPointer<Expression> t = comparisonExpression();
    if(_not)
    {
        t = QSharedPointer<Expression>(new UnaryOperation(tok, "not", t));
        while(LA(ANDNOT))
        {
            tok = lookAhead;
            match(ANDNOT);
            QSharedPointer<Expression> t2 = comparisonExpression();
            t = QSharedPointer<Expression>(new BinaryOperation(tok,
                                                               "and",
                                                               t,
                                                               QSharedPointer<Expression>(new UnaryOperation(tok, "not", t2))));
        }
    }
    return t;
}

QSharedPointer<Expression> KalimatParser::comparisonExpression()
{
    QSharedPointer<Expression> t = arithmeticExpression();

    while(true)
    {
        if(LA(LT) || LA(GT) || LA(EQ) || LA(NE) || LA(LE) || LA(GE))
        {
            Token tok  = lookAhead;
            QString operation = getOperation(lookAhead);
            match(lookAhead.Type);
            QSharedPointer<Expression> t2 = arithmeticExpression();
            t = QSharedPointer<Expression>(new BinaryOperation(tok, operation, t, t2));
        }
        else if(LA(IS))
        {
            Token tok  = lookAhead;
            match(lookAhead.Type);
            QSharedPointer<Identifier> t2 = identifier();
            t = QSharedPointer<Expression>(new IsaOperation(tok, t, t2));
        }
        else if(LA(MATCHES))
        {
            Token tok  = lookAhead;
            match(lookAhead.Type);
            QSharedPointer<Pattern> t2 = pattern();
            t = QSharedPointer<Expression>(new MatchOperation(tok, t, t2));
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

QSharedPointer<Pattern> KalimatParser::pattern()
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

QSharedPointer<Pattern> KalimatParser::simpleLiteralPattern()
{
    QSharedPointer<SimpleLiteral> sl = simpleLiteral();
    return QSharedPointer<Pattern>(new SimpleLiteralPattern(sl->getPos(), sl));
}

QSharedPointer<Pattern> KalimatParser::assignedVarPattern()
{
    match(QUESTION);
    QSharedPointer<Expression> expr = expression();

    QSharedPointer<AssignableExpression> id = expr.dynamicCast<AssignableExpression>();
    if(id == NULL)
    {
        throw ParserException(getPos(), "After ? must be an assignable expression");
    }
    return QSharedPointer<Pattern>(new AssignedVarPattern(id->getPos(), id));
}

QSharedPointer<Pattern> KalimatParser::varOrObjPattern()
{
    bool mustBeObj = false;
    QVector<QSharedPointer<Identifier> > fnames;
    QVector<QSharedPointer<Pattern> > fpatterns;

    if(LA(ANYOF))
    {
        match(ANYOF);
        mustBeObj = true;
    }

    QSharedPointer<Identifier> id = identifier();
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
        return QSharedPointer<Pattern>(new ObjPattern(id->getPos(), id, fnames, fpatterns));
    }
    else
    {
        return QSharedPointer<Pattern>(new VarPattern(id->getPos(), id));
    }
}

QSharedPointer<Pattern> KalimatParser::arrayPattern()
{
    bool fixedLen = true;
    QVector<QSharedPointer<Pattern> > patterns;
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
    return QSharedPointer<Pattern>(new ArrayPattern(pos, patterns));
}

QSharedPointer<Pattern> KalimatParser::mapPattern()
{
    QVector<QSharedPointer<Expression> > keys;
    QVector<QSharedPointer<Pattern> > patterns;
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
    return QSharedPointer<Pattern>(new MapPattern(pos, keys, patterns));
}

QSharedPointer<Expression> KalimatParser::arithmeticExpression()
{
    QSharedPointer<Expression> t = multiplicativeExpression();

    while(LA(ADD_OP) || LA(SUB_OP))
    {
        Token tok  = lookAhead;
        QString operation = getOperation(lookAhead);
        match(lookAhead.Type);
        QSharedPointer<Expression> t2 = multiplicativeExpression();
        t = QSharedPointer<Expression>(new BinaryOperation(tok, operation, t, t2));
    }
    return t;
}

QSharedPointer<Expression> KalimatParser::multiplicativeExpression()
{
    QSharedPointer<Expression> t = positiveOrNegativeExpression();
    while(LA(MUL_OP) || LA(DIV_OP))
    {
        Token tok  = lookAhead;
        QString operation = getOperation(lookAhead);
        match(lookAhead.Type);
        QSharedPointer<Expression> t2 = positiveOrNegativeExpression();
        t = QSharedPointer<Expression>(new BinaryOperation(tok, operation, t, t2));
    }
    return t;
}

QSharedPointer<Expression> KalimatParser::positiveOrNegativeExpression()
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

    QSharedPointer<Expression> t = primaryExpression();
    if(pos)
        t = QSharedPointer<Expression>(new UnaryOperation(tok, "pos", t));
    else if(neg)
        t = QSharedPointer<Expression>(new UnaryOperation(tok, "neg", t));

    return t;
}

QSharedPointer<Expression> KalimatParser::primaryExpression()
{
    QSharedPointer<Expression> ret = primaryExpressionNonInvokation();
    Token tok;
    while(LA(LPAREN) || LA(COLON) || LA(LBRACKET))
    {
        if(LA(LPAREN))
        {
            QVector<QSharedPointer<Expression> > args;
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
            ret = QSharedPointer<Expression>(new Invokation(tok, ret, args));
        }
        if(LA(COLON))
        {
            ParserState s = saveState();
            try
            {
                match(COLON);
                tok = lookAhead;
                QSharedPointer<Identifier> methodName = identifier();
                QVector<QSharedPointer<Expression> > args;
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
                ret = QSharedPointer<Expression>(new MethodInvokation(tok, ret, methodName, args));
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
            QSharedPointer<Expression> index = expression();
            if(LA(COMMA))
            {
                multiDim = true;
                QVector<QSharedPointer<Expression> > indexes;
                indexes.append(index);

                match(COMMA);
                indexes.append(expression());

                while(LA(COMMA))
                {
                    match(COMMA);
                    indexes.append(expression());
                }
                ret = QSharedPointer<Expression>(new MultiDimensionalArrayIndex(tok, ret, indexes));

            }
            match(RBRACKET);
            if(!multiDim)
            {
                ret = QSharedPointer<Exprrssion>(new ArrayIndex(tok, ret, index));
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
            LA(LPAREN);
                //todo: decide if we still want to keep the field accessor field(obj, id)
                //||  LA(FIELD);
}

QSharedPointer<SimpleLiteral> KalimatParser::simpleLiteral()
{
    QSharedPointer<SimpleLiteral> ret;
    if(LA(NUM_LITERAL))
    {
        ret = QSharedPointer<SimpleLiteral>(new NumLiteral(lookAhead, lookAhead.Lexeme));
        match(NUM_LITERAL); // will throw if wrong lookAhead
    }
    else if(LA(STR_LITERAL))
    {
        ret = QSharedPointer<SimpleLiteral>(new StrLiteral(lookAhead, prepareStringLiteral(lookAhead.Lexeme)));
        match(STR_LITERAL); // will throw if wrong lookAhead
    }
    else if(LA(NOTHING))
    {
        ret = QSharedPointer<SimpleLiteral>(new NullLiteral(lookAhead));
        match(NOTHING);
    }
    else if(LA(C_TRUE))
    {
        ret = QSharedPointer<SimpleLiteral>(new BoolLiteral(lookAhead, true));
        match(lookAhead.Type);
    }
    else if(LA(C_FALSE))
    {
        ret = QSharedPointer<SimpleLiteral>(new BoolLiteral(lookAhead, false));
        match(lookAhead.Type);
    }
    else
    {
        throw ParserException(getPos(), "Expected a simple literal");
    }
    return ret;
}

QSharedPointer<Expression> KalimatParser::primaryExpressionNonInvokation()
{
    Expression *ret = NULL;
    if(LA_first_simple_literal())
    {
        return simpleLiteral();
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
