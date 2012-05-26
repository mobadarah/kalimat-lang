#ifndef ANALYZER_H
#define ANALYZER_H

#include <QMap>
#include "../Parser/KalimatAst/declaration.h"
#include "../Parser/KalimatAst/expression.h"

QString getBeautifulName(shared_ptr<ProceduralDecl> proc);
struct ProcPosRange
{
    int from, to;
    shared_ptr<ProceduralDecl> proc;
    ProcPosRange(): from(-1), to(-1){}
    ProcPosRange(int _from, int _to, shared_ptr<ProceduralDecl> _proc)
        :from(_from), to(_to), proc(_proc)
    {
    }
};

struct CompilationUnitInfo
{
    QMap<QString, shared_ptr<ProceduralDecl> > funcNameToAst;

    // startPos -> {range}
    QMap<int, ProcPosRange> rangeOfEachProc;
};

class Analyzer
{
public:
    Analyzer();
    CompilationUnitInfo analyzeCompilationUnit(shared_ptr<CompilationUnit>);
};

#endif // ANALYZER_H
