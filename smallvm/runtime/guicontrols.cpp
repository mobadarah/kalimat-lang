#include "guicontrols.h"
#include "runwindow.h"

#include <QWidget>
#include <QPushButton>
#include <QVariant>
#include <QTextEdit>
#include <QLineEdit>
#include <QListWidget>
#include <QLabel>
#include <QCheckBox>
#include <QRadioButton>
#include <QButtonGroup>
#include <QComboBox>
#include <QApplication>
#include <QShowEvent>
#include <QMutex>

#include "../runtime_identifiers.h"

#define _ws(a) QString::fromStdWString(a)

void ensureValueIsWidget(Value *val)
{
    if(!(val->tag == ObjectVal))
        throw VMError(InternalError);
    if(!(val->unboxObj()->hasSlot("handle")))
        throw VMError(InternalError);
    val = val->unboxObj()->getSlotValue("handle");
    if(!(val->tag == QObjectVal))
        throw VMError(InternalError);
    QObject *obj = val->unboxQObj();
    QWidget *w = dynamic_cast<QWidget *>(obj);
    if(!w)
        throw VMError(InternalError);
}

ImageForeignClass::ImageForeignClass(QString name, RunWindow *rw, VM *vm)
    :EasyForeignClass(name, vm), rw(rw)
{
    attachVmMethod(VMId::get(RId::Rotated));
    attachVmMethod(VMId::get(RId::Stretched));
    attachVmMethod(VMId::get(RId::DrawLine));
    attachVmMethod(VMId::get(RId::Flipped));
    attachVmMethod(VMId::get(RId::Copied));
    attachVmMethod(VMId::get(RId::SetPixelColor));
    attachVmMethod(VMId::get(RId::PixelColor));
    attachVmMethod(VMId::get(RId::ImgWidth));
    attachVmMethod(VMId::get(RId::ImgHeight));
    attachVmMethod(VMId::get(RId::DrawText));
}

IObject *ImageForeignClass::newValue(Allocator *allocator)
{
    //todo: this is a hack
    this->allocator = allocator;

    Object *obj = new Object();
    obj->slotNames.append("handle");
    return obj;
}

Value *ImageForeignClass::dispatch(Process *proc, int id, QVector<Value *> args)
{
    return NULL;
}

WindowForeignClass::WindowForeignClass(QString name, RunWindow *rw, VM *vm)
    : EasyForeignClass(name, vm)
{
    this->rw = rw;

    attachVmMethod(VMId::get(RId::Maximize));
    attachVmMethod(VMId::get(RId::MoveTo));
    attachVmMethod(VMId::get(RId::Add));
    attachVmMethod(VMId::get(RId::SetSize));
    attachVmMethod(VMId::get(RId::SetTitle));

    fields.insert("handle");
}

IObject *WindowForeignClass::newValue(Allocator *allocator)
{
    WindowForeignObject *window = new WindowForeignObject();
    return window;
}

Value *WindowForeignClass::dispatch(Process *proc, int id, QVector<Value *> args)
{
    return NULL;
}

bool WindowForeignObject::hasSlot(QString name)
{
    if(name == "handle")
    {
        return true;
    }
    return false;
}

QList<QString> WindowForeignObject::getSlotNames()
{
    QList<QString> ret;
    ret.append("handle");
    return ret;
}

Value *WindowForeignObject::getSlotValue(QString name)
{
    if(name == "handle")
        return handle;
    return NULL;
}

void WindowForeignObject::setSlotValue(QString name, Value *val)
{
    if(name == "handle")
    {
        handle = val;
    }
}

QString WindowForeignObject::toString()
{
    return QString("<%1>").arg(VMId::get(RId::Window));
}

ControlForeignClass::ControlForeignClass(QString name, RunWindow *rw, VM *vm)
    : EasyForeignClass(name, vm)
{
    this->rw = rw;
    fields.insert("handle");

    attachVmMethod("control", VMId::get(RId::SetText));
    attachVmMethod("control", VMId::get(RId::SetSize));
    attachVmMethod("control", VMId::get(RId::SetLocation));
    attachVmMethod("control", VMId::get(RId::Text));
}

IObject *ControlForeignClass::newValue(Allocator *allocator)
{
    this->allocator = allocator; // todo: this is a hack
    Object *newObj = new Object();
    newObj->slotNames.append("handle");
    return newObj;
}

Value *ControlForeignClass::dispatch(Process *proc, int id, QVector<Value *> args)
{
    return NULL;
}

ButtonForeignClass::ButtonForeignClass(QString name, RunWindow *rw, VM *vm)
    : ControlForeignClass(name, rw, vm)
{
    attachVmMethod(VMId::get(RId::SetText));
    attachVmMethod(VMId::get(RId::Text));

    fields.insert(VMId::get(RId::Click));
}

IObject *ButtonForeignClass::newValue(Allocator *allocator)
{

    Object *newObj = (Object *) ControlForeignClass::newValue(allocator);

    newObj->slotNames.append(VMId::get(RId::Click));

    QPushButton *button = new QPushButton();
    button->setProperty("objectof", QVariant::fromValue<void *>(newObj));
    newObj->setSlotValue("handle", allocator->newQObject(button));

    connect(button, SIGNAL(clicked()), this, SLOT(on_button_clicked()));
    newObj->setSlotValue(VMId::get(RId::Click), allocator->newChannel());

    return newObj;
}

void ButtonForeignClass::on_button_clicked()
{
    QPushButton *pb = (QPushButton *) sender();
    Object *object = (Object *) pb->property("objectof").value<void *>();
    Channel *chan = object->getSlotValue(VMId::get(RId::Click))->unboxChan();
    chan->send(allocator->null(), NULL);
}

Value *ButtonForeignClass::dispatch(Process *proc, int id, QVector<Value *> args)
{
    return NULL;
}

TextboxForeignClass::TextboxForeignClass(QString name, RunWindow *rw, VM *vm)
    : ControlForeignClass(name, rw, vm)
{
    attachVmMethod(VMId::get(RId::SetText));
    attachVmMethod(VMId::get(RId::Text));
    attachVmMethod(VMId::get(RId::AppendText));

    fields.insert(VMId::get(RId::Changed));
}

IObject *TextboxForeignClass::newValue(Allocator *allocator)
{
    Object *newObj = (Object *) ControlForeignClass::newValue(allocator);

    newObj->slotNames.append(VMId::get(RId::Changed));

    QTextEdit *tb = new QTextEdit();
    tb->setProperty("objectof", QVariant::fromValue<void *>(newObj));
    newObj->setSlotValue("handle", allocator->newQObject(tb));

    connect(tb, SIGNAL(textChanged()), this, SLOT(on_text_changed()));
    newObj->setSlotValue(VMId::get(RId::Changed), allocator->newChannel());
    return newObj;
}

void TextboxForeignClass::on_text_changed()
{
    QTextEdit *te = (QTextEdit *) sender();
    Object *object = (Object *) te->property("objectof").value<void *>();
    Channel *chan = object->getSlotValue(VMId::get(RId::Changed))->unboxChan();
    chan->send(allocator->null(), NULL);
}

Value *TextboxForeignClass::dispatch(Process *proc, int id, QVector<Value *> args)
{
    return NULL;
}

LineEditForeignClass::LineEditForeignClass(QString name, RunWindow *rw, VM *vm)
    : ControlForeignClass(name, rw, vm)
{
    attachVmMethod(VMId::get(RId::SetText));
    attachVmMethod(VMId::get(RId::Text));
    attachVmMethod(VMId::get(RId::AppendText));

    fields.insert(VMId::get(RId::Changed));
}

IObject *LineEditForeignClass::newValue(Allocator *allocator)
{
    Object *newObj = (Object *) ControlForeignClass::newValue(allocator);

    newObj->slotNames.append(VMId::get(RId::Changed));

    QLineEdit *tb = new QLineEdit();
    tb->setProperty("objectof", QVariant::fromValue<void *>(newObj));
    newObj->setSlotValue("handle", allocator->newQObject(tb));

    connect(tb, SIGNAL(textChanged(QString)), this, SLOT(on_text_changed()));
    newObj->setSlotValue(VMId::get(RId::Changed), allocator->newChannel());
    return newObj;
}

void LineEditForeignClass::on_text_changed()
{
    QLineEdit *te = (QLineEdit *) sender();
    Object *object = (Object *) te->property("objectof").value<void *>();
    Channel *chan = object->getSlotValue(VMId::get(RId::Changed))->unboxChan();
    chan->send(allocator->null(), NULL);
}

Value *LineEditForeignClass::dispatch(Process *proc, int id, QVector<Value *> args)
{
    return NULL;
}

ListboxForeignClass::ListboxForeignClass(QString name, RunWindow *rw, VM *vm)
    : ControlForeignClass(name, rw, vm)
{
    attachVmMethod(VMId::get(RId::Add));
    attachVmMethod(VMId::get(RId::InsertItem));
    attachVmMethod(VMId::get(RId::GetItem));

    fields.insert(VMId::get(RId::SelectionChanged));
}

IObject *ListboxForeignClass::newValue(Allocator *allocator)
{
    Object *newObj = (Object *) ControlForeignClass::newValue(allocator);

    newObj->slotNames.append(VMId::get(RId::SelectionChanged));

    QListWidget *tb = new QListWidget();
    tb->setProperty("objectof", QVariant::fromValue<void *>(newObj));
    newObj->setSlotValue("handle", allocator->newQObject(tb));

    connect(tb, SIGNAL(currentRowChanged(int)), this, SLOT(on_select(int)));
    newObj->setSlotValue(VMId::get(RId::SelectionChanged), allocator->newChannel());
    return newObj;
}

void ListboxForeignClass::on_select(int selection)
{
    QListWidget *te = (QListWidget *) sender();
    Object *object = (Object *) te->property("objectof").value<void *>();
    Channel *chan = object->getSlotValue(VMId::get(RId::SelectionChanged))->unboxChan();
    chan->send(allocator->newInt(selection), NULL);
}

Value *ListboxForeignClass::dispatch(Process *proc, int id, QVector<Value *> args)
{
    return NULL;
}

ComboboxForeignClass::ComboboxForeignClass(QString name, RunWindow *rw, VM *vm)
    : ControlForeignClass(name, rw, vm)
{
    attachVmMethod(VMId::get(RId::Add));
    attachVmMethod(VMId::get(RId::InsertItem));
    attachVmMethod(VMId::get(RId::GetItem));

    attachVmMethod(VMId::get(RId::SetEditable));

    attachVmMethod(VMId::get(RId::SetText));
    attachVmMethod(VMId::get(RId::Text));

    fields.insert(VMId::get(RId::SelectionChanged));
    fields.insert(VMId::get(RId::TextChanged));
}

IObject *ComboboxForeignClass::newValue(Allocator *allocator)
{
    Object *newObj = (Object *) ControlForeignClass::newValue(allocator);

    newObj->slotNames.append(VMId::get(RId::SelectionChanged));
    newObj->slotNames.append(VMId::get(RId::TextChanged));

    QComboBox *cb = new QComboBox();
    cb->setEditable(true);
    cb->setProperty("objectof", QVariant::fromValue<void *>(newObj));
    newObj->setSlotValue("handle", allocator->newQObject(cb));

    connect(cb, SIGNAL(currentIndexChanged(int)), this, SLOT(on_select(int)));
    connect(cb, SIGNAL(editTextChanged(QString)), this, SLOT(on_text_changed(QString)));

    newObj->setSlotValue(VMId::get(RId::SelectionChanged), allocator->newChannel());
    newObj->setSlotValue(VMId::get(RId::TextChanged), allocator->newChannel());
    return newObj;
}

void ComboboxForeignClass::on_select(int selection)
{
    QComboBox *cb = (QComboBox *) sender();
    Object *object = (Object *) cb->property("objectof").value<void *>();
    Channel *chan = object->getSlotValue(VMId::get(RId::SelectionChanged))->unboxChan();
    chan->send(allocator->newInt(selection), NULL);
}

void ComboboxForeignClass::on_text_changed(QString)
{
    QComboBox *cb = (QComboBox *) sender();
    Object *object = (Object *) cb->property("objectof").value<void *>();
    Channel *chan = object->getSlotValue(VMId::get(RId::TextChanged))->unboxChan();
    chan->send(allocator->null(), NULL);
}

Value *ComboboxForeignClass::dispatch(Process *proc, int id, QVector<Value *> args)
{
    return NULL;
}

LabelForeignClass::LabelForeignClass(QString name, RunWindow *rw, VM *vm)
    : ControlForeignClass(name, rw, vm)
{
    attachVmMethod(VMId::get(RId::SetText));
    attachVmMethod(VMId::get(RId::Text));
}

IObject *LabelForeignClass::newValue(Allocator *allocator)
{
    Object *newObj = (Object *) ControlForeignClass::newValue(allocator);

    QLabel *tb = new QLabel();
    tb->setProperty("objectof", QVariant::fromValue<void *>(newObj));
    newObj->setSlotValue("handle", allocator->newQObject(tb));

    return newObj;
}

Value *LabelForeignClass::dispatch(Process *proc, int id, QVector<Value *> args)
{
    return NULL;
}

CheckboxForeignClass::CheckboxForeignClass(QString name, RunWindow *rw, VM *vm)
    : ControlForeignClass(name, rw, vm)
{
    attachVmMethod(VMId::get(RId::SetText));
    attachVmMethod(VMId::get(RId::Text));
    attachVmMethod(VMId::get(RId::SetValue));
    attachVmMethod(VMId::get(RId::Value));

    fields.insert(VMId::get(RId::ValueChanged));
}

IObject *CheckboxForeignClass::newValue(Allocator *allocator)
{
    Object *newObj = (Object *) ControlForeignClass::newValue(allocator);

    newObj->slotNames.append(VMId::get(RId::ValueChanged));

    QCheckBox *tb = new QCheckBox();
    tb->setProperty("objectof", QVariant::fromValue<void *>(newObj));
    newObj->setSlotValue("handle", allocator->newQObject(tb));

    connect(tb, SIGNAL(stateChanged(int)), this, SLOT(value_changed(int)));
    newObj->setSlotValue(VMId::get(RId::ValueChanged), allocator->newChannel());
    return newObj;
}

void CheckboxForeignClass::value_changed(int newState)
{
    QCheckBox *te = (QCheckBox *) sender();
    Object *object = (Object *) te->property("objectof").value<void *>();
    Channel *chan = object->getSlotValue(VMId::get(RId::ValueChanged))->unboxChan();
    chan->send(allocator->newInt(newState), NULL);
}

Value *CheckboxForeignClass::dispatch(Process *proc, int id, QVector<Value *> args)
{
    return NULL;
}

RadioButtonForeignClass::RadioButtonForeignClass(QString name, RunWindow *rw, VM *vm)
    : ControlForeignClass(name, rw, vm)
{
    attachVmMethod(VMId::get(RId::SetText));
    attachVmMethod(VMId::get(RId::Text));
    attachVmMethod(VMId::get(RId::SetValue));
    attachVmMethod(VMId::get(RId::Value));

    fields.insert(VMId::get(RId::Selection));
}

IObject *RadioButtonForeignClass::newValue(Allocator *allocator)
{
    Object *newObj = (Object *) ControlForeignClass::newValue(allocator);

    newObj->slotNames.append(VMId::get(RId::Selection));

    QRadioButton *tb = new QRadioButton();
    tb->setProperty("objectof", QVariant::fromValue<void *>(newObj));
    newObj->setSlotValue("handle", allocator->newQObject(tb));

    connect(tb, SIGNAL(toggled(bool)), this, SLOT(value_changed(bool)));
    newObj->setSlotValue(VMId::get(RId::Selection), allocator->newChannel());
    return newObj;
}

void RadioButtonForeignClass::value_changed(bool newState)
{
    if(newState)
    {
        QRadioButton *te = (QRadioButton *) sender();
        Object *object = (Object *) te->property("objectof").value<void *>();
        Channel *chan = object->getSlotValue(VMId::get(RId::Selection))->unboxChan();
        chan->send(allocator->null(), NULL);
    }
}

Value *RadioButtonForeignClass::dispatch(Process *proc, int id, QVector<Value *> args)
{
    return NULL;
}

ButtonGroupForeignClass::ButtonGroupForeignClass(QString name, RunWindow *rw, VM *vm)
    : EasyForeignClass(name, vm)
{
    this->rw = rw;
    runningIdCount = 1;

    attachVmMethod(VMId::get(RId::Add));
    attachVmMethod(VMId::get(RId::GetButton));

    fields.insert(VMId::get(RId::ButtonSelected));
}

IObject *ButtonGroupForeignClass::newValue(Allocator *allocator)
{
    this->allocator = allocator; // todo: this is a hack
    Object *newObj = new Object();
    newObj->slotNames.append("handle");

    newObj->slotNames.append(VMId::get(RId::ButtonSelected));

    QButtonGroup *tb = new QButtonGroup();
    tb->setProperty("objectof", QVariant::fromValue<void *>(newObj));
    newObj->setSlotValue("handle", allocator->newQObject(tb));

    connect(tb, SIGNAL(buttonClicked(int)), this, SLOT(button_clicked(int)));
    newObj->setSlotValue(VMId::get(RId::ButtonSelected), allocator->newChannel());
    return newObj;
}

void ButtonGroupForeignClass::button_clicked(int id)
{
    QButtonGroup *te = (QButtonGroup *) sender();
    Object *object = (Object *) te->property("objectof").value<void *>();
    Channel *chan = object->getSlotValue(VMId::get(RId::ButtonSelected))->unboxChan();
    chan->send(allocator->newInt(id), NULL);
}

Value *ButtonGroupForeignClass::dispatch(Process *proc, int id, QVector<Value *> args)
{
    if(id == methodAddButton)
    {
        // اضف
        IObject *receiver = args[0]->unboxObj();
        QButtonGroup *handle = dynamic_cast<QButtonGroup *>(receiver->getSlotValue("handle")->unboxQObj());

        ensureValueIsWidget(args[1]);
        IObject *toAdd = args[1]->unboxObj();
        Value *vhandle = toAdd->getSlotValue("handle");

        QAbstractButton *button = dynamic_cast<QAbstractButton *>(vhandle->unboxQObj());
        if(!button)
            throw VMError(InternalError);

        button->setProperty("valueptr", QVariant::fromValue<void *>(args[1]));

        int theId = this->runningIdCount++;
        handle->addButton(button, theId);
        return allocator->newInt(theId);
    }
    if(id == methodGetButton)
    {
        // الزر.الموسوم
        IObject *receiver = args[0]->unboxObj();
        Value *vhandle = receiver->getSlotValue("handle");
        QButtonGroup *handle = dynamic_cast<QButtonGroup*>(vhandle->unboxQObj());

        rw->typeCheck(proc, args[1], BuiltInTypes::IntType);
        int theId = args[1]->unboxInt();

        QAbstractButton *button = handle->button(theId);
        Value *btnObj = (Value *) button->property("valueptr").value<void *>();
        return btnObj;
    }

    return NULL;
}
