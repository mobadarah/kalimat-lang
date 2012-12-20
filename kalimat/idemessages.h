#ifndef IDEMESSAGES_H
#define IDEMESSAGES_H

#include "../smallvm/utils.h"

namespace IdeMsg
{
    enum IdeMessage
    {
        CannotRunProgram,
        CannotRunNewProgramAtMiddleOfBreakPoint,
        UnexpectedEof,
        UnexpectedToken1,
        SyntaxError,
        Line1,
        Column1,
        TokenPos3,
        CannotExecuteUnit2,
        CannotExecuteUnit,
        MainProgram,
        TopLevel,
        Untitled,
        FileNotFound,
        FileNotFoundCreateIt1,
        StartOfFileReached,
        EndOfFileReached,
        BreakpointReached,
        Kalimat,
        ChooseExePath,
        CreatingExecutable,
        ErrorCreatingExecutable1,
        SuccessCreatingExe,
        UpdateBanner,
        ClickToOpenFile,
        Procedure,
        Function
    };
}

class Ide
{
public:
    static Translation<IdeMsg::IdeMessage> msg;
};

#endif // IDEMESSAGES_H
