#ifndef GUIEDITWIDGETHANDLER_H
#define GUIEDITWIDGETHANDLER_H

#include <QObject>
#include "../smallvm/vm_incl.h"
#include "../smallvm/vm.h"

Value *ConvertStringToNumber(QString str, VM *vm);

class GUIEditWidgetHandler : public QObject
{
    Q_OBJECT
    Reference *ref;
    QWidget *widget;
    VM *vm;
public:
    explicit GUIEditWidgetHandler(Reference *r, QWidget *widget, VM *vm);

signals:

public slots:
    void lineEditChanged();
    void checkboxChanged(int v);
};

#endif // GUIEDITWIDGETHANDLER_H
