#include "breakpoint.h"

Breakpoint::Breakpoint(CodeDocument *doc, int line)
{
    this->doc = doc;
    this->line = line;
}

bool Breakpoint::operator ==(const Breakpoint &other) const
{
    return this->doc == other.doc && this->line == other.line;
}

bool Breakpoint::operator !=(const Breakpoint &other) const
{
    return !(*this == other);
}
