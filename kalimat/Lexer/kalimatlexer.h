/**************************************************************************
** The Kalimat programming language
**   Copyright (C) 2010 Mohamed Samy Ali - samy2004@gmail.com
**   This project is released under the Apache License version 2.0
**   as described in the included license.txt file
**************************************************************************/

#ifndef KALIMATLEXER_H
#define KALIMATLEXER_H

const int FORALL = 1;
const int FROM = 2;
const int TO = 3;
const int WHILE = 4;
const int CONTINUE = 5;
const int FUNCTION = 6;
const int PROCEDURE = 7;
const int RETURN = 8;
const int WITH = 9;
const int IF = 10;
const int ELSE = 11;
const int END = 12;
const int DONE = 13;
const int STOP = 14;
const int C_TRUE = 15;
const int C_FALSE = 16;
const int AND = 17;
const int OR = 18;
const int NOT = 19;
const int ANDNOT = 20;
const int WHEN = 21;
const int EVENT = 22;
const int DO = 23;
const int NEW = 24;
const int TYPE = 25;
const int HAS = 26;
const int RESPONDS = 27;
const int REPLIES = 28;
const int BUILT = 29;
const int ON = 30;
const int RESPONSEOF = 31;
const int UPON = 32;
const int REPLYOF = 33;
const int CALL_PREV_RESPONSE = 34;
const int READ = 35;
const int PRINT = 36;
const int ZOOM = 37;
const int DRAW_PIXEL = 38;
const int DRAW_LINE = 39;
const int DRAW_RECT = 40;
const int DRAW_CIRCLE = 41;
const int DRAW_SPRITE = 42;
const int IN = 43;
const int CLASS = 44;
const int MOUSE = 45;
const int KB = 46;
const int COLLISION = 47;
const int GLOBAL = 48;
const int FIELD = 49;
const int USING = 50;
const int UNIT = 51;
const int USING_WIDTH = 52;
const int NOTHING = 53;
const int LABEL = 54;
const int GO = 55;

const int KEYWORD_CUTOFF = 60; // As long as the ID of the token is <= KEYWORD_CUTOFF, the
                               // syntax hilighter will consider it a keyword.
const int NUM_LITERAL = 61;
const int ADD_OP = 62;
const int SUB_OP = 63;
const int MUL_OP = 64;
const int DIV_OP = 65;
const int EQ = 66;
const int LT = 67;
const int GT = 68;
const int LE = 69;
const int GE = 70;
const int NE = 71;


const int LPAREN = 72;
const int RPAREN = 73;
const int IDENTIFIER = 74;
const int COMMENT = 75;
const int NEWLINE = 76;
const int COMMA = 77;
const int SEMI = 78;
const int STR_LITERAL = 79;
const int COLON = 80;
const int KASRA = 81;
const int DOLLAR = 82;
const int LBRACKET = 83;
const int RBRACKET = 84;
const int HASH = 85;
const int ELLIPSIS = 86;

const int WHITESPACE = TokenNone;

QString TokenNameFromId(int id);

class KalimatLexer : public Lexer
{
    CharPredicate *digit, *arabicDigit, *europeanDigit;
    CharPredicate *arabDecimalSeparator, *europeanDecimalSeparator;
    CharPredicate *letter;
    CharPredicate *idSymbol;

    CharPredicate *plus, *minus;
    CharPredicate *mul, *div;

    CharPredicate *lparen, *rparen, *lbracket, *rbracket;
    CharPredicate *dollarSign;

    CharPredicate *spacer, *tab, *space;
    Predicate *lineComment;

    CharPredicate *addOp, *subOp, *mulOp, *divOp, *eq, *lt, *gt;

    Predicate *le,  *ge, *ne;

    CharPredicate *comma, *semi, *colon, *kasra, *hash;
    Predicate *ellipsis;
    CharPredicate *quote;
    Predicate *two_quotes;

    CharPredicate *anyChar, *nl, *noneNl;
    Predicate *sol, *eof;

    Action *retract;

    QMap<QString, int> keywords;
    QMap<QString, QString> arab_chars;

    void InitCharPredicates();
    void InitKeywords();
    void InitArabChars();
    Token accept(TokenType);
public:
    KalimatLexer();
    static QMap<int, QString> tokenNameMap;
};

class ColonUnsupportedInIdentifiersException
{

};

#endif // KALIMATLEXER_H
