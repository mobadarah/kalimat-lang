#ifndef BREAKPOINT_H
#define BREAKPOINT_H

#include "codedocument.h"

struct Breakpoint
{
    CodeDocument *doc;
    int line;
public:
    Breakpoint() {}
    Breakpoint(CodeDocument *doc, int line);
    bool operator ==(const Breakpoint &other) const;
    bool operator !=(const Breakpoint &other) const;
};

inline uint qHash(const Breakpoint &key)
{
    return qHash(key.line);
}

#endif // BREAKPOINT_H
