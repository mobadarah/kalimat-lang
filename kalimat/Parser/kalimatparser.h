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

    QSharedPointer<AST> parseRoot();
    QSharedPointer<AST> module();

    QMap<int, Token> closerFor;
    QMap<int, Token> openerFor;
private:
    QStack<QSharedPointer<ProceduralDecl> > varContext;
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

    QSharedPointer<AST> program();

    QSharedPointer<Statement> statement();
    QSharedPointer<Declaration> declaration();

    QSharedPointer<Statement> assignmentStmt_or_Invokation(ParserState);
    QSharedPointer<Statement> ifStmt();
    QSharedPointer<Statement> forEachStmt();
    QSharedPointer<Statement> whileStmt();
    QSharedPointer<Statement> returnStmt();
    QSharedPointer<Statement> delegateStmt();
    QSharedPointer<Statement> launchStmt();
    QSharedPointer<Statement> labelStmt();
    QSharedPointer<Statement> gotoStmt();

    QSharedPointer<Statement> ioStmt();
    QSharedPointer<Statement> grfxStatement();

    QSharedPointer<Statement> drawPixelStmt();
    QSharedPointer<Statement> drawLineStmt();
    QSharedPointer<Statement> drawRectStmt();
    QSharedPointer<Statement> drawCircleStmt();
    QSharedPointer<Statement> drawSpriteStmt();

    QSharedPointer<Statement> zoomStmt();
    QSharedPointer<Statement> eventHandlerStmt();

    QSharedPointer<SendStmt> sendStmt();
    QSharedPointer<ReceiveStmt> receiveStmt();
    QSharedPointer<Statement> selectStmt();

    QVector<QSharedPointer<Identifier> > formalParamList();
    QSharedPointer<Declaration> procedureDecl();
    QSharedPointer<Declaration> functionDecl();
    QSharedPointer<Declaration> classDecl();
    QSharedPointer<Declaration> globalDecl();
    QSharedPointer<Declaration> methodDecl();
    QSharedPointer<Declaration> ffiLibraryDecl();
    QSharedPointer<Declaration> rulesDecl();
    QSharedPointer<RuleDecl> ruleDecl();
    QSharedPointer<PegExpr> pegExpr();

    QSharedPointer<FFIProceduralDecl> ffiFunctionDecl();
    QSharedPointer<FFIProceduralDecl> ffiProcDecl();

    QSharedPointer<BlockStmt> block();

    bool LA_first_expression();
    QSharedPointer<Expression> expression();

    QSharedPointer<Expression> andOrExpression();
    QSharedPointer<Expression> notExpression();
    QSharedPointer<Expression> comparisonExpression();

    bool LA_first_pattern();
    QSharedPointer<Pattern> pattern();
    QSharedPointer<Pattern> simpleLiteralPattern();
    QSharedPointer<Pattern> varOrObjPattern();
    QSharedPointer<Pattern> assignedVarPattern();
    QSharedPointer<Pattern> arrayPattern();
    QSharedPointer<Pattern> mapPattern();

    QSharedPointer<Expression> arithmeticExpression();
    QSharedPointer<Expression> multiplicativeExpression();
    QSharedPointer<Expression> positiveOrNegativeExpression();
    QSharedPointer<Expression> primaryExpression();
    QSharedPointer<Expression> primaryExpressionNonInvokation();
    QSharedPointer<SimpleLiteral> simpleLiteral();
    QSharedPointer<Identifier> identifier();

    QSharedPointer<TypeExpression> typeExpression();

    QVector<QSharedPointer<Expression> > comma_separated_expressions();
    QSharedPointer<QVector<Expression> > comma_separated_pairs();
    QVector<QSharedPointer<StrLiteral> > usingDirectives();

    QString prepareStringLiteral(QString str);
    QString getOperation(Token token);
    void addPropertySetter(Token pos,
                           QSharedPointer<Identifier> methodName,
                           QVector<QSharedPointer<Identifier> > formals,
                           QMap<QString, PropInfo> &propertyInfo);
    void addPropertyGetter(Token pos,
                           QSharedPointer<Identifier> methodName,
                           QVector<QSharedPointer<Identifier> > formals,
                                          QMap<QString, PropInfo> &propertyInfo);
    void newLines();
};

#endif // KALIMATPARSER_H
