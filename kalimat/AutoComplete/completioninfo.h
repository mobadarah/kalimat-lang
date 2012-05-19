#ifndef COMPLETIONINFO_H
#define COMPLETIONINFO_H

#include <QMap>
#include <QVector>
#include <QString>
class CompletionInfo;
class FunctionCompletionInfo
{
public:
    FunctionCompletionInfo(){ name = "<error func>";}
    FunctionCompletionInfo(QString name,
                        bool isProcNotFunc,
                        QVector<QString> argNames)
    {
        this->name = name;
        this->isProcNotFunc = isProcNotFunc;
        this->argNames = argNames;
    }

    QString name;
    bool isProcNotFunc;
    QVector<QString> argNames;
    QString toolTipStr();
};

class ModuleCompletionInfo
{
    CompletionInfo *fullDb;
public:
    QString name;
    QString path;
    QVector<QString> includedModules;
    QMap<QString, FunctionCompletionInfo> functionsDefinedHere;
public:
    ModuleCompletionInfo() {name = "<error module>";}
    ModuleCompletionInfo(QString name, QString path,CompletionInfo *fullDb)
    {
        this->fullDb = fullDb;
        this->name = name;
        this->path = path;
    }

    bool moduleOfPath(QString path, ModuleCompletionInfo &ret);
    QVector<FunctionCompletionInfo> functionsVisibleFromHere();
};

class CompletionInfo
{
public:
    // From full module path to module info
    QMap<QString, ModuleCompletionInfo> modules;
public:
    CompletionInfo();
    ModuleCompletionInfo &module(QString path)
    {
        if(modules.contains(path))
            return modules[path];
        modules[path] = ModuleCompletionInfo("", path, this);
        return modules[path];
    }
};

#endif // COMPLETIONINFO_H
