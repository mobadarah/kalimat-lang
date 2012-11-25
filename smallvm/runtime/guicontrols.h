#ifndef GUICONTROLS_H
#define GUICONTROLS_H

#ifndef CLASSES_H
    #include "../smallvm/classes.h"
#endif

#ifndef EASYFOREIGNCLASS_H
    #include "../smallvm/easyforeignclass.h"
#endif

#ifndef VALUE_H
    #include "../smallvm/value.h"
#endif

#ifndef ALLOCATOR_H
    #include "../smallvm/allocator.h"
#endif

class Allocator;
class RunWindow;

const int methodSetText = 0;
const int methodSetSize = 1;
const int methodSetPos = 2;
const int methodGetText = 4;

const int controlMethodCutoff = 4;

const int methodAddItem = 5;
const int methodInsertItem = 6;
const int methodGetItem = 7;
const int methodGetValue = 8;
const int methodSetValue = 9;
const int methodAddButton = 10;
const int methodGetButton = 11;

const int methodSetEditable = 12;
const int methodAppendText = 13;

class ImageForeignClass : public EasyForeignClass
{
    Allocator *allocator;
    RunWindow *rw;
public:
    ImageForeignClass(QString name, RunWindow *rw);
    IObject *newValue(Allocator *allocator);
    Value *dispatch(Process *proc, int id, QVector<Value *>args);
};

class WindowForeignClass : public EasyForeignClass
{
    RunWindow *rw;
public:
    WindowForeignClass(QString name, RunWindow *rw);
    IObject *newValue(Allocator *allocator);
    Value *dispatch(Process *proc, int id, QVector<Value *>args);
};

class WindowForeignObject : public IObject
{
public:
    Value *handle;
public:
    virtual bool hasSlot(QString name);
    virtual QList<QString> getSlotNames();
    virtual Value *getSlotValue(QString name);
    virtual void setSlotValue(QString name, Value *val);
    QString toString();
};

class ControlForeignClass : public QObject, public EasyForeignClass
{
    Q_OBJECT
protected:
    RunWindow *rw; // for TX
    Allocator *allocator;
public:
    ControlForeignClass(QString name, RunWindow *rw);
    virtual IObject *newValue(Allocator *allocator);
    virtual Value *dispatch(Process *proc, int id, QVector<Value *>args);
};

class ButtonForeignClass : public ControlForeignClass
{
    Q_OBJECT
public:
    ButtonForeignClass(QString name, RunWindow *rw);
    IObject *newValue(Allocator *allocator);
    Value *dispatch(Process *proc, int id, QVector<Value *>args);
public slots:
    void on_button_clicked();
};

class TextboxForeignClass : public ControlForeignClass
{
    Q_OBJECT
public:
    TextboxForeignClass(QString name, RunWindow *rw);
    IObject *newValue(Allocator *allocator);
    Value *dispatch(Process *proc, int id, QVector<Value *>args);
public slots:
    void on_text_changed();
};

class LineEditForeignClass : public ControlForeignClass
{
    Q_OBJECT
public:
    LineEditForeignClass(QString name, RunWindow *rw);
    IObject *newValue(Allocator *allocator);
    Value *dispatch(Process *proc, int id, QVector<Value *>args);
public slots:
    void on_text_changed();
};


class ListboxForeignClass : public ControlForeignClass
{
    Q_OBJECT
public:
    ListboxForeignClass(QString name, RunWindow *rw);
    IObject *newValue(Allocator *allocator);
    Value *dispatch(Process *proc, int id, QVector<Value *>args);
public slots:
    void on_select(int);
};

class ComboboxForeignClass : public ControlForeignClass
{
    Q_OBJECT
public:
    ComboboxForeignClass(QString name, RunWindow *rw);
    IObject *newValue(Allocator *allocator);
    Value *dispatch(Process *proc, int id, QVector<Value *>args);
public slots:
    void on_select(int);
    void on_text_changed(QString);
};

class LabelForeignClass : public ControlForeignClass
{
    Q_OBJECT
public:
    LabelForeignClass(QString name, RunWindow *rw);
    IObject *newValue(Allocator *allocator);
    Value *dispatch(Process *proc, int id, QVector<Value *>args);
};

class CheckboxForeignClass : public ControlForeignClass
{
    Q_OBJECT
public:
    CheckboxForeignClass(QString name, RunWindow *rw);
    IObject *newValue(Allocator *allocator);
    Value *dispatch(Process *proc, int id, QVector<Value *>args);
public slots:
    void value_changed(int);
};

class RadioButtonForeignClass : public ControlForeignClass
{
    Q_OBJECT
public:
    RadioButtonForeignClass(QString name, RunWindow *rw);
    IObject *newValue(Allocator *allocator);
    Value *dispatch(Process *proc, int id, QVector<Value *>args);
public slots:
    void value_changed(bool);
};

class ButtonGroupForeignClass : public QObject, public EasyForeignClass
{
    Q_OBJECT
    RunWindow *rw;
    Allocator *allocator;
    int runningIdCount;
public:
    ButtonGroupForeignClass(QString name, RunWindow *rw);
    IObject *newValue(Allocator *allocator);
    Value *dispatch(Process *proc, int id, QVector<Value *>args);
public slots:
    void button_clicked(int);
};
#endif // GUICONTROLS_H
