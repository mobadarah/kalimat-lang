/**************************************************************************
**   The Kalimat programming language
**   Copyright (C) 2010 Mohamed Samy Ali - samy2004@gmail.com
**   This project is released under the Apache License version 2.0
**   as described in the included license.txt file
**************************************************************************/

#ifndef VMERROR_H
#define VMERROR_H

#ifndef FRAME_H
    #include "frame.h"
#endif

enum VMErrorType
{
    // The number after some of the errors indicates the # of arguments
    // required to complete the error message string
    NoSuchVariable1, NoSuchProcedure1, NoSuchProcedureOrFunction1,
    NoSuchField2, NoSuchMethod2, NoSuchExternalMethod1, NoSuchEvent,
    NoSuchClass1, NameDoesntIndicateAClass1,

    UnrecognizedInstruction,UnrecognizedMnemonic2,
    GettingFieldOnNonObject1, SettingFieldOnNonObject1, CallingMethodOnNonObject,
    GettingFieldOnNull, SettingFieldOnNull, CallingMethodOnNull,
    NumericOperationOnNonNumber2,
    NumericOperationOnNonNumber3, LogicOperationOnNonBoolean,

    DivisionByZero,
    BuiltInOperationOnNonBuiltn2,
    IncorrectJumpLabel1,
    JumpingToNonExistentLabel1,
    TypeError2, InternalError, InternalError1, WrongNumberOfArguments3, FunctionDidntReturnAValue1,
    SubscribingNonArray, SubscribtMustBeInteger, UnacceptableKeyFormMap1, IndexableNotFound1,
    SubscriptOutOfRange2, SubscriptOutOfRange3,
    ArgumentError, MD_IndexingWrongNumberOfDimensions,
    ElementAlreadyDefined1, MethodAlreadyDefinedInClass,
    RuntimeError,
    NoError
};

namespace ArgErr
{
    enum ArgError
    {
        InvalidCursorPosition,
        RandTakesPositiveValues,
        CannotConvertStrToInt1,
        NonExistingImageFile1,
        NonExistingSpriteFile1,
        InvalidArgRange3,
        X,
        Y,
        InvalidConstantName1,
        CannotWriteToClosedFile,
        CannotReadFromClosedFile,
        CannotOperateOnClosedFile,
        TryingToOpenMissingFile1,
        FailedToOpenFile1,
        SentValueHasToBeAnObject1,
        StackTopNotBacktrackPoint1,
        BadFrameNumber1,
        CannotUsePartialFileName1,
        NoMainFuncToExecute,
        Editing1,
        Ok,
        Cancel
    };

}

class VMError
{
public:

    VMErrorType type;
    QStack<Frame> callStack;
    QVector<QString> args;
public:
    VMError(VMErrorType type, QStack<Frame> callStack);
    VMError(VMErrorType type);
    VMError();
    VMError &arg(QString s);
};

#endif // VMERROR_H
