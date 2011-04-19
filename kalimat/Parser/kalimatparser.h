/**************************************************************************
** The Kalimat programming language
**   Copyright (C) 2010 Mohamed Samy Ali - samy2004@gmail.com
**   This project is released under the Apache License version 2.0
**   as described in the included license.txt file
**************************************************************************/

#ifndef KALIMATPARSER_H
#define KALIMATPARSER_H

class KalimatParser : public Parser
{
public:
    KalimatParser();
    ~KalimatParser();
    void init(QString s, Lexer *lxr, void *tag);
    void init(QString s, Lexer *lxr, void *tag, QString fileName);

    AST *parseRoot();
    AST *module();

    QMap<int, Token> closerFor;
    QMap<int, Token> openerFor;
private:
    QStack<ProceduralDecl *> varContext;
private:

    bool LA_first_statement();
    bool LA_first_declaration();
    bool LA_first_io_statement();
    bool LA_first_grfx_statement();
    bool LA_first_primary_expression();
    bool LA_first_method_declaration();
    bool LA_first_assignment_or_invokation();

    AST *program();

    Statement *statement();
    Declaration *declaration();

    Statement *assignmentStmt_or_Invokation(ParserState);
    Statement *ifStmt();
    Statement *forEachStmt();
    Statement *whileStmt();
    Statement *returnStmt();
    Statement *delegateStmt();
    Statement *launchStmt();
    Statement *labelStmt();
    Statement *gotoStmt();

    Statement *ioStmt();
    Statement *grfxStatement();

    Statement *drawPixelStmt();
    Statement *drawLineStmt();
    Statement *drawRectStmt();
    Statement *drawCircleStmt();
    Statement *drawSpriteStmt();

    Statement *zoomStmt();
    Statement *eventHandlerStmt();

    SendStmt *sendStmt();
    ReceiveStmt *receiveStmt();
    Statement *selectStmt();

    QVector<Identifier *> formalParamList();
    Declaration *procedureDecl();
    Declaration *functionDecl();
    Declaration *classDecl();
    Declaration *globalDecl();
    Declaration *methodDecl();

    BlockStmt *block();

    bool LA_first_expression();
    Expression *expression();

    Expression *andOrExpression();
    Expression *notExpression();
    Expression *comparisonExpression();
    Expression *arithmeticExpression();
    Expression *multiplicativeExpression();
    Expression *positiveOrNegativeExpression();
    Expression *primaryExpression();
    Expression *primaryExpressionNonInvokation();
    Identifier *identifier();

    QVector<Expression *> comma_separated_expressions();
    QVector<StrLiteral *> usingDirectives();

    QString prepareStringLiteral(QString str);
    QString getOperation(Token token);
    void newLines();
};

#endif // KALIMATPARSER_H
