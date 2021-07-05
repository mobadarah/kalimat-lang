#ifndef KALIMATPARSERERROR_H
#define KALIMATPARSERERROR_H

enum KalimatParserError
{
    ExpectedDeclaration,
    ExpectedExpression,
    ExpectedSimpleLiteral,
    ExpectedStatementOrDeclaration,
    ExpectedDrawingStatement,
    ExpectedIdentifier,
    ExpectedSendOrReceiveOperation,
    ExpressionExpectedAfterGoto,
    ExpectedStringLiteral,
    UsingKeywordMustBeFollowedByStringLiteral,
    ModulesCannotContainStatements,
    StatementNotImplemented,
    LeftOfAssignmentMustBeAssignableExpression,
    DeclaringTypeInAssignmentMustBeForVariable,
    MeaninglessTypeDeclaration,
    CanDelegateOnlyToInvokation,
    CanOnlyLaunchProcedureInvokation,
    PropertySetterMustBeginWithSet,
    PropertySetterTakesOneArgument,
    PropertyGetterTakesNoArguments,
    NoSuchMethodHandlerTakesTwoArguments,
    ExpectedStringWithSingleCharacter,
    InvalidStartOfPrimaryPegExpression
};

#endif // KALIMATPARSERERROR_H
