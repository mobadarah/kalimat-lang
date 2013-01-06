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
    if(oldVal->type == BuiltInTypes::StringType)
    {
        ref->Set(vm->GetAllocator().newString(le->text()));
    }
    else if(oldVal->type == BuiltInTypes::IntType ||
            oldVal->type == BuiltInTypes::DoubleType ||
            oldVal->type == BuiltInTypes::LongType)
    {
        ref->Set(ConvertStringToNumber(le->text(), vm));
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
