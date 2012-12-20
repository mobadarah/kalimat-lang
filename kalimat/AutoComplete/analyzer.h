#ifndef ANALYZER_H
#define ANALYZER_H

#include <QMap>
#include <functional>
#include "../Parser/KalimatAst/kalimat_ast_gen.h"
#include "idemessages.h"
#include "../smallvm/utils.h"

QString getBeautifulName(shared_ptr<ProceduralDecl> proc, Translation<IdeMsg::IdeMessage> &msg);
QString methodDeclarationForCompletion(shared_ptr<MethodDecl> md);

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

struct ClassInfo
{
    QVector<QString *> baseClasses;
    // All methods, here or inherited
    // if inheritance overides a function with differently
    // names parameters, use new names
    QMap<QString, shared_ptr<MethodDecl> > methods;
};

struct CompilationUnitInfo
{
    QMap<QString, shared_ptr<ProceduralDecl> > funcNameToAst;

    // startPos -> {range}
    QMap<int, ProcPosRange> rangeOfEachProc;
    QMap<QString, ClassInfo> classInfo;
    QMap<Identifier *, QString> varTypes;
};

class Analyzer
{
    Translation<IdeMsg::IdeMessage> &msg;
public:
    Analyzer(Translation<IdeMsg::IdeMessage> &msg);
    CompilationUnitInfo analyzeCompilationUnit(shared_ptr<CompilationUnit>);
    void analyzeFunctionDeclarations(shared_ptr<CompilationUnit> cu,
                                     CompilationUnitInfo &ret);
    void analyzeClassDeclarations(shared_ptr<CompilationUnit> cu,
                                     CompilationUnitInfo &ret);
    void forEachDecl(shared_ptr<CompilationUnit> cu, std::function<void(shared_ptr<Declaration>)>);
};

#endif // ANALYZER_H
