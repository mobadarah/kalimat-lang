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
        QString name = proc->procName()->name;
        if(name == "%main")
            return QString::fromStdWString(L"(البرنامج الرئيسي)");
        else
            return name;

    }

}

CompilationUnitInfo Analyzer::analyzeCompilationUnit(shared_ptr<CompilationUnit> cu)
{
    CompilationUnitInfo ret;
    analyzeFunctionDeclarations(cu, ret);
    analyzeClassDeclarations(cu, ret);
    return ret;
}

void Analyzer::
analyzeFunctionDeclarations(shared_ptr<CompilationUnit> cu,
                            CompilationUnitInfo &ret)
{
    forEachDecl(cu,
    [&ret](shared_ptr<Declaration> decl)->void
    {
        shared_ptr<ProceduralDecl> proc =
            dynamic_pointer_cast<ProceduralDecl>(
                    decl);
        if(proc)
        {
            QString s = getBeautifulName(proc);
            ret.funcNameToAst[getBeautifulName(proc)] = decl;
            Token start = proc->getPos();
            Token end = proc->_endingToken;
            ret.rangeOfEachProc[start.Pos] = ProcPosRange(start.Pos,
                                                          end.Pos + end.Lexeme.length(),
                                                          proc);
        }
    });
}
void Analyzer::
analyzeClassDeclarations(shared_ptr<CompilationUnit> cu,
                            CompilationUnitInfo &ret)
{
    // First pass: collect class names
    forEachDecl(cu,
    [&ret](shared_ptr<Declaration> decl)->void
    {
        shared_ptr<ClassDecl> clas =
            dynamic_pointer_cast<ClassDecl>(
                    decl);
        if(clas)
        {
        QString name = clas->name()->name;
        ClassInfo ci;
        for(int i=0; i<clas->methodCount(); i++)
        {
            shared_ptr<MethodDecl> md = clas->method(i);
            ci.methods[md->procName()->name] = md;
        }
        ret.classInfo[name] = ci;
        }
    });
}

void Analyzer::forEachDecl(shared_ptr<CompilationUnit> cu, std::function<void(shared_ptr<Declaration>)> func)
{
    shared_ptr<Module> module = dynamic_pointer_cast<Module>(cu);
    if(module)
    {

        for(int i=0; i<module->declCount(); i++)
        {
            shared_ptr<Declaration> decl = module->decl(i);
            func(decl);
        }
        return;
    }
    shared_ptr<Program> program = dynamic_pointer_cast<Program>(cu);
    if(program)
    {
        int n = program->elementCount();
        for(int i=0; i<program->elementCount(); i++)
        {
            shared_ptr<TopLevel> el= program->element(i);
            shared_ptr<Declaration> decl = dynamic_pointer_cast<Declaration>(
                        el);
            if(decl)
            {
                func(decl);
            }
        }
        return;
    }
}

QString methodDeclarationForCompletion(shared_ptr<MethodDecl> md)
{
    QString ret;
    SimpleCodeFormatter sc;
    md->prettyPrint(&sc);
    ret = sc.getOutput();
    return ret;
}
