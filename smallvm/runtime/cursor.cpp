#include "cursor.h"
#include "textlayer.h"

#include <algorithm> // for min
#include <QDebug>
using namespace std;

Cursor::Cursor(TextLayer *owner)
    :owner(owner)
{
    buffer = &owner->buffer;
    mode = Overwrite;
}

bool Cursor::inputState()
{
    return owner->inputState();
}

void Cursor::reset()
{
    line = col = 0;
}

void Cursor::fwd(int n)
{
    int line1 = line;
    for(int i=0; i<n; ++i)
    {
        col++;
        if(col == textLineWidth)
        {
            lf();
            cr();
            if(inputState())
            {
                // To remove old caret if we wrap around
                owner->updateChangedLines(line1, 1);
            }
        }
    }
}

// Like fwd, but wraps at the end of a line's text instead
// of at textLineWidth
void Cursor::fwdText(const QString &str)
{
    int n = str.length();
    for(int i=0; i<n; ++i)
    {
        QChar ch = str[i];
        if(ch == '\n')
        {
            lf();
            cr();
        }
        else
            fwd();
    }
}

void Cursor::back()
{
    if(col==0 && line ==0)
    {
        return;
    }
    col--;
    int line1 = line;
    if(col == -1)
    {
        line--;
        QString s = buffer->lineAt(line);
        if(s.length() > 0)
            col = s.length()-1;
        else
            col = 0;
        if(inputState())
        {
            // If we're NL'd, we need an update to remove the
            // old caret if we wrap around
            owner->updateChangedLines(line1, 1);
        }

    }
}

bool Cursor::inputfwd()
{
    if((inputCursorPos) < inputBuffer.length())
    {
        inputCursorPos++;
        fwd();
        return true;
    }
    else
    {
        return false;
    }
}

bool Cursor::inputback()
{
    if(inputCursorPos >0)
    {
        inputCursorPos--;
        back();
        return true;
    }
    return false;
}

void Cursor::backSpace()
{
    if(inputCursorPos == 0 || inputBuffer.length()==0)
        return;
    inputBuffer.remove(inputCursorPos-1,1);
    inputBuffer.append(' ');
    spreadInputBuffer();
    inputBuffer.remove(inputBuffer.length()-1);
    inputCursorPos--;
    back();
}

void Cursor::del()
{
    if((inputCursorPos) < inputBuffer.length())
    {
        inputCursorPos++;
        backSpace();
    }
}

void Cursor::spreadInputBuffer()
{
    line = inputStartLine;
    col = inputStartCol;
    int line1 = inputStartLine;
    for(int i=0; i<inputBuffer.length(); ++i)
    {
        buffer->printChar(inputBuffer[i]);
    }
    int line2 = line;
    line = inputStartLine;
    col = inputStartCol;
    if(!inputBuffer.contains('\n'))
        fwd(inputCursorPos);
    else
        fwdText(inputBuffer);
    buffer->dirtyState = true;
    owner->updateChangedLines(line1, line2 - line1 + 1);
}

void Cursor::typeIn(QString s)
{
    if(mode == Insert)
    {
        inputBuffer.insert(inputCursorPos, s);
        inputCursorPos += s.length();
        spreadInputBuffer();
    }
    else
    {
        inputBuffer.replace(inputCursorPos, s.length(), s);
        inputCursorPos += s.length();
        spreadInputBuffer();
    }
}

void Cursor::cr()
{
    col = 0;
}

void Cursor::lf()
{
    line++;
    if(line == visibleTextLines)
    {
        line--;
        inputStartLine--; // todo: this doesn't deal with a long input
                         // where inputStartLine would be negative
        buffer->scrollUp();
        owner->scrollUp();
    }
}


int Cursor::cursorLine()
{
    return line;
}

int Cursor::cursorColumn()
{
    return col;
}

void Cursor::lineCol(int &line, int &col)
{
    line = this->line;
    col = this->col;
}

bool Cursor::setCursorPos(int line, int col)
{
    if(line < 0 || line >= visibleTextLines || col < 0 || col>=textLineWidth)
    {
        return false;
    }
    this->line = line;
    this->col = col;
    return true;
}

void Cursor::beginInput()
{
    inputStartLine = line;
    inputStartCol = col;
    inputCursorPos = 0;
    inputBuffer = "";
    oldMode = mode;
    mode = Insert;
}

QString Cursor::endInput()
{
    mode = oldMode;
    return inputBuffer;
}
