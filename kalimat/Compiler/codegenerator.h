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

enum MethodCallStyle
{
    NonTailCall,
    TailCall
};

struct Context
{
    ProceduralDecl *proc;
    QSet<QString> bindings;
    QSet<QString> labels;
};


class CodeGenerator
{

    SmallVMCodeGenerator _asm;
    QSet<QString> declaredGlobalVariables; // these are declared by the programmer
    QSet<Identifier *> freeVariables; // and those are references to global variables from within functions
    QMap<QString, ClassDecl *> allClasses;
    QMap<QString, ProcedureDecl *> allProcedures;
    QMap<QString, FunctionDecl *> allFunctions;
    QSet<QString> loadedModules;
    QStack<Context> scopeStack;
    QString currentModuleName;

    int codePosKeyCount;
public:
    QMap<int, CodePosition> PositionInfo;
    CodeDocument *currentCodeDoc;
    QMap<int, CodePosition> getPositionInfo() {return PositionInfo;}
    QMap<QString, QString> &getStringConstants() { return _asm.StringConstants; }
public:
    CodeGenerator();
    void Init();
    void generate(Program *program, CodeDocument *curDoc);
    QString getOutput();
    void compileModule(Module *module, CodeDocument *curDoc);
private:

    void generateEntryPoint(QVector<Statement *> statements);
    void generateDeclaration(Declaration * decl);
    void generateStatement(Statement *stmt);
    void generateExpression(Expression *expr);
    void generateProcedureDeclaration(ProcedureDecl * decl);
    void generateFunctionDeclaration(FunctionDecl * decl);
    void generateClassDeclaration(ClassDecl * decl);
    void generateGlobalDeclaration(GlobalDecl * decl);
    void generateMethodDeclaration(MethodDecl * decl);

    void generateIOStatement(IOStatement *stmt);
    void generateGraphicsStatement(GraphicsStatement *stmt);
    void generateAssignmentStmt(AssignmentStmt *stmt);
    void generateIfStmt(IfStmt *stmt);
    void generateWhileStmt(WhileStmt *stmt);
    void generateForAllStmt(ForAllStmt *stmt);
    void generateLabelStmt(LabelStmt *stmt);
    void generateGotoStmt(GotoStmt *stmt);
    void generateReturnStmt(ReturnStmt *stmt);
    void generateDelegationStmt(DelegationStmt *stmt);
    void generateBlockStmt(BlockStmt *stmt);
    void generateInvokationStmt(InvokationStmt *stmt);
    void generatePrintStmt(PrintStmt *stmt);
    void generateReadStmt(ReadStmt *stmt);
    void generateDrawPixelStmt(DrawPixelStmt *stmt);
    void generateDrawLineStmt(DrawLineStmt *stmt);
    void generateDrawRectStmt(DrawRectStmt *stmt);
    void generateDrawCircleStmt(DrawCircleStmt *stmt);
    void generateDrawSpriteStmt(DrawSpriteStmt *stmt);
    void generateZoomStmt(ZoomStmt *stmt);
    void generateEventStatement(EventStatement *stmt);
    void generateBinaryOperation(BinaryOperation *expr);
    void generateUnaryOperation(UnaryOperation *expr);
    void generateIdentifier(Identifier *expr);
    void generateNumLiteral(NumLiteral *expr);
    void generateStrLiteral(StrLiteral *expr);
    void generateNullLiteral(NullLiteral *expr);
    void generateBoolLiteral(BoolLiteral *expr);
    void generateArrayLiteral(ArrayLiteral *expr);
    void generateInvokation(Invokation *expr, MethodCallStyle style = NonTailCall);
    void generateMethodInvokation(MethodInvokation *expr, MethodCallStyle style = NonTailCall);
    void generateIdafa(Idafa *expr);
    void generateArrayIndex(ArrayIndex *expr);
    void generateMultiDimensionalArrayIndex(MultiDimensionalArrayIndex *expr);
    void generateObjectCreation(ObjectCreation* expr);

    QString generateArrayFromValues(AST *src, QVector<QSharedPointer<Expression> > values);
    void firstPass(Declaration * decl);
    void secondPass(Declaration * decl);
    void thirdPass(Declaration * decl);
    void checkInheritanceCycles();
    void generateStringConstant(AST *src, QString str);
    void defineInCurrentScope(QString);
    bool isBountInCurrentScope(QString);

    void popProcedureScope();
    void pushProcedureScope(ProceduralDecl *pd);
private:
    void gen(QString str);
    void gen(QString str, int i);
    void gen(QString str, double d);

    void gen(AST *src, QString str);
    void gen(AST *src, QString str, int i);
    void gen(AST *src, QString str, double d);

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
UndefinedVariable,
UnacceptableNumberLiteral,
DeclarationNotSupported,
LValueFormNotImplemented,
UnimplementedExpressionForm,
UnimplementedStatementForm,
UnimplementedInvokationForm,
ProgramsCannotUseExternalModulesWithoutSavingThemFirst
};

class CompilerException
{
    QString message;
    QStringList args;
    CompilerError error;
    static QMap<CompilerError, QString> errorMap;
    QString translateErrorMessage(CompilerError error);
public:
    AST *source;
public:
    CompilerException(AST *source, CompilerError error);
    CompilerException &arg(QString);

    QString getMessage();
};
#endif // COMPILER_H
