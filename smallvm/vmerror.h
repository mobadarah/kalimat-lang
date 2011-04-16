/**************************************************************************
**   The Kalimat programming language
**   Copyright (C) 2010 Mohamed Samy Ali - samy2004@gmail.com
**   This project is released under the Apache License version 2.0
**   as described in the included license.txt file
**************************************************************************/

#ifndef VMERROR_H
#define VMERROR_H

enum VMErrorType
{
    // The number after sme of the errors indicates the # of arguments
    // required to complete the error message string
    NoSuchVariable1, NoSuchProcedure1, NoSuchProcedureOrFunction1,
    NoSuchField2, NoSuchMethod2, NoSuchExternalMethod1, NoSuchEvent,
    NoSuchClass, NameDoesntIndicateAClass1,

    UnrecognizedInstruction,UnrecognizedMnemonic,
    GettingFieldOnNonObject, SettingFieldOnNonObject, CallingMethodOnNonObject,
    GettingFieldOnNull, SettingFieldOnNull, CallingMethodOnNull,
    NumericOperationOnNonNumber1, LogicOperationOnNonBoolean,

    DivisionByZero,
    BuiltInOperationOnNonBuiltn,
    TypeError2, InternalError, WrongNumberOfArguments, FunctionDidntReturnAValue,
    SubscribingNonArray, SubscribtMustBeInteger, SubscriptOutOfRange2, SubscriptOutOfRange3,
    ArgumentError, MD_IndexingWrongNumberOfDimensions,
    ElementAlreadyDefined1, MethodAlreadyDefinedInClass,
    RuntimeError,
    NoError
};

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
