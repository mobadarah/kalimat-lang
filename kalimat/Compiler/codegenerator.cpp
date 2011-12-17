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

#include "codegenerator_incl.h"
#include "../../smallvm/utils.h"

template<typename T1, typename T2> bool isa(T2 *value)
{
    T1 *test = dynamic_cast<T1 *>(value);
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
        throw CompilerException(NULL, InternalCompilerErrorInFunc).arg("currentScopeFuncNotProc");
    ProceduralDecl *decl = scopeStack.top().proc;
    if(isa<ProceduralDecl>(decl))
        return false;
    else if(isa<FunctionDecl>(decl))
        return true;
    else if(isa<MethodDecl>(decl))
    {
        MethodDecl *md = (MethodDecl *) decl;
        return md->isFunctionNotProcedure;
    }
    else
    {
        throw CompilerException(NULL, InternalCompilerErrorInFunc).arg("currentScopeFuncNotProc");
    }
}

void CodeGenerator::generate(Program *program, CodeDocument *curDoc)
{
    QVector<Declaration *> declarations;
    QVector<Statement *> statements;
    currentCodeDoc = curDoc;
/*
    for(int i=0; i<program->usedModuleCount(); i++)
    {
        loadModule(program->usedModule(i));
    }
    */
    for(int i=0; i<program->elementCount(); i++)
    {
        TopLevel *tl = program->element(i);
        if(isa<Declaration>(tl))
            declarations.append(dynamic_cast<Declaration *>(tl));
        else if(isa<Statement>(tl))
            statements.append(dynamic_cast<Statement *>(tl));
        else
            throw CompilerException(tl, ASTMustBeStatementOrDeclaration).arg(tl->toString());
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

void CodeGenerator::compileModule(Module *module, CodeDocument *curDoc)
{
    // Copy-pasted from generate(Program *)
    currentCodeDoc = curDoc;
    QVector<Declaration *> declarations;

 /*   for(int i=0; i<module->usedModuleCount(); i++)
    {
        loadModule(module->usedModule(i));
    }
    */
    for(int i=0; i<module->declCount(); i++)
    {
        Declaration *decl = module->decl(i);
        declarations.append(dynamic_cast<Declaration *>(decl));
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

void CodeGenerator::firstPass(Declaration * decl)
{
    if(isa<ClassDecl>(decl))
    {
        ClassDecl *cd = (ClassDecl *) decl;
        QString name = cd->name()->name;
        if(allClasses.contains(name))
            throw CompilerException(decl, ClassAlreadyExists).arg(name);
        else
            allClasses[name] = cd;
        return;
    }

    if(isa<ProcedureDecl>(decl))
    {
        ProcedureDecl *proc = (ProcedureDecl *) decl;
        allProcedures[proc->procName()->name] = proc;
    }
    if(isa<FunctionDecl>(decl))
    {
        FunctionDecl *func = (FunctionDecl *) decl;
        allFunctions[func->procName()->name] = func;
    }
    if(isa<GlobalDecl>(decl))
    {
        generateGlobalDeclaration(dynamic_cast<GlobalDecl *>(decl));
    }

}
void CodeGenerator::secondPass(Declaration * decl)
{
    if(isa<ClassDecl>(decl))
    {
        ClassDecl *cd = (ClassDecl *) decl;
        if(cd->ancestor() !=NULL)
        {
            QString ancestorName = cd->ancestor()->name;
            if(!allClasses.contains(ancestorName))
            {
                throw CompilerException(decl, AncestorClassXforClassYdoesntExist)
                                            .arg(ancestorName).arg(cd->name()->name);
            }
            ClassDecl *ancestor = allClasses[ancestorName];
            cd->setAncestorClass(QSharedPointer<ClassDecl>(ancestor));
        }
    }
}

void CodeGenerator::thirdPass(Declaration * decl)
{
    if(isa<MethodDecl>(decl))
    {
        MethodDecl *md = (MethodDecl *) decl;
        QString name = md->procName()->name;
        QString className = md->className()->name;
        if(!allClasses.contains(className))
            throw CompilerException(decl, MethodDefinedForNotYetExistingClass).arg(className);
        else
        {
            ClassDecl *theClass = allClasses[className];
            if(theClass->containsMethod(name))
                throw CompilerException(decl, MethodCalledXwasAlreadyDefinedForClassY).arg(name).arg(className);
            if(!theClass->containsPrototype(name))
                throw CompilerException(decl, MethodXwasNotDeclaredInClassY).arg(name).arg(className);

            // We subtract the 1 to account for the extra 'this' parameter in md
            if(theClass->methodPrototype(name).arity != md->formalCount()-1)
                throw CompilerException(decl, MethodXwasDeclaredWithDifferentArityInClassY).arg(name).arg(className);
            if(theClass->methodPrototype(name).isFunction && !md->isFunctionNotProcedure)
                throw CompilerException(decl, MethodXwasDeclaredAfunctionButImplementedAsProcedureInClassY).arg(name).arg(className);
            if(!theClass->methodPrototype(name).isFunction && md->isFunctionNotProcedure)
                throw CompilerException(decl, MethodXwasDeclaredAprocedureButImplementedAsFunctionInClassY).arg(name).arg(className);
            allClasses[className]->insertMethod(name, QSharedPointer<MethodDecl>(md));
        }
        return;
    }
}

void CodeGenerator::checkInheritanceCycles()
{
    //todo:
}

void CodeGenerator::generateDeclaration(Declaration * decl)
{
    if(isa<ProceduralDecl>(decl))
    {
        ProceduralDecl *pd = (ProceduralDecl *) decl;
        pushProcedureScope(pd);
    }

    if(isa<ProcedureDecl>(decl))
    {
        ProcedureDecl *pd = (ProcedureDecl *) decl;
        if(pd->procName()->name == "%main")
            generateEntryPoint(pd->body()->getStatements());
        else
            generateProcedureDeclaration(dynamic_cast<ProcedureDecl *>(decl));
    }
    else if(isa<FunctionDecl>(decl))
    {
        generateFunctionDeclaration(dynamic_cast<FunctionDecl *>(decl));
    }
    else if(isa<ClassDecl>(decl))
    {
        generateClassDeclaration(dynamic_cast<ClassDecl *>(decl));
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
        generateFFILibraryDeclaration(dynamic_cast<FFILibraryDecl *>(decl));
    }
    else
    {
        throw CompilerException(decl, DeclarationNotSupported).arg(decl->toString());
    }

    if(isa<ProceduralDecl>(decl))
    {
        popProcedureScope();
    }
}

void CodeGenerator::pushProcedureScope(ProceduralDecl *pd)
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

void CodeGenerator::generateProcedureDeclaration(ProcedureDecl * decl)
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

void CodeGenerator::generateFunctionDeclaration(FunctionDecl * decl)
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

void CodeGenerator::generateFFILibraryDeclaration(FFILibraryDecl *decl)
{
    for(int i=0; i<decl->declCount(); i++)
    {
        Declaration *d = decl->decl(i);
        if(isa<FFIProceduralDecl>(d))
        {
            generateFFIProceduralDeclaration(dynamic_cast<FFIProceduralDecl *>(d), decl->libName);
        }
        else if(isa<FFIStructDecl>(d))
        {
            generateFFIStructDeclaration(dynamic_cast<FFIStructDecl *>(d));
        }
        else
        {
            throw CompilerException(decl, DeclarationNotSupported);
        }

    }
}

void CodeGenerator::generateFFIProceduralDeclaration(FFIProceduralDecl *decl, QString libName)
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
    // todo: we've called new() 3 times here, this is a memory leak which
    // can be rectified by cleaning up after we popProcedureScope at the end
    // of this function
    pushProcedureScope(new FunctionDecl(decl->getPos(),
                                        decl->getPos(),
                                        QSharedPointer<Identifier>(new Identifier(decl->getPos(),decl->procName)),
                                        QVector<QSharedPointer<Identifier> >(),
                                        QSharedPointer<BlockStmt>(new BlockStmt(decl->getPos(), QVector<QSharedPointer<Statement > >())),
                                        false
                                        ));
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

    //TODO: see this debuInfo thing on the next line
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

void CodeGenerator::generateFFIStructDeclaration(FFIStructDecl *decl)
{

}

void CodeGenerator::generateClassDeclaration(ClassDecl *decl)
{
    gen(decl, ".class "+decl->name()->name);
    if(decl->ancestor() != NULL)
    {
        QString parent = decl->ancestor()->name;
        gen(decl->ancestor(), ".extends "+ parent);
    }
   /* if(decl->prototypeCount() > decl->methodCount())
        throw CompilerException(decl, QString("Class '%1' has some unimplemented methods").arg(decl->name()->name));
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
        MethodDecl *method = decl->method(i);
        pushProcedureScope(method);
        generateMethodDeclaration(method);
        popProcedureScope();
    }
    gen(decl,".endclass");
}
void CodeGenerator::generateGlobalDeclaration(GlobalDecl *decl)
{
    this->declaredGlobalVariables.insert(decl->varName);
}

void CodeGenerator::generateMethodDeclaration(MethodDecl *decl)
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

void CodeGenerator::generateEntryPoint(QVector<Statement *> statements)
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

void CodeGenerator::generateStatement(Statement *stmt)
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
        generateIOStatement(dynamic_cast<IOStatement *>(stmt));
    }
    else if(isa<GraphicsStatement>(stmt))
    {
        generateGraphicsStatement(dynamic_cast<GraphicsStatement *>(stmt));
    }
    else if(isa<AssignmentStmt>(stmt))
    {
        generateAssignmentStmt(dynamic_cast<AssignmentStmt*>(stmt));
    }
    else if(isa<IfStmt>(stmt))
    {
        generateIfStmt(dynamic_cast<IfStmt *>(stmt));
    }
    else if(isa<WhileStmt>(stmt))
    {
        generateWhileStmt(dynamic_cast<WhileStmt *>(stmt));
    }
    else if(isa<ForAllStmt>(stmt))
    {
        generateForAllStmt(dynamic_cast<ForAllStmt *>(stmt));
    }
    else if(isa<LabelStmt>(stmt))
    {
        generateLabelStmt(dynamic_cast<LabelStmt*>(stmt));
    }
    else if(isa<GotoStmt>(stmt))
    {
        generateGotoStmt(dynamic_cast<GotoStmt*>(stmt));
    }
    else if(isa<DelegationStmt>(stmt))
    {
        generateDelegationStmt(dynamic_cast<DelegationStmt *>(stmt));
    }
    else if(isa<LaunchStmt>(stmt))
    {
        generateLaunchStmt(dynamic_cast<LaunchStmt *>(stmt));
    }
    else if(isa<ReturnStmt>(stmt))
    {
        generateReturnStmt(dynamic_cast<ReturnStmt *>(stmt));
    }
    else if(isa<BlockStmt>(stmt))
    {
        generateBlockStmt(dynamic_cast<BlockStmt *>(stmt));
    }
    else if(isa<InvokationStmt>(stmt))
    {
        generateInvokationStmt(dynamic_cast<InvokationStmt *>(stmt));
    }
    else if(isa<EventStatement>(stmt))
    {
        generateEventStatement(dynamic_cast<EventStatement *>(stmt));
    }
    else if(isa<SendStmt>(stmt))
    {
        generateSendStmt(dynamic_cast<SendStmt *>(stmt));
    }
    else if(isa<ReceiveStmt>(stmt))
    {
        generateReceiveStmt(dynamic_cast<ReceiveStmt*>(stmt));
    }
    else if(isa<SelectStmt>(stmt))
    {
        generateSelectStmt(dynamic_cast<SelectStmt*>(stmt));
    }
    else
    {
        throw new CompilerException(stmt, UnimplementedStatementForm);
    }
}
void CodeGenerator::generateIOStatement(IOStatement *stmt)
{
    if(isa<PrintStmt>(stmt))
    {
        generatePrintStmt(dynamic_cast<PrintStmt *>(stmt));
        return;
    }
    if(isa<ReadStmt>(stmt))
    {
        generateReadStmt(dynamic_cast<ReadStmt *>(stmt));
        return;
    }
}
void CodeGenerator::generatePrintStmt(PrintStmt *stmt)
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
        if(stmt->_widths[i].isNull())
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
            generateExpression(stmt->_widths[i].data());
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
void CodeGenerator::generateReadStmt(ReadStmt *stmt)
{
    struct GenerateRvalue : public Thunk
    {
        AST *_src; CodeGenerator *_g; QString _v;
        GenerateRvalue(AST *src, CodeGenerator *g, QString v):_src(src),_g(g), _v(v){}
        void operator()()
        {
            _g->gen(_src, "pushl " + _v);
        }
    };

    if(stmt->fileObject() != NULL)
    {
        if(stmt->prompt != NULL)
            throw CompilerException(stmt, ReadFromCannotContainAPrompt);
        if(stmt->variableCount() !=1)
            throw CompilerException(stmt, ReadFromCanReadOnlyOneVariable);
        AssignableExpression *lvalue = stmt->variable(0);

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
        AssignableExpression *lvalue = stmt->variable(i);
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
void CodeGenerator::generateGraphicsStatement(GraphicsStatement *stmt)
{
    if(isa<DrawPixelStmt>(stmt))
    {
        generateDrawPixelStmt(dynamic_cast<DrawPixelStmt *>(stmt));
        return;
    }
    if(isa<DrawLineStmt>(stmt))
    {
        generateDrawLineStmt(dynamic_cast<DrawLineStmt *>(stmt));
        return;
    }
    if(isa<DrawRectStmt>(stmt))
    {
        generateDrawRectStmt(dynamic_cast<DrawRectStmt *>(stmt));
        return;
    }
    if(isa<DrawCircleStmt>(stmt))
    {
        generateDrawCircleStmt(dynamic_cast<DrawCircleStmt *>(stmt));
        return;
    }
    if(isa<DrawSpriteStmt>(stmt))
    {
        generateDrawSpriteStmt(dynamic_cast<DrawSpriteStmt *>(stmt));
        return;
    }
    if(isa<ZoomStmt>(stmt))
    {
        generateZoomStmt(dynamic_cast<ZoomStmt *>(stmt));
        return;
    }
}
void CodeGenerator::generateDrawPixelStmt(DrawPixelStmt *stmt)
{
    if(stmt->color() == NULL)
        gen(stmt, "pushv -1");
    else
        generateExpression(stmt->color());
    generateExpression(stmt->y());
    generateExpression(stmt->x());
    gen(stmt, "callex drawpixel");
}
void CodeGenerator::generateDrawLineStmt(DrawLineStmt *stmt)
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
void CodeGenerator::generateDrawRectStmt(DrawRectStmt *stmt)
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

void CodeGenerator::generateDrawCircleStmt(DrawCircleStmt *stmt)
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

void CodeGenerator::generateDrawSpriteStmt(DrawSpriteStmt *stmt)
{
    generateExpression(stmt->y());
    generateExpression(stmt->x());
    generateExpression(stmt->sprite());
    gen(stmt, "callex drawsprite");
}
void CodeGenerator::generateZoomStmt(ZoomStmt *stmt)
{
    generateExpression(stmt->y2());
    generateExpression(stmt->x2());
    generateExpression(stmt->y1());
    generateExpression(stmt->x1());

    gen(stmt, "callex zoom");
}

void CodeGenerator::generateEventStatement(EventStatement *stmt)
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


void CodeGenerator::generateAssignmentStmt(AssignmentStmt *stmt)
{
    AssignableExpression *lval = stmt->variable();

    struct GenerateExpr : public Thunk
    {
        CodeGenerator *_g; Expression *_e;
        GenerateExpr(CodeGenerator *g, Expression *e):_g(g),_e(e) {}
        void operator() (){ _g->generateExpression(_e); }
    } myGen(this, stmt->value());

    generateAssignmentToLvalue(stmt, lval, myGen);
}

void CodeGenerator::generateAssignmentToLvalue(AST *src, AssignableExpression *lval,
                                               Thunk &genValue)
{
    if(isa<Identifier>(lval))
    {
        Identifier *variable = (Identifier *) lval;
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
        Idafa *fieldAccess = (Idafa *) lval;
        generateExpression(fieldAccess->modaf_elaih());
        genValue();
        gen(src, "setfld "+ fieldAccess->modaf()->name);
    }
    else if(isa<ArrayIndex>(lval))
    {
        ArrayIndex *arri= (ArrayIndex *) lval;
        generateExpression(arri->array());
        generateExpression(arri->index());
        genValue();
        gen(src, "setarr");
    }
    else if(isa<MultiDimensionalArrayIndex>(lval))
    {
        MultiDimensionalArrayIndex *arri = (MultiDimensionalArrayIndex *) lval;
        generateExpression(arri->array());
        generateArrayFromValues(lval, arri->indexes());
        genValue();
        gen(src, "setmdarr");
    }
    else
    {
        throw CompilerException(src, LValueFormNotImplemented).arg(lval->toString());
    }
}

void CodeGenerator::generateReference(AssignableExpression *lval)
{
    //todo: generating first-class references from lvalues
    gen(lval, "pushnull");
}

void CodeGenerator::generateIfStmt(IfStmt *stmt)
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

void CodeGenerator::generateWhileStmt(WhileStmt *stmt)
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

void CodeGenerator::generateForAllStmt(ForAllStmt *stmt)
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

void CodeGenerator::generateLabelStmt(LabelStmt *stmt)
{
    Expression *target = stmt->target();
    QString labelName = target->toString();
    if(scopeStack.top().labels.contains(labelName))
    {
        throw CompilerException(stmt, DuplicateLabel).arg(labelName).arg(getCurrentFunctionNameFormatted());
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
        throw CompilerException(stmt, TargetOfLabelMustBeNumberOrIdentifier);
    }
}

void CodeGenerator::generateGotoStmt(GotoStmt *stmt)
{
    if(stmt->numericTarget())
    {
        gen(stmt, "jmp "+ stmt->numericTarget()->toString());
    }
    else
    {
        gen(stmt, "jmp "+ stmt->idTarget()->name);
    }
}

void CodeGenerator::generateReturnStmt(ReturnStmt *stmt)
{
    bool notFunc = false;
    if(scopeStack.empty())
        notFunc = true;
    else if(isa<MethodDecl>(scopeStack.top().proc))
    {
        notFunc = !((MethodDecl *) scopeStack.top().proc)->isFunctionNotProcedure;
    }
    else if(!isa<FunctionDecl>(scopeStack.top().proc))
        notFunc = true;
    if(notFunc)
        throw CompilerException(stmt, ReturnCanBeUsedOnlyInFunctions);
    generateExpression(stmt->returnVal());
    gen(stmt,"ret");
}

void CodeGenerator::generateDelegationStmt(DelegationStmt *stmt)
{
    IInvokation *expr = stmt->invokation();

    InvokationContext context = currentScopeFuncNotProc()? FunctionInvokationContext : ProcedureInvokationContext;

    if(isa<Invokation>(expr))
    {
        generateInvokation(dynamic_cast<Invokation *>(expr), context, TailCallStyle);
        return;
    }
    if(isa<MethodInvokation>(expr))
    {
        generateMethodInvokation(dynamic_cast<MethodInvokation *>(expr), context, TailCallStyle);
        return;
    }

    throw CompilerException(expr, UnimplementedInvokationForm).arg(expr->toString());
}

void CodeGenerator::generateLaunchStmt(LaunchStmt *stmt)
{
    IInvokation *expr = stmt->invokation();
    if(isa<Invokation>(expr))
    {
        generateInvokation(dynamic_cast<Invokation *>(expr), ProcedureInvokationContext, LaunchProcessStyle);
        return;
    }
    if(isa<MethodInvokation>(expr))
    {
        generateMethodInvokation(dynamic_cast<MethodInvokation *>(expr), ProcedureInvokationContext, LaunchProcessStyle);
        return;
    }

    throw CompilerException(expr, UnimplementedInvokationForm).arg(expr->toString());
}

void CodeGenerator::generateBlockStmt(BlockStmt *stmt)
{
    ProceduralDecl *owningMethod = scopeStack.top().proc;
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

void CodeGenerator::generateInvokationStmt(InvokationStmt *stmt)
{
    Expression *expr = stmt->expression();

    if(isa<Invokation>(expr))
    {
        generateInvokation(dynamic_cast<Invokation *>(expr), ProcedureInvokationContext);
        return;
    }
    if(isa<MethodInvokation>(expr))
    {
        generateMethodInvokation(dynamic_cast<MethodInvokation *>(expr), ProcedureInvokationContext);
        return;
    }
    else
    {
        throw CompilerException(stmt, UnimplementedInvokationForm).arg(expr->toString());
    }
}

void CodeGenerator::generateSendStmt(SendStmt *stmt)
{
    generateExpression(stmt->channel());
    if(stmt->signal)
        gen(stmt, "pushnull");
    else
        generateExpression(stmt->value());
    gen(stmt, "send");
}

void CodeGenerator::generateReceiveStmt(ReceiveStmt *stmt)
{
    if(stmt->signal)
    {
        generateExpression(stmt->channel());
        gen(stmt, "receive");
        gen("popl " + _asm.uniqueVariable());
    }
    else
    {
        AssignableExpression *lval = stmt->value();

        struct GenerateExpr : public Thunk
        {
            CodeGenerator *_g; ReceiveStmt *_stmt;
            GenerateExpr(CodeGenerator *g, ReceiveStmt *s):_g(g),_stmt(s) {}
            void operator() ()
            {
                _g->generateExpression(_stmt->channel());
                _g->gen(_stmt, "receive");
            }
        } myGen(this, stmt);

        generateAssignmentToLvalue(stmt, lval, myGen);
    }
}

void CodeGenerator::generateSelectStmt(SelectStmt *stmt)
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
        ChannelCommunicationStmt *ccs = stmt->condition(i);
        SendStmt *isSend = dynamic_cast<SendStmt *>(ccs);
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
        SendStmt *ss = dynamic_cast<SendStmt *>(stmt->condition(sends[i]));

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
        ReceiveStmt *rs = dynamic_cast<ReceiveStmt*>(stmt->condition(receives[i]));

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

        Statement *action;
        ChannelCommunicationStmt *cond;
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
            ReceiveStmt *recv = dynamic_cast<ReceiveStmt *>(stmt->condition(i));
            class AssignRet : public Thunk
            {
                CodeGenerator *_g; QString _ret; AssignableExpression *_ae;
            public:
                AssignRet(CodeGenerator *_g, QString _ret, AssignableExpression *_ae)
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

void CodeGenerator::generateExpression(Expression *expr)
{
    if(isa<BinaryOperation>(expr))
    {
        generateBinaryOperation(dynamic_cast<BinaryOperation *>(expr));
        return;
    }
    if(isa<IsaOperation>(expr))
    {
        generateIsaOperation(dynamic_cast<IsaOperation *>(expr));
        return;
    }
    if(isa<MatchOperation>(expr))
    {
        generateMatchOperation(dynamic_cast<MatchOperation *>(expr));
        return;
    }
    if(isa<UnaryOperation>(expr))
    {
        generateUnaryOperation(dynamic_cast<UnaryOperation *>(expr));
        return;
    }
    if(isa<Identifier>(expr))
    {
        generateIdentifier(dynamic_cast<Identifier *>(expr));
        return;
    }
    if(isa<NumLiteral>(expr))
    {
        generateNumLiteral(dynamic_cast<NumLiteral *>(expr));
        return;
    }
    if(isa<StrLiteral>(expr))
    {
        generateStrLiteral(dynamic_cast<StrLiteral *>(expr));
        return;
    }
    if(isa<NullLiteral>(expr))
    {
        generateNullLiteral(dynamic_cast<NullLiteral *>(expr));
        return;
    }
    if(isa<BoolLiteral>(expr))
    {
        generateBoolLiteral(dynamic_cast<BoolLiteral *>(expr));
        return;
    }
    if(isa<ArrayLiteral>(expr))
    {
        generateArrayLiteral(dynamic_cast<ArrayLiteral *>(expr));
        return;
    }
    if(isa<MapLiteral>(expr))
    {
        generateMapLiteral(dynamic_cast<MapLiteral *>(expr));
        return;
    }
    if(isa<Invokation>(expr))
    {
        generateInvokation(dynamic_cast<Invokation *>(expr), FunctionInvokationContext);
        return;
    }
    if(isa<MethodInvokation>(expr))
    {
        generateMethodInvokation(dynamic_cast<MethodInvokation *>(expr), FunctionInvokationContext);
        return;
    }
    if(isa<Idafa>(expr))
    {
        generateIdafa(dynamic_cast<Idafa *>(expr));
        return;
    }
    if(isa<ArrayIndex>(expr))
    {
        generateArrayIndex(dynamic_cast<ArrayIndex *>(expr));
        return;
    }
    if(isa<MultiDimensionalArrayIndex>(expr))
    {
        generateMultiDimensionalArrayIndex(dynamic_cast<MultiDimensionalArrayIndex *>(expr));
        return;
    }
    if(isa<ObjectCreation>(expr))
    {
        generateObjectCreation(dynamic_cast<ObjectCreation *>(expr));
        return;
    }
    throw CompilerException(expr, UnimplementedExpressionForm).arg(expr->toString());
}

void CodeGenerator::generateBinaryOperation(BinaryOperation *expr)
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

void CodeGenerator::generateIsaOperation(IsaOperation *expr)
{
    generateExpression(expr->expression());
    QString typeId = expr->type()->name;
    gen(expr, "isa " + typeId);
}

void CodeGenerator::generateMatchOperation(MatchOperation *expr)
{

    QMap<AssignableExpression *, Identifier*> bindings;
    generatePattern(expr->pattern(), expr->expression(), bindings);

    // Now use all the bindings we've collected
    QString bind = _asm.uniqueLabel(), nobind = _asm.uniqueLabel(), exit = _asm.uniqueLabel();
    gen(expr, QString("if %1,%2").arg(bind).arg(nobind));
    gen(expr, QString("%1:").arg(bind));
    for(QMap<AssignableExpression*, Identifier*>::const_iterator i=bindings.begin(); i!=bindings.end();++i)
    {
        // todo: we want to execute the equivalent of matchedVar = tempVar
        // but tempVar is undefined to the compiler refuses the dummy assignment
        // we currently define the temp var, but this pollutes the namespace
        // so we should use generateAssignmentToLVal instead
        defineInCurrentScope(i.value()->name);
        // todo: this leaks!!
        generateAssignmentStmt(new AssignmentStmt(i.key()->getPos(),
                                                  QSharedPointer<AssignableExpression>(i.key()),
                                                  QSharedPointer<Identifier>(i.value())));
    }
    gen(expr, "pushv true");
    gen(expr, QString("jmp %1").arg(exit));
    gen(expr, QString("%1:").arg(nobind));
    gen(expr, "pushv false");
    gen(expr, QString("jmp %1").arg(exit));
    gen(expr, QString("%1:").arg(exit));
}

void CodeGenerator::generatePattern(Pattern *pattern,
                                    Expression *expression,
                                    QMap<AssignableExpression *, Identifier *> &bindings)
{
    if(isa<SimpleLiteralPattern>(pattern))
    {
        generateSimpleLiteralPattern(dynamic_cast<SimpleLiteralPattern *>(pattern), expression, bindings);
    }
    else if(isa<VarPattern>(pattern))
    {
        generateVarPattern(dynamic_cast<VarPattern *>(pattern), expression, bindings);
    }
    else if(isa<AssignedVarPattern>(pattern))
    {
        generateAssignedVarPattern(dynamic_cast<AssignedVarPattern *>(pattern), expression, bindings);
    }
    else if(isa<ArrayPattern>(pattern))
    {
        generateArrayPattern(dynamic_cast<ArrayPattern*>(pattern), expression, bindings);
    }
    else if(isa<ObjPattern>(pattern))
    {
        generateObjPattern(dynamic_cast<ObjPattern *>(pattern), expression, bindings);
    }
    else if(isa<MapPattern>(pattern))
    {
        generateMapPattern(dynamic_cast<MapPattern *>(pattern), expression, bindings);
    }
    else
    {
        throw CompilerException(pattern, UnimplementedPatternForm).arg(pattern->toString());
    }
}

void CodeGenerator::generateSimpleLiteralPattern(SimpleLiteralPattern *pattern,
                                                 Expression *matchee,
                                                 QMap<AssignableExpression *, Identifier *> &bindings)
{
    generateExpression(pattern->value());
    generateExpression(matchee);
    gen(matchee, "eq");
}

void CodeGenerator::generateVarPattern(VarPattern *pattern,
                                       Expression *matchee,
                                       QMap<AssignableExpression *, Identifier *> &bindings)
{
    generateExpression(pattern->id());
    generateExpression(matchee);
    gen(matchee, "eq");
}

void CodeGenerator::generateAssignedVarPattern(AssignedVarPattern *pattern,
                                               Expression *matchee,
                                               QMap<AssignableExpression *, Identifier *> &bindings)
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
    bindings.insert(pattern->lv(), new Identifier(matchee->getPos(), tempVar));
}

void CodeGenerator::generateArrayPattern(ArrayPattern *pattern,
                                         Expression *matchee,
                                         QMap<AssignableExpression *, Identifier *> &bindings)
{
    QString arrVar = _asm.uniqueVariable();
    // todo: for why this is ugly please refer to the comment
    // about defineInCurrentScope() in generateMatchOperation()
    defineInCurrentScope(arrVar);
    // todo: this leaks
    Identifier *arrVarExpr = new Identifier(pattern->getPos(), arrVar);
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
        NumLiteral *idx = new NumLiteral(pattern->element(i)->getPos(), QString("%1").arg(i+1));
        Expression *expr = new ArrayIndex(pattern->element(i)->getPos(), arrVarExpr, idx);
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

void CodeGenerator::generateObjPattern(ObjPattern *pattern,
                                       Expression *matchee,
                                       QMap<AssignableExpression *, Identifier *> &bindings)
{
    QString objVar = _asm.uniqueVariable();
    // todo: for why this is ugly please refer to the comment
    // about defineInCurrentScope() in generateMatchOperation()
    defineInCurrentScope(objVar);
    // todo: this leaks
    Identifier *objVarExpr = new Identifier(pattern->getPos(), objVar);
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
        Expression *expr = new Idafa(pattern->fieldName(i)->getPos(), pattern->fieldName(i), objVarExpr);
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

void CodeGenerator::generateMapPattern(MapPattern *pattern,
                                       Expression *matchee,
                                       QMap<AssignableExpression *, Identifier *> &bindings)
{
    QString mapVar = _asm.uniqueVariable();
    // todo: for why this is ugly please refer to the comment
    // about defineInCurrentScope() in generateMatchOperation()
    defineInCurrentScope(mapVar);
    // todo: this leaks
    Identifier *mapVarExpr = new Identifier(pattern->getPos(), mapVar);
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
        Expression *idx = new Identifier(pattern->key(i)->getPos(), evaluatedKeys[i]);
        Expression *expr = new ArrayIndex(pattern->key(i)->getPos(), mapVarExpr, idx);
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

void CodeGenerator::generateUnaryOperation(UnaryOperation *expr)
{
    generateExpression(expr->operand());
    gen(expr, expr->_operator);
}

void CodeGenerator::generateIdentifier(Identifier *expr)
{
    if(!scopeStack.empty() && scopeStack.top().bindings.contains(expr->name))
        gen(expr, "pushl "+expr->name);
    else if(declaredGlobalVariables.contains(expr->name))
        gen(expr, "pushg "+expr->name);
    else
        throw CompilerException(expr, UndefinedVariable).arg(expr->name);
}

void CodeGenerator::generateNumLiteral(NumLiteral *expr)
{
    if(!expr->valueRecognized)
    {
        throw CompilerException(expr, UnacceptableNumberLiteral).arg(expr->toString());
    }
    if(expr->longNotDouble)
        gen(expr, "pushv "+ QString("%1").arg(expr->lValue));
    else
        gen(expr, "pushv "+ QString("%1").arg(expr->dValue));
}

void CodeGenerator::generateStrLiteral(StrLiteral *expr)
{
    generateStringConstant(expr, expr->value);
}

void CodeGenerator::generateNullLiteral(NullLiteral *expr)
{
    gen(expr, "pushnull");
}

void CodeGenerator::generateBoolLiteral(BoolLiteral *expr)
{
    if(expr->value)
        gen(expr, "pushv true");
    else
        gen(expr, "pushv false");
}

void CodeGenerator::generateArrayLiteral(ArrayLiteral *expr)
{
    QString newVar = generateArrayFromValues(expr, expr->dataVector());
}

void CodeGenerator::generateMapLiteral(MapLiteral *expr)
{
    QString newVar = _asm.uniqueVariable();

    gen(expr, QString::fromStdWString(L"callex newmap"));
    gen(expr, "popl "+ newVar);

    for(int i=0; i<expr->dataCount(); i+=2)
    {
       Expression *key = expr->data(i);
       Expression *value = expr->data(i+1);

       gen(value, "pushl "+ newVar);
       generateExpression(key);
       generateExpression(value);
       gen(key, "setarr");
    }
    gen(expr, "pushl " + newVar);
}


void CodeGenerator::generateInvokation(Invokation *expr, InvokationContext context, MethodCallStyle style)
{
    if(context == FunctionInvokationContext && this->allProcedures.contains(expr->functor()->toString()))
    {
        throw CompilerException(expr, CannotCallProcedureInExpression1).arg(expr->functor()->toString());
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

void CodeGenerator::generateMethodInvokation(MethodInvokation *expr, InvokationContext context, MethodCallStyle style)
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

void CodeGenerator::generateIdafa(Idafa *expr)
{
    generateExpression(expr->modaf_elaih());
    gen(expr->modaf(), "getfld "+expr->modaf()->name);
}

void CodeGenerator::generateArrayIndex(ArrayIndex *expr)
{
    generateExpression(expr->array());
    generateExpression(expr->index());
    gen(expr, "getarr");
}

void CodeGenerator::generateMultiDimensionalArrayIndex(MultiDimensionalArrayIndex *expr)
{
    generateExpression(expr->array());
    generateArrayFromValues(expr, expr->indexes());
    gen(expr, "getmdarr");
}

QString CodeGenerator::generateArrayFromValues(AST *src, QVector<QSharedPointer<Expression> >values)
{
    QString newVar = _asm.uniqueVariable();
    gen(src, "pushv ", values.count());
    gen(src, "newarr");
    gen(src, "popl "+ newVar);
    // Notice that Kalimat arrays are one-based, not zero-based
    for(int i=1; i<=values.count(); i++)
    {
       Expression *value = values[i-1].data();
       gen(value, "pushl "+ newVar);
       gen(value, "pushv ", i);
       generateExpression(value);
       gen(value, "setarr");
    }
    gen(src,"pushl " + newVar);
    return newVar;
}

void CodeGenerator::generateObjectCreation(ObjectCreation *expr)
{
    gen(expr, "new "+expr->className()->name);
}

QString CodeGenerator::typeExpressionToAssemblyTypeId(TypeExpression *expr)
{
    if(isa<TypeIdentifier>(expr))
    {
        return ((TypeIdentifier *) expr)->name;
    }
    else if(isa<PointerTypeExpression>(expr))
    {
        PointerTypeExpression *p = (PointerTypeExpression *) expr;
        return QString("*%1").arg(typeExpressionToAssemblyTypeId(p->pointeeType()));
    }
    else
    {
        throw CompilerException(expr, UnimplementedTypeForm);
    }
}

void CodeGenerator::generateStringConstant(AST *src, QString str)
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

void CodeGenerator::gen(AST *src,QString str)
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

void CodeGenerator::gen(AST *src,QString str, int i)
{
    gen(src, QString(str)+" "+ QString("%1").arg(i));
}

void CodeGenerator::gen(AST *src,QString str, double d)
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

CompilerException::CompilerException(AST *source, CompilerError error)
{
    this->source = source;
    this->message = translateErrorMessage(error);
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
