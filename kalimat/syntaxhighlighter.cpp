/**************************************************************************
**   The Kalimat programming language
**   Copyright (C) 2010 Mohamed Samy Ali - samy2004@gmail.com
**   This project is released under the Apache License version 2.0
**   as described in the included license.txt file
**************************************************************************/

#include "Lexer/lexer_incl.h"
#include "Lexer/kalimatlexer.h"
#include "Parser/parser_incl.h"
#include "Parser/kalimatast/kalimatast_incl.h"
#include "Parser/kalimatparser.h"
#include "syntaxhighlighter.h"
#include <QMap>
#include <QTextDocument>

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

void SyntaxHighlighter::highlightToHtml(QString program, QStringList &output)
{
    this->lexer->init(program);
    try
    {
        lexer->tokenize();
        QVector<Token> tokens = lexer->getTokens();
        output.append("<div dir=\"rtl\" style=\"background-color: rgb(240, 240, 255);\"><pre>");
        int indentlevel = 0;
        for(int i=0; i<tokens.size(); i++)
        {
           Token  t = tokens[i];
           if(t.Type>=1 && t.Type <=KEYWORD_CUTOFF)
           {
               output.append(QString("<span style=\"color:blue;\">%1</span>").arg(t.Lexeme));
           }
           else if(t.Type == STR_LITERAL)
           {
               output.append(QString("<span style=\"color:magenta;\">%1</span>").arg(t.Lexeme));
           }
           else if(t.Type == COMMENT)
           {
                output.append(QString("<span style=\"color:magenta;\">%1</span>").arg(t.Lexeme));
           }
           else if(t.Type == NEWLINE)
           {
               output.append("<br>");
           }
           else
           {
               output.append(t.Lexeme);
           }

           if(i+1<tokens.size())
           {
               Token &next = tokens[i+1];
               if(t.Type == COLON && next.Type == NEWLINE)
               {
                   indentlevel++;
               }
               else if(t.Type == NEWLINE &&
                       (next.Type == END || next.Type == DONE ||
                        next.Type == CONTINUE || next.Type == ELSE ||
                        next.Type == OR))
               {
                   indentlevel--;
               }
               else if(t.Type !=NEWLINE)
               {
                  output.append(" ");
                  //output.append("&nbsp;");
               }
           }
           if(t.Type == NEWLINE)
           {
             for(int j=0; j<indentlevel * 4; j++)
             {
              //  output.append("&nbsp;");
                output.append(" ");
             }
           }
       }
       output.append("</pre></div>");
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

void SyntaxHighlighter::highlightToWiki(QString program, QStringList &output)
{
    this->lexer->init(program);
    try
    {
        lexer->tokenize();
        QVector<Token> tokens = lexer->getTokens();
        output.append("<div dir=\"rtl\">\n ");
        int indentlevel = 0;
        for(int i=0; i<tokens.size(); i++)
        {
           Token  t = tokens[i];
           if(t.Type>=1 && t.Type <=KEYWORD_CUTOFF)
           {
               output.append(QString("<span style=\"color:blue;\">%1</span>").arg(t.Lexeme));
           }
           else if(t.Type == STR_LITERAL)
           {
               output.append(QString("<span style=\"color:magenta;\">%1</span>").arg(t.Lexeme));
           }
           else if(t.Type == COMMENT)
           {
                output.append(QString("<span style=\"color:magenta;\">%1</span>").arg(t.Lexeme));
           }
           else if(t.Type == NEWLINE)
           {
               output.append("\n ");
           }
           else
           {
               output.append(t.Lexeme);
           }

           if(i+1<tokens.size())
           {
               Token &next = tokens[i+1];
               if(t.Type == COLON && next.Type == NEWLINE)
               {
                   indentlevel++;
               }
               else if(t.Type == NEWLINE &&
                       (next.Type == END || next.Type == DONE || next.Type == CONTINUE || next.Type == ELSE))
               {
                   indentlevel--;
               }
               else if(t.Type !=NEWLINE)
               {
                  output.append(" ");
               }
           }
           if(t.Type == NEWLINE)
           {
             for(int j=0; j<indentlevel * 4; j++)
                output.append(" ");
           }
       }
       output.append("</div>");
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

void SyntaxHighlighter::highlightLiterateHtml(QString program, QStringList &output)
{
    KalimatLexer lxr;
    KalimatParser parser;
    SimpleCodeFormatter fmt;
    try
    {
        parser.init(program, &lxr, NULL);
        QSharedPointer<AST> tree = parser.parse();
        tree->prettyPrint(&fmt);
        output.append(fmt.getOutput());
    }
    catch(UnexpectedCharException ex)
    {
    }
    catch(UnexpectedEndOfFileException ex)
    {
    }
    catch(ParserException ex)
    {
    }
}
