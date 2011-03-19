#include "codeformatter.h"

IndentingCodeFormatter::IndentingCodeFormatter()
{
    atLineStart = true;
    indentLevel = 0;
    lastPrinted = "";
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
    lastPrinted = ""; // we are only interested in spaces and lines
}

void IndentingCodeFormatter::nl()
{
    rawNl();
    atLineStart = true;
}

void IndentingCodeFormatter::blankLine()
{
    if(lastPrinted != "\n")
    {
        nl();
        lastPrinted = "\n";
    }
}

void IndentingCodeFormatter::space()
{
    if(lastPrinted != " ")
    {
        CommonCodeFormatter::space();
        lastPrinted = " ";
    }
}

void IndentingCodeFormatter::indent()
{
    indentLevel++;
}

void IndentingCodeFormatter::deindent()
{
    indentLevel--;
}
