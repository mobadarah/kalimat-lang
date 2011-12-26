/**************************************************************************
** The Kalimat programming language
**   Copyright (C) 2010 Mohamed Samy Ali - samy2004@gmail.com
**   This project is released under the Apache License version 2.0
**   as described in the included license.txt file
**************************************************************************/

#ifndef CODEGENERATOR_H
#define CODEGENERATOR_H
#include "codeposition.h"
#include "codegenhelper.h"

#ifndef DEBUGINFO_H
    #include "../smallvm/debuginfo.h"
#endif

#include <memory>
using namespace std;

enum MethodCallStyle
{
    NonTailCallStyle,
    TailCallStyle,
    LaunchProcessStyle
};

struct Context
{
    shared_ptr<ProceduralDecl>  proc;
    QSet<QString> bindings;
    QSet<QString> labels;
    int instructionCount;
};


struct Thunk
{
    virtual void operator()()=0;
};

enum InvokationContext
{
    ProcedureInvokationContext, FunctionInvokationContext
};

class CodeGenerator
{

    SmallVMCodeGenerator _asm;
    QSet<QString> declaredGlobalVariables; // these are declared by the programmer
    QSet<shared_ptr<Identifier> > freeVariables; // and those are references to global variables from within functions
    QMap<QString, shared_ptr<ClassDecl> > allClasses;
    QMap<QString, shared_ptr<ProcedureDecl> > allProcedures;
    QMap<QString, shared_ptr<FunctionDecl> > allFunctions;
    QSet<QString> loadedModules;
    QStack<Context> scopeStack;
    QString currentModuleName;

    int codePosKeyCount;
public:
    DebugInfo debugInfo;
    QMap<int, CodePosition> PositionInfo;
    CodeDocument *currentCodeDoc;
    QMap<int, CodePosition> getPositionInfo() {return PositionInfo;}
    QMap<QString, QString> &getStringConstants() { return _asm.StringConstants; }
public:
    CodeGenerator();
    void Init();
    void generate(shared_ptr<Program> program, CodeDocument *curDoc);
    QString getOutput();
    void compileModule(shared_ptr<Module> module, CodeDocument *curDoc);
    QString getStringConstantsAsOpCodes();
private:

    void generateEntryPoint(QVector<shared_ptr<Statement> > statements);
    void generateDeclaration(shared_ptr<Declaration> decl);
    void generateStatement(shared_ptr<Statement> stmt);
    void generateExpression(shared_ptr<Expression> expr);
    void generateProcedureDeclaration(shared_ptr<ProcedureDecl> decl);
    void generateFunctionDeclaration(shared_ptr<FunctionDecl> decl);
    void generateFFILibraryDeclaration(shared_ptr<FFILibraryDecl> decl);
    void generateFFIProceduralDeclaration(shared_ptr<FFIProceduralDecl> decl, QString libName);
    void generateFFIStructDeclaration(shared_ptr<FFIStructDecl> decl);
    void generateClassDeclaration(shared_ptr<ClassDecl> decl);
    void generateGlobalDeclaration(shared_ptr<GlobalDecl> decl);
    void generateMethodDeclaration(shared_ptr<MethodDecl> decl);

    void generateIOStatement(shared_ptr<IOStatement> stmt);
    void generateGraphicsStatement(shared_ptr<GraphicsStatement> stmt);
    void generateAssignmentStmt(shared_ptr<AssignmentStmt> stmt);
    void generateIfStmt(shared_ptr<IfStmt> stmt);
    void generateWhileStmt(shared_ptr<WhileStmt> stmt);
    void generateForAllStmt(shared_ptr<ForAllStmt> stmt);
    void generateLabelStmt(shared_ptr<LabelStmt> stmt);
    void generateGotoStmt(shared_ptr<GotoStmt> stmt);
    void generateReturnStmt(shared_ptr<ReturnStmt> stmt);
    void generateDelegationStmt(shared_ptr<DelegationStmt> stmt);
    void generateLaunchStmt(shared_ptr<LaunchStmt> stmt);
    void generateBlockStmt(shared_ptr<BlockStmt> stmt);
    void generateInvokationStmt(shared_ptr<InvokationStmt> stmt);
    void generatePrintStmt(shared_ptr<PrintStmt> stmt);
    void generateReadStmt(shared_ptr<ReadStmt> stmt);
    void generateDrawPixelStmt(shared_ptr<DrawPixelStmt> stmt);
    void generateDrawLineStmt(shared_ptr<DrawLineStmt> stmt);
    void generateDrawRectStmt(shared_ptr<DrawRectStmt> stmt);
    void generateDrawCircleStmt(shared_ptr<DrawCircleStmt> stmt);
    void generateDrawSpriteStmt(shared_ptr<DrawSpriteStmt> stmt);
    void generateZoomStmt(shared_ptr<ZoomStmt> stmt);
    void generateEventStatement(shared_ptr<EventStatement> stmt);
    void generateSendStmt(shared_ptr<SendStmt> stmt);
    void generateReceiveStmt(shared_ptr<ReceiveStmt> stmt);
    void generateSelectStmt(shared_ptr<SelectStmt> stmt);
    void generateBinaryOperation(shared_ptr<BinaryOperation> expr);
    void generateIsaOperation(shared_ptr<IsaOperation> expr);
    void generateMatchOperation(shared_ptr<MatchOperation> expr);
    void generatePattern(shared_ptr<Pattern> patterns,
                                        shared_ptr<Expression> expression,
                                        QMap<AssignableExpression *, Identifier *> &bindings);
    void generateSimpleLiteralPattern(shared_ptr<SimpleLiteralPattern> pattern, shared_ptr<Expression> matchee, QMap<AssignableExpression *, Identifier *> &bindings);
    void generateVarPattern(shared_ptr<VarPattern> pattern, shared_ptr<Expression> matchee, QMap<AssignableExpression *, Identifier *> &bindings);
    void generateAssignedVarPattern(shared_ptr<AssignedVarPattern> pattern, shared_ptr<Expression> matchee, QMap<AssignableExpression *, Identifier *> &bindings);
    void generateArrayPattern(ArrayPattern *pattern, Expression *matchee, QMap<AssignableExpression *, Identifier *> &bindings);
    void generateObjPattern(shared_ptr<ObjPattern> pattern, shared_ptr<Expression> matchee, QMap<AssignableExpression *, Identifier *> &bindings);
    void generateMapPattern(shared_ptr<MapPattern> pattern, shared_ptr<Expression> matchee, QMap<AssignableExpression *, Identifier *> &bindings);
    void generateUnaryOperation(shared_ptr<UnaryOperation> expr);
    void generateIdentifier(shared_ptr<Identifier> expr);
    void generateNumLiteral(shared_ptr<NumLiteral> expr);
    void generateStrLiteral(shared_ptr<StrLiteral> expr);
    void generateNullLiteral(shared_ptr<NullLiteral> expr);
    void generateBoolLiteral(shared_ptr<BoolLiteral> expr);
    void generateArrayLiteral(shared_ptr<ArrayLiteral> expr);
    void generateMapLiteral(shared_ptr<MapLiteral> expr);
    void generateInvokation(shared_ptr<Invokation> expr, InvokationContext context, MethodCallStyle style = NonTailCallStyle);
    void generateMethodInvokation(shared_ptr<MethodInvokation> expr, InvokationContext context, MethodCallStyle style = NonTailCallStyle);
    void generateIdafa(shared_ptr<Idafa> expr);
    void generateArrayIndex(shared_ptr<ArrayIndex> expr);
    void generateMultiDimensionalArrayIndex(shared_ptr<MultiDimensionalArrayIndex> expr);
    void generateObjectCreation(shared_ptr<ObjectCreation> expr);

    QString typeExpressionToAssemblyTypeId(shared_ptr<TypeExpression> expr);
    QString generateArrayFromValues(shared_ptr<AST> src, QVector<shared_ptr<Expression> > values);
    void generateAssignmentToLvalue(shared_ptr<AST> src, shared_ptr<AssignableExpression> lval,
                                        Thunk &);
    void generateReference(shared_ptr<AssignableExpression> lval);

    void firstPass(shared_ptr<Declaration> decl);
    void secondPass(shared_ptr<Declaration> decl);
    void thirdPass(shared_ptr<Declaration> decl);
    void checkInheritanceCycles();
    void generateStringConstant(shared_ptr<AST> src, QString str);
    void defineInCurrentScope(QString);
    bool isBountInCurrentScope(QString);
    bool currentScopeFuncNotProc();

    void popProcedureScope();
    void pushProcedureScope(shared_ptr<ProceduralDecl> pd);
private:
    void gen(QString str);
    void gen(QString str, int i);
    void gen(QString str, double d);

    void gen(shared_ptr<AST> src, QString str);
    void gen(shared_ptr<AST> src, QString str, int i);
    void gen(shared_ptr<AST> src, QString str, double d);

    QString getCurrentFunctionNameFormatted();

};
enum CompilerError
{
ASTMustBeStatementOrDeclaration,
ClassAlreadyExists,
AncestorClassXforClassYdoesntExist,
MethodDefinedForNotYetExistingClass,
MethodCalledXwasAlreadyDefinedForClassY,
MethodXwasNotDeclaredInClassY,
MethodXwasDeclaredWithDifferentArityInClassY,
MethodXwasDeclaredAfunctionButImplementedAsProcedureInClassY,
MethodXwasDeclaredAprocedureButImplementedAsFunctionInClassY,
ReadFromCannotContainAPrompt,
ReadFromCanReadOnlyOneVariable,
TargetOfLabelMustBeNumberOrIdentifier,
DuplicateLabel,
ReturnCanBeUsedOnlyInFunctions,
CannotCallProcedureInExpression1,
UndefinedVariable,
UnacceptableNumberLiteral,
DeclarationNotSupported,
LValueFormNotImplemented,
UnimplementedExpressionForm,
UnimplementedStatementForm,
UnimplementedInvokationForm,
UnimplementedTypeForm,
UnimplementedPatternForm,
ProgramsCannotUseExternalModulesWithoutSavingThemFirst,
InternalCompilerErrorInFunc
};

class CompilerException
{
    QString message;
    QStringList args;
    CompilerError error;
    static QMap<CompilerError, QString> errorMap;
    QString translateErrorMessage(CompilerError error);
public:
    shared_ptr<AST> source;
public:
    CompilerException(shared_ptr<AST> source, CompilerError error);
    static CompilerException no_source(CompilerError error);
    CompilerException &arg(QString);

    QString getMessage();
};
#endif // COMPILER_H
