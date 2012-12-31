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

#include "../smallvm/utils.h"

#include <memory>
using namespace std;

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
OnlyFirstAssignmentToVarCanContainType,
LValueFormNotImplemented,
UnimplementedExpressionForm,
UnimplementedStatementForm,
UnimplementedInvokationForm,
UnimplementedTypeForm1,
UnimplementedPatternForm1,
ProgramsCannotUseExternalModulesWithoutSavingThemFirst,
InternalCompilerErrorInFunc,
CannotRunAModule,
RuleAlreadyDefined,
InvokingUndefinedRule,
ModuleDoesntExist,
ProcedureOrFunctionDoesntExist
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
    QString fileName;
public:
    CompilerException(QString fileName, shared_ptr<AST> source, CompilerError error);
    static CompilerException no_source(CompilerError error);
    CompilerException &arg(QString);
    CompilerError getError() {return error;}
    QString getMessage();
};

enum MethodCallStyle
{
    NonTailCallStyle,
    TailCallStyle,
    LaunchProcessStyle
};

struct VarUsageInfo
{
    shared_ptr<Identifier> pointOfDeclaration;

    VarUsageInfo(shared_ptr<Identifier> pointOfDeclaration)
        :pointOfDeclaration(pointOfDeclaration)
    {

    }
    VarUsageInfo(const VarUsageInfo &other)
        :pointOfDeclaration(other.pointOfDeclaration)
    {

    }

    VarUsageInfo() {}
    VarUsageInfo &operator =(const VarUsageInfo &other)
    {
        pointOfDeclaration = other.pointOfDeclaration;
        return *this;
    }
};

struct Context
{
    shared_ptr<ProceduralDecl>  proc;
    QSet<QString> bindings;
    QSet<QString> labels;
    QMap<QString, shared_ptr<Identifier> > declarationOf;
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

enum CodeGenerationMode
{
    CompilationMode,
    AnalysisMode
};

class CodeGenerator
{
    SmallVMCodeGenerator _asm;
public:
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
    CodeGenerationMode mode;
    // from position of token of identifier
    // to its VarUsageInfo
    QMap<int, VarUsageInfo> varInfos;
    QMap<int, QString> varTypeInfo;
    DebugInfo debugInfo;
    QMap<int, CodePosition> PositionInfo;
    CodeDocument *currentCodeDoc;
    QString currentFileName;
    QMap<int, CodePosition> getPositionInfo() {return PositionInfo;}
    QMap<QString, QString> &getStringConstants() { return _asm.StringConstants; }

    // Synthesic functions, classes...etc generated from other code, ex converting lambda expressions
    // to normal functions.
    // This vector is cleared at the start of compiling each module or program, and after the normal code generation
    // passes any declaration in the vector is also generated. The previous passes are free to add what they like here.
    // see compileModule() and generate()
    QVector<shared_ptr<Declaration> > extraDeclarations;
public:
    CodeGenerator();
    void Init();
    void generate(shared_ptr<Program> program, QString fileName, CodeDocument *curDoc);
    QString getOutput();
    void compileModule(shared_ptr<Module> module, QString fileName, CodeDocument *curDoc);
    QString getStringConstantsAsOpCodes();
    void error(CompilerException );
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
    void generateRulesDeclaration(shared_ptr<RulesDecl> decl);
    QVector<shared_ptr<Statement> > generateRuleImplementation(
            shared_ptr<PegRuleInvokation> rule,
            QList<QString> locals, Labeller &labeller);
    QVector<shared_ptr<Statement> > pegExprToStatements(
            shared_ptr<PegExpr> expr,
            QList<QString> locals, Labeller &labeller,
            QMap<QString, shared_ptr<RuleDecl> > ruleTable);
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
    void generateDrawImageStmt(shared_ptr<DrawImageStmt> stmt);
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
                                         QMap<shared_ptr<AssignableExpression>, shared_ptr<Identifier> > &bindings);
    void generateSimpleLiteralPattern(shared_ptr<SimpleLiteralPattern> pattern,
                                      shared_ptr<Expression> matchee,
                                      QMap<shared_ptr<AssignableExpression>, shared_ptr<Identifier> > &bindings);
    void generateVarPattern(shared_ptr<VarPattern> pattern,
                            shared_ptr<Expression> matchee,
                            QMap<shared_ptr<AssignableExpression> , shared_ptr<Identifier> > &bindings);
    void generateAssignedVarPattern(shared_ptr<AssignedVarPattern> pattern,
                                    shared_ptr<Expression> matchee,
                                    QMap<shared_ptr<AssignableExpression> , shared_ptr<Identifier> > &bindings);
    void generateArrayPattern(shared_ptr<ArrayPattern> pattern,
                              shared_ptr<Expression> matchee,
                              QMap<shared_ptr<AssignableExpression>, shared_ptr<Identifier> > &bindings);
    void generateObjPattern(shared_ptr<ObjPattern> pattern,
                            shared_ptr<Expression> matchee,
                            QMap<shared_ptr<AssignableExpression> , shared_ptr<Identifier> > &bindings);
    void generateMapPattern(shared_ptr<MapPattern> pattern,
                            shared_ptr<Expression> matchee,
                            QMap<shared_ptr<AssignableExpression>,
                            shared_ptr<Identifier> > &bindings);
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
    void generateTimingExpression(shared_ptr<TimingExpression> expr);
    void generateTheSomething(shared_ptr<TheSomething> expr);
    void generateIdafa(shared_ptr<Idafa> expr);
    void generateArrayIndex(shared_ptr<ArrayIndex> expr);
    void generateMultiDimensionalArrayIndex(shared_ptr<MultiDimensionalArrayIndex> expr);
    void generateObjectCreation(shared_ptr<ObjectCreation> expr);
    void generateLambdaExpression(shared_ptr<LambdaExpression> expr);
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

    void defineInCurrentScopeNoSource(QString);
    void defineInCurrentScope(QString, shared_ptr<Identifier> defPoint);
    void defineInCurrentScope(QString var, shared_ptr<Identifier> defPoint, QString type);
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

#endif // COMPILER_H
