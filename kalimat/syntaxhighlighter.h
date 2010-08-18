/**************************************************************************
** The Kalimat programming language
**   Copyright (C) 2010 Mohamed Samy Ali - samy2004@gmail.com
**   This project is released under the Apache License version 2.0
**   as described in the included license.txt file
**************************************************************************/

#ifndef SYNTAXHIGHLIGHTER_H
#define SYNTAXHIGHLIGHTER_H

#include <QSyntaxHighlighter>

class KalimatLexer;
class SyntaxHighlighter : public QSyntaxHighlighter
{
Q_OBJECT
public:
     SyntaxHighlighter (QTextDocument *parent, KalimatLexer *);

 protected:
    void highlightBlock(const QString &text);

 private:
     KalimatLexer *lexer;
     QMap<int, QTextCharFormat> tokenFormats;
     QTextCharFormat keywords;
     QTextCharFormat comments;
     QTextCharFormat stringLiterals;



};

#endif // SYNTAXHIGHLIGHTER_H
