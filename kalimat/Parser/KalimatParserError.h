#ifndef KALIMATPARSERERROR_H
#define KALIMATPARSERERROR_H

enum KalimatParserError
{
    ExpectedDeclaration,
    ExpectedExpression,
    ExpectedStatementOrDeclaration,
    ExpectedDrawingStatement,
    ExpectedIdentifier,
    ExpectedSendOrReceiveOperation,
    IdentifierOrNumberExpectedAfterGoto,
    UsingKeywordMustBeFollowedByStringLiteral,
    ModulesCannotContainStatements,
    StatementNotImplemented,
    LeftOfAssignmentMustBeAssignableExpression,
    CanDelegateOnlyToInvokation,
    CanOnlyLaunchProcedureInvokation
};

#endif // KALIMATPARSERERROR_H
