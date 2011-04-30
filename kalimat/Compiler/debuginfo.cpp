#include "debuginfo.h"

DebugInfo::DebugInfo()
{
}

void DebugInfo::setInstructionForLine(CodeDocument *doc, int lineNo, QString methodName, int offset)
{
    InstructionLocation loc;
    loc.methodName = methodName;
    loc.offset = offset;
    info[doc][lineNo] = loc;
}

bool DebugInfo::instructionFromLine(CodeDocument *doc, int lineNo, QString &methodName, int &offset)
{
    if(info.contains(doc) && info[doc].contains(lineNo))
    {
        methodName = info[doc][lineNo].methodName;
        offset = info[doc][lineNo].offset;
        return true;
    }
    return false;
}
