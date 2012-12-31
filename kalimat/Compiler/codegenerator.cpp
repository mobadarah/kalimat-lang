/**************************************************************************
**   The Kalimat programming language
**   Copyright (C) 2010 Mohamed Samy Ali - samy2004@gmail.com
**   This project is released under the Apache License version 2.0
**   as described in the included license.txt file
**************************************************************************/

#include <QVector>
#include <QString>
#include <QStringList>
#include <QMap>
#include <QSet>
#include <QStack>
#include <memory>
#include "codegenerator_incl.h"
#include "../../smallvm/utils.h"
#include "../smallvm/runtime_identifiers.h"
using namespace std;
template<typename T1, typename T2> bool isa(T2 *value)
{
    T1 *test = dynamic_cast<T1 *>(value);
    return test!=NULL;
}

template<typename T1, typename T2> bool isa(shared_ptr<T2> value)
{
    T1 *test = dynamic_cast<T1 *>(value.get());
    return test!=NULL;
}

CodeGenerator::CodeGenerator()
{
    Init();
}

void CodeGenerator::Init()
{
    _asm.Init();
    this->debugInfo = debugInfo;
    codePosKeyCount = 0;
    currentModuleName = "";
    currentCodeDoc = NULL;
    mode = CompilationMode;
}

QString CodeGenerator::getOutput()
{
    return _asm.getOutput();
}

QString CodeGenerator::getStringConstantsAsOpCodes()
{
    QStringList ret;
    QMap<QString,QString>::const_iterator i = getStringConstants().begin();
    while(i != getStringConstants().end())
    {
        QString sym = i.value();
        QString data = i.key();


        QString encoding = base64encode(data);

        /*
          Uncomment this only when debugging! Having arbitrary strings in the assembly code
          stops the Pascal code (in the .exe creation component) from loading it correctly
          */
        //ret.append(QString("// %1 :: %2").arg(sym).arg(data.replace("\n","\\n")));

        ret.append(QString(".strconst %1 %2").arg(sym).arg(encoding));
        ++i;
    }
    ret.append("");
    return ret.join("\n");
}

void CodeGenerator::error(CompilerException ex)
{
    switch(mode)
    {
    case CompilationMode:
        throw ex;
    case AnalysisMode:
        return;
    }
}

void CodeGenerator::defineInCurrentScopeNoSource(QString var)
{
    defineInCurrentScope(var, shared_ptr<Identifier>(), "");
}

void CodeGenerator::defineInCurrentScope(QString var, shared_ptr<Identifier> defPoint)
{
    defineInCurrentScope(var, defPoint, "");
}

void CodeGenerator::defineInCurrentScope(QString var, shared_ptr<Identifier> defPoint, QString type)
{
    if(!scopeStack.empty() && !scopeStack.top().bindings.contains(var))
    {
        scopeStack.top().bindings.insert(var);
        if(defPoint) // it can be NULL due to defineInCurrentScopeNoSource
        {
            scopeStack.top().declarationOf[var] = defPoint;
            if(!varInfos.contains(defPoint->getPos().Pos))
            {
                varInfos[defPoint->getPos().Pos] = VarUsageInfo(defPoint);
                if(type!="")
                    varTypeInfo[defPoint->getPos().Pos]
                            = type;
            }
        }
    }
}

bool CodeGenerator::currentScopeFuncNotProc()
{
    if(scopeStack.empty())
        error(CompilerException::no_source(InternalCompilerErrorInFunc).arg("currentScopeFuncNotProc"));
    shared_ptr<ProceduralDecl> decl = scopeStack.top().proc;
    if(isa<ProceduralDecl>(decl))
        return false;
    else if(isa<FunctionDecl>(decl))
        return true;
    else if(isa<MethodDecl>(decl))
    {
        shared_ptr<MethodDecl> md = dynamic_pointer_cast<MethodDecl>(decl);
        return md->isFunctionNotProcedure();
    }
    else
    {
        error(CompilerException::no_source(InternalCompilerErrorInFunc).arg("currentScopeFuncNotProc"));
        return false;
    }

}

void CodeGenerator::generate(shared_ptr<Program> program, QString fileName, CodeDocument *curDoc)
{
    QVector<shared_ptr<Declaration> > declarations;
    QVector<shared_ptr<Statement> > statements;
    currentCodeDoc = curDoc;
/*
    for(int i=0; i<program->usedModuleCount(); i++)
    {
        loadModule(program->usedModule(i));
    }
    */
    extraDeclarations.clear();
    for(int i=0; i<program->elementCount(); i++)
    {
        shared_ptr<TopLevel> tl = program->element(i);
        if(isa<Declaration>(tl))
            declarations.append(dynamic_pointer_cast<Declaration>(tl));
        else if(isa<Statement>(tl))
            statements.append(dynamic_pointer_cast<Statement>(tl));
        else
            error(CompilerException(currentFileName,tl, ASTMustBeStatementOrDeclaration).arg(tl->toString()));
    }

    for(int i=0; i<declarations.count(); i++)
    {
        firstPass(declarations[i]);
    }
    for(int i=0; i<declarations.count(); i++)
    {
        secondPass(declarations[i]);
    }
    for(int i=0; i<declarations.count(); i++)
    {
        thirdPass(declarations[i]);
    }
    checkInheritanceCycles();
    for(int i=0; i<declarations.count(); i++)
    {
        generateDeclaration(declarations[i]);
    }

    for(int i=0; i<extraDeclarations.count(); i++)
    {
        generateDeclaration(extraDeclarations[i]);
    }

}

void CodeGenerator::compileModule(shared_ptr<Module> module, QString fileName, CodeDocument *curDoc)
{
    // Copy-pasted from generate(Program *)
    currentCodeDoc = curDoc;
    currentFileName = fileName;
    QVector<shared_ptr<Declaration> > declarations;

    extraDeclarations.clear();
 /*   for(int i=0; i<module->usedModuleCount(); i++)
    {
        loadModule(module->usedModule(i));
    }
    */
    for(int i=0; i<module->declarationCount(); i++)
    {
        shared_ptr<Declaration> decl = module->declaration(i);
        declarations.append(decl);
    }

    for(int i=0; i<declarations.count(); i++)
    {
        firstPass(declarations[i]);
    }
    for(int i=0; i<declarations.count(); i++)
    {
        secondPass(declarations[i]);
    }
    for(int i=0; i<declarations.count(); i++)
    {
        thirdPass(declarations[i]);
    }
    checkInheritanceCycles();

    for(int i=0; i<declarations.count(); i++)
    {
        generateDeclaration(declarations[i]);
    }

    for(int i=0; i<extraDeclarations.count(); i++)
    {
        generateDeclaration(extraDeclarations[i]);
    }


}

void CodeGenerator::firstPass(shared_ptr<Declaration> decl)
{
    if(isa<ClassDecl>(decl))
    {
        shared_ptr<ClassDecl> cd = dynamic_pointer_cast<ClassDecl>(decl);
        QString name = cd->name()->name();
        if(allClasses.contains(name))
            error(CompilerException(currentFileName,decl, ClassAlreadyExists).arg(name));
        else
            allClasses[name] = cd;
        return;
    }

    if(isa<ProcedureDecl>(decl))
    {
        shared_ptr<ProcedureDecl> proc =
                dynamic_pointer_cast<ProcedureDecl>(decl);
        allProcedures[proc->procName()->name()] = proc;
    }
    if(isa<FunctionDecl>(decl))
    {
        shared_ptr<FunctionDecl> func = dynamic_pointer_cast<FunctionDecl>(decl);
        allFunctions[func->procName()->name()] = func;
    }
    if(isa<GlobalDecl>(decl))
    {
        generateGlobalDeclaration(dynamic_pointer_cast<GlobalDecl>(decl));
    }

}

void CodeGenerator::secondPass(shared_ptr<Declaration>  decl)
{
    if(isa<ClassDecl>(decl))
    {
        shared_ptr<ClassDecl> cd =
                dynamic_pointer_cast<ClassDecl>(decl);
        if(cd->ancestorName() != NULL)
        {
            QString ancestorName = cd->ancestorName()->name();
            if(!allClasses.contains(ancestorName))
            {
                error(CompilerException(currentFileName,decl, AncestorClassXforClassYdoesntExist)
                                            .arg(ancestorName).arg(cd->name()->name()));
            }
            shared_ptr<ClassDecl> ancestor = allClasses[ancestorName];
            cd->setAncestorClass(ancestor);
        }
    }
}

void CodeGenerator::thirdPass(shared_ptr<Declaration> decl)
{
    if(isa<MethodDecl>(decl))
    {
        shared_ptr<MethodDecl> md = dynamic_pointer_cast<MethodDecl>(decl);
        QString name = md->procName()->name();
        QString className = md->className()->name();
        if(!allClasses.contains(className))
            error(CompilerException(currentFileName,decl, MethodDefinedForNotYetExistingClass).arg(className));
        else
        {
            shared_ptr<ClassDecl> theClass = allClasses[className];
            if(theClass->containsMethod(name))
                error(CompilerException(currentFileName,decl, MethodCalledXwasAlreadyDefinedForClassY).arg(name).arg(className));
            if(!theClass->containsMethodPrototype(name))
                error(CompilerException(currentFileName,decl, MethodXwasNotDeclaredInClassY).arg(name).arg(className));

            // We subtract the 1 to account for the extra 'this' parameter in md
            if(theClass->methodPrototype(name).arity() != md->formalCount()-1)
                error(CompilerException(currentFileName,decl, MethodXwasDeclaredWithDifferentArityInClassY).arg(name).arg(className));
            if(theClass->methodPrototype(name).isFunction() && !md->isFunctionNotProcedure())
                error(CompilerException(currentFileName,decl, MethodXwasDeclaredAfunctionButImplementedAsProcedureInClassY).arg(name).arg(className));
            if(!theClass->methodPrototype(name).isFunction() && md->isFunctionNotProcedure())
                error(CompilerException(currentFileName,decl, MethodXwasDeclaredAprocedureButImplementedAsFunctionInClassY).arg(name).arg(className));
            allClasses[className]->insertMethod(name, md);
        }
        return;
    }
}

void CodeGenerator::checkInheritanceCycles()
{
    //todo:
}

void CodeGenerator::generateDeclaration(shared_ptr<Declaration> decl)
{
    if(isa<ProceduralDecl>(decl))
    {
        shared_ptr<ProceduralDecl> pd = dynamic_pointer_cast<ProceduralDecl>(decl);
        pushProcedureScope(pd);
    }

    if(isa<ProcedureDecl>(decl))
    {
        shared_ptr<ProcedureDecl> pd = dynamic_pointer_cast<ProcedureDecl>(decl);
        if(pd->procName()->name() == "%main")
            generateEntryPoint(pd->body()->statements());
        else
            generateProcedureDeclaration(pd);
    }
    else if(isa<FunctionDecl>(decl))
    {
        generateFunctionDeclaration(dynamic_pointer_cast<FunctionDecl>(decl));
    }
    else if(isa<ClassDecl>(decl))
    {
        generateClassDeclaration(dynamic_pointer_cast<ClassDecl>(decl));
    }
    else if(isa<MethodDecl>(decl))
    {
        // It'll be handled by it's owning class, the thirdPass()
        // introduced them to each other. But we need this 'else' clause
        // to stop the final 'else' from throwing a "supported yet" exception
    }
    else if(isa<GlobalDecl>(decl))
    {
        // It's handled in the firstPass().
        // But we need this 'else' clause to stop the final 'else' from throwing a "not supported yet" exception
    }
    else if(isa<FFILibraryDecl>(decl))
    {
        generateFFILibraryDeclaration(dynamic_pointer_cast<FFILibraryDecl>(decl));
    }
    else if(isa<RulesDecl>(decl))
    {
        generateRulesDeclaration(dynamic_pointer_cast<RulesDecl>(decl));
    }
    else
    {
        error(CompilerException(currentFileName,decl, DeclarationNotSupported).arg(decl->toString()));
    }

    if(isa<ProceduralDecl>(decl))
    {
        popProcedureScope();
    }
}

void CodeGenerator::pushProcedureScope(shared_ptr<ProceduralDecl> pd)
{
    Context c;
    c.proc = pd;
    c.instructionCount = 0;
    scopeStack.push(c);
    for(int i=0; i<pd->formalCount(); i++)
    {
        shared_ptr<TypeExpression> type = pd->formal(i)->type();
        QString typeName = type? typeExpressionToAssemblyTypeId(type) : "";
        defineInCurrentScope(pd->formal(i)->name()->name(),
                             pd->formal(i)->name(),
                             typeName);
    }
}
void CodeGenerator::popProcedureScope()
{
    scopeStack.pop();
}

void CodeGenerator::generateProcedureDeclaration(shared_ptr<ProcedureDecl> decl)
{
    gen(decl, QString(".method %1 %2 0").arg(decl->procName()->name()).arg(decl->formalCount()));
    for(int i=0; i<decl->formalCount(); i++)
    {
        gen(decl, "popl " + decl->formal(i)->name()->name());
    }
    generateStatement(decl->body());
    debugInfo.setInstructionForLine(currentCodeDoc, decl->_endingToken.Line,
                                    decl->procName()->name(), scopeStack.top().instructionCount);
    gen(decl, "ret");
    gen(decl,".endmethod");
}

void CodeGenerator::generateFunctionDeclaration(shared_ptr<FunctionDecl> decl)
{
    gen(decl, QString(".method %1 %2 1").arg(decl->procName()->name()).arg(decl->formalCount()));
    for(int i=0; i<decl->formalCount(); i++)
    {
        gen(decl, "popl " + decl->formal(i)->name()->name());
    }

    generateStatement(decl->body());
    debugInfo.setInstructionForLine(currentCodeDoc, decl->_endingToken.Line,
                                    decl->procName()->name(), scopeStack.top().instructionCount);
    gen(decl, "ret");
    gen(decl,".endmethod");
}

void CodeGenerator::generateFFILibraryDeclaration(shared_ptr<FFILibraryDecl> decl)
{
    for(int i=0; i<decl->declCount(); i++)
    {
        shared_ptr<Declaration> d = decl->decl(i);
        if(isa<FFIProceduralDecl>(d))
        {
            generateFFIProceduralDeclaration(dynamic_pointer_cast<FFIProceduralDecl>(d), decl->libName());
        }
        else if(isa<FFIStructDecl>(d))
        {
            generateFFIStructDeclaration(dynamic_pointer_cast<FFIStructDecl>(d));
        }
        else
        {
            error(CompilerException(currentFileName,decl, DeclarationNotSupported));
        }

    }
}

void CodeGenerator::generateFFIProceduralDeclaration(shared_ptr<FFIProceduralDecl> decl, QString libName)
{
    /*
      .method رسالة 4 1
      -- 1: set arg type array
      pushv 4
      newarr
      popl %argTypeArr

      pushl %argTypeArr
      pushv 1
      pushc مشير.سي
      setarr

      pushl %argTypeArr
      pushv 2
      pushc نص.عريض.سي
      setarr
      ...

      -- 2: load library and set function address
      pushc %symbolId
      pushc %libNameId
      callex loadlibrary
      callex getprocaddress
      popl %funcPtr

      -- 3: set args array
      pushv 4
      newarr
      popl %argArr


      popl %anArg
      pushl %argArr
      pushl %anArg
      pushv 1
      setarr

      popl %anArg
      pushl %argArr
      pushl %anArg
      pushv 2
      setarr
      ...

      -- 4: call ff

      pushc retTypeName
      pushl %argTypeArr
      pushl %argArr
      pushl %funcPtr

      callex callforeign

      .endmethod
    */
    // Push a dummy procedure scope since all calls to gen()
    // attempt to increment an instruction count in the current procedure scope
    pushProcedureScope(shared_ptr<FunctionDecl>(new FunctionDecl(decl->getPos(),
                                        false,
                                        decl->getPos(),
                                        shared_ptr<Identifier>(new Identifier(decl->getPos(),decl->procName())),
                                        QVector<shared_ptr<FormalParam> >(),
                                        shared_ptr<BlockStmt>(new BlockStmt(decl->getPos(), QVector<shared_ptr<Statement > >()))

                                        )));
    gen(decl, QString(".method %1 %2 %3").
                             arg(decl->procName())
                            .arg(decl->paramTypeCount())
                            .arg(decl->isFunctionNotProc()?1:0));

    // -- 1: set arg type array
    /*
    pushv 4
    newarr
    popl %argTypeArr

    pushl %argTypeArr
    pushv 1
    pushc مشير.سي
    setarr
    */
    gen(decl, "pushv ", decl->paramTypeCount());
    gen(decl, "newarr");
    gen(decl, "popl %argTypeArr");

    for(int i=0; i<decl->paramTypeCount(); i++)
    {
        gen(decl, "pushl %argTypeArr");
        gen(decl, "pushv ", i+1);

        generateStringConstant(decl, typeExpressionToAssemblyTypeId(decl->paramType(i)));
        gen(decl, "callex typefromid");

        gen(decl, "setarr");
    }

    //-- 2: load library and set function address
    generateStringConstant(decl, decl->symbol());
    generateStringConstant(decl, libName);
    gen(decl, "callex loadlibrary");
    gen(decl, "callex getprocaddress");
    gen(decl, "popl %funcPtr");

    //-- 3: set args array
    /*
    pushv 4
    newarr
    popl %argArr

    popl %anArg
    pushl %argArr
    pushl %anArg
    pushv 1
    setarr
    */
    gen(decl, "pushv ", decl->paramTypeCount());
    gen(decl, "newarr");
    gen(decl, "popl %argArr");

    for(int i=0; i<decl->paramTypeCount(); i++)
    {
        gen(decl, "popl %anArg");
        gen(decl, "pushl %argArr");
        gen(decl, "pushv ", i+1);
        gen(decl, "pushl %anArg");

        gen(decl, "setarr");
    }

    //-- 4: call ff

    if(decl->isFunctionNotProc())
    {
        generateStringConstant(decl, typeExpressionToAssemblyTypeId(decl->returnType()));
        gen(decl, "callex typefromid");
    }
    else
    {
        gen(decl, QString("pushc %1").arg(VMId::get(RId::c_void)));
    }

    gen(decl, "pushl %argTypeArr");
    gen(decl, "pushl %argArr");
    gen(decl, "pushl %funcPtr");
    gen(decl, "callex callforeign");

    //TODO: see this debugInfo thing on the next line
    //debugInfo.setInstructionForLine(currentCodeDoc, decl->_endingToken.Line,
     //                               decl->procName()->name, scopeStack.top().instructionCount);
    if(!decl->isFunctionNotProc())
    {
        // For convinience, a foreign proc can call a foreign function and ignore its return type
        gen(decl, "popl %dummy");
    }
    gen(decl, "ret");
    gen(decl,".endmethod");
    popProcedureScope();
}

void CodeGenerator::generateFFIStructDeclaration(shared_ptr<FFIStructDecl> decl)
{

}

shared_ptr<VarAccess> varOf(Token pos, QString id)
{
    return shared_ptr<VarAccess>(new VarAccess(pos,shared_ptr<Identifier>(new Identifier(pos, id))));
}

shared_ptr<VarAccess> varOf(shared_ptr<Identifier> id)
{
    return shared_ptr<VarAccess>(new VarAccess(id->getPos(), id));
}

shared_ptr<Identifier> idOf(Token pos, QString id)
{
    return shared_ptr<Identifier>(new Identifier(pos, id));
}

shared_ptr<StrLiteral> strLitOf(Token pos, QString value)
{
    return shared_ptr<StrLiteral>(new StrLiteral(pos, value));
}

shared_ptr<NumLiteral> numLitOf(Token pos, int value)
{
    return shared_ptr<NumLiteral>(new NumLiteral(pos, value));
}

shared_ptr<Idafa> fieldAccessOf(shared_ptr<Expression> obj, QString fname)
{
    return shared_ptr<Idafa>(new Idafa(obj->getPos(),
                                       idOf(obj->getPos(), fname),
                                                 obj));
}

shared_ptr<Invokation> invokationOf(Token pos, QString fname)
{
    QVector<shared_ptr<Expression> > args;
    return shared_ptr<Invokation>(new Invokation(pos,
                                                 idOf(pos, fname),
                                                 args));
}

shared_ptr<Invokation> invokationOf(Token pos, QString fname, shared_ptr<Expression> arg0)
{
    QVector<shared_ptr<Expression> > args;
    args.append(arg0);
    return shared_ptr<Invokation>(new Invokation(pos,
                                                 idOf(pos, fname),
                                                 args));
}

shared_ptr<Invokation> invokationOf(Token pos, QString fname, shared_ptr<Expression> arg0,
                                    shared_ptr<Expression> arg1)
{
    QVector<shared_ptr<Expression> > args;
    args.append(arg0);
    args.append(arg1);
    return shared_ptr<Invokation>(new Invokation(pos,
                                                 idOf(pos, fname),
                                                 args));
}

shared_ptr<MethodInvokation> methodOf(shared_ptr<Expression> target,
                                QString mname)
{
    QVector<shared_ptr<Expression> > args;
    return shared_ptr<MethodInvokation>(new MethodInvokation(target->getPos(),
                                                             target,
                                                 idOf(target->getPos(), mname),
                                                 args));
}

shared_ptr<MethodInvokation> methodOf(shared_ptr<Expression> target,
                                    QString mname,
                                    shared_ptr<Expression> arg0)
{
    QVector<shared_ptr<Expression> > args;
    args.append(arg0);
    return shared_ptr<MethodInvokation>(new MethodInvokation(target->getPos(),
                                                             target,
                                                 idOf(target->getPos(), mname),
                                                 args));
}

shared_ptr<MethodInvokation> methodOf(shared_ptr<Expression> target,
                                    QString mname,
                                    shared_ptr<Expression> arg0,
                                    shared_ptr<Expression> arg1)
{
    QVector<shared_ptr<Expression> > args;
    args.append(arg0);
    args.append(arg1);
    return shared_ptr<MethodInvokation>(new MethodInvokation(target->getPos(),
                                                             target,
                                                 idOf(target->getPos(), mname),
                                                 args));
}

shared_ptr<MethodInvokation> methodOf(shared_ptr<Expression> target,
                                    QString mname,
                                    shared_ptr<Expression> arg0,
                                    shared_ptr<Expression> arg1,
                                      shared_ptr<Expression> arg2)
{
    QVector<shared_ptr<Expression> > args;
    args.append(arg0);
    args.append(arg1);
    args.append(arg2);
    return shared_ptr<MethodInvokation>(new MethodInvokation(target->getPos(),
                                                             target,
                                                 idOf(target->getPos(), mname),
                                                 args));
}

shared_ptr<MethodInvokation> methodOf(shared_ptr<Expression> target,
                                    QString mname,
                                    shared_ptr<Expression> arg0,
                                    shared_ptr<Expression> arg1,
                                    shared_ptr<Expression> arg2,
                                    shared_ptr<Expression> arg3
                                      )
{
    QVector<shared_ptr<Expression> > args;
    args.append(arg0);
    args.append(arg1);
    args.append(arg2);
    args.append(arg3);
    return shared_ptr<MethodInvokation>(new MethodInvokation(target->getPos(),
                                                             target,
                                                 idOf(target->getPos(), mname),
                                                 args));
}

shared_ptr<LabelStmt> labelOf(Token pos, QString lbl, Labeller &lblr)
{
    /*
    return shared_ptr<LabelStmt>(new LabelStmt(pos,
                                               strLitOf(pos,lbl)));
    //*/
    //*
    int label = lblr.labelOf(lbl);
    return shared_ptr<LabelStmt>(new LabelStmt(pos,
                                               shared_ptr<NumLiteral>(
                                               new NumLiteral(pos,
                                                              label))));
    //*/
}

shared_ptr<LabelStmt> labelOf(Token pos, QString lbl)
{
    return shared_ptr<LabelStmt>(new LabelStmt(pos,
                                               varOf(pos,lbl)));
}


shared_ptr<GotoStmt> gotoOf(Token pos, QString lbl, Labeller &lblr)
{
    return shared_ptr<GotoStmt>(new GotoStmt(pos,
                                             numLitOf(pos, lblr.labelOf(lbl))));
}

shared_ptr<GotoStmt> gotoOf(Token pos, shared_ptr<Expression> lbl)
{
    return shared_ptr<GotoStmt>(new GotoStmt(pos, lbl));
}

shared_ptr<AssignmentStmt> assignmentOf(Token pos, shared_ptr<AssignableExpression> lval,
                                        shared_ptr<Expression> rval)
{
    return shared_ptr<AssignmentStmt>(new AssignmentStmt(pos, lval, rval,
                                                         shared_ptr<TypeExpression>()));
}

shared_ptr<AssignmentStmt> assignmentOf(shared_ptr<AssignableExpression> lval,
                                        shared_ptr<Expression> rval)
{
    return shared_ptr<AssignmentStmt>(new AssignmentStmt(lval->getPos(),
                                                         lval,
                                                         rval,
                                                         shared_ptr<TypeExpression>()));
}

shared_ptr<IfStmt> ifOf(Token pos, shared_ptr<Expression> cond,
                                        shared_ptr<Statement> thenpart,
                                        shared_ptr<Statement> elsepart)
{
    return shared_ptr<IfStmt>(new IfStmt(pos, cond, thenpart, elsepart));
}


shared_ptr<InvokationStmt> fromInvokation(shared_ptr<IInvokation> inv)
{
    return shared_ptr<InvokationStmt>(new InvokationStmt(inv->getPos(), inv));
}

void CodeGenerator::generateRulesDeclaration(shared_ptr<RulesDecl> decl)
{
    QMap<QString, shared_ptr<RuleDecl> > ruleTable;
    for(int i=0; i<decl->subRuleCount(); i++)
    {
        shared_ptr<RuleDecl> rule = decl->subRule(i);
        if(ruleTable.contains(rule->ruleName()))
            error(CompilerException(currentFileName,rule, RuleAlreadyDefined).arg(rule->ruleName()));
        ruleTable[rule->ruleName()] = rule;
    }
    Labeller labeller;
    QVector<shared_ptr<Statement> > stmts;
    Token pos0 = decl->getPos();
    // %parser = make.parser(%input)
    // %result = null -- to declare the variable
    // ...and they say Lisp isn't used anymore
    stmts.append(assignmentOf(pos0, varOf(pos0, VMId::get(RId::ParserGeneratedVarName)),
                     invokationOf(pos0,_ws(L"صنع.معرب"), varOf(pos0, VMId::get(RId::ParserGeneratedInputArgName)))));

    stmts.append(assignmentOf(pos0, varOf(pos0, VMId::get(RId::ParserGeneratedPosVarName)),
                              shared_ptr<NumLiteral>(new NumLiteral(pos0,0))));

    stmts.append(assignmentOf(pos0, varOf(pos0, VMId::get(RId::ParserGeneratedResultVarName)),
                              shared_ptr<NullLiteral>(new NullLiteral(pos0))));


    // initialize the associated vars with null
    // to make them 'declared'
    QList<QString> identifiersForRules
            = decl->getAllAssociatedVars().toList();
    for(int k=0; k<identifiersForRules.count(); k++)
    {

        stmts.append(
                    assignmentOf(pos0,
                                 varOf(pos0, identifiersForRules[k]),
                                 shared_ptr<NullLiteral>(new NullLiteral(pos0))));
    }

    if(decl->subRuleCount() > 0)
    {
        // manage start rule:
        // we shall push a locals frame since each 'call' assumes
        // both return address and locals frames
        // %parser : pushLocals([])
        stmts.append(fromInvokation(methodOf(
                                        varOf(pos0, VMId::get(RId::ParserGeneratedVarName)),
                                        VMId::get(RId::PushLocals),
                                        shared_ptr<ArrayLiteral>(
                                            new ArrayLiteral(pos0,
                                                             QVector<shared_ptr<Expression> >())
                                            ))));
        // %parser : call(startRuleLabel, %endOfParsing)
        shared_ptr<RuleDecl> rule = decl->subRule(0);
        shared_ptr<NumLiteral> toCall =
                    numLitOf(pos0, labeller.labelOf(rule->ruleName()));
        shared_ptr<NumLiteral> returnHere =
                    numLitOf(pos0, labeller.labelOf(VMId::get(RId::LblEndOfParsing)));
        stmts.append(gotoOf(pos0,
                          methodOf(varOf(pos0, VMId::get(RId::ParserGeneratedVarName)),
                                   VMId::get(RId::RuleCall),
                                   toCall,
                                   returnHere)));

    }
    for(int i=0; i<decl->subRuleCount(); i++)
    {
        shared_ptr<RuleDecl> rule = decl->subRule(i);
        Token rulePos = rule->getPos();
        // a label for our top-level rule
        // todo: since 'Rule' and it's  children aren't yet derived from KalimatAst
        // we'll use the position of the "RulesDecl" in place of their positions
        stmts.append(labelOf(rulePos, rule->ruleName(), labeller));

        for(int j=0; j<rule->options.count(); j++)
        {
            shared_ptr<RuleOption> opt = rule->options[j];
            Token optPos = opt->getPos();
            // a label for each option
            stmts.append(
                        labelOf(optPos, QString("%1%%2").arg(rule->ruleName()).arg(j), labeller)
                        );
            if((j+1) < rule->options.count())
            {
                stmts.append(fromInvokation(methodOf(varOf(optPos, VMId::get(RId::ParserGeneratedVarName)),
                                                         VMId::get(RId::PushBacktrackPoint),
                                                        numLitOf(optPos, labeller.labelOf(QString("%1%%2")
                                                     .arg(rule->ruleName()).arg(j+1))))));
            }



            // now generate code for the PEG expression
            QList<QString> identifiersForRuleOption
                    = opt->expression()->getAllAssociatedVars().toList();
            appendAll(stmts, pegExprToStatements(opt->expression(),
                                                 identifiersForRuleOption,
                                                 labeller,
                                                 ruleTable));
            // ...and if there's an associated resultant expression
            // generate its evaluation and storage in the 'register' result
            // I'll cut my arm of if this works :D
            if(opt->resultExpr())
            {
                Token pos1 = opt->resultExpr()->getPos();
                stmts.append(assignmentOf(pos1, varOf(pos1, VMId::get(RId::ParserGeneratedResultVarName)), opt->resultExpr()));
            }
            else
            {
                Token pos1 = opt->expression()->getPos();
                stmts.append(assignmentOf(pos1, varOf(pos1, VMId::get(RId::ParserGeneratedResultVarName)),
                                          shared_ptr<NullLiteral>(
                                              new NullLiteral(pos1))));
            }
            // if we've succeded, ignore the most recent backtrack point
            if((j+1) < rule->options.count())
            {
                stmts.append(fromInvokation(methodOf(varOf(optPos, VMId::get(RId::ParserGeneratedVarName)),
                                      VMId::get(RId::IgnoreLastBacktrackPoint)
                                     )));
                stmts.append(gotoOf(optPos, VMId::get(RId::ParserSuccessVarName).arg(rule->ruleName()), labeller)
                        );
            }
        } // for each ruleOption

        stmts.append(
                    labelOf(rulePos, VMId::get(RId::ParserSuccessVarName).arg(rule->ruleName()), labeller)
                    );
        // at the end of each rule:
        // first, memoize:
        // %parser: remember("ruleName", %pos, %result)
        stmts.append(fromInvokation(methodOf(varOf(rulePos, VMId::get(RId::ParserGeneratedVarName)),
                                             VMId::get(RId::Memoize),
                                             numLitOf(rulePos, labeller.labelOf(rule->ruleName())),
                                             varOf(rulePos, VMId::get(RId::ParserGeneratedPosVarName)),
                                             fieldAccessOf(varOf(rulePos, VMId::get(RId::ParserGeneratedVarName)), VMId::get(RId::InputPos)),
                                             varOf(rulePos, VMId::get(RId::ParserGeneratedResultVarName)))));
        // then return
        // goto %parser: ret()
        stmts.append(gotoOf(rulePos, methodOf(varOf(rulePos, VMId::get(RId::ParserGeneratedVarName)), VMId::get(RId::RuleReturn))));
    } // for each rule

    // a label %endOfParsing for the start rule to return to
    stmts.append(labelOf(pos0, VMId::get(RId::LblEndOfParsing), labeller));
    // pop dummy 'locals' frame that was first pushed
    stmts.append(assignmentOf(pos0,
                               varOf(pos0, VMId::get(RId::TmpParseFrame)),
                                methodOf(varOf(pos0, VMId::get(RId::ParserGeneratedVarName)),
                           VMId::get(RId::PopLocals))));
    // now return our precious result :D

    shared_ptr<ReturnStmt> returnNow(new ReturnStmt(pos0,
                                                    varOf(pos0, VMId::get(RId::ParserGeneratedResultVarName))));
    stmts.append(returnNow);

    // and have a label to whom VM routines will jump on error
    stmts.append(labelOf(pos0, VMId::get(RId::ParseLblParseError)));
    shared_ptr<ReturnStmt> returnErr(new ReturnStmt(pos0,
                                                    shared_ptr<Expression>(new NullLiteral(pos0))));
    stmts.append(returnErr);
    shared_ptr<BlockStmt> body(
            new BlockStmt(decl->getPos(), stmts));
    QVector<shared_ptr<FormalParam> > formals;
    formals.append(shared_ptr<FormalParam>(
                       new FormalParam(idOf(Token(), VMId::get(RId::ParserGeneratedInputArgName)))));
    shared_ptr<FunctionDecl> func(
            new FunctionDecl(decl->getPos(), true, decl->getPos(),
                             decl->ruleName(), formals, body));
    /*
    QFile f("pargen_debug.txt");
    f.open(QFile::Text | QFile::WriteOnly | QFile::Truncate);
    QTextStream out(&f);
    out.setCodec("UTF-8");
    out.setGenerateByteOrderMark(true);
    SimpleCodeFormatter fmt;
    func->prettyPrint(&fmt);
    out << fmt.getOutput();
    f.close();
    //*/
    pushProcedureScope(func);
    generateFunctionDeclaration(func);
    popProcedureScope();
}

QVector<shared_ptr<Statement> > CodeGenerator::generateRuleImplementation(
        shared_ptr<PegRuleInvokation> rule,
        QList<QString> locals,
        Labeller &labeller)
{
    QVector<shared_ptr<Statement> > result;
    Token pos0 = rule->getPos();
    // save locals
    // %parser : pushLocals([%pos, loc1, loc2, loc3])
    QVector<shared_ptr<Expression> > elems;
    elems.append(varOf(pos0, VMId::get(RId::ParserGeneratedPosVarName)));
    for(int i=0; i<locals.count(); i++)
    {
        elems.append(varOf(pos0, locals[i]));
    }
    shared_ptr<ArrayLiteral> arr(
                new ArrayLiteral(pos0, elems));

    result.append(fromInvokation(methodOf(varOf(pos0, VMId::get(RId::ParserGeneratedVarName)),
                           VMId::get(RId::PushLocals),
                           arr)));
    // done save locals
    // %pos = pos of %parser
    result.append(assignmentOf(varOf(pos0, VMId::get(RId::ParserGeneratedPosVarName)),
                               fieldAccessOf(varOf(pos0, VMId::get(RId::ParserGeneratedVarName)),
                                             VMId::get(RId::InputPos))));
    //now invoke the rule
    QString continuationLabel = _asm.uniqueLabel();
    shared_ptr<NumLiteral> toCall(
                numLitOf(pos0, labeller.labelOf(rule->ruleName()->name())));
    shared_ptr<NumLiteral> returnHere =
                numLitOf(pos0, labeller.labelOf(continuationLabel));

    result.append(gotoOf(pos0,
                      methodOf(varOf(pos0, VMId::get(RId::ParserGeneratedVarName)),
                               VMId::get(RId::RuleCall),
                               toCall,
                               returnHere)));
    result.append(labelOf(pos0, continuationLabel, labeller));
    // restore locals
    // %tempFrame = %parser: restoreLocals()
    // %pos = %tempFrame[1]
    // loc1 = %tempFrame[2]
    // loc2 = %tempFrame[3]
    // loc3 = %tempFrame[4]
    result.append(assignmentOf(pos0,
                               varOf(pos0, VMId::get(RId::TmpParseFrame)),
                                methodOf(varOf(pos0, VMId::get(RId::ParserGeneratedVarName)),
                                         VMId::get(RId::PopLocals))));
    locals.prepend(VMId::get(RId::ParserGeneratedPosVarName));
    for(int i=0;i<locals.count(); i++)
    {
        shared_ptr<NumLiteral> idx(
                    new NumLiteral(pos0, i+1));
        shared_ptr<ArrayIndex> arrAccess(
                    new ArrayIndex(pos0,
                                   varOf(pos0,VMId::get(RId::TmpParseFrame)),
                                   idx));
        result.append(assignmentOf(pos0,
                                   varOf(pos0, locals[i]),
                                   arrAccess
                                        ));
    }
    locals.removeFirst();
    return result;
}

QVector<shared_ptr<Statement> > CodeGenerator::pegExprToStatements(
        shared_ptr<PegExpr> expr, QList<QString> locals,
        Labeller &labeller,
        QMap<QString, shared_ptr<RuleDecl> > ruleTable)
{
    QVector<shared_ptr<Statement> > result;
    if(isa<PegRuleInvokation>(expr))
    {
        shared_ptr<PegRuleInvokation> rule = dynamic_pointer_cast<PegRuleInvokation>(expr);
        if(!ruleTable.contains(rule->ruleName()->name()))
        {
            error(CompilerException(currentFileName,rule, InvokingUndefinedRule).arg(rule->ruleName()->name()));
        }
        Token pos0 = rule->getPos();
        // if %parser: youRemember("ruleName", pos of %parser):
        //     %tmp = %parser : getMemory("ruleName", pos of %parser)
        //     pos of %parser = pos of %tmp
        //     %result = result of %tmp
        // else:
        //    ...rule invokation...
        // done
        shared_ptr<Expression> rememberCond(
                    methodOf(varOf(pos0, VMId::get(RId::ParserGeneratedVarName)),
                             VMId::get(RId::IsMemoized),
                             numLitOf(pos0, labeller.labelOf(rule->ruleName()->name())),
                             fieldAccessOf(varOf(pos0, VMId::get(RId::ParserGeneratedVarName)),
                                           VMId::get(RId::InputPos))));

        QVector<shared_ptr<Statement> > assignStmts;
        shared_ptr<VarAccess> tmpId = varOf(pos0, VMId::get(RId::ParserTempVarName));
        shared_ptr<VarAccess> parserId = varOf(pos0, VMId::get(RId::ParserGeneratedVarName));
        assignStmts.append(assignmentOf(tmpId,
                                        methodOf(parserId,
                                                 VMId::get(RId::GetMemoized),
                                                 numLitOf(pos0, labeller.labelOf(rule->ruleName()->name())),
                                                 fieldAccessOf(varOf(pos0, VMId::get(RId::ParserGeneratedVarName)),
                                                                VMId::get(RId::InputPos))
                                                 )));
        assignStmts.append(assignmentOf(fieldAccessOf(parserId, VMId::get(RId::InputPos)),
                                        fieldAccessOf(tmpId, VMId::get(RId::InputPos))));

        assignStmts.append(assignmentOf(varOf(pos0, VMId::get(RId::ParserGeneratedResultVarName)),
                                        fieldAccessOf(tmpId, VMId::get(RId::ParseResultOf))));



        shared_ptr<BlockStmt> assignBlock(
                    new BlockStmt(pos0, assignStmts));

        QVector<shared_ptr<Statement> > invokeStmts =
                generateRuleImplementation(rule, locals, labeller);

        shared_ptr<BlockStmt> invokeBlock(
                    new BlockStmt(pos0, invokeStmts));

        result.append(ifOf(pos0,
                           rememberCond,
                           assignBlock,
                           invokeBlock));
        if(rule->associatedVar())
        {
            Token pos1 = rule->associatedVar()->getPos();
            result.append(assignmentOf(pos1,
                                       varOf(rule->associatedVar()),
                                       varOf(pos1, VMId::get(RId::ParserGeneratedResultVarName))));
        }
    }
    if(isa<PegLiteral>(expr))
    {

        // if %parser: lookAt(lit)
        //    associatedVar = %parser: look()
        //    %parser: progress()
        // else
        //     %parser : backtrack()
        // end
        shared_ptr<PegLiteral> lit = dynamic_pointer_cast<PegLiteral>(expr);
        bool multiChar = lit->value()->value().count() !=1;
        int charCount = lit->value()->value().count();
        Token pos0 = lit->getPos();
        QVector<shared_ptr<Statement> > thenStmts;
        QVector<shared_ptr<Statement> > elseStmts;
        Token pos1; // of associated variable
        if(lit->associatedVar())
        {
            pos1 = lit->associatedVar()->getPos();
        }
        shared_ptr<MethodInvokation> lookAheadCall, conditionCall, progressCall;
        if(multiChar)
        {
            lookAheadCall = (
                        methodOf(varOf(pos1, VMId::get(RId::ParserGeneratedVarName)),
                                 VMId::get(RId::PeekMany),
                                 shared_ptr<Expression>(new NumLiteral(pos1,
                                                                       charCount))));
            progressCall = methodOf(varOf(pos0,VMId::get(RId::ParserGeneratedVarName)),
                                    VMId::get(RId::MoveNextMany),
                                    shared_ptr<Expression>(new NumLiteral(pos1,
                                                                          charCount)));
            conditionCall = methodOf(varOf(pos0, VMId::get(RId::ParserGeneratedVarName)),
                     VMId::get(RId::LookAheadMany),
                     lit->value());
        }
        else
        {
            lookAheadCall = (
                        methodOf(varOf(pos1, VMId::get(RId::ParserGeneratedVarName)),
                                 VMId::get(RId::Peek)));
            progressCall = methodOf(varOf(pos0,VMId::get(RId::ParserGeneratedVarName)),
                                    VMId::get(RId::MoveNext));
            conditionCall = methodOf(varOf(pos0, VMId::get(RId::ParserGeneratedVarName)),
                     VMId::get(RId::LookAhead),lit->value());
        }
        if(lit->associatedVar())
        {
            thenStmts.append(assignmentOf(pos1,
                                       varOf(lit->associatedVar()),
                                          lookAheadCall));
        }
        thenStmts.append(fromInvokation(progressCall));
        elseStmts.append(gotoOf(pos0, methodOf(varOf(pos0,VMId::get(RId::ParserGeneratedVarName)),
                                        VMId::get(RId::FailAndBackTrack))));
        shared_ptr<BlockStmt> thenPart(new BlockStmt(pos0, thenStmts));
        shared_ptr<BlockStmt> elsePart(new BlockStmt(pos0, elseStmts));
        shared_ptr<IfStmt> ifStmt = ifOf(pos0,
                    conditionCall,
                    thenPart,
                    elsePart);
        result.append(ifStmt);
    }
    if(isa<PegCharRange>(expr))
    {

        // if %parser: lookAtRange(lit)
        //    associatedVar = %parser: look()
        //    %parser: progress()
        // else
        //     %parser : backtrack()
        // end
        shared_ptr<PegCharRange> range = dynamic_pointer_cast<PegCharRange>(expr);
        Token pos0 = range->getPos();
        QVector<shared_ptr<Statement> > thenStmts;
        QVector<shared_ptr<Statement> > elseStmts;
        Token pos1; // of associated variable
        if(range->associatedVar())
        {
            pos1 = range->associatedVar()->getPos();
        }
        shared_ptr<MethodInvokation> lookAheadCall, conditionCall, progressCall;


            lookAheadCall = (
                        methodOf(varOf(pos1, VMId::get(RId::ParserGeneratedVarName)),
                                 VMId::get(RId::Peek)));
            progressCall = methodOf(varOf(pos0,VMId::get(RId::ParserGeneratedVarName)),
                                    VMId::get(RId::MoveNext));
            conditionCall = methodOf(varOf(pos0, VMId::get(RId::ParserGeneratedVarName)),
                                     VMId::get(RId::LookAheadRange),range->value1(), range->value2());

        if(range->associatedVar())
        {
            thenStmts.append(assignmentOf(pos1,
                                       varOf(range->associatedVar()),
                                          lookAheadCall));
        }
        thenStmts.append(fromInvokation(progressCall));
        elseStmts.append(gotoOf(pos0, methodOf(varOf(pos0,VMId::get(RId::ParserGeneratedVarName)),
                                        VMId::get(RId::FailAndBackTrack))));
        shared_ptr<BlockStmt> thenPart(new BlockStmt(pos0, thenStmts));
        shared_ptr<BlockStmt> elsePart(new BlockStmt(pos0, elseStmts));
        shared_ptr<IfStmt> ifStmt = ifOf(pos0,
                    conditionCall,
                    thenPart,
                    elsePart);
        result.append(ifStmt);
    }
    if(isa<PegSequence>(expr))
    {
        shared_ptr<PegSequence> seq = dynamic_pointer_cast<PegSequence>(expr);
        for(int i=0; i<seq->elementCount(); i++)
        {
            appendAll(result, pegExprToStatements(seq->element(i), locals, labeller, ruleTable));
        }
    }
    return result;
}

void CodeGenerator::generateClassDeclaration(shared_ptr<ClassDecl> decl)
{
    gen(decl, ".class "+decl->name()->name());
    if(decl->ancestorName() != NULL)
    {
        QString parent = decl->ancestorName()->name();
        gen(decl->ancestorName(), ".extends "+ parent);
    }
   /* if(decl->prototypeCount() > decl->methodCount())
        error(CompilerException(currentFileName,decl, QString("Class '%1' has some unimplemented methods").arg(decl->name()->name)));
        */
    for(int i=0;i<decl->fieldCount();i++)
    {
        QString fieldName = decl->field(i)->name();
        QString attrs = "";
        if(decl->containsFieldMarshallAs(fieldName))
        {
            attrs = QString(" marshalas=%1").arg(typeExpressionToAssemblyTypeId(decl->fieldMarshallAs(fieldName)));
        }
        gen(decl,".field " + fieldName + attrs);
    }
    for(QMap<QString, shared_ptr<MethodDecl> >::const_iterator i= decl->_methods.begin();
       i != decl->_methods.end(); ++i)
    {
        shared_ptr<MethodDecl> method = i.value();
        pushProcedureScope(method);
        generateMethodDeclaration(method);
        popProcedureScope();
    }
    gen(decl,".endclass");
}
void CodeGenerator::generateGlobalDeclaration(shared_ptr<GlobalDecl> decl)
{
    this->declaredGlobalVariables.insert(decl->varName());
}

void CodeGenerator::generateMethodDeclaration(shared_ptr<MethodDecl> decl)
{
    QString name = decl->procName()->name();
    //varTypeInfo[decl->receiverName()->getPos().Pos]
    //        = decl->className()->name;
    int numRet = decl->isFunctionNotProcedure()? 1: 0;
    gen(decl, QString(".method %1 %2 %3").arg(name).arg(decl->formalCount()).arg(numRet));
    for(int i=0; i<decl->formalCount(); i++)
    {
        gen(decl, "popl " + decl->formal(i)->name()->name());
    }
    generateStatement(decl->body());
    debugInfo.setInstructionForLine(currentCodeDoc, decl->_endingToken.Line,
                                    decl->procName()->name(), scopeStack.top().instructionCount);
    gen(decl, "ret");
    gen(decl,".endmethod");
}

void CodeGenerator::generateEntryPoint(QVector<shared_ptr<Statement> > statements)
{
    gen(".method main");
    for(int i=0; i<statements.count(); i++)
    {
        generateStatement(statements[i]);
    }
    //gen("pushv 0");
    //gen("ret");
    gen(".endmethod");
}

void CodeGenerator::generateStatement(shared_ptr<Statement> stmt)
{
    // todo: this is a hack to fix the disparity between names
    // for main: the vm takes a 'main' and the codegen generates a '%main'
    QString procName = scopeStack.top().proc->procName()->name();
    if(procName == "%main")
        procName = "main";
    debugInfo.setInstructionForLine(currentCodeDoc,
                                    stmt->getPos().Line,
                                    procName,
                                    scopeStack.top().instructionCount);
    if(isa<ReturnStmt>(stmt))
    {
        debugInfo.setReturnLine(currentCodeDoc, stmt->getPos().Line);
    }

    if(isa<IOStatement>(stmt))
    {
        generateIOStatement(dynamic_pointer_cast<IOStatement>(stmt));
    }
    else if(isa<GraphicsStatement>(stmt))
    {
        generateGraphicsStatement(dynamic_pointer_cast<GraphicsStatement>(stmt));
    }
    else if(isa<AssignmentStmt>(stmt))
    {
        generateAssignmentStmt(dynamic_pointer_cast<AssignmentStmt>(stmt));
    }
    else if(isa<IfStmt>(stmt))
    {
        generateIfStmt(dynamic_pointer_cast<IfStmt>(stmt));
    }
    else if(isa<WhileStmt>(stmt))
    {
        generateWhileStmt(dynamic_pointer_cast<WhileStmt>(stmt));
    }
    else if(isa<ForAllStmt>(stmt))
    {
        generateForAllStmt(dynamic_pointer_cast<ForAllStmt>(stmt));
    }
    else if(isa<LabelStmt>(stmt))
    {
        generateLabelStmt(dynamic_pointer_cast<LabelStmt>(stmt));
    }
    else if(isa<GotoStmt>(stmt))
    {
        generateGotoStmt(dynamic_pointer_cast<GotoStmt>(stmt));
    }
    else if(isa<DelegationStmt>(stmt))
    {
        generateDelegationStmt(dynamic_pointer_cast<DelegationStmt>(stmt));
    }
    else if(isa<LaunchStmt>(stmt))
    {
        generateLaunchStmt(dynamic_pointer_cast<LaunchStmt>(stmt));
    }
    else if(isa<ReturnStmt>(stmt))
    {
        generateReturnStmt(dynamic_pointer_cast<ReturnStmt>(stmt));
    }
    else if(isa<BlockStmt>(stmt))
    {
        generateBlockStmt(dynamic_pointer_cast<BlockStmt>(stmt));
    }
    else if(isa<InvokationStmt>(stmt))
    {
        generateInvokationStmt(dynamic_pointer_cast<InvokationStmt>(stmt));
    }
    else if(isa<EventStatement>(stmt))
    {
        generateEventStatement(dynamic_pointer_cast<EventStatement>(stmt));
    }
    else if(isa<SendStmt>(stmt))
    {
        generateSendStmt(dynamic_pointer_cast<SendStmt>(stmt));
    }
    else if(isa<ReceiveStmt>(stmt))
    {
        generateReceiveStmt(dynamic_pointer_cast<ReceiveStmt>(stmt));
    }
    else if(isa<SelectStmt>(stmt))
    {
        generateSelectStmt(dynamic_pointer_cast<SelectStmt>(stmt));
    }
    else
    {
        error(CompilerException(currentFileName, stmt, UnimplementedStatementForm));
    }
}

void CodeGenerator::generateIOStatement(shared_ptr<IOStatement> stmt)
{
    if(isa<PrintStmt>(stmt))
    {
        generatePrintStmt(dynamic_pointer_cast<PrintStmt>(stmt));
        return;
    }
    if(isa<ReadStmt>(stmt))
    {
        generateReadStmt(dynamic_pointer_cast<ReadStmt>(stmt));
        return;
    }
}

void CodeGenerator::generatePrintStmt(shared_ptr<PrintStmt> stmt)
{
    QString fileVar = "";
    if(stmt->fileObject() != NULL)
    {
        fileVar = _asm.uniqueVariable();
        generateExpression(stmt->fileObject());
        gen(stmt, "popl " + fileVar);
    }
    int n = stmt->argCount();
    for(int i=0; i<n; i++)
    {
        if(!stmt->width(i))
        {
            generateExpression(stmt->arg(i));
            if(fileVar != "")
            {
                gen(stmt, "pushl "+ fileVar);
                gen(stmt, "callm " + QString::fromStdWString(L"اكتب"));
            }
            else
            {
                gen(stmt, "callex print");
            }
        }
        else
        {
            generateExpression(stmt->width(i));
            generateExpression(stmt->arg(i));
            if(fileVar != "")
            {
                gen(stmt, "pushl "+ fileVar);
                gen(stmt, "callm "+ QString::fromStdWString(L"اكتب.بعرض"));
            }
            else
            {
                gen(stmt,"callex printw");
            }
        }
    }
    if(!stmt->printOnSameLine())
    {
        if(fileVar != "")
        {
            QString emptyStrConstant = _asm.makeStringConstant("");
            gen(stmt, "pushc "+ emptyStrConstant);
            gen(stmt, "pushl "+ fileVar);
            gen(stmt, "callm "  + QString::fromStdWString(L"اطبع.سطر"));
        }
        else
        {
            gen(stmt,"pushc new_line");
            gen(stmt, "callex print");
        }
    }
}

void CodeGenerator::generateReadStmt(shared_ptr<ReadStmt> stmt)
{
    struct GenerateRvalue : public Thunk
    {
        shared_ptr<AST> _src; CodeGenerator *_g; QString _v;
        GenerateRvalue(shared_ptr<AST> src, CodeGenerator *g, QString v):_src(src),_g(g), _v(v){}
        void operator()()
        {
            _g->gen(_src, "pushl " + _v);
        }
    };

    if(stmt->fileObject() != NULL)
    {
        if(stmt->prompt() != NULL)
            error(CompilerException(currentFileName,stmt, ReadFromCannotContainAPrompt));
        if(stmt->variableCount() !=1)
            error(CompilerException(currentFileName,stmt, ReadFromCanReadOnlyOneVariable));
        shared_ptr<AssignableExpression> lvalue = stmt->variable(0);

        // We first read the data into a temporary variable,
        // then we generate the equivalent of a hidden assignment statement that assigns
        // the value of the temp. variable to the lvalue that we wanted to read.
        QString readVar = _asm.uniqueVariable();
        defineInCurrentScopeNoSource(readVar);
        bool readNum = stmt->readNumberFlags[0];

        QString fileVar = _asm.uniqueVariable();
        generateExpression(stmt->fileObject());
        gen(stmt, "popl "+fileVar);

        gen(stmt, "pushl " + fileVar);
        gen(stmt, "callm " + QString::fromStdWString(L"اقرأ.سطر"));
        if(readNum)
        {
            gen(lvalue, "callex to_num");
        }
        gen(lvalue, "popl " + readVar);
        // The read statement is one of the methods to introduce a new variable
        // into scope,but since in this current implementation generateAssignmentToLvalue(..)
        // already adds the lvalue to the current scope if it's an identifier, we don't need
        // to do so here.
        GenerateRvalue genReadVar(lvalue, this, readVar);

        generateAssignmentToLvalue(lvalue, lvalue, genReadVar);

        return;
    }
    if(stmt->prompt() !=NULL)
    {
        generateStringConstant(stmt, stmt->prompt());
        gen(stmt, "callex print");
    }

    for(int i=0; i<stmt->variableCount(); i++)
    {
        shared_ptr<AssignableExpression> lvalue = stmt->variable(i);
        QString readVar = _asm.uniqueVariable();
        defineInCurrentScopeNoSource(readVar);

        bool readNum = stmt->readNumberFlags[i];

        if(readNum)
            gen(lvalue, "pushv 1");
        else
            gen(lvalue, "pushv 0");

        gen(lvalue,"callex input");
        gen(lvalue, "callex pushreadchan");
        gen(lvalue, "receive");
        gen(lvalue, "popl "+readVar);
        GenerateRvalue genReadVar(lvalue, this, readVar);
        generateAssignmentToLvalue(lvalue, lvalue, genReadVar);
    }
}
void CodeGenerator::generateGraphicsStatement(shared_ptr<GraphicsStatement> stmt)
{
    if(isa<DrawPixelStmt>(stmt))
    {
        generateDrawPixelStmt(dynamic_pointer_cast<DrawPixelStmt>(stmt));
        return;
    }
    if(isa<DrawLineStmt>(stmt))
    {
        generateDrawLineStmt(dynamic_pointer_cast<DrawLineStmt>(stmt));
        return;
    }
    if(isa<DrawRectStmt>(stmt))
    {
        generateDrawRectStmt(dynamic_pointer_cast<DrawRectStmt>(stmt));
        return;
    }
    if(isa<DrawCircleStmt>(stmt))
    {
        generateDrawCircleStmt(dynamic_pointer_cast<DrawCircleStmt>(stmt));
        return;
    }
    if(isa<DrawImageStmt>(stmt))
    {
        generateDrawImageStmt(dynamic_pointer_cast<DrawImageStmt>(stmt));
        return;
    }
    if(isa<DrawSpriteStmt>(stmt))
    {
        generateDrawSpriteStmt(dynamic_pointer_cast<DrawSpriteStmt>(stmt));
        return;
    }
    if(isa<ZoomStmt>(stmt))
    {
        generateZoomStmt(dynamic_pointer_cast<ZoomStmt>(stmt));
        return;
    }
}

void CodeGenerator::generateDrawPixelStmt(shared_ptr<DrawPixelStmt> stmt)
{
    if(stmt->color() == NULL)
        gen(stmt, "pushv -1");
    else
        generateExpression(stmt->color());
    generateExpression(stmt->y());
    generateExpression(stmt->x());
    gen(stmt, "callex drawpixel");
}

void CodeGenerator::generateDrawLineStmt(shared_ptr<DrawLineStmt> stmt)
{
    if(stmt->color() == NULL)
        gen(stmt, "pushv -1");
    else
        generateExpression(stmt->color());
    generateExpression(stmt->y2());
    generateExpression(stmt->x2());
    generateExpression(stmt->y1());
    generateExpression(stmt->x1());

    gen(stmt, "callex drawline");
}

void CodeGenerator::generateDrawRectStmt(shared_ptr<DrawRectStmt> stmt)
{
    if(stmt->filled())
        generateExpression(stmt->filled());
    else
        gen(stmt, "pushv false");
    if(stmt->color() == NULL)
        gen(stmt, "pushv -1");
    else
        generateExpression(stmt->color());

    generateExpression(stmt->y2());
    generateExpression(stmt->x2());
    generateExpression(stmt->y1());
    generateExpression(stmt->x1());

    gen(stmt, "callex drawrect");
}

void CodeGenerator::generateDrawCircleStmt(shared_ptr<DrawCircleStmt> stmt)
{

    if(stmt->filled())
        generateExpression(stmt->filled());
    else
        gen(stmt, "pushv false");

    if(stmt->color() == NULL)
        gen(stmt, "pushv -1");
    else
        generateExpression(stmt->color());

    generateExpression(stmt->radius());
    generateExpression(stmt->cy());
    generateExpression(stmt->cx());

    gen(stmt, "callex drawcircle");
}

void CodeGenerator::generateDrawImageStmt(shared_ptr<DrawImageStmt> stmt)
{
    generateExpression(stmt->y());
    generateExpression(stmt->x());
    generateExpression(stmt->image());
    gen(stmt, "callex drawimage");
}

void CodeGenerator::generateDrawSpriteStmt(shared_ptr<DrawSpriteStmt> stmt)
{
    generateExpression(stmt->y());
    generateExpression(stmt->x());
    generateExpression(stmt->sprite());
    gen(stmt, "call drawspr");
}

void CodeGenerator::generateZoomStmt(shared_ptr<ZoomStmt> stmt)
{
    generateExpression(stmt->y2());
    generateExpression(stmt->x2());
    generateExpression(stmt->y1());
    generateExpression(stmt->x1());

    gen(stmt, "callex zoom");
}

void CodeGenerator::generateEventStatement(shared_ptr<EventStatement> stmt)
{
    QString type;
    if(stmt->type() == KalimatKeyDownEvent)
        type = "keydown";
    else if(stmt->type() == KalimatKeyUpEvent)
        type = "keyup";
    else if(stmt->type() == KalimatKeyPressEvent)
        type = "keypress";
    else if(stmt->type() == KalimatMouseDownEvent)
        type = "mousedown";
    else if(stmt->type() == KalimatMouseUpEvent)
        type = "mouseup";
    else if(stmt->type() == KalimatMouseMoveEvent)
        type = "mousemove";
    else if(stmt->type() == KalimatSpriteCollisionEvent)
        type = "collision";
    gen(stmt, QString("regev %1,%2").arg(type).arg(stmt->handler()->name()));
}

void CodeGenerator::generateAssignmentStmt(shared_ptr<AssignmentStmt> stmt)
{
    shared_ptr<AssignableExpression> lval = stmt->variable();

    struct GenerateExpr : public Thunk
    {
        CodeGenerator *_g; shared_ptr<Expression> _e;
        GenerateExpr(CodeGenerator *g, shared_ptr<Expression> e):_g(g),_e(e) {}
        void operator() (){ _g->generateExpression(_e); }
    } myGen(this, stmt->value());

    generateAssignmentToLvalue(stmt, lval, myGen);
    if(stmt->type() && isa<VarAccess>(lval))
    {
        shared_ptr<TypeExpression> type = stmt->type();
        shared_ptr<VarAccess> var = dynamic_pointer_cast<VarAccess>(lval);
        QString typeName = typeExpressionToAssemblyTypeId(type);
        int pos = var->getPos().Pos;
        if(!varTypeInfo.contains(pos))
        {
            varTypeInfo[pos] = typeName;
        }
        else
        {
            error(CompilerException(currentModuleName, stmt, OnlyFirstAssignmentToVarCanContainType));
        }
    }
}

void CodeGenerator::generateAssignmentToLvalue(shared_ptr<AST> src, shared_ptr<AssignableExpression> lval,
                                               Thunk &genValue)
{
    if(isa<VarAccess>(lval))
    {
        shared_ptr<VarAccess> variable = dynamic_pointer_cast<VarAccess>(lval);
        genValue();
        if(declaredGlobalVariables.contains(variable->name()->name()))
            gen(src, "popg "+variable->name()->name());
        else
        {
            defineInCurrentScope(variable->name()->name(), variable->name());
            gen(src, "popl "+variable->name()->name());
        }
    }
    else if(isa<Idafa>(lval))
    {
        shared_ptr<Idafa> fieldAccess = dynamic_pointer_cast<Idafa>(lval);
        generateExpression(fieldAccess->modaf_elaih());
        genValue();
        gen(src, "setfld "+ fieldAccess->modaf()->name());
    }
    else if(isa<ArrayIndex>(lval))
    {
        shared_ptr<ArrayIndex> arri= dynamic_pointer_cast<ArrayIndex>(lval);
        generateExpression(arri->array());
        generateExpression(arri->index());
        genValue();
        gen(src, "setarr");
    }
    else if(isa<MultiDimensionalArrayIndex>(lval))
    {
        shared_ptr<MultiDimensionalArrayIndex> arri =
                dynamic_pointer_cast<MultiDimensionalArrayIndex>(lval);
        generateExpression(arri->array());
        generateArrayFromValues(lval, arri->indexes());
        genValue();
        gen(src, "setmdarr");
    }
    else
    {
        error(CompilerException(currentFileName,src, LValueFormNotImplemented).arg(lval->toString()));
    }
}

void CodeGenerator::generateReference(shared_ptr<AssignableExpression> lval)
{
    //todo: generating first-class references from lvalues
    gen(lval, "pushnull");
}

void CodeGenerator::generateIfStmt(shared_ptr<IfStmt> stmt)
{
    generateExpression(stmt->condition());
    QString trueLabel = _asm.uniqueLabel();
    QString falseLabel = _asm.uniqueLabel();
    QString endLabel = _asm.uniqueLabel();

    if(stmt->elsePart()==NULL)
        falseLabel = endLabel;

    gen(stmt, "if "+ trueLabel+","+falseLabel);
    gen(stmt->thenPart(), trueLabel+":");
    generateStatement(stmt->thenPart());
    gen(stmt->thenPart(), "jmp "+endLabel);
    if(stmt->elsePart()!= NULL)
    {
        gen(stmt->elsePart(), falseLabel+":");
        generateStatement(stmt->elsePart());
    }
    gen(stmt, endLabel+":");
}

void CodeGenerator::generateWhileStmt(shared_ptr<WhileStmt> stmt)
{
    /*
      while(cond)
         stmt

      label0
      cond
      if label1 label2
      label1:
      stmt
      jmp label0
      label2:
    */
    QString testLabel = _asm.uniqueLabel();
    QString doLabel = _asm.uniqueLabel();
    QString endLabel = _asm.uniqueLabel();

    gen(stmt, testLabel+":");
    generateExpression(stmt->condition());
    gen(stmt->condition(), "if "+doLabel+","+endLabel);
    gen(stmt->statement(), doLabel+":");
    generateStatement(stmt->statement());
    gen(stmt, "jmp "+testLabel);
    gen(stmt, endLabel+":");
}

void CodeGenerator::generateForAllStmt(shared_ptr<ForAllStmt> stmt)
{
    /*
      forall(var, from, to, step)
          stmt

      from
      popl var
      label0:
      pushl var
      to
      gt
      if label2, label1
      label1:
      stmt
      pushl var
      <step>
      add
      popl var
      jmp label0
      label2:
    */

    QString testLabel = _asm.uniqueLabel();
    QString doLabel = _asm.uniqueLabel();
    QString endLabel = _asm.uniqueLabel();

    generateExpression(stmt->from());
    gen(stmt->variable(), "popl " + stmt->variable()->name());
    gen(stmt, testLabel+":");
    gen(stmt->variable(), "pushl "+ stmt->variable()->name());
    generateExpression(stmt->to());
    if(stmt->downTo())
        gen(stmt, "lt");
    else
        gen(stmt, "gt");
    gen(stmt, "if "+endLabel+","+doLabel);
    gen(stmt, doLabel+":");

    defineInCurrentScope(stmt->variable()->name(), stmt->variable());

    generateStatement(stmt->statement());
    gen(stmt, "pushl "+ stmt->variable()->name());
    generateExpression(stmt->step());
    gen(stmt, "add");
    gen(stmt, "popl "+ stmt->variable()->name());
    gen(stmt, "jmp "+testLabel);
    gen(stmt, endLabel+":");
}

void CodeGenerator::generateLabelStmt(shared_ptr<LabelStmt> stmt)
{
    shared_ptr<Expression> target = stmt->target();



    if(isa<NumLiteral>(target))
    {
        QString labelName = dynamic_pointer_cast<NumLiteral>(target)->repr();
        if(scopeStack.top().labels.contains(labelName))
        {
            error(CompilerException(currentFileName,stmt, DuplicateLabel).arg(labelName).arg(getCurrentFunctionNameFormatted()));
        }
        scopeStack.top().labels.insert(labelName);
        gen(stmt, labelName + ": ");
    }
    else if(isa<VarAccess>(target))
    {
        QString labelName = dynamic_pointer_cast<VarAccess>(target)->name()->name();
        if(scopeStack.top().labels.contains(labelName))
        {
            error(CompilerException(currentFileName,stmt, DuplicateLabel).arg(labelName).arg(getCurrentFunctionNameFormatted()));
        }
        scopeStack.top().labels.insert(labelName);
        gen(stmt, labelName + ": ");
    }
    else
    {
        error(CompilerException(currentFileName,stmt, TargetOfLabelMustBeNumberOrIdentifier));
    }
}

void CodeGenerator::generateGotoStmt(shared_ptr<GotoStmt> stmt)
{
    shared_ptr<Expression> target = stmt->target();
    if(isa<NumLiteral>(target))
    {
        gen(stmt, "jmp "+ dynamic_pointer_cast<NumLiteral>(target)->repr());
    }
    else if(isa<VarAccess>(target))
    {
        gen(stmt, "jmp "+ dynamic_pointer_cast<VarAccess>(target)->name()->name());
    }

    else
    {
        generateExpression(target);
        gen(stmt, "jmpv");
    }
}

void CodeGenerator::generateReturnStmt(shared_ptr<ReturnStmt> stmt)
{
    bool notFunc = false;
    if(scopeStack.empty())
        notFunc = true;
    else if(isa<MethodDecl>(scopeStack.top().proc))
    {
        shared_ptr<MethodDecl> proc = dynamic_pointer_cast<MethodDecl>(scopeStack.top().proc);
        notFunc = !(proc)->isFunctionNotProcedure();
    }
    else if(!isa<FunctionDecl>(scopeStack.top().proc))
        notFunc = true;
    if(notFunc)
        error(CompilerException(currentFileName,stmt, ReturnCanBeUsedOnlyInFunctions));
    generateExpression(stmt->returnVal());
    gen(stmt,"ret");
}

void CodeGenerator::generateDelegationStmt(shared_ptr<DelegationStmt> stmt)
{
    shared_ptr<IInvokation> expr = stmt->invokation();

    InvokationContext context = currentScopeFuncNotProc()? FunctionInvokationContext : ProcedureInvokationContext;

    if(isa<Invokation>(expr))
    {
        generateInvokation(dynamic_pointer_cast<Invokation>(expr), context, TailCallStyle);
        return;
    }
    if(isa<MethodInvokation>(expr))
    {
        generateMethodInvokation(dynamic_pointer_cast<MethodInvokation>(expr), context, TailCallStyle);
        return;
    }

    error(CompilerException(currentFileName,expr, UnimplementedInvokationForm).arg(expr->toString()));
}

void CodeGenerator::generateLaunchStmt(shared_ptr<LaunchStmt> stmt)
{
    shared_ptr<IInvokation> expr = stmt->invokation();
    if(isa<Invokation>(expr))
    {
        generateInvokation(dynamic_pointer_cast<Invokation>(expr), ProcedureInvokationContext, LaunchProcessStyle);
        return;
    }
    if(isa<MethodInvokation>(expr))
    {
        generateMethodInvokation(dynamic_pointer_cast<MethodInvokation>(expr), ProcedureInvokationContext, LaunchProcessStyle);
        return;
    }

    error(CompilerException(currentFileName,expr, UnimplementedInvokationForm).arg(expr->toString()));
}

void CodeGenerator::generateBlockStmt(shared_ptr<BlockStmt> stmt)
{
    shared_ptr<ProceduralDecl> owningMethod = scopeStack.top().proc;
    bool blockIsMethodBody = owningMethod->body() == stmt;

    for(int i=0; i<stmt->statementCount(); i++)
    {
        generateStatement(stmt->statement(i));

        if(blockIsMethodBody && i == stmt->statementCount() -1)
        {
            debugInfo.setReturnLine(currentCodeDoc, stmt->getPos().Line);
        }
    }
}

void CodeGenerator::generateInvokationStmt(shared_ptr<InvokationStmt> stmt)
{
    shared_ptr<Expression> expr = stmt->expression();

    if(isa<Invokation>(expr))
    {
        generateInvokation(dynamic_pointer_cast<Invokation>(expr), ProcedureInvokationContext);
        return;
    }
    if(isa<MethodInvokation>(expr))
    {
        generateMethodInvokation(dynamic_pointer_cast<MethodInvokation>(expr), ProcedureInvokationContext);
        return;
    }
    if(isa<ForAutocomplete>(expr))
    {
        generateExpression(expr);
    }
    else
    {
        error(CompilerException(currentFileName,stmt, UnimplementedInvokationForm).arg(expr->toString()));
    }
}

void CodeGenerator::generateSendStmt(shared_ptr<SendStmt> stmt)
{
    generateExpression(stmt->channel());
    if(stmt->signal())
        gen(stmt, "pushnull");
    else
        generateExpression(stmt->value());
    gen(stmt, "send");
}

void CodeGenerator::generateReceiveStmt(shared_ptr<ReceiveStmt> stmt)
{
    if(stmt->signal())
    {
        generateExpression(stmt->channel());
        gen(stmt, "receive");
        gen("popl " + _asm.uniqueVariable());
    }
    else
    {
        shared_ptr<AssignableExpression> lval = stmt->value();

        struct GenerateExpr : public Thunk
        {
            CodeGenerator *_g; shared_ptr<ReceiveStmt> _stmt;
            GenerateExpr(CodeGenerator *g, shared_ptr<ReceiveStmt> s):_g(g),_stmt(s) {}
            void operator() ()
            {
                _g->generateExpression(_stmt->channel());
                _g->gen(_stmt, "receive");
            }
        } myGen(this, stmt);

        generateAssignmentToLvalue(stmt, lval, myGen);
    }
}

void CodeGenerator::generateSelectStmt(shared_ptr<SelectStmt> stmt)
{
    /*
      The select instruction works like this:
       ... arr sendcount => ... ret? activeIndex
      where arr is an array of interleaved channels and values in the following form:
      ch0 val0 ch1 val1 ... chi lvali chi+1 lvali+1 ....chn lvaln
      (the send channels and values are first, followed by receive channels and rvalues)

      sendcount is the number of sends in the array, used to separate sends from receives

      The activeIndex is the index of the channel (in the list of channels; not in the array)
      that successfully communicated. We will use activeIndex to generate a switch statement

      ret is either absent in the case of a successful send, or the received value in case of
      a successful receive
    */

    QVector<int> sends;
    QVector<int> receives;

    // Sift through sends and receives;
    for(int i=0; i<stmt->conditionCount(); i++)
    {
        shared_ptr<ChannelCommunicationStmt> ccs = stmt->condition(i);
        shared_ptr<SendStmt> isSend = dynamic_pointer_cast<SendStmt>(ccs);
        if(isSend)
            sends.append(i);
        else
            receives.append(i);
    }

    int sendCount = sends.count();
    QString tempArrName = _asm.uniqueVariable();

    // Create the array
    gen(stmt, "pushv ", stmt->conditionCount()*2);
    gen(stmt, "newarr");
    gen(stmt, "popl " +tempArrName);

    int current = 1;
    // Fill the array
    for(int i=0; i<sendCount; i++)
    {
        shared_ptr<SendStmt> ss = dynamic_pointer_cast<SendStmt>(stmt->condition(sends[i]));

        gen(ss, "pushl " + tempArrName);
        gen(ss, "pushv ", current);
        generateExpression(ss->channel());
        gen(ss, "setarr");
        current++;

        gen(ss, "pushl " + tempArrName);
        gen(ss, "pushv ", current);
        if(ss->signal())
            gen(ss, "pushnull");
        else
            generateExpression(ss->value());

        gen(ss, "setarr");
        current++;
    }

    for(int i=0; i<receives.count(); i++)
    {
        shared_ptr<ReceiveStmt> rs = dynamic_pointer_cast<ReceiveStmt>(stmt->condition(receives[i]));

        gen(rs, "pushl " + tempArrName);
        gen(rs, "pushv ", current);
        generateExpression(rs->channel());
        gen(rs, "setarr");
        current++;

        gen(rs, "pushl " + tempArrName);
        gen(rs, "pushv ", current);
        if(rs->signal())
            gen(rs, "pushnull");
        else
            generateReference(rs->value());
        gen(rs, "setarr");
        current++;
    }
    // Finally; select!
    gen(stmt, "pushl " + tempArrName);
    gen(stmt, "pushv ", sendCount);
    gen(stmt, "select");

    // Now the switch...
    /*
      popl temp_index

      pushl temp_index
      pushv 0
      eq
      if lbl0,lbl1
      lbl0:
      ....code
      lbl1:

      // this is a receive
      pushl temp_index
      pushv 1
      eq
      if lbl2,lbl3
      lbl2:
      popl ret_val
      (lvalue assignment involving ret_val and the received lval)
      ...code
      lbl3:

      ...etc ...etc
    */
    QString retName = _asm.uniqueVariable();
    QString indexName = _asm.uniqueVariable();

    gen(stmt, "popl " + indexName);

    for(int i=0; i<stmt->conditionCount(); i++)
    {
        QString lbl_a = _asm.uniqueLabel();
        QString lbl_b = _asm.uniqueLabel();

        shared_ptr<Statement> action;
        shared_ptr<ChannelCommunicationStmt> cond;
        if(i<sendCount)
        { action = stmt->action(sends[i]); cond = stmt->condition(sends[i]); }
        else
        { action = stmt->action(receives[i - sendCount]); cond = stmt->condition(receives[i - sendCount]); }
        gen(cond, "pushl " + indexName);
        gen(cond, "pushv ", i);
        gen(cond, "eq");
        gen(cond, "if " + lbl_a +"," + lbl_b);
        gen(cond, lbl_a + ":");
        if(i>=sendCount)
        {
            shared_ptr<ReceiveStmt> recv =
                    dynamic_pointer_cast<ReceiveStmt>(stmt->condition(i));
            class AssignRet : public Thunk
            {
                CodeGenerator *_g; QString _ret; shared_ptr<AssignableExpression> _ae;
            public:
                AssignRet(CodeGenerator *_g, QString _ret, shared_ptr<AssignableExpression> _ae)
                 { this->_g = _g; this->_ret = _ret; this->_ae = _ae; }
                 void operator() ()
                 {
                     _g->gen(_ae, "pushl " + _ret);
                 }

            } assignRet(this, retName, recv->value());

            if(recv->signal())
            {
                gen(recv, "popl " + retName);
                gen(recv, "pushl " + retName);
                gen(recv, "popl " + _asm.uniqueVariable());
            }
            else
            {
                gen(recv->value(), "popl " + retName);
                generateAssignmentToLvalue(recv->value(), recv->value(), assignRet);
            }
        }
        generateStatement(action);
        gen(cond, lbl_b + ":");
    }
    // We are done. If you reached here, take a breath.
}

void CodeGenerator::generateExpression(shared_ptr<Expression> expr)
{
    if(isa<BinaryOperation>(expr))
    {
        generateBinaryOperation(dynamic_pointer_cast<BinaryOperation>(expr));
        return;
    }
    if(isa<IsaOperation>(expr))
    {
        generateIsaOperation(dynamic_pointer_cast<IsaOperation>(expr));
        return;
    }
    if(isa<MatchOperation>(expr))
    {
        generateMatchOperation(dynamic_pointer_cast<MatchOperation>(expr));
        return;
    }
    if(isa<UnaryOperation>(expr))
    {
        generateUnaryOperation(dynamic_pointer_cast<UnaryOperation>(expr));
        return;
    }
    if(isa<VarAccess>(expr))
    {
        generateIdentifier(dynamic_pointer_cast<VarAccess>(expr)->name());
        return;
    }
    if(isa<ForAutocomplete>(expr))
    {
        generateExpression(dynamic_pointer_cast<ForAutocomplete>(expr)
                           ->toBeCompleted());
        return;
    }

    if(isa<NumLiteral>(expr))
    {
        generateNumLiteral(dynamic_pointer_cast<NumLiteral>(expr));
        return;
    }
    if(isa<StrLiteral>(expr))
    {
        generateStrLiteral(dynamic_pointer_cast<StrLiteral>(expr));
        return;
    }
    if(isa<NullLiteral>(expr))
    {
        generateNullLiteral(dynamic_pointer_cast<NullLiteral>(expr));
        return;
    }
    if(isa<BoolLiteral>(expr))
    {
        generateBoolLiteral(dynamic_pointer_cast<BoolLiteral>(expr));
        return;
    }
    if(isa<ArrayLiteral>(expr))
    {
        generateArrayLiteral(dynamic_pointer_cast<ArrayLiteral>(expr));
        return;
    }
    if(isa<MapLiteral>(expr))
    {
        generateMapLiteral(dynamic_pointer_cast<MapLiteral>(expr));
        return;
    }
    if(isa<Invokation>(expr))
    {
        generateInvokation(dynamic_pointer_cast<Invokation>(expr), FunctionInvokationContext);
        return;
    }
    if(isa<MethodInvokation>(expr))
    {
        generateMethodInvokation(dynamic_pointer_cast<MethodInvokation>(expr), FunctionInvokationContext);
        return;
    }
    if(isa<TimingExpression>(expr))
    {
        generateTimingExpression(dynamic_pointer_cast<TimingExpression>(expr));
        return;
    }
    if(isa<TheSomething>(expr))
    {
        generateTheSomething(dynamic_pointer_cast<TheSomething>(expr));
        return;
    }
    if(isa<Idafa>(expr))
    {
        generateIdafa(dynamic_pointer_cast<Idafa>(expr));
        return;
    }
    if(isa<ArrayIndex>(expr))
    {
        generateArrayIndex(dynamic_pointer_cast<ArrayIndex>(expr));
        return;
    }
    if(isa<MultiDimensionalArrayIndex>(expr))
    {
        generateMultiDimensionalArrayIndex(dynamic_pointer_cast<MultiDimensionalArrayIndex>(expr));
        return;
    }
    if(isa<ObjectCreation>(expr))
    {
        generateObjectCreation(dynamic_pointer_cast<ObjectCreation>(expr));
        return;
    }
    if(isa<LambdaExpression>(expr))
    {
        generateLambdaExpression(dynamic_pointer_cast<LambdaExpression>(expr));
        return;
    }
    error(CompilerException(currentFileName,expr, UnimplementedExpressionForm).arg(expr->toString()));
}

void CodeGenerator::generateBinaryOperation(shared_ptr<BinaryOperation> expr)
{
    if(expr->operator_() == "and")
    {
        // if(op1)
        //     op2
        // else
        //     false
        // =>
        // <op1>
        // if goon, else
        // goon:
        // <op2>
        // jmp theend
        // else:
        // pushv false
        // theend:

        QString goOn = _asm.uniqueLabel();
        QString elSe = _asm.uniqueLabel();
        QString theEnd = _asm.uniqueLabel();

        generateExpression(expr->operand1());
        gen(expr, "if " + goOn + "," + elSe);
        gen(expr, goOn+":");
        generateExpression(expr->operand2());
        gen(expr, "jmp " + theEnd);
        gen(expr, elSe+":");
        gen(expr->operand1(), "pushv false");
        gen(expr, theEnd + ":");
    }
    else if(expr->operator_() == "or")
    {
        // if(op1)
        //     op2
        // else
        //     false
        // =>
        // <op1>
        // if else,goon
        // goon:
        // <op2>
        // jmp theend
        // else:
        // pushv true
        // theend:

        QString goOn = _asm.uniqueLabel();
        QString elSe = _asm.uniqueLabel();
        QString theEnd = _asm.uniqueLabel();

        generateExpression(expr->operand1());
        gen(expr, "if " + elSe + "," + goOn);
        gen(expr, goOn+":");
        generateExpression(expr->operand2());
        gen(expr, "jmp " + theEnd);
        gen(expr, elSe+":");
        gen(expr->operand1(), "pushv true");
        gen(expr, theEnd + ":");
    }
    else
    {
        generateExpression(expr->operand1());
        generateExpression(expr->operand2());
        gen(expr, expr->operator_());
    }
}

void CodeGenerator::generateIsaOperation(shared_ptr<IsaOperation> expr)
{
    generateExpression(expr->expression());
    QString typeId = expr->type()->name();
    gen(expr, "isa " + typeId);
}

void CodeGenerator::generateMatchOperation(shared_ptr<MatchOperation> expr)
{
    QMap<shared_ptr<AssignableExpression>, shared_ptr<Identifier> > bindings;
    generatePattern(expr->pattern(), expr->expression(), bindings);

    // Now use all the bindings we've collected
    QString bind = _asm.uniqueLabel(), nobind = _asm.uniqueLabel(), exit = _asm.uniqueLabel();
    gen(expr, QString("if %1,%2").arg(bind).arg(nobind));
    gen(expr, QString("%1:").arg(bind));
    for(QMap<shared_ptr<AssignableExpression>, shared_ptr<Identifier> >::const_iterator i=bindings.begin(); i!=bindings.end();++i)
    {
        // todo: we want to execute the equivalent of matchedVar = tempVar
        // but tempVar is undefined so the compiler refuses the dummy assignment
        // we currently define the temp var, but this pollutes the namespace
        // so we should use generateAssignmentToLVal instead
        defineInCurrentScopeNoSource(i.value()->name());
        generateAssignmentStmt(shared_ptr<AssignmentStmt>(new AssignmentStmt(i.key()->getPos(),
                                                    i.key(),
                                                  varOf(i.value()),
                                                shared_ptr<TypeExpression>())));
    }
    gen(expr, "pushv true");
    gen(expr, QString("jmp %1").arg(exit));
    gen(expr, QString("%1:").arg(nobind));
    gen(expr, "pushv false");
    gen(expr, QString("jmp %1").arg(exit));
    gen(expr, QString("%1:").arg(exit));
}

void CodeGenerator::generatePattern(shared_ptr<Pattern> pattern,
                                    shared_ptr<Expression> expression,
                                     QMap<shared_ptr<AssignableExpression> , shared_ptr<Identifier> > &bindings)
{
    if(isa<SimpleLiteralPattern>(pattern))
    {
        generateSimpleLiteralPattern(dynamic_pointer_cast<SimpleLiteralPattern>(pattern), expression, bindings);
    }
    else if(isa<VarPattern>(pattern))
    {
        generateVarPattern(dynamic_pointer_cast<VarPattern>(pattern), expression, bindings);
    }
    else if(isa<AssignedVarPattern>(pattern))
    {
        generateAssignedVarPattern(dynamic_pointer_cast<AssignedVarPattern>(pattern), expression, bindings);
    }
    else if(isa<ArrayPattern>(pattern))
    {
        generateArrayPattern(dynamic_pointer_cast<ArrayPattern>(pattern), expression, bindings);
    }
    else if(isa<ObjPattern>(pattern))
    {
        generateObjPattern(dynamic_pointer_cast<ObjPattern>(pattern), expression, bindings);
    }
    else if(isa<MapPattern>(pattern))
    {
        generateMapPattern(dynamic_pointer_cast<MapPattern>(pattern), expression, bindings);
    }
    else
    {
        error(CompilerException(currentFileName,pattern, UnimplementedPatternForm1).arg(pattern->toString()));
    }
}

void CodeGenerator::generateSimpleLiteralPattern(shared_ptr<SimpleLiteralPattern> pattern,
                                                 shared_ptr<Expression> matchee,
                                                   QMap<shared_ptr<AssignableExpression>, shared_ptr<Identifier> > &bindings)
{
    generateExpression(pattern->value());
    generateExpression(matchee);
    gen(matchee, "eq");
}

void CodeGenerator::generateVarPattern(shared_ptr<VarPattern> pattern,
                                       shared_ptr<Expression> matchee,
                                        QMap<shared_ptr<AssignableExpression>, shared_ptr<Identifier> > &bindings)
{
    generateExpression(pattern->id());
    generateExpression(matchee);
    gen(matchee, "eq");
}

void CodeGenerator::generateAssignedVarPattern(shared_ptr<AssignedVarPattern> pattern,
                                               shared_ptr<Expression> matchee,
                                                QMap<shared_ptr<AssignableExpression>, shared_ptr<Identifier> > &bindings)
{
    /*
      <matchee>
      popl %tmp
      pushv true
    */
    QString tempVar = _asm.uniqueVariable();
    generateExpression(matchee);
    gen(matchee, QString("popl ") + tempVar);
    gen(pattern->lv(), "pushv true");
    bindings.insert(pattern->lv(),
                    shared_ptr<Identifier>(new Identifier(matchee->getPos(), tempVar)));
}

void CodeGenerator::generateArrayPattern(shared_ptr<ArrayPattern> pattern,
                                         shared_ptr<Expression> matchee,
                                          QMap<shared_ptr<AssignableExpression>, shared_ptr<Identifier> > &bindings)
{
    QString arrVar = _asm.uniqueVariable();
    // todo: for why this is ugly please refer to the comment
    // about defineInCurrentScope() in generateMatchOperation()
    defineInCurrentScopeNoSource(arrVar);
    // todo: this leaks
    shared_ptr<VarAccess> arrVarExpr= varOf(pattern->getPos(), arrVar);
    QString exit = _asm.uniqueLabel();
    QString dummy = _asm.uniqueVariable();
    QString ok, no, goon;
    generateExpression(matchee);
    gen(matchee, QString("popl %1").arg(arrVar));
    gen(matchee, QString("pushl %1").arg(arrVar));
    gen(pattern, _ws(L"isa %1").arg(VMId::get(RId::VArray)));
    ok = _asm.uniqueLabel();
    no = _asm.uniqueLabel();
    goon = _asm.uniqueLabel();
    gen(pattern, QString("if %1,%2").arg(ok).arg(no));
    gen(pattern, QString("%1:").arg(ok));
    gen(pattern, "pushv true");
    gen(pattern, QString("jmp %1").arg(goon));
    gen(pattern, QString("%1:").arg(no));
    gen(pattern, "pushv false");
    gen(pattern, QString("jmp %1").arg(exit));
    gen(pattern, QString("%1:").arg(goon));

    if(pattern->fixedLength())
    {
        // If we reached here, we have a true on the stack
        // therefore we pop it
        gen(pattern, QString("popl ") + dummy);
        gen(pattern, "pushv ", pattern->elementCount());
        gen(pattern, QString("pushl ") + arrVar);
        gen(pattern, "arrlength");
        gen(pattern, "eq");

        ok = _asm.uniqueLabel();
        no = _asm.uniqueLabel();
        goon = _asm.uniqueLabel();
        gen(pattern, QString("if %1,%2").arg(ok).arg(no));
        gen(pattern, QString("%1:").arg(ok));
        gen(pattern, "pushv true");
        gen(pattern, QString("jmp %1").arg(goon));
        gen(pattern, QString("%1:").arg(no));
        gen(pattern, "pushv false");
        gen(pattern, QString("jmp %1").arg(exit));
        gen(pattern, QString("%1:").arg(goon));
    }
    for(int i=0; i<pattern->elementCount(); i++)
    {
        // If we reached here, we have a true on the stack
        // therefore we pop it
        gen(pattern, QString("popl ") + dummy);

        // todo: this leaks also
        shared_ptr<NumLiteral> idx(new NumLiteral(pattern->element(i)->getPos(), QString("%1").arg(i+1)));
        shared_ptr<Expression> expr(new ArrayIndex(pattern->element(i)->getPos(), arrVarExpr, idx));
        generatePattern(pattern->element(i), expr, bindings);

        ok = _asm.uniqueLabel();
        no = _asm.uniqueLabel();
        goon = _asm.uniqueLabel();
        gen(pattern, QString("if %1,%2").arg(ok).arg(no));
        gen(pattern, QString("%1:").arg(ok));
        gen(pattern, "pushv true");
        gen(pattern, QString("jmp %1").arg(goon));
        gen(pattern, QString("%1:").arg(no));
        gen(pattern, "pushv false");
        gen(pattern, QString("jmp %1").arg(exit));
        gen(pattern, QString("%1:").arg(goon));
    }
    gen(pattern, QString("%1:").arg(exit));
}

void CodeGenerator::generateObjPattern(shared_ptr<ObjPattern> pattern,
                                       shared_ptr<Expression> matchee,
                                        QMap<shared_ptr<AssignableExpression>, shared_ptr<Identifier> > &bindings)
{
    QString objVar = _asm.uniqueVariable();
    // todo: for why this is ugly please refer to the comment
    // about defineInCurrentScope() in generateMatchOperation()
    defineInCurrentScopeNoSource(objVar);
    // todo: this leaks
    shared_ptr<Expression> objVarExpr = varOf(pattern->getPos(), objVar);
    QString exit = _asm.uniqueLabel();
    QString dummy = _asm.uniqueVariable();
    QString ok, no, goon;
    generateExpression(matchee);
    gen(matchee, QString("popl %1").arg(objVar));
    gen(matchee, QString("pushl %1").arg(objVar));
    gen(pattern, _ws(L"isa %1").arg(pattern->classId()->name()));
    ok = _asm.uniqueLabel();
    no = _asm.uniqueLabel();
    goon = _asm.uniqueLabel();
    gen(pattern, QString("if %1,%2").arg(ok).arg(no));
    gen(pattern, QString("%1:").arg(ok));
    gen(pattern, "pushv true");
    gen(pattern, QString("jmp %1").arg(goon));
    gen(pattern, QString("%1:").arg(no));
    gen(pattern, "pushv false");
    gen(pattern, QString("jmp %1").arg(exit));
    gen(pattern, QString("%1:").arg(goon));


    for(int i=0; i<pattern->fieldNameCount(); i++)
    {
        // If we reached here, we have a true on the stack
        // therefore we pop it
        gen(pattern, QString("popl ") + dummy);

        // todo: this leaks also
        shared_ptr<Expression> expr(new Idafa(pattern->fieldName(i)->getPos(),
                                                  pattern->fieldName(i),
                                                    objVarExpr));
        generatePattern(pattern->fieldPattern(i), expr, bindings);

        ok = _asm.uniqueLabel();
        no = _asm.uniqueLabel();
        goon = _asm.uniqueLabel();
        gen(pattern, QString("if %1,%2").arg(ok).arg(no));
        gen(pattern, QString("%1:").arg(ok));
        gen(pattern, "pushv true");
        gen(pattern, QString("jmp %1").arg(goon));
        gen(pattern, QString("%1:").arg(no));
        gen(pattern, "pushv false");
        gen(pattern, QString("jmp %1").arg(exit));
        gen(pattern, QString("%1:").arg(goon));
    }
    gen(pattern, QString("%1:").arg(exit));
}

void CodeGenerator::generateMapPattern(shared_ptr<MapPattern> pattern,
                                       shared_ptr<Expression> matchee,
                                        QMap<shared_ptr<AssignableExpression>, shared_ptr<Identifier> > &bindings)
{
    QString mapVar = _asm.uniqueVariable();
    // todo: for why this is ugly please refer to the comment
    // about defineInCurrentScope() in generateMatchOperation()
    defineInCurrentScopeNoSource(mapVar);
    // todo: this leaks
    shared_ptr<VarAccess> mapVarExpr = varOf(pattern->getPos(), mapVar);
    QString exit = _asm.uniqueLabel();
    QString dummy = _asm.uniqueVariable();
    QString ok, no, goon;
    generateExpression(matchee);
    gen(matchee, QString("popl %1").arg(mapVar));
    gen(matchee, QString("pushl %1").arg(mapVar));
    gen(pattern, _ws(L"isa %1").arg(VMId::get(RId::VMap)));
    ok = _asm.uniqueLabel();
    no = _asm.uniqueLabel();
    goon = _asm.uniqueLabel();
    gen(pattern, QString("if %1,%2").arg(ok).arg(no));
    gen(pattern, QString("%1:").arg(ok));
    gen(pattern, "pushv true");
    gen(pattern, QString("jmp %1").arg(goon));
    gen(pattern, QString("%1:").arg(no));
    gen(pattern, "pushv false");
    gen(pattern, QString("jmp %1").arg(exit));
    gen(pattern, QString("%1:").arg(goon));


    QVector<QString> evaluatedKeys;

    for(int i=0; i<pattern->keyCount(); i++)
    {
        // If we reached here, we have a true on the stack
        // therefore we pop it
        gen(pattern, QString("popl ") + dummy);

        generateExpression(pattern->key(i));
        QString keyI = _asm.uniqueVariable();
        evaluatedKeys.append(keyI);
        //todo: ugly ugly ugly
        // to see why please refer to the comment
        // about defineInCurrentScope() in generateMatchOperation()
        defineInCurrentScopeNoSource(keyI);
        gen(pattern->key(i), QString("popl ")+ keyI);
        gen(pattern->key(i), QString("pushl ")+ keyI);
        gen(pattern, QString("pushl ")+ mapVar);
        gen(pattern->key(i), "callex haskey");


        ok = _asm.uniqueLabel();
        no = _asm.uniqueLabel();
        goon = _asm.uniqueLabel();
        gen(pattern, QString("if %1,%2").arg(ok).arg(no));
        gen(pattern, QString("%1:").arg(ok));
        gen(pattern, "pushv true");
        gen(pattern, QString("jmp %1").arg(goon));
        gen(pattern, QString("%1:").arg(no));
        gen(pattern, "pushv false");
        gen(pattern, QString("jmp %1").arg(exit));
        gen(pattern, QString("%1:").arg(goon));
    }

    for(int i=0; i<pattern->keyCount(); i++)
    {
        // If we reached here, we have a true on the stack
        // therefore we pop it
        gen(pattern, QString("popl ") + dummy);

        // todo: this leaks also
        shared_ptr<Expression> idx = varOf(pattern->key(i)->getPos(), evaluatedKeys[i]);
        shared_ptr<Expression> expr(new ArrayIndex(pattern->key(i)->getPos(), mapVarExpr, idx));
        generatePattern(pattern->value(i), expr, bindings);

        ok = _asm.uniqueLabel();
        no = _asm.uniqueLabel();
        goon = _asm.uniqueLabel();
        gen(pattern, QString("if %1,%2").arg(ok).arg(no));
        gen(pattern, QString("%1:").arg(ok));
        gen(pattern, "pushv true");
        gen(pattern, QString("jmp %1").arg(goon));
        gen(pattern, QString("%1:").arg(no));
        gen(pattern, "pushv false");
        gen(pattern, QString("jmp %1").arg(exit));
        gen(pattern, QString("%1:").arg(goon));
    }
    gen(pattern, QString("%1:").arg(exit));
}

void CodeGenerator::generateUnaryOperation(shared_ptr<UnaryOperation> expr)
{
    generateExpression(expr->operand());
    gen(expr, expr->operator_());
}

void CodeGenerator::generateIdentifier(shared_ptr<Identifier> expr)
{
    if(!scopeStack.empty() && scopeStack.top().bindings.contains(expr->name()))
    {
        //todo: this is a hack
        if(scopeStack.top().declarationOf.contains(expr->name()))
        {
            shared_ptr<Identifier> definition = scopeStack.top().declarationOf[expr->name()];
            varInfos[expr->getPos().Pos] = varInfos[definition->getPos().Pos];
        }
        gen(expr, "pushl "+expr->name());
    }
    else if(declaredGlobalVariables.contains(expr->name()))
        gen(expr, "pushg "+expr->name());
    else
        error(CompilerException(currentFileName,expr, UndefinedVariable).arg(expr->name()));
}

void CodeGenerator::generateNumLiteral(shared_ptr<NumLiteral> expr)
{
    if(!expr->valueRecognized())
    {
        error(CompilerException(currentFileName,expr, UnacceptableNumberLiteral).arg(expr->toString()));
    }
    if(expr->longNotDouble())
        gen(expr, "pushv "+ QString("%1").arg(expr->lValue()));
    else
        gen(expr, "pushv "+ QString("%1").arg(expr->dValue()));
}

void CodeGenerator::generateStrLiteral(shared_ptr<StrLiteral> expr)
{
    generateStringConstant(expr, expr->value());
}

void CodeGenerator::generateNullLiteral(shared_ptr<NullLiteral> expr)
{
    gen(expr, "pushnull");
}

void CodeGenerator::generateBoolLiteral(shared_ptr<BoolLiteral> expr)
{
    if(expr->value())
        gen(expr, "pushv true");
    else
        gen(expr, "pushv false");
}

void CodeGenerator::generateArrayLiteral(shared_ptr<ArrayLiteral> expr)
{
    QString newVar = generateArrayFromValues(expr, expr->getData());
}

void CodeGenerator::generateMapLiteral(shared_ptr<MapLiteral> expr)
{
    QString newVar = _asm.uniqueVariable();

    gen(expr, QString::fromStdWString(L"callex newmap"));
    gen(expr, "popl "+ newVar);

    for(int i=0; i<expr->dataCount(); i+=2)
    {
       shared_ptr<Expression> key = expr->data(i);
       shared_ptr<Expression> value = expr->data(i+1);

       gen(value, "pushl "+ newVar);
       generateExpression(key);
       generateExpression(value);
       gen(key, "setarr");
    }
    gen(expr, "pushl " + newVar);
}


void CodeGenerator::generateInvokation(shared_ptr<Invokation> expr,
                                       InvokationContext context,
                                       MethodCallStyle style)
{
    shared_ptr<Identifier> functor = dynamic_pointer_cast<Identifier>(expr->functor());
    QString procName = functor->name();

    if(context == FunctionInvokationContext && this->allProcedures.contains(procName))
    {
        error(CompilerException(currentFileName,expr, CannotCallProcedureInExpression1).arg(procName));
    }

    for(int i=expr->argumentCount()-1; i>=0; i--)
    {
        generateExpression(expr->argument(i));
    }
    if(style == TailCallStyle)
    {
        gen(expr->functor(), "tail");
    }
    if(style == LaunchProcessStyle)
    {
        gen(expr->functor(), "launch");
    }
    gen(expr->functor(), QString("call %1,%2").arg(procName).arg(expr->argumentCount()));

}

void CodeGenerator::generateMethodInvokation(shared_ptr<MethodInvokation> expr,
                                             InvokationContext context, MethodCallStyle style)
{
    // We cannot statically check if the method is a function or proc
    // So there's really nothing we can do about the context here
    // we'll just let the virtual machine handle it. The VM will find an empty stack and signal
    // internal error, if it's lucky, otherwise crash or behave weirdly :(
    // so todo: we should find a better way
    // I'm actually tempted to go all pythonic and make everything functions that return null
    // by default
    for(int i=expr->argumentCount()-1; i>=0; i--)
    {
        generateExpression(expr->argument(i));
    }
    generateExpression(expr->receiver());
    if(style == TailCallStyle)
    {
        gen(expr->methodSelector(), "tail");
    }
    if(style == LaunchProcessStyle)
    {
        gen(expr->methodSelector(), "launch");
    }
    gen(expr->methodSelector(), QString("callm %1,%2").arg(expr->methodSelector()->name()).arg(expr->argumentCount()+1));
}

void CodeGenerator::generateTimingExpression(shared_ptr<TimingExpression> expr)
{
    QString dummy = _asm.uniqueVariable();
    gen(expr, "tick");
    generateExpression(expr->toTime());
    gen(expr, "popl " + dummy);
    gen(expr, "tick");
    gen(expr, "sub");
    gen(expr, "neg");
}

void CodeGenerator::generateTheSomething(shared_ptr<TheSomething> expr)
{

    shared_ptr<ProceduralDecl> proc;
    QVector<shared_ptr<Statement> > procCallStmts;

    if(expr->what() == Proc)

    {
        if(!allProcedures.contains(expr->name()))
            error(CompilerException(currentFileName, expr, ProcedureOrFunctionDoesntExist).arg(expr->name()));
        proc = allProcedures[expr->name()];
    }
    else if(expr->what() == Function)
    {
        if(!allFunctions.contains(expr->name()))
            error(CompilerException(currentFileName, expr, ProcedureOrFunctionDoesntExist).arg(expr->name()));
        proc = allFunctions[expr->name()];
    }

    QVector<shared_ptr<Expression> > args;
    for(int i=0; i<proc->formals.count(); ++i)
    {
        args.append(shared_ptr<VarAccess>(new VarAccess(expr->getPos(), proc->formal(i)->name())));
    }

    if(expr->what() == Proc)
    {
        procCallStmts.append(shared_ptr<Statement>(new InvokationStmt(expr->getPos(),
                                                                      shared_ptr<Invokation>(new Invokation(expr->getPos(),

                                                                                                idOf(expr->getPos(),expr->name()),
                                                                                args)))));
    }
    else if(expr->what() == Function)
    {
        procCallStmts.append(shared_ptr<Statement>(new ReturnStmt(expr->getPos(),
                                                                      shared_ptr<Invokation>(new Invokation(expr->getPos(),

                                                                                                idOf(expr->getPos(),expr->name()),
                                                                                args)))));
    }
    shared_ptr<LambdaExpression> lambda(new LambdaExpression(expr->getPos(),
                                                             proc->formals,
                                                             procCallStmts,
                                                             expr->what() == Proc
                                                             ));

    generateLambdaExpression(lambda);
    //gen(expr, "pushc " + expr->name());
}


void CodeGenerator::generateIdafa(shared_ptr<Idafa> expr)
{
    generateExpression(expr->modaf_elaih());
    gen(expr->modaf(), "getfld "+expr->modaf()->name());
}

void CodeGenerator::generateArrayIndex(shared_ptr<ArrayIndex> expr)
{
    generateExpression(expr->array());
    generateExpression(expr->index());
    gen(expr, "getarr");
}

void CodeGenerator::generateMultiDimensionalArrayIndex(shared_ptr<MultiDimensionalArrayIndex> expr)
{
    generateExpression(expr->array());
    generateArrayFromValues(expr, expr->indexes());
    gen(expr, "getmdarr");
}

QString CodeGenerator::generateArrayFromValues(shared_ptr<AST> src,
                                               QVector<shared_ptr<Expression> >values)
{
    QString newVar = _asm.uniqueVariable();
    gen(src, "pushv ", values.count());
    gen(src, "newarr");
    gen(src, "popl "+ newVar);
    // Notice that Kalimat arrays are one-based, not zero-based
    for(int i=1; i<=values.count(); i++)
    {
       shared_ptr<Expression> value = values[i-1];
       gen(value, "pushl "+ newVar);
       gen(value, "pushv ", i);
       generateExpression(value);
       gen(value, "setarr");
    }
    gen(src,"pushl " + newVar);
    return newVar;
}

void CodeGenerator::generateObjectCreation(shared_ptr<ObjectCreation> expr)
{

    gen(expr, "new "+expr->className()->name());
    if(expr->fieldInitNameCount() > 0)
    {
        QString v = _asm.uniqueVariable();
        gen(expr, "popl " + v);
        for(int i=0; i<expr->fieldInitNameCount(); i++)
        {
            shared_ptr<Identifier> name = expr->fieldInitName(i);
            shared_ptr<Expression> value = expr->fieldInitValue(i);
            gen(value, "pushl " + v);
            generateExpression(value);
            gen(name, "setfld " + name->name());
        }
        gen(expr, "pushl " + v);
    }

}

class FreeVarAnalysisTraverser : public Traverser
{
    CodeGenerator &gen;
    QStack<shared_ptr<LambdaExpression> > lambdaStack;
public:
    FreeVarAnalysisTraverser(CodeGenerator &gen): gen(gen)
    {

    }
    virtual void visit(shared_ptr<PrettyPrintable> tree)
    {
        shared_ptr<LambdaExpression> le = dynamic_pointer_cast<LambdaExpression>(tree);
        if(le)
        {
            lambdaStack.push(le);
            le->freeVariables.clear();
            return;
        }

        shared_ptr<VarAccess> id = dynamic_pointer_cast<VarAccess>(tree);
        if(id)
        {
            if(!lambdaStack.empty() && !lookup(id->name(), lambdaStack.top()))
            {
                // We have a lambda with a free variable
                lambdaStack.top()->addFreeVariable(id->name());
            }
            return;
        }

    }

    bool lookup(shared_ptr<Identifier> id, shared_ptr<LambdaExpression> le)
    {
        // If it's a reference to a global var, it's a bound variable
        if(gen.declaredGlobalVariables.contains(id->name()))
            return true;
        // othewise see if it's bound because it's a parameter
        // todo: we need to take all case where a local var is bound in a kalimat function
        // like loop var, assignment, ...etc
        //
        // also: we need to handle the case where a var is neither free but defined outside nor bound
        // i.e undefined var
        for(int i=0; i<le->argListCount(); ++i)
        {
            if(le->argList(i)->name()->name() == id->name())
                return true;
        }
        return false;
    }

    virtual void exit(shared_ptr<PrettyPrintable> tree)
    {
        shared_ptr<LambdaExpression> le = dynamic_pointer_cast<LambdaExpression>(tree);
        if(le)
        {
            lambdaStack.pop();
        }
    }
};

void CodeGenerator::generateLambdaExpression(shared_ptr<LambdaExpression> expr)
{

    QString name = _asm.uniqueLambda();
    shared_ptr<Identifier> procName(new Identifier(expr->getPos(), name));
    FreeVarAnalysisTraverser fvTraverser(*this);
    expr->traverse(expr, &fvTraverser);
    // Now we have the free vars of our lambda expression!
    shared_ptr<BlockStmt> body(new BlockStmt(expr->getPos(), expr->statements));

    if(false && expr->freeVariableCount() == 0)
    {
        if(!expr->hasDoToken())
        {
            shared_ptr<FunctionDecl> func(new FunctionDecl(expr->getPos(), false, expr->getPos(), procName, expr->_argList, body));
            extraDeclarations.append(func);
        }
        else
        {
            shared_ptr<ProcedureDecl> proc(new ProcedureDecl(expr->getPos(), false, expr->getPos(), procName, expr->_argList, body));
            extraDeclarations.append(proc);
        }
        gen(expr, "pushc " + name);
    }
    else
    {
        //throw CompilerException(currentFileName, expr, UnimplementedExpressionForm).arg("Lambda has some free variables");

        QMap<QString, MethodInfo > proto;
        proto[VMId::get(RId::Invoke)] = MethodInfo(1, true);

        QVector<shared_ptr<Identifier> > fields;
        for(int i=0; i<expr->freeVariableCount(); ++i)
        {
            fields.append(expr->freeVariable(i));
        }

        shared_ptr<ClassDecl> klass(new ClassDecl(expr->getPos(), false, shared_ptr<Identifier>(),
                                                  procName, fields, proto, QVector<shared_ptr<ClassInternalDecl> >(),
                                                  QMap<QString, shared_ptr<TypeExpression> >()));

        klass->_ancestorName = idOf(Token(), "%lambda");
        shared_ptr<VarAccess> recvName = varOf(Token(), "%this");
        shared_ptr<Identifier> execName(new Identifier(Token(), VMId::get(RId::Invoke)));
        for(int i=0; i< expr->freeVariableCount(); ++i)
        {
            shared_ptr<Idafa> fieldAccess(new Idafa(Token(), fields[i], recvName));
            shared_ptr<AssignmentStmt> stm(new AssignmentStmt(expr->getPos(),
                                                              varOf(expr->freeVariable(i)),
                                                              fieldAccess,
                                                              shared_ptr<TypeExpression>()));
            // todo: we should not mutate the parse tree here
            body->_statements.prepend(stm);
        }

        QVector<shared_ptr<FormalParam> > methodArgs;
        methodArgs = expr->_argList;

        shared_ptr<MethodDecl> decl(new MethodDecl(expr->getPos(), false, expr->getPos(),
                                                   execName, methodArgs, body, procName, recvName->name(), !expr->hasDoToken()));
        klass->insertMethod(VMId::get(RId::Invoke), decl);
        extraDeclarations.append(klass);

        // now finally generate the object to represent the closure
        QVector<shared_ptr<Expression> > fieldVals;
        for(int i=0; i< fields.count(); ++i)
            fieldVals.append(varOf(fields[i]));

        shared_ptr<ObjectCreation> oc(new ObjectCreation(expr->getPos(),
                                                         procName, fields, fieldVals));
        generateObjectCreation(oc);
    }

}

QString CodeGenerator::typeExpressionToAssemblyTypeId(shared_ptr<TypeExpression> expr)
{
    if(isa<TypeIdentifier>(expr))
    {
        return dynamic_pointer_cast<TypeIdentifier>(expr)->name();
    }
    else if(isa<PointerTypeExpression>(expr))
    {
        shared_ptr<PointerTypeExpression> p
                = dynamic_pointer_cast<PointerTypeExpression>(expr);
        return QString("*%1").arg(typeExpressionToAssemblyTypeId(p->pointeeType()));
    }
    else if(isa<FunctionTypeExpression>(expr))
    {
        shared_ptr<FunctionTypeExpression> f
                = dynamic_pointer_cast<FunctionTypeExpression>(expr);
        QStringList args;
        QString arrow;
        QString ret;
        for(int i=0; i<f->argTypes.count(); ++i)
        {
            args.append(typeExpressionToAssemblyTypeId(f->argTypes[i]));
        }
        if(f->retType())
        {
            arrow = "->";
            ret = typeExpressionToAssemblyTypeId(f->retType());
        }
        return QString ("^(%1)%2%3")
                .arg(args.join(","))
                .arg(arrow)
                .arg(ret);
    }
    else
    {
        error(CompilerException(currentFileName,expr, UnimplementedTypeForm1).arg(expr->toString()));
    }
    return "";
}

void CodeGenerator::generateStringConstant(shared_ptr<AST> src, QString str)
{
    QString constId = _asm.makeStringConstant(str);

    gen(src, "pushc "+ constId);
}

void CodeGenerator::gen(QString str)
{
    if(!str.startsWith("."))
        scopeStack.top().instructionCount++;
    _asm.gen(str);
}

void CodeGenerator::gen(QString str, int i)
{
    if(!str.startsWith("."))
        scopeStack.top().instructionCount++;
    _asm.gen(str, i);
}

void CodeGenerator::gen(QString str, double d)
{
    if(!str.startsWith("."))
        scopeStack.top().instructionCount++;
    _asm.gen(str, d);
}

void CodeGenerator::gen(shared_ptr<AST> src,QString str)
{
    CodePosition pos;
    pos.doc = currentCodeDoc;
    pos.pos = src->getPos().Pos;
    pos.ast = src;
    PositionInfo[codePosKeyCount] = pos;
    if(!str.startsWith("."))
        scopeStack.top().instructionCount++;
    _asm.genWithMetaData(codePosKeyCount, str);
    codePosKeyCount++;
}

void CodeGenerator::gen(shared_ptr<AST> src,QString str, int i)
{
    gen(src, QString(str)+" "+ QString("%1").arg(i));
}

void CodeGenerator::gen(shared_ptr<AST> src,QString str, double d)
{
    gen(src, QString(str)+" "+ QString("%1").arg(d));
}

QString CodeGenerator::getCurrentFunctionNameFormatted()
{
    QString fname = scopeStack.top().proc->procName()->name();
    if(fname == "%main")
        fname = VMId::get(RId::MainProgram); // todo: this is not the suitable place for this constant
    else
        fname = QString("'%1'").arg(fname);
    return fname;
}

QMap<CompilerError, QString> CompilerException::errorMap;

CompilerException::CompilerException(QString fileName, shared_ptr<AST> source, CompilerError error)
{
    this->fileName = fileName;
    this->error = error;
    this->source = source;
    this->message = translateErrorMessage(error);
}

CompilerException CompilerException::no_source(CompilerError error)
{
    return CompilerException("", shared_ptr<AST>(), error);
}

QString CompilerException::getMessage()
{
    QString ret = message;
    for(int i=0; i<args.count(); i++)
        ret = ret.arg(args.at(i));
    return ret;
}

QString CompilerException::translateErrorMessage(CompilerError error)
{
    if(errorMap.empty())
    {
        LineIterator in = Utils::readResourceTextFile(":/compiler_error_map.txt");
        int i=0;
        while(!in.atEnd())
        {
            QString val = in.readLine().trimmed();
            errorMap[(CompilerError) i++] = val;
        }
        in.close();
    }
    return errorMap[error];
}

CompilerException &CompilerException::arg(QString arg)
{
    args.append(arg);
    return *this;
}
