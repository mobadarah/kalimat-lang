/**************************************************************************
**   The Kalimat programming language
**   Copyright (C) 2010 Mohamed Samy Ali - samy2004@gmail.com
**   This project is released under the Apache License version 2.0
**   as described in the included license.txt file
**************************************************************************/

#include "Lexer/lexer_incl.h"
#include "Lexer/kalimatlexer.h"

#include "syntaxhighlighter.h"
#include <QMap>

SyntaxHighlighter::SyntaxHighlighter(QTextDocument *parent, KalimatLexer *_lexer) :
    QSyntaxHighlighter(parent)
{
    lexer = _lexer;

    keywords.setForeground(Qt::blue);
    comments.setForeground(Qt::green);
    stringLiterals.setForeground(Qt::darkMagenta);

}
void SyntaxHighlighter::highlightBlock(const QString &text)
{
    this->lexer->init(text);
    try
    {
        lexer->tokenize();
        QVector<Token> tokens = lexer->getTokens();

        for(int i=0; i<tokens.size(); i++)
        {
           Token  t = tokens[i];
           if(t.Type>=1 && t.Type <=KEYWORD_CUTOFF)
               setFormat(t.Pos, t.Lexeme.length(), keywords);
           else if(t.Type == STR_LITERAL)
               setFormat(t.Pos, t.Lexeme.length(), stringLiterals);
           else if(t.Type == COMMENT)
               setFormat(t.Pos, t.Lexeme.length(), comments);
       }
    }
    catch(UnexpectedCharException ex)
    {

    }
    catch(ColonUnsupportedInIdentifiersException ex)
    {

    }
    catch(UnexpectedEndOfFileException ex)
    {

    }
}
