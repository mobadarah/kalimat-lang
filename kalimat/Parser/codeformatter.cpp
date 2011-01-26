#include "codeformatter.h"

IndentingCodeFormatter::IndentingCodeFormatter()
{
    atLineStart = true;
    indentLevel = false;
}

void IndentingCodeFormatter::print(QString code)
{
    if(atLineStart)
    {
        for(int i=0; i<indentLevel *4; i++)
            printRaw(" ");
        atLineStart = false;
    }
    printRaw(code);
}

void IndentingCodeFormatter::nl()
{
    atLineStart = true;
}

void IndentingCodeFormatter::indent()
{
    indentLevel++;
}

void IndentingCodeFormatter::deindent()
{
    indentLevel--;
}
