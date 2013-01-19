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
    pos = 0;
}

bool Cursor::fwd()
{
    pos++;
    if(pos == textLineWidth * visibleTextLines)
    {
        pos--;
        return false;
    }
    return true;
}

bool Cursor::back()
{
    if(pos == 0)
        return false;
    pos--;
    return true;
}

bool Cursor::down()
{
    // todo:
    return false;
}

bool Cursor::up()
{
    // todo:
    return false;
}

void Cursor::backSpace()
{
   /*
    Backspace is meaningful only if we are in an input state
    and within the range of the input area
    */

    if(!inputState())
        return;
    if(pos <= inputStartPos)
        return;

    if(pos > (inputStartPos + inputLength))
        return;

    int line1 = line();
    int line2 = (inputStartPos + inputLength) / textLineWidth;

    buffer->insertChar(inputStartPos + inputLength , ' ');
    buffer->removeChar(pos-1);

    inputLength--;
    back();
    owner->updateChangedLines(line(), (line2-line1)+1);
    buffer->dirtyState = true;
}

void Cursor::del()
{
    if((pos) >= (inputStartPos + inputLength))
        return;
    pos++;
    backSpace();
}

void Cursor::typeIn(QString s)
{
    if(mode == Insert)
    {
        QString remainder = buffer->visibleTextBuffer.mid(pos, (inputStartPos + inputLength - pos));
        s+= remainder;
        owner->print(s);
        pos-=remainder.length();
        inputLength = pos - inputStartPos;
    }
    else
    {
        owner->print(s);
        if((pos + s.length()) > (inputStartPos + inputLength))
        {
            inputLength = (pos + s.length()) - inputStartPos;
        }
    }
}

void Cursor::cr()
{
    pos -= column();
}

void Cursor::lf()
{
    pos += textLineWidth;
    if(pos ==(visibleTextLines * textLineWidth))
    {
        pos -= textLineWidth;
        buffer->scrollUp();
        owner->scrollUp();
    }
}

int Cursor::cursorPos()
{
    return pos;
}

int Cursor::line()
{
    return pos / textLineWidth;
}

int Cursor::column()
{
    return pos % textLineWidth;
}

void Cursor::lineCol(int &line, int &col)
{
    line = pos / textLineWidth;
    col = pos % textLineWidth;
}

bool Cursor::setCursorPos(int line, int col)
{
    if(line < 0 || line >= visibleTextLines || col < 0 || col>=textLineWidth)
    {
        return false;
    }
    pos = line * textLineWidth + col;
    return true;
}

void Cursor::beginInput()
{
    inputStartPos = pos;
    inputLength = 0;
    oldMode = mode;
    mode = Insert;
}

QString Cursor::endInput()
{
    mode = oldMode;
    return buffer->visibleTextBuffer.mid(inputStartPos, inputLength);
}
