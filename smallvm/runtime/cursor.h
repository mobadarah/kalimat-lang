#ifndef CURSOR_H
#define CURSOR_H

#include <QString>

class TextLayer;
class TextBuffer;
struct Cursor
{
    enum Mode { Insert, Overwrite };
    Mode mode, oldMode;
    int line, col;

    int inputStartLine, inputStartCol;
    QString inputBuffer;
    int inputCursorPos;

    TextLayer *owner;
    TextBuffer *buffer;
    Cursor(TextLayer *owner);

    bool inputState();
    void reset();

    void fwd(int n=1);
    void fwdText(const QString &str);
    void back();
    bool inputfwd();
    bool inputback();
    void backSpace();
    void del();

    void cr();
    void lf();
    int cursorPos();
    void lineCol(int &line, int &col);
    bool setCursorPos(int line, int col);
    int cursorLine();
    int cursorColumn();
    void beginInput();
    QString endInput();
    void spreadInputBuffer();
    void typeIn(QString s);
};

#endif // CURSOR_H
