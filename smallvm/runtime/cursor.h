#ifndef CURSOR_H
#define CURSOR_H

#include <QString>

class TextLayer;
class TextBuffer;
struct Cursor
{
    enum Mode { Insert, Overwrite };
    Mode mode, oldMode;
    int pos;

    int inputStartPos, inputLength;

    TextLayer *owner;
    TextBuffer *buffer;
    Cursor(TextLayer *owner);

    bool inputState();
    void reset();

    bool fwd();
    bool back();
    bool down();
    bool up();

    void backSpace();
    void del();

    void cr();
    void lf();
    int cursorPos();
    void lineCol(int &line, int &col);
    bool setCursorPos(int line, int col);
    int line();
    int column();
    void beginInput();
    QString endInput();
    void typeIn(QString s);
};

#endif // CURSOR_H
