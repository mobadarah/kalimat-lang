#include "guieditwidgethandler.h"

#include <QLineEdit>
#include "../smallvm/value.h"

GUIEditWidgetHandler::GUIEditWidgetHandler(Reference *r, QWidget *widget, VM *vm) :
    QObject(NULL)
{
    ref = r;
    this->widget = widget;
    this->vm = vm;
}

void GUIEditWidgetHandler::lineEditChanged()
{
    QLineEdit *le = (QLineEdit *) widget;
    QString str = le->text();
    Value *oldVal = ref->Get();
    switch(oldVal->tag)
    {
    case StringVal:
        ref->Set(vm->GetAllocator().newString(new QString(le->text())));
        break;
    case Int:
    case Double:
            ref->Set(ConvertStringToNumber(le->text(), vm));
        break;
    default:
        break;
    }
}

void GUIEditWidgetHandler::checkboxChanged(int v)
{
    if(v == Qt::Checked)
    {
        ref->Set(vm->GetAllocator().newBool(true));
    }
    else
    {
        ref->Set(vm->GetAllocator().newBool(false));
    }
}
