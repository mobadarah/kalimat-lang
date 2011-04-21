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

#define _ws(a) QString::fromStdWString(a)

WindowForeignClass::WindowForeignClass(QString name)
    : EasyForeignClass(name)
{
    methodIds[QString::fromStdWString(L"كبر")
            ] = 0;
    methodIds[QString::fromStdWString(L"تحرك.إلى")
            ] = 1;
    methodIds[QString::fromStdWString(L"اضف")
            ] = 2;

    methodArities[QString::fromStdWString(L"كبر")
            ] = 1;
    methodArities[QString::fromStdWString(L"تحرك.إلى")
            ] = 3;
    methodArities[QString::fromStdWString(L"اضف")
            ] = 2;


    fields.insert("handle");
}

IObject *WindowForeignClass::newValue(Allocator *allocator)
{
    WindowForeignObject *window = new WindowForeignObject();
    return window;
}

Value *WindowForeignClass::dispatch(int id, QVector<Value *> args)
{
    if(id == 0)
    {
        // كبر
        WindowForeignObject *foreignWindow = dynamic_cast<WindowForeignObject*>(args[0]->unboxObj());
        Value *raw = foreignWindow->handle;
        void *praw = raw->unboxRaw();
        QWidget *widget = (QWidget *)(praw);
        widget->setWindowState(Qt::WindowMaximized);
        return NULL;
    }
    if(id == 1)
    {
        // تحرك.إلى
        WindowForeignObject *foreignWindow = dynamic_cast<WindowForeignObject*>(args[0]->unboxObj());
        Value *raw = foreignWindow->handle;
        void *praw = raw->unboxRaw();
        QWidget *widget = (QWidget *)(praw);

        int x = args[1]->unboxNumeric();
        int y = args[2]->unboxNumeric();
        widget->move(x, y);
        return NULL;
    }
    if(id == 2)
    {
        // اضف
        WindowForeignObject *foreignWindow = dynamic_cast<WindowForeignObject*>(args[0]->unboxObj());
        Value *raw = foreignWindow->handle;
        void *praw = raw->unboxRaw();
        QWidget *widget = (QWidget *)(praw);

        QWidget *control = (QWidget *) args[1]->unboxObj()->getSlotValue("handle")->unboxRaw();
        control->setParent(widget);
        control->show();
        return NULL;
    }
}

bool WindowForeignObject::hasSlot(QString name)
{
    if(name == "handle")
    {
        return true;
    }
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
}

void WindowForeignObject::setSlotValue(QString name, Value *val)
{
    if(name == "handle")
        handle = val;
}

QString WindowForeignObject::toString()
{
    return QString::fromStdWString(L"<نافذة>");
}

ControlForeignClass::ControlForeignClass(QString name, RunWindow *rw)
    : EasyForeignClass(name)
{
    this->rw = rw;
    fields.insert("handle");

    methodIds[_ws(L"حدد.النص")
            ] = methodSetText;
    methodIds[QString::fromStdWString(L"حدد.الحجم")
            ] = methodSetSize;
    methodIds[QString::fromStdWString(L"حدد.المكان")
            ] = methodSetPos;
    methodIds[QString::fromStdWString(L"نصه")
            ] = methodGetText;


    methodArities[QString::fromStdWString(L"حدد.النص")
            ] = 2;
    methodArities[QString::fromStdWString(L"حدد.الحجم")
            ] = 3;
    methodArities[QString::fromStdWString(L"حدد.المكان")
            ] = 3;
    methodArities[QString::fromStdWString(L"نصه")
            ] = 1;
}

IObject *ControlForeignClass::newValue(Allocator *allocator)
{
    this->allocator = allocator; // todo: this is a hack
    Object *newObj = new Object();
    newObj->slotNames.append("handle");
    return newObj;
}

ButtonForeignClass::ButtonForeignClass(QString name, RunWindow *rw)
    : ControlForeignClass(name, rw)
{
    fields.insert(QString::fromStdWString(L"ضغط"));
}

IObject *ButtonForeignClass::newValue(Allocator *allocator)
{

    Object *newObj = (Object *) ControlForeignClass::newValue(allocator);

    newObj->slotNames.append(QString::fromStdWString(L"ضغط"));

    QPushButton *button = new QPushButton();
    button->setProperty("objectof", QVariant::fromValue<void *>(newObj));
    newObj->setSlotValue("handle", allocator->newRaw(button, this));

    connect(button, SIGNAL(clicked()), this, SLOT(on_button_clicked()));
    newObj->setSlotValue(QString::fromStdWString(L"ضغط"), allocator->newChannel());

    return newObj;
}

Value *ControlForeignClass::dispatch(int id, QVector<Value *> args)
{
    if(id == methodSetText)
    {
        // حدد.النص
        IObject *receiver = args[0]->unboxObj();
        QWidget *handle = (QWidget *) receiver->getSlotValue("handle")->unboxRaw();
        handle->setWindowTitle(*args[1]->unboxStr());
        return NULL;
    }
    if(id == methodSetSize)
    {
        // حدد.الحجم
        IObject *receiver = args[0]->unboxObj();
        QWidget *handle = (QWidget *) receiver->getSlotValue("handle")->unboxRaw();
        int originalx = handle->x() + handle->width();

        handle->resize(args[1]->unboxNumeric(), args[2]->unboxNumeric());

        originalx -= handle->width();
        handle->move(originalx, handle->y());
        return NULL;
    }
    if(id == methodSetPos)
    {
        // حدد.المكان
        IObject *receiver = args[0]->unboxObj();
        QWidget *handle = (QWidget *) receiver->getSlotValue("handle")->unboxRaw();
        int x = args[1]->unboxNumeric();
        int y = args[2]->unboxNumeric();
        rw->paintSurface->TX(x);
        x -= handle->width();
        handle->move(x, y);
        return NULL;
    }
    if(id == methodGetText)
    {
        // نصه
        IObject *receiver = args[0]->unboxObj();
        QWidget *handle = (QWidget *) receiver->getSlotValue("handle")->unboxRaw();
        return allocator->newString(new QString(handle->windowTitle()));
    }
    return NULL;
}

void ButtonForeignClass::on_button_clicked()
{
    QPushButton *pb = (QPushButton *) sender();
    Object *object = (Object *) pb->property("objectof").value<void *>();
    Channel *chan = object->getSlotValue(QString::fromStdWString(L"ضغط"))->unboxChan();
    chan->send(allocator->null(), NULL);
}

Value *ButtonForeignClass::dispatch(int id, QVector<Value *> args)
{
    if(id == methodSetText)
    {
        // حدد.النص
        IObject *receiver = args[0]->unboxObj();
        QPushButton *handle = (QPushButton *) receiver->getSlotValue("handle")->unboxRaw();
        handle->setText(*args[1]->unboxStr());
        return NULL;
    }
    if(id == methodGetText)
    {
        // نصه
        IObject *receiver = args[0]->unboxObj();
        QPushButton *handle = (QPushButton *) receiver->getSlotValue("handle")->unboxRaw();
        return allocator->newString(new QString(handle->text()));
    }
    if(id <= controlMethodCutoff)
        return ControlForeignClass::dispatch(id, args);
    return NULL;
}

TextboxForeignClass::TextboxForeignClass(QString name, RunWindow *rw)
    : ControlForeignClass(name, rw)
{
    fields.insert(QString::fromStdWString(L"تغير"));
}

IObject *TextboxForeignClass::newValue(Allocator *allocator)
{
    Object *newObj = (Object *) ControlForeignClass::newValue(allocator);

    newObj->slotNames.append(QString::fromStdWString(L"تغير"));

    QTextEdit *tb = new QTextEdit();
    tb->setProperty("objectof", QVariant::fromValue<void *>(newObj));
    newObj->setSlotValue("handle", allocator->newRaw(tb, this));

    connect(tb, SIGNAL(textChanged()), this, SLOT(on_text_changed()));
    newObj->setSlotValue(QString::fromStdWString(L"تغير"), allocator->newChannel());
    return newObj;
}

void TextboxForeignClass::on_text_changed()
{
    QTextEdit *te = (QTextEdit *) sender();
    Object *object = (Object *) te->property("objectof").value<void *>();
    Channel *chan = object->getSlotValue(QString::fromStdWString(L"تغير"))->unboxChan();
    chan->send(allocator->null(), NULL);
}

Value *TextboxForeignClass::dispatch(int id, QVector<Value *> args)
{
    if(id == methodSetText)
    {
        // حدد.النص
        IObject *receiver = args[0]->unboxObj();
        QTextEdit *handle = (QTextEdit *) receiver->getSlotValue("handle")->unboxRaw();
        handle->document()->setPlainText(*args[1]->unboxStr());
        return NULL;
    }
    if(id == methodGetText)
    {
        // نصه
        IObject *receiver = args[0]->unboxObj();
        QTextEdit *handle = (QTextEdit *) receiver->getSlotValue("handle")->unboxRaw();
        return allocator->newString(new QString(handle->document()->toPlainText()));
    }
    if(id <= controlMethodCutoff)
        return ControlForeignClass::dispatch(id, args);
    return NULL;
}

LineEditForeignClass::LineEditForeignClass(QString name, RunWindow *rw)
    : ControlForeignClass(name, rw)
{
    fields.insert(QString::fromStdWString(L"تغير"));
}

IObject *LineEditForeignClass::newValue(Allocator *allocator)
{
    Object *newObj = (Object *) ControlForeignClass::newValue(allocator);

    newObj->slotNames.append(QString::fromStdWString(L"تغير"));

    QLineEdit *tb = new QLineEdit();
    tb->setProperty("objectof", QVariant::fromValue<void *>(newObj));
    newObj->setSlotValue("handle", allocator->newRaw(tb, this));

    connect(tb, SIGNAL(textChanged(QString)), this, SLOT(on_text_changed()));
    newObj->setSlotValue(QString::fromStdWString(L"تغير"), allocator->newChannel());
    return newObj;
}

void LineEditForeignClass::on_text_changed()
{
    QLineEdit *te = (QLineEdit *) sender();
    Object *object = (Object *) te->property("objectof").value<void *>();
    Channel *chan = object->getSlotValue(QString::fromStdWString(L"تغير"))->unboxChan();
    chan->send(allocator->null(), NULL);
}

Value *LineEditForeignClass::dispatch(int id, QVector<Value *> args)
{
    if(id == methodSetText)
    {
        // حدد.النص
        IObject *receiver = args[0]->unboxObj();
        QLineEdit *handle = (QLineEdit *) receiver->getSlotValue("handle")->unboxRaw();
        handle->setText(*args[1]->unboxStr());
        return NULL;
    }
    if(id == methodGetText)
    {
        // نصه
        IObject *receiver = args[0]->unboxObj();
        QLineEdit *handle = (QLineEdit *) receiver->getSlotValue("handle")->unboxRaw();
        return allocator->newString(new QString(handle->text()));
    }
    if(id <= controlMethodCutoff)
        return ControlForeignClass::dispatch(id, args);
    return NULL;
}

ListboxForeignClass::ListboxForeignClass(QString name, RunWindow *rw)
    : ControlForeignClass(name, rw)
{
    methodIds[_ws(L"اضف")] = methodAddItem;

    methodArities[_ws(L"اضف")]
            = 2;

    methodIds[_ws(L"اضف.في")] = methodInsertItem;

    methodArities[_ws(L"اضف.في")]
            = 3;

    methodIds[_ws(L"عنصر.رقم")] = methodGetItem;

    methodArities[_ws(L"عنصر.رقم")]
            = 2;


    fields.insert(QString::fromStdWString(L"تغير.اختيار"));
}

IObject *ListboxForeignClass::newValue(Allocator *allocator)
{
    Object *newObj = (Object *) ControlForeignClass::newValue(allocator);

    newObj->slotNames.append(QString::fromStdWString(L"تغير.اختيار"));

    QListWidget *tb = new QListWidget();
    tb->setProperty("objectof", QVariant::fromValue<void *>(newObj));
    newObj->setSlotValue("handle", allocator->newRaw(tb, this));

    connect(tb, SIGNAL(currentRowChanged(int)), this, SLOT(on_select(int)));
    newObj->setSlotValue(QString::fromStdWString(L"تغير.اختيار"), allocator->newChannel());
    return newObj;
}

void ListboxForeignClass::on_select(int selection)
{
    QListWidget *te = (QListWidget *) sender();
    Object *object = (Object *) te->property("objectof").value<void *>();
    Channel *chan = object->getSlotValue(QString::fromStdWString(L"تغير.اختيار"))->unboxChan();
    chan->send(allocator->newInt(selection), NULL);
}

Value *ListboxForeignClass::dispatch(int id, QVector<Value *> args)
{
    if(id == methodAddItem)
    {
        // اضف
        IObject *receiver = args[0]->unboxObj();
        QListWidget *handle = (QListWidget *) receiver->getSlotValue("handle")->unboxRaw();
        handle->addItem(args[1]->toString());
        return NULL;
    }
    if(id == methodInsertItem)
    {
        // اضف.في
        IObject *receiver = args[0]->unboxObj();
        QListWidget *handle = (QListWidget *) receiver->getSlotValue("handle")->unboxRaw();

        Value *v = args[1];
        int index = args[2]->unboxInt();

        handle->insertItem(index, v->toString());
        return NULL;
    }
    if(id == methodGetItem)
    {
        // عنصر.رقم
        IObject *receiver = args[0]->unboxObj();
        QListWidget *handle = (QListWidget *) receiver->getSlotValue("handle")->unboxRaw();

        int index = args[1]->unboxInt();

        return allocator->newString(new QString(handle->item(index)->text()));
    }
    if(id <= controlMethodCutoff)
        return ControlForeignClass::dispatch(id, args);
    return NULL;
}

LabelForeignClass::LabelForeignClass(QString name, RunWindow *rw)
    : ControlForeignClass(name, rw)
{

}

IObject *LabelForeignClass::newValue(Allocator *allocator)
{
    Object *newObj = (Object *) ControlForeignClass::newValue(allocator);

    QLabel *tb = new QLabel();
    tb->setProperty("objectof", QVariant::fromValue<void *>(newObj));
    newObj->setSlotValue("handle", allocator->newRaw(tb, this));

    return newObj;
}

Value *LabelForeignClass::dispatch(int id, QVector<Value *> args)
{
    if(id == methodSetText)
    {
        // حدد.النص
        IObject *receiver = args[0]->unboxObj();
        QLabel *handle = (QLabel*) receiver->getSlotValue("handle")->unboxRaw();
        handle->setText(*args[1]->unboxStr());
        return NULL;
    }
    if(id == methodGetText)
    {
        // نصه
        IObject *receiver = args[0]->unboxObj();
        QLabel *handle = (QLabel*) receiver->getSlotValue("handle")->unboxRaw();
        return allocator->newString(new QString(handle->text()));
    }

    if(id <= controlMethodCutoff)
        return ControlForeignClass::dispatch(id, args);
    return NULL;
}

CheckboxForeignClass::CheckboxForeignClass(QString name, RunWindow *rw)
    : ControlForeignClass(name, rw)
{
    methodIds[_ws(L"حدد.القيمة")] = methodSetValue;

    methodArities[_ws(L"حدد.القيمة")]
            = 2;

    methodIds[_ws(L"قيمته")] = methodGetValue;

    methodArities[_ws(L"قيمته")]
            = 1;

    fields.insert(QString::fromStdWString(L"تغير.قيمة"));
}

IObject *CheckboxForeignClass::newValue(Allocator *allocator)
{
    Object *newObj = (Object *) ControlForeignClass::newValue(allocator);

    newObj->slotNames.append(QString::fromStdWString(L"تغير.قيمة"));

    QCheckBox *tb = new QCheckBox();
    tb->setProperty("objectof", QVariant::fromValue<void *>(newObj));
    newObj->setSlotValue("handle", allocator->newRaw(tb, this));

    connect(tb, SIGNAL(stateChanged(int)), this, SLOT(value_changed(int)));
    newObj->setSlotValue(QString::fromStdWString(L"تغير.قيمة"), allocator->newChannel());
    return newObj;
}

void CheckboxForeignClass::value_changed(int newState)
{
    QCheckBox *te = (QCheckBox *) sender();
    Object *object = (Object *) te->property("objectof").value<void *>();
    Channel *chan = object->getSlotValue(QString::fromStdWString(L"تغير.قيمة"))->unboxChan();
    chan->send(allocator->newInt(newState), NULL);
}

Value *CheckboxForeignClass::dispatch(int id, QVector<Value *> args)
{
    if(id == methodSetText)
    {
        // حدد.النص
        IObject *receiver = args[0]->unboxObj();
        QCheckBox *handle = (QCheckBox*) receiver->getSlotValue("handle")->unboxRaw();
        handle->setText(*args[1]->unboxStr());
        return NULL;
    }
    if(id == methodGetText)
    {
        // نصه
        IObject *receiver = args[0]->unboxObj();
        QCheckBox *handle = (QCheckBox*) receiver->getSlotValue("handle")->unboxRaw();
        return allocator->newString(new QString(handle->text()));
    }

    if(id == methodGetValue)
    {
        // قيمته
        IObject *receiver = args[0]->unboxObj();
        QCheckBox *handle = (QCheckBox *) receiver->getSlotValue("handle")->unboxRaw();

        return allocator->newInt(handle->checkState());
    }

    if(id == methodSetValue)
    {
        // حدد.القيمة
        IObject *receiver = args[0]->unboxObj();
        QCheckBox *handle = (QCheckBox *) receiver->getSlotValue("handle")->unboxRaw();

        int newState = args[1]->unboxInt();
        handle->setCheckState((Qt::CheckState)newState);
        return NULL;
    }

    if(id <= controlMethodCutoff)
        return ControlForeignClass::dispatch(id, args);
    return NULL;
}

RadioButtonForeignClass::RadioButtonForeignClass(QString name, RunWindow *rw)
    : ControlForeignClass(name, rw)
{
    methodIds[_ws(L"حدد.القيمة")] = methodSetValue;

    methodArities[_ws(L"حدد.القيمة")]
            = 2;

    methodIds[_ws(L"قيمته")] = methodGetValue;

    methodArities[_ws(L"قيمته")]
            = 1;

    fields.insert(QString::fromStdWString(L"اختيار"));
}

IObject *RadioButtonForeignClass::newValue(Allocator *allocator)
{
    Object *newObj = (Object *) ControlForeignClass::newValue(allocator);

    newObj->slotNames.append(QString::fromStdWString(L"اختيار"));

    QRadioButton *tb = new QRadioButton();
    tb->setProperty("objectof", QVariant::fromValue<void *>(newObj));
    newObj->setSlotValue("handle", allocator->newRaw(tb, this));

    connect(tb, SIGNAL(toggled(bool)), this, SLOT(value_changed(bool)));
    newObj->setSlotValue(QString::fromStdWString(L"اختيار"), allocator->newChannel());
    return newObj;
}

void RadioButtonForeignClass::value_changed(bool newState)
{
    if(newState)
    {
        QRadioButton *te = (QRadioButton *) sender();
        Object *object = (Object *) te->property("objectof").value<void *>();
        Channel *chan = object->getSlotValue(QString::fromStdWString(L"اختيار"))->unboxChan();
        chan->send(allocator->null(), NULL);
    }
}

Value *RadioButtonForeignClass::dispatch(int id, QVector<Value *> args)
{
    if(id == methodSetText)
    {
        // حدد.النص
        IObject *receiver = args[0]->unboxObj();
        QRadioButton *handle = (QRadioButton*) receiver->getSlotValue("handle")->unboxRaw();
        handle->setText(*args[1]->unboxStr());
        return NULL;
    }
    if(id == methodGetText)
    {
        // نصه
        IObject *receiver = args[0]->unboxObj();
        QRadioButton *handle = (QRadioButton*) receiver->getSlotValue("handle")->unboxRaw();
        return allocator->newString(new QString(handle->text()));
    }

    if(id == methodGetValue)
    {
        // قيمته
        IObject *receiver = args[0]->unboxObj();
        QRadioButton *handle = (QRadioButton *) receiver->getSlotValue("handle")->unboxRaw();

        return allocator->newBool(handle->isChecked());
    }

    if(id == methodSetValue)
    {
        // حدد.القيمة
        IObject *receiver = args[0]->unboxObj();
        QRadioButton *handle = (QRadioButton *) receiver->getSlotValue("handle")->unboxRaw();

        bool newState = args[1]->unboxBool();
        handle->setChecked(newState);
        return NULL;
    }

    if(id <= controlMethodCutoff)
        return ControlForeignClass::dispatch(id, args);
    return NULL;
}

