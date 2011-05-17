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

struct LineLocation
{
    CodeDocument *doc;
    int lineNo;
};

class DebugInfo
{
    QMap<CodeDocument *, QMap<int, InstructionLocation> > info;
    QMap<QString, QMap<int, LineLocation> > reverseInfo;
    QMap<CodeDocument *, QSet<int> > returnLines;
public:
    DebugInfo();
    void setInstructionForLine(CodeDocument *doc, int lineNo, QString methodName, int offset);
    void setReturnLine(CodeDocument *, int);

    bool instructionFromLine(CodeDocument *doc, int lineNo, QString &methodName, int &offset) const;
    bool lineFromInstruction(QString methodName, int offset, CodeDocument *&doc, int &lineNo) const;
    bool isReturnLine(CodeDocument *, int);
};

#endif // DEBUGINFO_H
