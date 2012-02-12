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

        data = base64encode(data);

        ret.append(QString(".strconst %1 %2").arg(sym).arg(data));
        ++i;
    }
    ret.append("");
    return ret.join("\n");
}

void CodeGenerator::defineInCurrentScope(QString var)
{
    if(!scopeStack.empty())
        scopeStack.top().bindings.insert(var);
}

bool CodeGenerator::currentScopeFuncNotProc()
{
    if(scopeStack.empty())
        throw CompilerException::no_source(InternalCompilerErrorInFunc).arg("currentScopeFuncNotProc");
    shared_ptr<ProceduralDecl> decl = scopeStack.top().proc;
    if(isa<ProceduralDecl>(decl))
        return false;
    else if(isa<FunctionDecl>(decl))
        return true;
    else if(isa<MethodDecl>(decl))
    {
        shared_ptr<MethodDecl> md = dynamic_pointer_cast<MethodDecl>(decl);
        return md->isFunctionNotProcedure;
    }
    else
    {
        throw CompilerException::no_source(InternalCompilerErrorInFunc).arg("currentScopeFuncNotProc");
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
    for(int i=0; i<program->elementCount(); i++)
    {
        shared_ptr<TopLevel> tl = program->element(i);
        if(isa<Declaration>(tl))
            declarations.append(dynamic_pointer_cast<Declaration>(tl));
        else if(isa<Statement>(tl))
            statements.append(dynamic_pointer_cast<Statement>(tl));
        else
            throw CompilerException(currentFileName,tl, ASTMustBeStatementOrDeclaration).arg(tl->toString());
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
}

void CodeGenerator::compileModule(shared_ptr<Module> module, QString fileName, CodeDocument *curDoc)
{
    // Copy-pasted from generate(Program *)
    currentCodeDoc = curDoc;
    currentFileName = fileName;
    QVector<shared_ptr<Declaration> > declarations;

 /*   for(int i=0; i<module->usedModuleCount(); i++)
    {
        loadModule(module->usedModule(i));
    }
    */
    for(int i=0; i<module->declCount(); i++)
    {
        shared_ptr<Declaration> decl = module->declPtr(i);
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
}

void CodeGenerator::firstPass(shared_ptr<Declaration> decl)
{
    if(isa<ClassDecl>(decl))
    {
        shared_ptr<ClassDecl> cd = dynamic_pointer_cast<ClassDecl>(decl);
        QString name = cd->name()->name;
        if(allClasses.contains(name))
            throw CompilerException(currentFileName,decl, ClassAlreadyExists).arg(name);
        else
            allClasses[name] = cd;
        return;
    }

    if(isa<ProcedureDecl>(decl))
    {
        shared_ptr<ProcedureDecl> proc =
                dynamic_pointer_cast<ProcedureDecl>(decl);
        allProcedures[proc->procName()->name] = proc;
    }
    if(isa<FunctionDecl>(decl))
    {
        shared_ptr<FunctionDecl> func = dynamic_pointer_cast<FunctionDecl>(decl);
        allFunctions[func->procName()->name] = func;
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
        if(cd->ancestor() != NULL)
        {
            QString ancestorName = cd->ancestor()->name;
            if(!allClasses.contains(ancestorName))
            {
                throw CompilerException(currentFileName,decl, AncestorClassXforClassYdoesntExist)
                                            .arg(ancestorName).arg(cd->name()->name);
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
        QString name = md->procName()->name;
        QString className = md->className()->name;
        if(!allClasses.contains(className))
            throw CompilerException(currentFileName,decl, MethodDefinedForNotYetExistingClass).arg(className);
        else
        {
            shared_ptr<ClassDecl> theClass = allClasses[className];
            if(theClass->containsMethod(name))
                throw CompilerException(currentFileName,decl, MethodCalledXwasAlreadyDefinedForClassY).arg(name).arg(className);
            if(!theClass->containsPrototype(name))
                throw CompilerException(currentFileName,decl, MethodXwasNotDeclaredInClassY).arg(name).arg(className);

            // We subtract the 1 to account for the extra 'this' parameter in md
            if(theClass->methodPrototype(name).arity != md->formalCount()-1)
                throw CompilerException(currentFileName,decl, MethodXwasDeclaredWithDifferentArityInClassY).arg(name).arg(className);
            if(theClass->methodPrototype(name).isFunction && !md->isFunctionNotProcedure)
                throw CompilerException(currentFileName,decl, MethodXwasDeclaredAfunctionButImplementedAsProcedureInClassY).arg(name).arg(className);
            if(!theClass->methodPrototype(name).isFunction && md->isFunctionNotProcedure)
                throw CompilerException(currentFileName,decl, MethodXwasDeclaredAprocedureButImplementedAsFunctionInClassY).arg(name).arg(className);
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
        if(pd->procName()->name == "%main")
            generateEntryPoint(pd->body()->getStatements());
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
        throw CompilerException(currentFileName,decl, DeclarationNotSupported).arg(decl->toString());
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
        defineInCurrentScope(pd->formal(i)->name);
}
void CodeGenerator::popProcedureScope()
{
    scopeStack.pop();
}

void CodeGenerator::generateProcedureDeclaration(shared_ptr<ProcedureDecl> decl)
{
    gen(decl, QString(".method %1 %2 0").arg(decl->procName()->name).arg(decl->formalCount()));
    for(int i=0; i<decl->formalCount(); i++)
    {
        gen(decl, "popl " + decl->formal(i)->name);
    }
    generateStatement(decl->body());
    debugInfo.setInstructionForLine(currentCodeDoc, decl->_endingToken.Line,
                                    decl->procName()->name, scopeStack.top().instructionCount);
    gen(decl, "ret");
    gen(decl,".endmethod");
}

void CodeGenerator::generateFunctionDeclaration(shared_ptr<FunctionDecl> decl)
{
    gen(decl, QString(".method %1 %2 1").arg(decl->procName()->name).arg(decl->formalCount()));
    for(int i=0; i<decl->formalCount(); i++)
    {
        gen(decl, "popl " + decl->formal(i)->name);
    }

    generateStatement(decl->body());
    debugInfo.setInstructionForLine(currentCodeDoc, decl->_endingToken.Line,
                                    decl->procName()->name, scopeStack.top().instructionCount);
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
            generateFFIProceduralDeclaration(dynamic_pointer_cast<FFIProceduralDecl>(d), decl->libName);
        }
        else if(isa<FFIStructDecl>(d))
        {
            generateFFIStructDeclaration(dynamic_pointer_cast<FFIStructDecl>(d));
        }
        else
        {
            throw CompilerException(currentFileName,decl, DeclarationNotSupported);
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
                                        decl->getPos(),
                                        shared_ptr<Identifier>(new Identifier(decl->getPos(),decl->procName)),
                                        QVector<shared_ptr<Identifier> >(),
                                        shared_ptr<BlockStmt>(new BlockStmt(decl->getPos(), QVector<shared_ptr<Statement > >())),
                                        false
                                        )));
    gen(decl, QString(".method %1 %2 %3").
                             arg(decl->procName)
                            .arg(decl->paramTypeCount())
                            .arg(decl->isFunctionNotProc?1:0));

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
    generateStringConstant(decl, decl->symbol);
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

    if(decl->isFunctionNotProc)
    {
        generateStringConstant(decl, typeExpressionToAssemblyTypeId(decl->returnType()));
        gen(decl, "callex typefromid");
    }
    else
    {
        gen(decl, QString::fromStdWString(L"pushc معدوم.سي"));
    }

    gen(decl, "pushl %argTypeArr");
    gen(decl, "pushl %argArr");
    gen(decl, "pushl %funcPtr");
    gen(decl, "callex callforeign");

    //TODO: see this debugInfo thing on the next line
    //debugInfo.setInstructionForLine(currentCodeDoc, decl->_endingToken.Line,
     //                               decl->procName()->name, scopeStack.top().instructionCount);
    if(!decl->isFunctionNotProc)
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

int Labeller::labelOf(QString str)
{
    if(!labelMap.contains(str))
    {
        labelMap[str] = count++;
    }
    if(labelMap[str] == 48)
    {
        int x = 5;
    }
    return labelMap[str];
}
shared_ptr<LabelStmt> labelOf(Token pos, QString lbl)
{
    return shared_ptr<LabelStmt>(new LabelStmt(pos,
                                               idOf(pos,lbl)));
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
    return shared_ptr<AssignmentStmt>(new AssignmentStmt(pos, lval, rval));
}

shared_ptr<AssignmentStmt> assignmentOf(shared_ptr<AssignableExpression> lval,
                                        shared_ptr<Expression> rval)
{
    return shared_ptr<AssignmentStmt>(new AssignmentStmt(lval->getPos(),
                                                         lval,
                                                         rval));
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
        if(ruleTable.contains(rule->ruleName))
            throw CompilerException(currentFileName,rule, RuleAlreadyDefined).arg(rule->ruleName);
        ruleTable[rule->ruleName] = rule;
    }
    Labeller labeller;
    QVector<shared_ptr<Statement> > stmts;
    Token pos0 = decl->getPos();
    // %parser = make.parser(%input)
    // %result = null -- to declare the variable
    // ...and they say Lisp isn't used anymore
    stmts.append(assignmentOf(pos0, idOf(pos0, _ws(L"%المعرب")),
                     invokationOf(pos0,_ws(L"صنع.معرب"), idOf(pos0, _ws(L"%المدخل")))));

    stmts.append(assignmentOf(pos0, idOf(pos0, _ws(L"%الموقع")),
                              shared_ptr<NumLiteral>(new NumLiteral(pos0,0))));

    stmts.append(assignmentOf(pos0, idOf(pos0, _ws(L"%النتيجة")),
                              shared_ptr<NullLiteral>(new NullLiteral(pos0))));


    // initialize the associated vars with null
    // to make them 'declared'
    QList<QString> identifiersForRules
            = decl->getAllAssociatedVars().toList();
    for(int k=0; k<identifiersForRules.count(); k++)
    {

        stmts.append(
                    assignmentOf(pos0,
                                 idOf(pos0, identifiersForRules[k]),
                                 shared_ptr<NullLiteral>(new NullLiteral(pos0))));
    }

    if(decl->subRuleCount() > 0)
    {
        // manage start rule:
        // we shall push a locals frame since each 'call' assumes
        // both return address and locals frames
        // %parser : pushLocals([])
        stmts.append(fromInvokation(methodOf(
                                        idOf(pos0, _ws(L"%المعرب")),
                                        _ws(L"ادفع.المتغيرات.المحلية"),
                                        shared_ptr<ArrayLiteral>(
                                            new ArrayLiteral(pos0,
                                                             QVector<shared_ptr<Expression> >())
                                            ))));
        // %parser : call(startRuleLabel, %endOfParsing)
        shared_ptr<RuleDecl> rule = decl->subRule(0);
        shared_ptr<NumLiteral> toCall =
                    numLitOf(pos0, labeller.labelOf(rule->ruleName));
        shared_ptr<NumLiteral> returnHere =
                    numLitOf(pos0, labeller.labelOf(_ws(L"نهاية.الإعراب")));
        stmts.append(gotoOf(pos0,
                          methodOf(idOf(pos0, _ws(L"%المعرب")),
                                   _ws(L"تفرع"),
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
        stmts.append(labelOf(rulePos, rule->ruleName, labeller));

        for(int j=0; j<rule->options.count(); j++)
        {
            shared_ptr<RuleOption> opt = rule->options[j];
            Token optPos = opt->getPos();
            // a label for each option
            stmts.append(
                        labelOf(optPos, QString("%1%%2").arg(rule->ruleName).arg(j), labeller)
                        );
            if((j+1) < rule->options.count())
            {
                stmts.append(fromInvokation(methodOf(idOf(optPos, _ws(L"%المعرب")),
                                      _ws(L"ادفع.مسار.بديل"),
                                      numLitOf(optPos, labeller.labelOf(QString("%1%%2")
                                                     .arg(rule->ruleName).arg(j+1))))));
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
                stmts.append(assignmentOf(pos1, idOf(pos1, _ws(L"%النتيجة")), opt->resultExpr()));
            }
            else
            {
                Token pos1 = opt->expression()->getPos();
                stmts.append(assignmentOf(pos1, idOf(pos1, _ws(L"%النتيجة")),
                                          shared_ptr<NullLiteral>(
                                              new NullLiteral(pos1))));
            }
            // if we've succeded, ignore the most recent backtrack point
            if((j+1) < rule->options.count())
            {
                stmts.append(fromInvokation(methodOf(idOf(optPos, _ws(L"%المعرب")),
                                      _ws(L"تجاهل.آخر.مسار.بديل"))));
                stmts.append(gotoOf(optPos, _ws(L"%نجاح.%1").arg(rule->ruleName), labeller)
                        );
            }
        } // for each ruleOption

        stmts.append(
                    labelOf(rulePos, _ws(L"%نجاح.%1").arg(rule->ruleName), labeller)
                    );
        // at the end of each rule:
        // first, memoize:
        // %parser: remember("ruleName", %pos, %result)
        stmts.append(fromInvokation(methodOf(idOf(rulePos, _ws(L"%المعرب")),
                                             _ws(L"تذكر"),
                                             numLitOf(rulePos, labeller.labelOf(rule->ruleName)),
                                             idOf(rulePos, _ws(L"%الموقع")),
                                             fieldAccessOf(idOf(rulePos, _ws(L"%المعرب")), _ws(L"موقع")),
                                             idOf(rulePos, _ws(L"%النتيجة")))));
        // then return
        // goto %parser: ret()
        stmts.append(gotoOf(rulePos, methodOf(idOf(rulePos, _ws(L"%المعرب")), _ws(L"عد"))));
    } // for each rule

    // a label %endOfParsing for the start rule to return to
    stmts.append(labelOf(pos0, _ws(L"نهاية.الإعراب"), labeller));
    // pop dummy 'locals' frame that was first pushed
    stmts.append(assignmentOf(pos0,
                               idOf(pos0, _ws(L"%اطار.مؤقت")),
                                methodOf(idOf(pos0, _ws(L"%المعرب")),
                           _ws(L"ارفع.المتغيرات.المحلية"))));
    // now return our precious result :D

    shared_ptr<ReturnStmt> returnNow(new ReturnStmt(pos0,
                                                    idOf(pos0, _ws(L"%النتيجة"))));
    stmts.append(returnNow);

    // and have a label to whom VM routines will jump on error
    stmts.append(labelOf(pos0, _ws(L"خطأ.في.الإعراب")));
    shared_ptr<ReturnStmt> returnErr(new ReturnStmt(pos0,
                                                    shared_ptr<Expression>(new NullLiteral(pos0))));
    stmts.append(returnErr);
    shared_ptr<BlockStmt> body(
            new BlockStmt(decl->getPos(), stmts));
    QVector<shared_ptr<Identifier> > formals;
    formals.append(idOf(Token(), _ws(L"%المدخل")));
    shared_ptr<FunctionDecl> func(
            new FunctionDecl(decl->getPos(), decl->getPos(),
                             decl->name(), formals, body, true));
    QFile f("pargen_debug.txt");
    f.open(QFile::Text | QFile::WriteOnly | QFile::Truncate);
    QTextStream out(&f);
    out.setCodec("UTF-8");
    out.setGenerateByteOrderMark(true);
    SimpleCodeFormatter fmt;
    func->prettyPrint(&fmt);
    out << fmt.getOutput();
    f.close();
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
    elems.append(idOf(pos0, _ws(L"%الموقع")));
    for(int i=0; i<locals.count(); i++)
    {
        elems.append(idOf(pos0, locals[i]));
    }
    shared_ptr<ArrayLiteral> arr(
                new ArrayLiteral(pos0, elems));

    result.append(fromInvokation(methodOf(idOf(pos0, _ws(L"%المعرب")),
                           _ws(L"ادفع.المتغيرات.المحلية"),
                           arr)));
    // done save locals
    // %pos = pos of %parser
    result.append(assignmentOf(idOf(pos0, _ws(L"%الموقع")),
                               fieldAccessOf(idOf(pos0, _ws(L"%المعرب")),
                                             _ws(L"موقع"))));
    //now invoke the rule
    QString continuationLabel = _asm.uniqueLabel();
    shared_ptr<NumLiteral> toCall(
                numLitOf(pos0, labeller.labelOf(rule->ruleName()->name)));
    shared_ptr<NumLiteral> returnHere =
                numLitOf(pos0, labeller.labelOf(continuationLabel));

    result.append(gotoOf(pos0,
                      methodOf(idOf(pos0, _ws(L"%المعرب")),
                               _ws(L"تفرع"),
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
                               idOf(pos0, _ws(L"%اطار.مؤقت")),
                                methodOf(idOf(pos0, _ws(L"%المعرب")),
                           _ws(L"ارفع.المتغيرات.المحلية"))));
    locals.prepend(_ws(L"%الموقع"));
    for(int i=0;i<locals.count(); i++)
    {
        shared_ptr<NumLiteral> idx(
                    new NumLiteral(pos0, i+1));
        shared_ptr<ArrayIndex> arrAccess(
                    new ArrayIndex(pos0,
                                   idOf(pos0,_ws(L"%اطار.مؤقت")),
                                   idx));
        result.append(assignmentOf(pos0,
                                   idOf(pos0, locals[i]),
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
        if(!ruleTable.contains(rule->ruleName()->name))
        {
            throw CompilerException(currentFileName,rule, InvokingUndefinedRule).arg(rule->ruleName()->name);
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
                    methodOf(idOf(pos0, _ws(L"%المعرب")),
                             _ws(L"هل.تذكر"),
                             numLitOf(pos0, labeller.labelOf(rule->ruleName()->name)),
                             fieldAccessOf(idOf(pos0, _ws(L"%المعرب")),
                                           _ws(L"موقع"))));

        QVector<shared_ptr<Statement> > assignStmts;
        shared_ptr<Identifier> tmpId = idOf(pos0, _ws(L"%مؤقت"));
        shared_ptr<Identifier> parserId = idOf(pos0, _ws(L"%المعرب"));
        assignStmts.append(assignmentOf(tmpId,
                                        methodOf(parserId,
                                                 _ws(L"استرد.ذكرى"),
                                                 numLitOf(pos0, labeller.labelOf(rule->ruleName()->name)),
                                                 fieldAccessOf(idOf(pos0, _ws(L"%المعرب")),
                                                                _ws(L"موقع"))
                                                 )));
        assignStmts.append(assignmentOf(fieldAccessOf(parserId, _ws(L"موقع")),
                                        fieldAccessOf(tmpId, _ws(L"موقع"))));

        assignStmts.append(assignmentOf(idOf(pos0, _ws(L"%النتيجة")),
                                        fieldAccessOf(tmpId, _ws(L"نتيجة"))));



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
                                       rule->associatedVar(),
                                       idOf(pos1, _ws(L"%النتيجة"))));
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
        bool multiChar = lit->value()->value.count() !=1;
        int charCount = lit->value()->value.count();
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
                        methodOf(idOf(pos1, _ws(L"%المعرب")),
                                 _ws(L"انظر.عديد"),
                                 shared_ptr<Expression>(new NumLiteral(pos1,
                                                                       charCount))));
            progressCall = methodOf(idOf(pos0,_ws(L"%المعرب")),
                                    _ws(L"تقدم.عديد"),
                                    shared_ptr<Expression>(new NumLiteral(pos1,
                                                                          charCount)));
            conditionCall = methodOf(idOf(pos0, _ws(L"%المعرب")),
                     _ws(L"يطل.على.عديد"),
                     lit->value());
        }
        else
        {
            lookAheadCall = (
                        methodOf(idOf(pos1, _ws(L"%المعرب")),
                                 _ws(L"انظر")));
            progressCall = methodOf(idOf(pos0,_ws(L"%المعرب")),
                                    _ws(L"تقدم"));
            conditionCall = methodOf(idOf(pos0, _ws(L"%المعرب")),
                     _ws(L"يطل.على"),lit->value());
        }
        if(lit->associatedVar())
        {
            thenStmts.append(assignmentOf(pos1,
                                       lit->associatedVar(),
                                          lookAheadCall));
        }
        thenStmts.append(fromInvokation(progressCall));
        elseStmts.append(gotoOf(pos0, methodOf(idOf(pos0,_ws(L"%المعرب")),
                                        _ws(L"الجأ.لبديل"))));
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
    gen(decl, ".class "+decl->name()->name);
    if(decl->ancestor() != NULL)
    {
        QString parent = decl->ancestor()->name;
        gen(decl->ancestor(), ".extends "+ parent);
    }
   /* if(decl->prototypeCount() > decl->methodCount())
        throw CompilerException(currentFileName,decl, QString("Class '%1' has some unimplemented methods").arg(decl->name()->name));
        */
    for(int i=0;i<decl->fieldCount();i++)
    {
        QString fieldName = decl->field(i)->name;
        QString attrs = "";
        if(decl->fieldHasMarshalType(fieldName))
        {
            attrs = QString(" marshalas=%1").arg(typeExpressionToAssemblyTypeId(decl->marshalTypeOf(fieldName)));
        }
        gen(decl,".field " + fieldName + attrs);
    }
    for(int i=0; i<decl->methodCount(); i++)
    {
        shared_ptr<MethodDecl> method = decl->method(i);
        pushProcedureScope(method);
        generateMethodDeclaration(method);
        popProcedureScope();
    }
    gen(decl,".endclass");
}
void CodeGenerator::generateGlobalDeclaration(shared_ptr<GlobalDecl> decl)
{
    this->declaredGlobalVariables.insert(decl->varName);
}

void CodeGenerator::generateMethodDeclaration(shared_ptr<MethodDecl> decl)
{
    QString name = decl->procName()->name;
    int numRet = decl->isFunctionNotProcedure? 1: 0;
    gen(decl, QString(".method %1 %2 %3").arg(name).arg(decl->formalCount()).arg(numRet));
    for(int i=0; i<decl->formalCount(); i++)
    {
        gen(decl, "popl " + decl->formal(i)->name);
    }
    generateStatement(decl->body());
    debugInfo.setInstructionForLine(currentCodeDoc, decl->_endingToken.Line,
                                    decl->procName()->name, scopeStack.top().instructionCount);
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
    QString procName = scopeStack.top().proc->procName()->name;
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
        throw new CompilerException(currentFileName, stmt, UnimplementedStatementForm);
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
        if(!stmt->_widths[i])
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
            generateExpression(stmt->_widths[i]);
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
    if(!stmt->printOnSameLine)
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
        if(stmt->prompt != NULL)
            throw CompilerException(currentFileName,stmt, ReadFromCannotContainAPrompt);
        if(stmt->variableCount() !=1)
            throw CompilerException(currentFileName,stmt, ReadFromCanReadOnlyOneVariable);
        shared_ptr<AssignableExpression> lvalue = stmt->variable(0);

        // We first read the data into a temporary variable,
        // then we generate the equivalent of a hidden assignment statement that assigns
        // the value of the temp. variable to the lvalue that we wanted to read.
        QString readVar = _asm.uniqueVariable();
        defineInCurrentScope(readVar);
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
    if(stmt->prompt !=NULL)
    {
        generateStringConstant(stmt, stmt->prompt);
        gen(stmt, "callex print");
    }

    for(int i=0; i<stmt->variableCount(); i++)
    {
        shared_ptr<AssignableExpression> lvalue = stmt->variable(i);
        QString readVar = _asm.uniqueVariable();
        defineInCurrentScope(readVar);

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
    gen(stmt, "launch");
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
    if(stmt->type == KalimatKeyDownEvent)
        type = "keydown";
    else if(stmt->type == KalimatKeyUpEvent)
        type = "keyup";
    else if(stmt->type == KalimatKeyPressEvent)
        type = "keypress";
    else if(stmt->type == KalimatMouseDownEvent)
        type = "mousedown";
    else if(stmt->type == KalimatMouseUpEvent)
        type = "mouseup";
    else if(stmt->type == KalimatMouseMoveEvent)
        type = "mousemove";
    else if(stmt->type == KalimatSpriteCollisionEvent)
        type = "collision";
    gen(stmt, QString("regev %1,%2").arg(type).arg(stmt->handler()->name));
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
}

void CodeGenerator::generateAssignmentToLvalue(shared_ptr<AST> src, shared_ptr<AssignableExpression> lval,
                                               Thunk &genValue)
{
    if(isa<Identifier>(lval))
    {
        shared_ptr<Identifier> variable = dynamic_pointer_cast<Identifier>(lval);
        genValue();
        if(declaredGlobalVariables.contains(variable->name))
            gen(src, "popg "+variable->name);
        else
        {
            defineInCurrentScope(variable->name);
            gen(src, "popl "+variable->name);
        }
    }
    else if(isa<Idafa>(lval))
    {
        shared_ptr<Idafa> fieldAccess = dynamic_pointer_cast<Idafa>(lval);
        generateExpression(fieldAccess->modaf_elaih());
        genValue();
        gen(src, "setfld "+ fieldAccess->modaf()->name);
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
        throw CompilerException(currentFileName,src, LValueFormNotImplemented).arg(lval->toString());
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
      forall(var, from, to)
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
      pushv 1
      add
      popl var
      jmp label0
      label2:
    */

    QString testLabel = _asm.uniqueLabel();
    QString doLabel = _asm.uniqueLabel();
    QString endLabel = _asm.uniqueLabel();

    generateExpression(stmt->from());
    gen(stmt->variable(), "popl " + stmt->variable()->name);
    gen(stmt, testLabel+":");
    gen(stmt->variable(), "pushl "+ stmt->variable()->name);
    generateExpression(stmt->to());
    gen(stmt, "gt");
    gen(stmt, "if "+endLabel+","+doLabel);
    gen(stmt, doLabel+":");

    defineInCurrentScope(stmt->variable()->name);

    generateStatement(stmt->statement());
    gen(stmt, "pushl "+ stmt->variable()->name);
    gen(stmt, "pushv 1");
    gen(stmt, "add");
    gen(stmt, "popl "+ stmt->variable()->name);
    gen(stmt, "jmp "+testLabel);
    gen(stmt, endLabel+":");
}

void CodeGenerator::generateLabelStmt(shared_ptr<LabelStmt> stmt)
{
    shared_ptr<Expression> target = stmt->target();
    QString labelName = target->toString();
    if(scopeStack.top().labels.contains(labelName))
    {
        throw CompilerException(currentFileName,stmt, DuplicateLabel).arg(labelName).arg(getCurrentFunctionNameFormatted());
    }
    scopeStack.top().labels.insert(labelName);

    if(isa<NumLiteral>(target))
    {
        gen(stmt, labelName + ": ");
    }
    else if(isa<Identifier>(target))
    {
        gen(stmt, labelName + ": ");
    }
    else
    {
        throw CompilerException(currentFileName,stmt, TargetOfLabelMustBeNumberOrIdentifier);
    }
}

void CodeGenerator::generateGotoStmt(shared_ptr<GotoStmt> stmt)
{
    shared_ptr<Expression> target = stmt->target();
    if(isa<NumLiteral>(target) || isa<Identifier>(target))
    {
        gen(stmt, "jmp "+ target->toString());
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
        notFunc = !(proc)->isFunctionNotProcedure;
    }
    else if(!isa<FunctionDecl>(scopeStack.top().proc))
        notFunc = true;
    if(notFunc)
        throw CompilerException(currentFileName,stmt, ReturnCanBeUsedOnlyInFunctions);
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

    throw CompilerException(currentFileName,expr, UnimplementedInvokationForm).arg(expr->toString());
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

    throw CompilerException(currentFileName,expr, UnimplementedInvokationForm).arg(expr->toString());
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
    else
    {
        throw CompilerException(currentFileName,stmt, UnimplementedInvokationForm).arg(expr->toString());
    }
}

void CodeGenerator::generateSendStmt(shared_ptr<SendStmt> stmt)
{
    generateExpression(stmt->channel());
    if(stmt->signal)
        gen(stmt, "pushnull");
    else
        generateExpression(stmt->value());
    gen(stmt, "send");
}

void CodeGenerator::generateReceiveStmt(shared_ptr<ReceiveStmt> stmt)
{
    if(stmt->signal)
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
    for(int i=0; i<stmt->count(); i++)
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
    gen(stmt, "pushv ", stmt->count()*2);
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
        if(ss->signal)
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
        if(rs->signal)
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

    for(int i=0; i<stmt->count(); i++)
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

            if(recv->signal)
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
    if(isa<Identifier>(expr))
    {
        generateIdentifier(dynamic_pointer_cast<Identifier>(expr));
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
    throw CompilerException(currentFileName,expr, UnimplementedExpressionForm).arg(expr->toString());
}

void CodeGenerator::generateBinaryOperation(shared_ptr<BinaryOperation> expr)
{
    if(expr->_operator == "and")
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
    else if(expr->_operator == "or")
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
        gen(expr, expr->_operator);
    }
}

void CodeGenerator::generateIsaOperation(shared_ptr<IsaOperation> expr)
{
    generateExpression(expr->expression());
    QString typeId = expr->type()->name;
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
        // but tempVar is undefined to the compiler refuses the dummy assignment
        // we currently define the temp var, but this pollutes the namespace
        // so we should use generateAssignmentToLVal instead
        defineInCurrentScope(i.value()->name);
        generateAssignmentStmt(shared_ptr<AssignmentStmt>(new AssignmentStmt(i.key()->getPos(),
                                                    i.key(),
                                                  i.value())));
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
        throw CompilerException(currentFileName,pattern, UnimplementedPatternForm).arg(pattern->toString());
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
    defineInCurrentScope(arrVar);
    // todo: this leaks
    shared_ptr<Identifier> arrVarExpr(new Identifier(pattern->getPos(), arrVar));
    QString exit = _asm.uniqueLabel();
    QString dummy = _asm.uniqueVariable();
    QString ok, no, goon;
    generateExpression(matchee);
    gen(matchee, QString("popl %1").arg(arrVar));
    gen(matchee, QString("pushl %1").arg(arrVar));
    gen(pattern, _ws(L"isa مصفوفة.قيم"));
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

    if(pattern->fixedLength)
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
    defineInCurrentScope(objVar);
    // todo: this leaks
    shared_ptr<Expression> objVarExpr(new Identifier(pattern->getPos(), objVar));
    QString exit = _asm.uniqueLabel();
    QString dummy = _asm.uniqueVariable();
    QString ok, no, goon;
    generateExpression(matchee);
    gen(matchee, QString("popl %1").arg(objVar));
    gen(matchee, QString("pushl %1").arg(objVar));
    gen(pattern, _ws(L"isa %1").arg(pattern->classId()->name));
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


    for(int i=0; i<pattern->fieldCount(); i++)
    {
        // If we reached here, we have a true on the stack
        // therefore we pop it
        gen(pattern, QString("popl ") + dummy);

        // todo: this leaks also
        shared_ptr<Expression> expr(new Idafa(pattern->fieldName(i)->getPos(),
                                                  pattern->fieldNamePtr(i),
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
    defineInCurrentScope(mapVar);
    // todo: this leaks
    shared_ptr<Identifier> mapVarExpr(new Identifier(pattern->getPos(), mapVar));
    QString exit = _asm.uniqueLabel();
    QString dummy = _asm.uniqueVariable();
    QString ok, no, goon;
    generateExpression(matchee);
    gen(matchee, QString("popl %1").arg(mapVar));
    gen(matchee, QString("pushl %1").arg(mapVar));
    gen(pattern, _ws(L"isa قاموس.قيم"));
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

    for(int i=0; i<pattern->pairCount(); i++)
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
        defineInCurrentScope(keyI);
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

    for(int i=0; i<pattern->pairCount(); i++)
    {
        // If we reached here, we have a true on the stack
        // therefore we pop it
        gen(pattern, QString("popl ") + dummy);

        // todo: this leaks also
        shared_ptr<Expression> idx(new Identifier(pattern->key(i)->getPos(), evaluatedKeys[i]));
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
    gen(expr, expr->_operator);
}

void CodeGenerator::generateIdentifier(shared_ptr<Identifier> expr)
{
    if(!scopeStack.empty() && scopeStack.top().bindings.contains(expr->name))
        gen(expr, "pushl "+expr->name);
    else if(declaredGlobalVariables.contains(expr->name))
        gen(expr, "pushg "+expr->name);
    else
        throw CompilerException(currentFileName,expr, UndefinedVariable).arg(expr->name);
}

void CodeGenerator::generateNumLiteral(shared_ptr<NumLiteral> expr)
{
    if(!expr->valueRecognized)
    {
        throw CompilerException(currentFileName,expr, UnacceptableNumberLiteral).arg(expr->toString());
    }
    if(expr->longNotDouble)
        gen(expr, "pushv "+ QString("%1").arg(expr->lValue));
    else
        gen(expr, "pushv "+ QString("%1").arg(expr->dValue));
}

void CodeGenerator::generateStrLiteral(shared_ptr<StrLiteral> expr)
{
    generateStringConstant(expr, expr->value);
}

void CodeGenerator::generateNullLiteral(shared_ptr<NullLiteral> expr)
{
    gen(expr, "pushnull");
}

void CodeGenerator::generateBoolLiteral(shared_ptr<BoolLiteral> expr)
{
    if(expr->value)
        gen(expr, "pushv true");
    else
        gen(expr, "pushv false");
}

void CodeGenerator::generateArrayLiteral(shared_ptr<ArrayLiteral> expr)
{
    QString newVar = generateArrayFromValues(expr, expr->dataVector());
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
    if(context == FunctionInvokationContext && this->allProcedures.contains(expr->functor()->toString()))
    {
        throw CompilerException(currentFileName,expr, CannotCallProcedureInExpression1).arg(expr->functor()->toString());
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
    gen(expr->functor(), QString("call %1,%2").arg(expr->functor()->toString()).arg(expr->argumentCount()));

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
    gen(expr->methodSelector(), QString("callm %1,%2").arg(expr->methodSelector()->name).arg(expr->argumentCount()+1));
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

void CodeGenerator::generateIdafa(shared_ptr<Idafa> expr)
{
    generateExpression(expr->modaf_elaih());
    gen(expr->modaf(), "getfld "+expr->modaf()->name);
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
    gen(expr, "new "+expr->className()->name);
}

QString CodeGenerator::typeExpressionToAssemblyTypeId(shared_ptr<TypeExpression> expr)
{
    if(isa<TypeIdentifier>(expr))
    {
        return dynamic_pointer_cast<TypeIdentifier>(expr)->name;
    }
    else if(isa<PointerTypeExpression>(expr))
    {
        shared_ptr<PointerTypeExpression> p
                = dynamic_pointer_cast<PointerTypeExpression>(expr);
        return QString("*%1").arg(typeExpressionToAssemblyTypeId(p->pointeeType()));
    }
    else
    {
        throw CompilerException(currentFileName,expr, UnimplementedTypeForm);
    }
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
    QString fname = scopeStack.top().proc->procName()->name;
    if(fname == "%main")
        fname = _ws(L"البرنامج الرئيسي");
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
