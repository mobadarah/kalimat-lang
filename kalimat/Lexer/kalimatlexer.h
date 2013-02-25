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
const int RETURN_WITH = 8;
const int IF = 9;
const int ELSE = 10;
const int END = 11;
const int DONE = 12;
const int STOP = 13;
const int C_TRUE = 14;
const int C_FALSE = 15;
const int AND = 16;
const int OR = 17;
const int NOT = 18;
const int ANDNOT = 19;
const int WHEN = 20;
const int EVENT = 21;
const int DO = 22;
const int NEW = 23;
const int TYPE = 24;
const int HAS = 25;
const int RESPONDS = 26;
const int REPLIES = 27;
const int BUILT = 28;
const int ON = 29;
const int RESPONSEOF = 30;
const int UPON = 31;
const int REPLYOF = 32;
const int CALL_PREV_RESPONSE = 33;
const int READ = 34;
const int PRINT = 35;
const int ZOOM = 36;
const int DRAW_PIXEL = 37;
const int DRAW_LINE = 38;
const int DRAW_RECT = 39;
const int DRAW_CIRCLE = 40;
const int DRAW_IMAGE = 41;
const int DRAW_SPRITE = 42;
const int IN = 43;
const int CLASS = 44;
const int MOUSE_BUTTON_DOWN = 45;
const int MOUSE_BUTTON_UP = 46;
const int MOUSE_MOVE = 47;
const int KEY_DOWN = 48;
const int KEY_UP = 49;
const int KEY_PRESS = 50;
const int COLLISION = 51;
const int GLOBAL = 52;
const int PROPERTY = 53;
const int USING = 54;
const int UNIT = 55;
const int USING_WIDTH = 56;
const int NOTHING = 57;
const int LABEL = 58;
const int GO = 59;
const int DELEGATE = 60;
const int IS = 61;
const int LAUNCH = 62;
const int SEND = 63;
const int RECEIVE = 64;
const int SELECT = 65;
const int SIGNAL_ = 66;
const int MESSAGE = 67;
const int OTHER = 68;
const int LIBRARY = 69;
const int SYMBOL = 70;
const int MARSHALLAS = 71;
const int POINTER = 72;
const int ANYOF = 73;
const int RULES = 74;
const int TIMING = 75;
const int REPETITION = 76;
const int STEP = 77;
const int DOWNTO = 78;
const int REPEAT = 79;
const int THEPROC = 80;
const int THEFUNCTION = 81;
const int OF = 82;
const int KEYWORD_CUTOFF = 82; // As long as the ID of the token is <= KEYWORD_CUTOFF, the
                               // syntax highlighter will consider it a keyword.
const int NUM_LITERAL = 91;
const int ADD_OP = 92;
const int SUB_OP = 93;
const int MUL_OP = 94;
const int DIV_OP = 95;
const int EQ = 96;
const int LT = 97;
const int GT = 98;
const int LE = 99;
const int GE = 100;
const int NE = 101;


const int LPAREN = 102;
const int RPAREN = 103;
const int IDENTIFIER = 104;
const int COMMENT = 105;
const int NEWLINE = 106;
const int COMMA = 107;
const int SEMI = 108;
const int STR_LITERAL = 109;
const int COLON = 110;
const int KASRA = 111;
const int DOLLAR = 112;
const int LBRACKET = 113;
const int RBRACKET = 114;
const int HASH = 115;
const int ELLIPSIS = 116;
const int LBRACE = 117;
const int RBRACE = 118;
const int ROCKET = 119;
const int MATCHES = 120;
const int QUESTION = 121;
const int LAMBDA = 122;
const int NAMESEP = 123;

const int WHITESPACE = TokenNone;

QString TokenNameFromId(int id);
QString TokenLexemeFromId(int id);

class KalimatLexer : public Lexer
{
    CharPredicate *digit, *arabicDigit, *europeanDigit;
    CharPredicate *arabDecimalSeparator, *europeanDecimalSeparator;
    CharPredicate *letter;
    CharPredicate *idSymbol;

    CharPredicate *plus, *minus;
    CharPredicate *mul, *div;

    CharPredicate *lparen, *rparen, *lbracket, *rbracket, *lbrace, *rbrace;
    CharPredicate *dollarSign;
    Predicate *rocket;
    Predicate *matches, *question;

    CharPredicate *spacer, *tab, *space;
    Predicate *lineComment;

    CharPredicate *addOp, *subOp, *mulOp, *divOp, *eq, *lt, *gt;

    Predicate *le,  *ge, *ne;
    Predicate *returnWith;

    CharPredicate *comma, *semi, *colon, *kasra, *hash;

    Predicate *ellipsis;
    Predicate *lambda;
    Predicate *nameSep;
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
    static QMap<int, QString> keywordLexemes;
};

class ColonUnsupportedInIdentifiersException
{

};

#endif // KALIMATLEXER_H
