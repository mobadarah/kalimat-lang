/**************************************************************************
** The Kalimat programming language
**   Copyright (C) 2010 Mohamed Samy Ali - samy2004@gmail.com
**   This project is released under the Apache License version 2.0
**   as described in the included license.txt file
**************************************************************************/

#ifndef KALIMATPARSER_H
#define KALIMATPARSER_H

#include "KalimatParserError.h"
class KalimatParser : public Parser
{
    struct PropInfo
    {
        bool read, write;
        PropInfo() { read = write = false;}
    };
public:
    KalimatParser();
    ~KalimatParser();
    void init(QString s, Lexer *lxr, void *tag);
    void init(QString s, Lexer *lxr, void *tag, QString fileName);

    shared_ptr<AST> parseRoot();
    shared_ptr<AST> module();

    QVector<Token> getTokens() { return tokens; }
    QMap<int, Token> closerFor;
    QMap<int, Token> openerFor;

private:
    QStack<shared_ptr<ProceduralDecl> > varContext;
private:

    bool LA_first_statement();
    bool LA_first_declaration();
    bool LA_first_io_statement();
    bool LA_first_grfx_statement();
    bool LA_first_simple_literal();
    bool LA_first_primary_expression();
    bool LA_first_method_declaration();
    bool LA_first_assignment_or_invokation();
    bool LA_first_typeExpression();
    bool LA_first_primary_peg_expression();

    shared_ptr<AST> program();

    shared_ptr<Statement> statement();
    shared_ptr<Declaration> declaration();

    shared_ptr<Statement> assignmentStmt_or_Invokation(ParserState);
    shared_ptr<Statement> ifStmt();
    shared_ptr<Statement> forEachStmt();
    shared_ptr<Statement> whileStmt();
    shared_ptr<Statement> returnStmt();
    shared_ptr<Statement> delegateStmt();
    shared_ptr<Statement> launchStmt();
    shared_ptr<Statement> labelStmt();
    shared_ptr<Statement> gotoStmt();

    shared_ptr<Statement> ioStmt();
    shared_ptr<Statement> grfxStatement();

    shared_ptr<Statement> drawPixelStmt();
    shared_ptr<Statement> drawLineStmt();
    shared_ptr<Statement> drawRectStmt();
    shared_ptr<Statement> drawCircleStmt();
    shared_ptr<Statement> drawImageStmt();
    shared_ptr<Statement> drawSpriteStmt();

    shared_ptr<Statement> zoomStmt();
    shared_ptr<Statement> eventHandlerStmt();

    shared_ptr<SendStmt> sendStmt();
    shared_ptr<ReceiveStmt> receiveStmt();
    shared_ptr<Statement> selectStmt();

    QVector<shared_ptr<FormalParam> > formalParamList();
    QVector<shared_ptr<FormalParam> > commaSeparatedFormalParams();
    shared_ptr<Declaration> procedureDecl();
    shared_ptr<Declaration> functionDecl();
    shared_ptr<Declaration> classDecl();
    shared_ptr<Declaration> globalDecl();
    shared_ptr<Declaration> methodDecl();
    shared_ptr<Declaration> ffiLibraryDecl();
    shared_ptr<Declaration> rulesDecl();
    shared_ptr<RuleDecl> ruleDecl();
    shared_ptr<PegExpr> pegExpr();
    shared_ptr<PegExpr> primaryPegExpression();

    shared_ptr<FFIProceduralDecl> ffiFunctionDecl();
    shared_ptr<FFIProceduralDecl> ffiProcDecl();

    // blockTerminator is needed for recovery
    shared_ptr<BlockStmt> block(QSet<TokenType> blockTerminators);

    bool LA_first_expression();
    shared_ptr<Expression> expression();

    shared_ptr<Expression> andOrExpression();
    shared_ptr<Expression> notExpression();
    shared_ptr<Expression> comparisonExpression();

    bool LA_first_pattern();
    shared_ptr<Pattern> pattern();
    shared_ptr<Pattern> simpleLiteralPattern();
    shared_ptr<Pattern> varOrObjPattern();
    shared_ptr<Pattern> assignedVarPattern();
    shared_ptr<Pattern> arrayPattern();
    shared_ptr<Pattern> mapPattern();

    shared_ptr<Expression> arithmeticExpression();
    shared_ptr<Expression> multiplicativeExpression();
    shared_ptr<Expression> positiveOrNegativeExpression();
    shared_ptr<Expression> primaryExpression();
    shared_ptr<Expression> primaryExpressionNonInvokation();
    shared_ptr<SimpleLiteral> simpleLiteral();
    shared_ptr<Identifier> identifier();

    shared_ptr<TypeExpression> typeExpression();

    QVector<shared_ptr<Expression> > comma_separated_expressions();
    QVector<shared_ptr<Expression> > comma_separated_pairs();
    QVector<shared_ptr<StrLiteral> > usingDirectives();

    QString prepareStringLiteral(QString str);
    QString getOperation(Token token);
    void addPropertySetter(Token pos,
                           shared_ptr<Identifier> methodName,
                           QVector<shared_ptr<FormalParam> > formals,
                           QMap<QString, PropInfo> &propertyInfo);
    void addPropertyGetter(Token pos,
                           shared_ptr<Identifier> methodName,
                           QVector<shared_ptr<FormalParam> > formals,
                                          QMap<QString, PropInfo> &propertyInfo);
    void newLines();
};

#endif // KALIMATPARSER_H
