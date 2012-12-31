#ifndef RUNTIMEIDENTIFIERS_H
#define RUNTIMEIDENTIFIERS_H

#include "utils.h"

namespace RId
{
    enum RuntimeId
    {
        Object,
        Numeric,
        Integer,
        Long,
        Double,
        Boolean,
        Method,
        ExternalMethod,
        ExternalLibrary,
        Class,
        Indexable,
        VArray,
        VMap,
        String,
        Sprite,
        RawFile,
        Window,
        Reference,
        FieldReference,
        ArrayReference,
        NullType,
        Channel,
        QObject,
        ActivationRecord,
        c_int32,
        c_long,
        c_float,
        c_double,
        c_char,
        c_ascii,
        c_wstr,
        c_pointer,
        c_void,
        ForeignWindow,
        Button,
        TextBox,
        TextLine,
        ListBox,
        Label,
        CheckBox,
        RadioButton,
        ButtonGroup,
        ComboBox,
        Image,
        Parser,
        ParseResultClass,
        PointerOf1,
        NullValue,
        ChannelValue,
        TrueValue,
        FalseValue,
        ArrayWithDims1,
        NewObject,
        SetStoredValue,
        GetStoredValue,
        DisableCollision,
        EnableCollision,
        Rotated, // Image methods
        Stretched,
        DrawLine,
        Flipped,
        Copied,
        SetPixelColor,
        PixelColor,
        ImgWidth,
        ImgHeight,
        DrawText, // end image methods
        Maximize,
        MoveTo,
        Add,
        SetSize,
        SetTitle,
        SetText,
        SetLocation,
        Text,
        Click,
        AppendText,
        Changed,
        InsertItem,
        GetItem,
        SelectionChanged,
        SetEditable,
        TextChanged,
        SetValue,
        Value,
        ValueChanged,
        Selection,
        GetButton,
        ButtonSelected,
        MouseEventInfo,
        X,
        Y,
        LeftButton,
        RightButton,
        KBEventInfo,
        Key,
        Character,
        Invoke,
        LocalVar,
        SetLocalVar,
        PushBacktrackPoint,
        GotoBacktrackPoint,
        IgnoreLastBacktrackPoint,
        FailAndBackTrack,
        MoveNext,
        MoveNextMany,
        Peek,
        PeekMany,
        LookAhead,
        LookAheadMany,
        RuleCall,
        RuleReturn,
        PushLocals,
        PopLocals,
        EndOfInput,
        Dump,
        Memoize,
        GetMemoized,
        IsMemoized,
        CallCount,
        LookAheadRange,
        Data,
        InputPos,
        ParseResultOf,
        ParserGeneratedVarName,
        ParserGeneratedInputArgName,
        ParserGeneratedResultVarName,
        ParserGeneratedPosVarName,
        LblEndOfParsing,
        TmpParseFrame,
        ParserSuccessVarName,
        ParserTempVarName,
        ParseLblParseError,
        NewLine,
        MainProgram,
        Subtraction,
        Multiplication,
        Division
    };
}

class VMId
{
    static Translation<RId::RuntimeId> *runtime;
public:
    static void init();
    static QString get(RId::RuntimeId id);
    static QString get(RId::RuntimeId id, QString);
};

#endif
