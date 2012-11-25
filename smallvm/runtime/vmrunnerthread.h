#ifndef VMRUNNERTHREAD_H
#define VMRUNNERTHREAD_H

#include <QThread>
#include <QMetaType>
#include <QWaitCondition>
#include <QMutex>

class VM;
class RunWindow;

struct ObjContainer
{
    QObject *content;
    QWaitCondition cond;
    ObjContainer() : content(NULL) { }
};

typedef QObject *(*OBJ_MAKER)();

class VMRunthread : public QThread
{
    Q_OBJECT
    VM *vm;
    RunWindow *rw;
public:
    QMutex mutex;
public:
    VMRunthread(VM *vm, RunWindow *rw);
    void run();
    void doGUI(QObject *control, QString method);
    void createNew(ObjContainer *box, OBJ_MAKER);
signals:
    void callGUI(QObject *obj, QString method);
    void callNew(ObjContainer *box, OBJ_MAKER);

};

#endif // VMRUNNERTHREAD_H
