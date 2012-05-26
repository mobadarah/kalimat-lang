#include "analyzer.h"

Analyzer::Analyzer()
{
}

QString getBeautifulName(shared_ptr<ProceduralDecl> proc)
{
    shared_ptr<MethodDecl> method = dynamic_pointer_cast<MethodDecl>(proc);
    if(method)
    {
        return QString("%1 : %2").arg(method->className()->name).arg(proc->procName()->name);
    }
    else
    {
        return proc->procName()->name;
    }

}

CompilationUnitInfo Analyzer::analyzeCompilationUnit(shared_ptr<CompilationUnit> cu)
{
    CompilationUnitInfo ret;

    shared_ptr<Module> module = dynamic_pointer_cast<Module>(cu);
    if(module)
    {
        for(int i=0; i<module->declCount(); i++)
        {
            shared_ptr<Declaration> decl = module->decl(i);
            shared_ptr<ProceduralDecl> proc = dynamic_pointer_cast<ProceduralDecl>(
                        decl);
            if(proc)
            {
                ret.funcNameToAst[getBeautifulName(proc)] = decl;
                Token start = proc->getPos();
                Token end = proc->_endingToken;
                ret.rangeOfEachProc[start.Pos] = ProcPosRange(start.Pos,
                                                              end.Pos + end.Lexeme.length(),
                                                              proc);

            }
        }
        return ret;
    }
    shared_ptr<Program> program = dynamic_pointer_cast<Program>(cu);
    if(program)
    {
        for(int i=0; i<program->elementCount(); i++)
        {
            shared_ptr<TopLevel> el= program->element(i);
            shared_ptr<ProceduralDecl> proc = dynamic_pointer_cast<ProceduralDecl>(
                        el);
            if(proc)
            {
                ret.funcNameToAst[getBeautifulName(proc)] = el;
                Token start = proc->getPos();
                Token end = proc->_endingToken;
                ret.rangeOfEachProc[start.Pos] = ProcPosRange(start.Pos,
                                                              end.Pos + end.Lexeme.length(),
                                                              proc);
            }
        }
        return ret;
    }
    return ret;
}
