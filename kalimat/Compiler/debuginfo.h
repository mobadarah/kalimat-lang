#ifndef DEBUGINFO_H
#define DEBUGINFO_H

#include <QMap>

#ifndef KALIMATAST_H
    #include "../Parser/kalimatast.h"
#endif

#ifndef CODEDOCUMENT_H
    #include "../codedocument.h"
#endif

struct InstructionLocation
{
    QString methodName;
    int offset;
};

class DebugInfo
{
    QMap<CodeDocument *, QMap<int, InstructionLocation> > info;
public:
    DebugInfo();
    void setInstructionForLine(CodeDocument *doc, int lineNo, QString methodName, int offset);
    bool instructionFromLine(CodeDocument *doc, int lineNo, QString &methodName, int &offset);
};

#endif // DEBUGINFO_H
