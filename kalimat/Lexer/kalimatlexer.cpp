/**************************************************************************
**   The Kalimat programming language
**   Copyright (C) 2010 Mohamed Samy Ali - samy2004@gmail.com
**   This project is released under the Apache License version 2.0
**   as described in the included license.txt file
**************************************************************************/

#include "lexer_incl.h"
#include "kalimatlexer.h"
#include "../utils.h"
#include <QString>

QMap<int, QString> KalimatLexer::tokenNameMap;

KalimatLexer::KalimatLexer() : Lexer()
{

    InitArabChars();
    InitCharPredicates();
    InitKeywords();

    const int AR_NUM = 1;
    const int EU_NUM = 2;
    const int AR_NUM_DECIMAL  = 3;
    const int EU_NUM_DECIMAL  = 4;
    const int WS  = 5;
    const int ID  = 6;
    const int STR = 7;

    // Just making a shorter name for the member variable 'stateMachine'
    StateMachine &sm = stateMachine;
    sm.add          (READY,   la(arabicDigit),  AR_NUM);
    sm.add          (READY,   la(europeanDigit),  EU_NUM);
    sm.addAccepting (READY,   returnWith,       READY, RETURN_WITH);
    sm.add          (READY,   la(letter), ID);
    sm.add          (READY,   la(quote),  STR);
    sm.addAccepting (READY,   seq(     sol,
                                       loop(la(space)),
                                       lineComment,
                                       loop(la(noneNl)),
                                       choice(la(nl), eof)),
                                                READY, COMMENT);


    sm.addAccepting (READY,   la(addOp),  READY,   ADD_OP);
    sm.addAccepting (READY,   la(subOp),  READY,   SUB_OP);
    sm.addAccepting (READY,   la(mulOp),  READY,   MUL_OP);
    sm.addAccepting (READY,   la(divOp),  READY,   DIV_OP);

    sm.addAccepting (READY,   le,         READY,   LE); // longer sequences must come first!
    sm.addAccepting (READY,   ge,         READY,   GE);
    sm.addAccepting (READY,   ne,         READY,   NE);

    sm.addAccepting (READY,   la(eq),     READY,   EQ);
    sm.addAccepting (READY,   la(lt),     READY,   LT);
    sm.addAccepting (READY,   la(gt),     READY,   GT);

    sm.addAccepting (READY,   ellipsis,  READY,   ELLIPSIS);
    sm.addAccepting (READY,   la(lparen), READY,   LPAREN);
    sm.addAccepting (READY,   la(rparen), READY,   RPAREN);
    sm.addAccepting (READY,   la(lbracket), READY,   LBRACKET);
    sm.addAccepting (READY,   la(rbracket), READY,   RBRACKET);
    sm.addAccepting (READY,   la(comma),  READY,   COMMA);
    sm.addAccepting (READY,   la(semi),   READY,   SEMI);
    sm.addAccepting (READY,   la(colon),  READY,   COLON);
    sm.addAccepting (READY,   la(kasra),  READY,   KASRA);
    sm.addAccepting (READY,   la(dollarSign),  READY,   DOLLAR);
    sm.addAccepting (READY,   la(hash),  READY,   HASH);


    sm.addAccepting (READY,   la(nl),  READY, NEWLINE);
    sm.add          (READY,   la(space),  WS);
    // consume the EOF, as the lexer won't stop until one cycle after the EOF
    sm.add          (READY,   eof,        READY);

    sm.add          (AR_NUM,  la(arabicDigit),          AR_NUM);
    sm.add          (AR_NUM,  la(arabDecimalSeparator), AR_NUM_DECIMAL);
    sm.addAccepting (AR_NUM,  la(anyChar),              READY,   NUM_LITERAL, retract);
    sm.addAccepting (AR_NUM,  eof,                      READY, NUM_LITERAL);

    sm.add          (AR_NUM_DECIMAL,  la(arabicDigit),          AR_NUM_DECIMAL);
    sm.addAccepting (AR_NUM_DECIMAL,  la(anyChar),              READY, NUM_LITERAL, retract);
    sm.addAccepting (AR_NUM_DECIMAL,  eof,                      READY, NUM_LITERAL);

    sm.add          (EU_NUM,  la(europeanDigit),            EU_NUM);
    sm.add          (EU_NUM,  la(europeanDecimalSeparator), EU_NUM_DECIMAL);
    sm.addAccepting (EU_NUM,  la(anyChar),                  READY, NUM_LITERAL, retract);
    sm.addAccepting (EU_NUM,  eof,                          READY, NUM_LITERAL);

    sm.add          (EU_NUM_DECIMAL,  la(europeanDigit),    EU_NUM_DECIMAL);
    sm.addAccepting (EU_NUM_DECIMAL,  la(anyChar),          READY, NUM_LITERAL, retract);
    sm.addAccepting (EU_NUM_DECIMAL,  eof,                  READY, NUM_LITERAL);


    sm.add          (ID,      la(digit),    ID);
    sm.add          (ID,      la(letter),   ID);
    sm.add          (ID,      la(idSymbol), ID);
    sm.addAccepting (ID,      la(colon),   READY, IDENTIFIER);
    sm.addAccepting (ID,      la(anyChar), READY, IDENTIFIER, retract);
    sm.addAccepting (ID,      eof,         READY, IDENTIFIER);


    sm.add          (STR,     two_quotes,   STR);
    sm.addAccepting (STR,     la(quote),    READY, STR_LITERAL);
    sm.add          (STR,     la(noneNl),   STR);

    sm.add          (WS,      la(space),  WS);
    sm.addAccepting (WS,      eof,        READY, WHITESPACE);
    sm.addAccepting (WS,      la(anyChar),READY, WHITESPACE, retract);
}

void KalimatLexer::InitCharPredicates()
{
    europeanDigit = new CharRange('0','9');
    arabicDigit = new CharRange(arab_chars["sifr"][0], arab_chars["tes3a"][0]);

    arabDecimalSeparator = new CharOr(new CharEquals('.'), new CharEquals(arab_chars["momayyez"][0]));
    europeanDecimalSeparator = new CharEquals('.');
    digit = new CharOr(arabicDigit, europeanDigit);
    letter = new CharIsLetter();
    //idSymbol = new CharOr(new CharEquals('.'), new CharEquals(':'));
    idSymbol = new CharEquals('.');

    addOp = new CharEquals('+');
    subOp = new CharEquals('-');

    mulOp = new CharOr(new CharEquals('*'), new CharEquals(arab_chars["mul"].at(0)));
    divOp = new CharOr(new CharEquals('/'), new CharEquals(arab_chars["div"].at(0)));


    eq = new CharEquals('=');
    lt = new CharEquals('<');
    gt = new CharEquals('>');

    le = new LAStr("<=");
    ge = new LAStr(">=");
    ne = new LAStr("<>");

    comma = new CharOr(new CharEquals(','), new CharEquals(arab_chars["comma"].at(0)));
    semi  = new CharOr(new CharEquals(';'), new CharEquals(arab_chars["semi"].at(0)));
    colon = new CharEquals(':');
    kasra = new CharEquals(arab_chars["kasra"].at(0));
    dollarSign = new CharEquals('$');
    hash = new CharEquals('#');
    ellipsis = new LAStr("...");

    lparen = new CharEquals('(');
    rparen = new CharEquals(')');

    lbracket = new CharEquals('[');
    rbracket = new CharEquals(']');

    spacer = new CharEquals(' ');
    tab= new CharEquals('\t');

    space = new CharOr(spacer, tab);
    anyChar = new AnyChar();

    quote = new CharEquals('"');
    two_quotes = new LAStr("\"\"");

    sol = new LineStart();
    eof = new Eof();
    nl = new CharEquals('\n');
    noneNl = new CharNotEqual('\n');
    retract = new Retract();
    lineComment = new LAStr("--");
    returnWith = seq(new LAStr(L"ارجع"), la(space), loop(la(space)), new LAStr(L"ب:"));
}

Token KalimatLexer::accept(TokenType t)
{

    if(t == IDENTIFIER)
    {
        QString toAccept = buffer.acceptable();

        if(keywords.contains(toAccept))
        {
            t = keywords[toAccept];
        }
        else if(toAccept.endsWith(':'))
        {
            buffer.retract(1);
            toAccept = toAccept.left(toAccept.length()-1);
            if(keywords.contains(toAccept))
                t = keywords[toAccept];
        }
    }
    Token result = Lexer::accept(t);
    return result;

}
void KalimatLexer::InitKeywords()
{
    LineIterator in = Utils::readResourceTextFile(":/keywords.txt");
    int id = 1;
    while(!in.atEnd())
    {
        QString kw = in.readLine();
        keywords[kw] = id++;
    }
    in.close();


}
void KalimatLexer::InitArabChars()
{
    LineIterator in = Utils::readResourceTextFile(":/arab_char.txt");
    while(!in.atEnd())
    {
        QString kw = in.readLine().trimmed();
        QString val = in.readLine().trimmed();
        arab_chars[kw] = val;
    }
    in.close();

}

QString TokenNameFromId(int id)
{
    if(KalimatLexer::tokenNameMap.count()==0)
    {
        LineIterator in = Utils::readResourceTextFile(":/tokens.txt");
        while(!in.atEnd())
        {
            QStringList data = in.readLine().split(' ', QString::SkipEmptyParts, Qt::CaseSensitive);
            QString name = data.at(0);
            QString _id = data.at(1);
            int id = _id.toInt();
            KalimatLexer::tokenNameMap[id] = name;
        }
        in.close();
    }
    return KalimatLexer::tokenNameMap[id];
}
