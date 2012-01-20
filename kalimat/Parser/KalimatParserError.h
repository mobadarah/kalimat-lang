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
    ModulesCannotContainStatements,
    StatementNotImplemented,
    LeftOfAssignmentMustBeAssignableExpression
};

#endif // KALIMATPARSERERROR_H
