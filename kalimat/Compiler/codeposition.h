/**************************************************************************
** The Kalimat programming language
**   Copyright (C) 2010 Mohamed Samy Ali - samy2004@gmail.com
**   This project is released under the Apache License version 2.0
**   as described in the included license.txt file
**************************************************************************/

#ifndef CODEPOSITION_H
#define CODEPOSITION_H
class AST;
struct CodePosition
{
    CodeDocument *doc;
    int pos;
    AST *ast;
};

#endif // CODEPOSITION_H
