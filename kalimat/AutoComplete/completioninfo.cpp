#include "completioninfo.h"
#include "idemessages.h"

#include <QStringList>
#include <QStack>

QString FunctionCompletionInfo::toolTipStr()
{
    QStringList strs;
    for(int i=0; i<argNames.count(); i++)
        strs.append(argNames[i]);

    QString what;
    if(isProcNotFunc)
        what = Ide::msg[IdeMsg::Procedure];
    else
        what = Ide::msg[IdeMsg::Function];

    return QString("%1 %2(%3)").arg(what)
            .arg(name).arg(strs.join(", "));
}

bool ModuleCompletionInfo::moduleOfPath(QString path, ModuleCompletionInfo &ret)
{
    if(fullDb->modules.contains(path))
    {
        ret = fullDb->modules[path];
        return true;
    }
    return false;
}

QVector<FunctionCompletionInfo> ModuleCompletionInfo::functionsVisibleFromHere()
{
    QStack<ModuleCompletionInfo> stack;
    QVector<QString> visited;
    visited.append(this->path);
    QVector<FunctionCompletionInfo> ret;

    while(!stack.empty())
    {
        ModuleCompletionInfo mi = stack.pop();


        for(QMap<QString, FunctionCompletionInfo>::const_iterator i
            = mi.functionsDefinedHere.begin();
            i!= mi.functionsDefinedHere.end();
            ++i)
        {
            ret.append(i.value());
        }

        for(int i=0; i<mi.includedModules.count(); i++)
        {
            ModuleCompletionInfo mi2;
            if(mi.moduleOfPath(mi.includedModules[i], mi2))
            {
                if(visited.contains(mi2.path))
                    continue;
                stack.push(mi2);
            }
        }
    }
    return ret;
}

CompletionInfo::CompletionInfo()
{
}
