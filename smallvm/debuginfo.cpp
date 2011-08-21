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

    LineLocation loc2;
    loc2.doc = doc;
    loc2.lineNo = lineNo;
    reverseInfo[methodName][offset] = loc2;
}

bool DebugInfo::instructionFromLine(CodeDocument *doc, int lineNo, QString &methodName, int &offset) const
{
    if(info.contains(doc) && info[doc].contains(lineNo))
    {
        methodName = info[doc][lineNo].methodName;
        offset = info[doc][lineNo].offset;
        return true;
    }
    return false;
}

bool DebugInfo::lineFromInstruction(QString methodName, int offset, CodeDocument *&doc, int &lineNo) const
{
    if(reverseInfo.contains(methodName) && reverseInfo[methodName].contains(offset))
    {
        doc = reverseInfo[methodName][offset].doc;
        lineNo = reverseInfo[methodName][offset].lineNo;
        return true;
    }
    return false;
}

void DebugInfo::setReturnLine(CodeDocument *doc, int line)
{
    returnLines[doc].insert(line);
}

bool DebugInfo::isReturnLine(CodeDocument *doc, int line)
{
    return returnLines[doc].contains(line);
}
