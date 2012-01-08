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

ImageForeignClass::ImageForeignClass(QString name, RunWindow *rw)
    :EasyForeignClass(name), rw(rw)
{
    QImage img;

    methodIds[_ws(L"تدويرها")]
            = 0;
    methodArities[_ws(L"تدويرها")]
            = 2;

    methodIds[_ws(L"ممطوطة")]
            = 1;
    methodArities[_ws(L"ممطوطة")]
            = 3;

    methodIds[_ws(L"خط")]
            = 2;
    methodArities[_ws(L"خط")]
            = 5;

    methodIds[_ws(L"قلبها")]
            = 3;
    methodArities[_ws(L"قلبها")]
            = 3;

    methodIds[_ws(L"نسخة.منها")]
            = 4;
    methodArities[_ws(L"نسخة.منها")]
            = 1;

    methodIds[_ws(L"حدد.لون.النقطة")]
            = 5;
    methodArities[_ws(L"حدد.لون.النقطة")]
            = 4;

    methodIds[_ws(L"لون.النقطة")]
            = 6;
    methodArities[_ws(L"لون.النقطة")]
            = 3;

    methodIds[_ws(L"عرضها")]
            = 7;
    methodArities[_ws(L"عرضها")]
            = 1;

    methodIds[_ws(L"ارتفاعها")]
            = 8;
    methodArities[_ws(L"ارتفاعها")]
            = 1;
}

IObject *ImageForeignClass::newValue(Allocator *allocator)
{
    //todo: this is a hack
    this->allocator = allocator;

    Object *obj = new Object();
    obj->slotNames.append("handle");
    return obj;
}

Value *ImageForeignClass::dispatch(int id, QVector<Value *> args)
{
    IObject *receiver = args[0]->unboxObj();
    QImage *handle = reinterpret_cast<QImage*>
            (receiver->getSlotValue("handle")->unboxRaw());

    QImage *img2;
    QTransform trans;
    IObject *obj;
    double w, h;
    double degrees;
    double s1, s2;
    int x1, y1, x2, y2;
    int clr;
    switch(id)
    {
        case 0: // rotated
        rw->typeCheck(args[1], BuiltInTypes::NumericType);
        degrees = args[1]->unboxNumeric();
        w = handle->width()/2;
        h = handle->height() /2 ;
        // negative because of 'Arabic' coordinate system
        trans = trans.translate(w,h).rotate(-degrees).translate(-w,-h);
        //trans = trans.translate(-w,-h).rotate(-degrees).translate(w,h);
        //trans = trans.rotate(-degrees);
        //img2 = new QImage(handle->transformed(trans, Qt::SmoothTransformation));
        //if(false)
        {
            img2 = new QImage(handle->width(),handle->height(), handle->format());
            QPainter p(img2);
            QBrush brsh(handle->pixel(0,0));
            p.setBrush(brsh);
            p.fillRect(0,0,handle->width(), handle->height(), Qt::SolidPattern);
            p.translate(w, h);
            p.rotate(-degrees);
            p.translate(-w, -h);
            p.drawImage(0,0, *handle);
        }
        obj = this->newValue(allocator);
        obj->setSlotValue("handle", allocator->newRaw(img2, BuiltInTypes::ObjectType));
        return allocator->newObject(obj, this);

    case 1: //scaled
        rw->typeCheck(args[1], BuiltInTypes::NumericType);
        rw->typeCheck(args[2], BuiltInTypes::NumericType);
        s1= args[1]->unboxNumeric();
        s2= args[2]->unboxNumeric();
        trans = trans.scale(s1, s2);
        img2 = new QImage(handle->transformed(trans));
        obj = this->newValue(allocator);
        obj->setSlotValue("handle", allocator->newRaw(img2, BuiltInTypes::ObjectType));
        return allocator->newObject(obj, this);
    case 2: // line
        rw->typeCheck(args[1], BuiltInTypes::NumericType);
        x1 = (int) args[1]->unboxNumeric();
        rw->typeCheck(args[2], BuiltInTypes::NumericType);
        y1 = (int) args[2]->unboxNumeric();
        rw->typeCheck(args[3], BuiltInTypes::NumericType);
        x2 = (int) args[3]->unboxNumeric();
        rw->typeCheck(args[4], BuiltInTypes::NumericType);
        y2 = (int) args[4]->unboxNumeric();
        {
            QPainter p(handle);
            p.drawLine(x1,y1,x2,y2);
        }
        return NULL;
    case 3: // flipped
        rw->typeCheck(args[1], BuiltInTypes::NumericType);
        rw->typeCheck(args[2], BuiltInTypes::NumericType);
        s1= args[1]->unboxNumeric();
        s2= args[2]->unboxNumeric();
        if(s1>0)
            s1 = 1;
        else if(s1<0)
            s1 = -1;
        if(s2>0)
            s2 = 1;
        else if(s2<0)
            s2 = -1;

        trans = trans.scale(s1, s2);
        img2 = new QImage(handle->transformed(trans));
        obj = this->newValue(allocator);
        obj->setSlotValue("handle", allocator->newRaw(img2, BuiltInTypes::ObjectType));
        return allocator->newObject(obj, this);
    case 4:
        img2 = new QImage(handle->copy());
        obj = this->newValue(allocator);
        obj->setSlotValue("handle", allocator->newRaw(img2, BuiltInTypes::ObjectType));
        return allocator->newObject(obj, this);
    case 5:
        rw->typeCheck(args[1], BuiltInTypes::NumericType);
        rw->typeCheck(args[2], BuiltInTypes::NumericType);
        rw->typeCheck(args[3], BuiltInTypes::NumericType);
        {
            int x = (int) args[1]->unboxNumeric();
            int y = (int) args[2]->unboxNumeric();
            clr = (int) args[3]->unboxNumeric();
            handle->setPixel(x,y, clr);
        }
        return NULL;
    case 6:
        rw->typeCheck(args[1], BuiltInTypes::NumericType);
        rw->typeCheck(args[2], BuiltInTypes::NumericType);
        {
            int x = (int) args[1]->unboxNumeric();
            int y = (int) args[2]->unboxNumeric();
            clr = handle->pixel(x, y);
        }
        return allocator->newInt(clr);
    case 7:
        return allocator->newInt(handle->width());
    case 8:
        return allocator->newInt(handle->height());
    }
}

WindowForeignClass::WindowForeignClass(QString name, RunWindow *rw)
    : EasyForeignClass(name)
{
    this->rw = rw;
    methodIds[QString::fromStdWString(L"كبر")
            ] = 0;
    methodIds[QString::fromStdWString(L"تحرك.إلى")
            ] = 1;
    methodIds[QString::fromStdWString(L"اضف")
            ] = 2;
    methodIds[_ws(L"حدد.الحجم")] =
            3;
    methodIds[_ws(L"حدد.العنوان")] =
            4;

    methodArities[QString::fromStdWString(L"كبر")
            ] = 1;
    methodArities[QString::fromStdWString(L"تحرك.إلى")
            ] = 3;
    methodArities[QString::fromStdWString(L"اضف")
            ] = 2;
    methodArities[_ws(L"حدد.الحجم")] =
            3;
    methodArities[_ws(L"حدد.العنوان")] =
            2;

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
        QObject *pwin = raw->unboxQObj();
        QWidget *widget = dynamic_cast<QWidget *>(pwin);
        widget->setWindowState(Qt::WindowMaximized);
        return NULL;
    }
    if(id == 1)
    {
        // تحرك.إلى
        WindowForeignObject *foreignWindow = dynamic_cast<WindowForeignObject*>(args[0]->unboxObj());
        Value *raw = foreignWindow->handle;
        QObject *praw = raw->unboxQObj();
        QWidget *widget = dynamic_cast<QWidget *>(praw);

        rw->typeCheck(args[1], BuiltInTypes::NumericType);
        rw->typeCheck(args[2], BuiltInTypes::NumericType);
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
        QObject *praw = raw->unboxQObj();
        QWidget *widget = dynamic_cast<QWidget *>(praw);

        ensureValueIsWidget(args[1]);
        QWidget *control = dynamic_cast<QWidget *>(args[1]->unboxObj()->getSlotValue("handle")->unboxQObj());
        control->setParent(widget);
        QFont f = control->font();
        control->setFont(QFont(f.family(), f.pointSize()+3));
        control->show();
        return NULL;
    }
    if(id == 3)
    {
        // حدد.الحجم
        WindowForeignObject *foreignWindow = dynamic_cast<WindowForeignObject*>(args[0]->unboxObj());
        Value *raw = foreignWindow->handle;
        QObject *praw = raw->unboxQObj();
        QWidget *widget = dynamic_cast<QWidget *>(praw);

        rw->typeCheck(args[1], BuiltInTypes::NumericType);
        rw->typeCheck(args[2], BuiltInTypes::NumericType);
        int w = args[1]->unboxNumeric();
        int h = args[2]->unboxNumeric();
        int wdiff = widget->width() - w;
        for(int i=0; i<widget->children().count(); i++)
        {
            QWidget *c = dynamic_cast<QWidget *>(widget->children().at(i));
            if(c)
                c->move(c->x() - wdiff, c->y());
        }
        widget->setFixedSize(w, h);
        return NULL;
    }
    if(id == 4)
    {
        // حدد.العنوان
        WindowForeignObject *foreignWindow = dynamic_cast<WindowForeignObject*>(args[0]->unboxObj());
        Value *raw = foreignWindow->handle;
        QObject *praw = raw->unboxQObj();
        QWidget *widget = dynamic_cast<QWidget *>(praw);

        rw->typeCheck(args[1], BuiltInTypes::StringType);
        QString *t = args[1]->unboxStr();
        widget->setWindowTitle(*t);
        return NULL;
    }
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
    newObj->setSlotValue("handle", allocator->newQObject(button));

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
        QWidget *handle = dynamic_cast<QWidget *>(receiver->getSlotValue("handle")->unboxQObj());

        rw->typeCheck(args[1], BuiltInTypes::StringType);
        handle->setWindowTitle(*args[1]->unboxStr());
        return NULL;
    }
    if(id == methodSetSize)
    {
        // حدد.الحجم
        IObject *receiver = args[0]->unboxObj();
        QWidget *handle = dynamic_cast<QWidget *>(receiver->getSlotValue("handle")->unboxQObj());
        int originalx = handle->x() + handle->width();

        rw->typeCheck(args[1], BuiltInTypes::NumericType);
        rw->typeCheck(args[2], BuiltInTypes::NumericType);
        handle->resize(args[1]->unboxNumeric(), args[2]->unboxNumeric());

        originalx -= handle->width();
        handle->move(originalx, handle->y());
        return NULL;
    }
    if(id == methodSetPos)
    {
        // حدد.المكان
        IObject *receiver = args[0]->unboxObj();
        QWidget *handle = dynamic_cast<QWidget *>(receiver->getSlotValue("handle")->unboxQObj());

        rw->typeCheck(args[1], BuiltInTypes::NumericType);
        rw->typeCheck(args[2], BuiltInTypes::NumericType);
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
        QWidget *handle = dynamic_cast<QWidget *>(receiver->getSlotValue("handle")->unboxQObj());
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
        QPushButton *handle = dynamic_cast<QPushButton *>(receiver->getSlotValue("handle")->unboxQObj());

        rw->typeCheck(args[1], BuiltInTypes::StringType);
        handle->setText(*args[1]->unboxStr());
        return NULL;
    }
    if(id == methodGetText)
    {
        // نصه
        IObject *receiver = args[0]->unboxObj();
        QPushButton *handle = dynamic_cast<QPushButton *>(receiver->getSlotValue("handle")->unboxQObj());
        return allocator->newString(new QString(handle->text()));
    }
    if(id <= controlMethodCutoff)
        return ControlForeignClass::dispatch(id, args);
    return NULL;
}

TextboxForeignClass::TextboxForeignClass(QString name, RunWindow *rw)
    : ControlForeignClass(name, rw)
{
    methodIds[_ws(L"الحق.نص")] = methodAppendText;

    methodArities[_ws(L"الحق.نص")]
            = 2;

    fields.insert(QString::fromStdWString(L"تغير"));
}

IObject *TextboxForeignClass::newValue(Allocator *allocator)
{
    Object *newObj = (Object *) ControlForeignClass::newValue(allocator);

    newObj->slotNames.append(QString::fromStdWString(L"تغير"));

    QTextEdit *tb = new QTextEdit();
    tb->setProperty("objectof", QVariant::fromValue<void *>(newObj));
    newObj->setSlotValue("handle", allocator->newQObject(tb));

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
        QTextEdit *handle = dynamic_cast<QTextEdit *>(receiver->getSlotValue("handle")->unboxQObj());

        rw->typeCheck(args[1], BuiltInTypes::StringType);
        handle->document()->setPlainText(*args[1]->unboxStr());
        return NULL;
    }
    if(id == methodGetText)
    {
        // نصه
        IObject *receiver = args[0]->unboxObj();
        QTextEdit *handle = dynamic_cast<QTextEdit*>(receiver->getSlotValue("handle")->unboxQObj());
        return allocator->newString(new QString(handle->document()->toPlainText()));
    }
    if(id == methodAppendText)
    {
        // الحق.نص
        IObject *receiver = args[0]->unboxObj();
        QTextEdit *handle = dynamic_cast<QTextEdit*>(receiver->getSlotValue("handle")->unboxQObj());
        rw->typeCheck(args[1], BuiltInTypes::StringType);
        handle->append(*args[1]->unboxStr());
        return NULL;
    }
    if(id <= controlMethodCutoff)
        return ControlForeignClass::dispatch(id, args);
    return NULL;
}

LineEditForeignClass::LineEditForeignClass(QString name, RunWindow *rw)
    : ControlForeignClass(name, rw)
{
    methodIds[_ws(L"الحق.نص")] = methodAppendText;

    methodArities[_ws(L"الحق.نص")]
            = 2;
    fields.insert(QString::fromStdWString(L"تغير"));
}

IObject *LineEditForeignClass::newValue(Allocator *allocator)
{
    Object *newObj = (Object *) ControlForeignClass::newValue(allocator);

    newObj->slotNames.append(QString::fromStdWString(L"تغير"));

    QLineEdit *tb = new QLineEdit();
    tb->setProperty("objectof", QVariant::fromValue<void *>(newObj));
    newObj->setSlotValue("handle", allocator->newQObject(tb));

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
        QLineEdit *handle = dynamic_cast<QLineEdit*>(receiver->getSlotValue("handle")->unboxQObj());

        rw->typeCheck(args[1], BuiltInTypes::StringType);
        handle->setText(*args[1]->unboxStr());
        return NULL;
    }
    if(id == methodGetText)
    {
        // نصه
        IObject *receiver = args[0]->unboxObj();
        QLineEdit *handle = dynamic_cast<QLineEdit*>(receiver->getSlotValue("handle")->unboxQObj());
        return allocator->newString(new QString(handle->text()));
    }
    if(id == methodAppendText)
    {
        // الحق.نص
        IObject *receiver = args[0]->unboxObj();
        QLineEdit *handle = dynamic_cast<QLineEdit*>(receiver->getSlotValue("handle")->unboxQObj());
        rw->typeCheck(args[1], BuiltInTypes::StringType);
        handle->setText(handle->text()+ *args[1]->unboxStr());
        return NULL;
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
    newObj->setSlotValue("handle", allocator->newQObject(tb));

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
        QListWidget *handle = dynamic_cast<QListWidget *>(receiver->getSlotValue("handle")->unboxQObj());
        handle->addItem(args[1]->toString());
        return NULL;
    }
    if(id == methodInsertItem)
    {
        // اضف.في
        IObject *receiver = args[0]->unboxObj();
        QListWidget *handle = dynamic_cast<QListWidget *>(receiver->getSlotValue("handle")->unboxQObj());

        Value *v = args[1];

        rw->typeCheck(args[2], BuiltInTypes::IntType);
        int index = args[2]->unboxInt();

        handle->insertItem(index, v->toString());
        return NULL;
    }
    if(id == methodGetItem)
    {
        // عنصر.رقم
        IObject *receiver = args[0]->unboxObj();
        QListWidget *handle = dynamic_cast<QListWidget *>(receiver->getSlotValue("handle")->unboxQObj());

        rw->typeCheck(args[1], BuiltInTypes::IntType);
        int index = args[1]->unboxInt();

        return allocator->newString(new QString(handle->item(index)->text()));
    }
    if(id <= controlMethodCutoff)
        return ControlForeignClass::dispatch(id, args);
    return NULL;
}

ComboboxForeignClass::ComboboxForeignClass(QString name, RunWindow *rw)
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

    methodIds[_ws(L"حدد.أيحرر")] = methodSetEditable;

    methodArities[_ws(L"حدد.أيحرر")]
            = 2;

    fields.insert(QString::fromStdWString(L"تغير.اختيار"));
    fields.insert(QString::fromStdWString(L"تغير.نص"));
}

IObject *ComboboxForeignClass::newValue(Allocator *allocator)
{
    Object *newObj = (Object *) ControlForeignClass::newValue(allocator);

    newObj->slotNames.append(QString::fromStdWString(L"تغير.اختيار"));
    newObj->slotNames.append(QString::fromStdWString(L"تغير.نص"));

    QComboBox *cb = new QComboBox();
    cb->setEditable(true);
    cb->setProperty("objectof", QVariant::fromValue<void *>(newObj));
    newObj->setSlotValue("handle", allocator->newQObject(cb));

    connect(cb, SIGNAL(currentIndexChanged(int)), this, SLOT(on_select(int)));
    connect(cb, SIGNAL(editTextChanged(QString)), this, SLOT(on_text_changed(QString)));

    newObj->setSlotValue(QString::fromStdWString(L"تغير.اختيار"), allocator->newChannel());
    newObj->setSlotValue(QString::fromStdWString(L"تغير.نص"), allocator->newChannel());
    return newObj;
}

void ComboboxForeignClass::on_select(int selection)
{
    QComboBox *cb = (QComboBox *) sender();
    Object *object = (Object *) cb->property("objectof").value<void *>();
    Channel *chan = object->getSlotValue(QString::fromStdWString(L"تغير.اختيار"))->unboxChan();
    chan->send(allocator->newInt(selection), NULL);
}

void ComboboxForeignClass::on_text_changed(QString)
{
    QComboBox *cb = (QComboBox *) sender();
    Object *object = (Object *) cb->property("objectof").value<void *>();
    Channel *chan = object->getSlotValue(QString::fromStdWString(L"تغير.نص"))->unboxChan();
    chan->send(allocator->null(), NULL);
}

Value *ComboboxForeignClass::dispatch(int id, QVector<Value *> args)
{

    if(id == methodSetText)
    {
        // حدد.النص
        IObject *receiver = args[0]->unboxObj();
        QComboBox *handle = dynamic_cast<QComboBox *>(receiver->getSlotValue("handle")->unboxQObj());

        rw->typeCheck(args[1], BuiltInTypes::StringType);
        handle->setEditText(*args[1]->unboxStr());
        return NULL;
    }
    if(id == methodGetText)
    {
        // نصه
        IObject *receiver = args[0]->unboxObj();
        QComboBox *handle = dynamic_cast<QComboBox *>(receiver->getSlotValue("handle")->unboxQObj());
        return allocator->newString(new QString(handle->currentText()));
    }
    if(id == methodAddItem)
    {
        // اضف
        IObject *receiver = args[0]->unboxObj();
        QComboBox *handle = dynamic_cast<QComboBox *>(receiver->getSlotValue("handle")->unboxQObj());
        handle->addItem(args[1]->toString());
        return NULL;
    }
    if(id == methodInsertItem)
    {
        // اضف.في
        IObject *receiver = args[0]->unboxObj();
        QComboBox *handle = dynamic_cast<QComboBox *>(receiver->getSlotValue("handle")->unboxQObj());

        Value *v = args[1];

        rw->typeCheck(args[2], BuiltInTypes::IntType);
        int index = args[2]->unboxInt();

        handle->insertItem(index, v->toString());
        return NULL;
    }
    if(id == methodGetItem)
    {
        // عنصر.رقم
        IObject *receiver = args[0]->unboxObj();
        QComboBox *handle = dynamic_cast<QComboBox *>(receiver->getSlotValue("handle")->unboxQObj());

        rw->typeCheck(args[1], BuiltInTypes::IntType);
        int index = args[1]->unboxInt();

        return allocator->newString(new QString(handle->itemText(index)));
    }
    if(id == methodSetEditable)
    {
        // حدد.أيحرر
        IObject *receiver = args[0]->unboxObj();
        QComboBox *handle = dynamic_cast<QComboBox *>(receiver->getSlotValue("handle")->unboxQObj());

        rw->typeCheck(args[1], BuiltInTypes::BoolType);
        handle->setEditable(args[1]->unboxBool());
        return NULL;
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
    newObj->setSlotValue("handle", allocator->newQObject(tb));

    return newObj;
}

Value *LabelForeignClass::dispatch(int id, QVector<Value *> args)
{
    if(id == methodSetText)
    {
        // حدد.النص
        IObject *receiver = args[0]->unboxObj();
        QLabel *handle = dynamic_cast<QLabel*>(receiver->getSlotValue("handle")->unboxQObj());

        rw->typeCheck(args[1], BuiltInTypes::StringType);
        handle->setText(*args[1]->unboxStr());
        return NULL;
    }
    if(id == methodGetText)
    {
        // نصه
        IObject *receiver = args[0]->unboxObj();
        QLabel *handle = dynamic_cast<QLabel*>(receiver->getSlotValue("handle")->unboxQObj());
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
    newObj->setSlotValue("handle", allocator->newQObject(tb));

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
        QCheckBox *handle = dynamic_cast<QCheckBox*>(receiver->getSlotValue("handle")->unboxQObj());

        rw->typeCheck(args[1], BuiltInTypes::StringType);
        handle->setText(*args[1]->unboxStr());
        return NULL;
    }
    if(id == methodGetText)
    {
        // نصه
        IObject *receiver = args[0]->unboxObj();
        QCheckBox *handle = dynamic_cast<QCheckBox*>(receiver->getSlotValue("handle")->unboxQObj());
        return allocator->newString(new QString(handle->text()));
    }

    if(id == methodGetValue)
    {
        // قيمته
        IObject *receiver = args[0]->unboxObj();
        QCheckBox *handle = dynamic_cast<QCheckBox*>(receiver->getSlotValue("handle")->unboxQObj());

        return allocator->newInt(handle->checkState());
    }

    if(id == methodSetValue)
    {
        // حدد.القيمة
        IObject *receiver = args[0]->unboxObj();
        QCheckBox *handle = dynamic_cast<QCheckBox*>(receiver->getSlotValue("handle")->unboxQObj());

        rw->typeCheck(args[1], BuiltInTypes::IntType);
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
    newObj->setSlotValue("handle", allocator->newQObject(tb));

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
        QRadioButton *handle = dynamic_cast<QRadioButton *>(receiver->getSlotValue("handle")->unboxQObj());

        rw->typeCheck(args[1], BuiltInTypes::StringType);
        handle->setText(*args[1]->unboxStr());
        return NULL;
    }
    if(id == methodGetText)
    {
        // نصه
        IObject *receiver = args[0]->unboxObj();
        QRadioButton *handle = dynamic_cast<QRadioButton *>(receiver->getSlotValue("handle")->unboxQObj());
        return allocator->newString(new QString(handle->text()));
    }

    if(id == methodGetValue)
    {
        // قيمته
        IObject *receiver = args[0]->unboxObj();
        QRadioButton *handle = dynamic_cast<QRadioButton *>(receiver->getSlotValue("handle")->unboxQObj());

        return allocator->newBool(handle->isChecked());
    }

    if(id == methodSetValue)
    {
        // حدد.القيمة
        IObject *receiver = args[0]->unboxObj();
        QRadioButton *handle = dynamic_cast<QRadioButton *>(receiver->getSlotValue("handle")->unboxQObj());

        rw->typeCheck(args[1], BuiltInTypes::BoolType);
        bool newState = args[1]->unboxBool();
        handle->setChecked(newState);
        return NULL;
    }

    if(id <= controlMethodCutoff)
        return ControlForeignClass::dispatch(id, args);
    return NULL;
}

ButtonGroupForeignClass::ButtonGroupForeignClass(QString name, RunWindow *rw)
    : EasyForeignClass(name)
{
    this->rw = rw;
    runningIdCount = 1;

    methodIds[_ws(L"اضف")] = methodAddButton;

    methodArities[_ws(L"اضف")]
            = 2;

    methodIds[_ws(L"الزر.الموسوم")] = methodGetButton;

    methodArities[_ws(L"الزر.الموسوم")]
            = 2;

    fields.insert(QString::fromStdWString(L"اختيار.زر"));
}

IObject *ButtonGroupForeignClass::newValue(Allocator *allocator)
{
    this->allocator = allocator; // todo: this is a hack
    Object *newObj = new Object();
    newObj->slotNames.append("handle");

    newObj->slotNames.append(QString::fromStdWString(L"اختيار.زر"));

    QButtonGroup *tb = new QButtonGroup();
    tb->setProperty("objectof", QVariant::fromValue<void *>(newObj));
    newObj->setSlotValue("handle", allocator->newQObject(tb));

    connect(tb, SIGNAL(buttonClicked(int)), this, SLOT(button_clicked(int)));
    newObj->setSlotValue(QString::fromStdWString(L"اختيار.زر"), allocator->newChannel());
    return newObj;
}

void ButtonGroupForeignClass::button_clicked(int id)
{
        QButtonGroup *te = (QButtonGroup *) sender();
        Object *object = (Object *) te->property("objectof").value<void *>();
        Channel *chan = object->getSlotValue(QString::fromStdWString(L"اختيار.زر"))->unboxChan();
        chan->send(allocator->newInt(id), NULL);
}

Value *ButtonGroupForeignClass::dispatch(int id, QVector<Value *> args)
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

        rw->typeCheck(args[1], BuiltInTypes::IntType);
        int theId = args[1]->unboxInt();

        QAbstractButton *button = handle->button(theId);
        Value *btnObj = (Value *) button->property("valueptr").value<void *>();
        return btnObj;
    }

    return NULL;
}
