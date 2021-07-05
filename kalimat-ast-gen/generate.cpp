#include "typeinfo.h"
#include <QStringList>
#include <QTextStream>
#include <iostream>
using namespace std;

QString singularForm(QString s)
{
    QString prefix = "";
    if(s.left(1) == "_")
    {
        prefix = "_";
        s = s.mid(1);
    }

    if(s == "indexes" || s == "indices")
        return prefix + "index";
    if(s == "children")
        return prefix + "child";
    if(s == "data")
        return prefix + "data";
    return prefix + s.mid(0, s.length()-1);
}

QString upCaseFirstChar(QString s)
{
    if(s.mid(0,1)== "_")
        s = s.mid(1);
    return s.mid(0,1).toUpper() + s.mid(1);
}

QString noStartWithUnderScore(QString s)
{
    if(s.left(1) == "_")
        s = s.mid(1);
    return s;
}

/*
  Generation rules:

1) If the class is not abstract, it has to implement prettyPrint & toString

2) For each field that is utlimately derived from KalimatAst, either generate a shared_ptr or a vector of shared_ptr
unless it's a map, then deal with the key an value.

3) Also for each field that hasConstructorArg, create a constructor arg that sets it

4) A class has to call it's base class's constructor and take any neccessary args to do so,
so we need to keep track of each class's constructor args and do so in dependency order

5) Also for each field: if it's a vector && it hasAddFunction, create a method like e.g addStatement
or insertStatement for maps, and sets

6) And for each field generate a getXXX method, but if it's a vector generate getXXXCount and getXXX(int index)

7) And if it's a vector and has genGetWholeVector, generate a special getXXX() to return the whole vector
*/

QString TypeInfo::generateDeclaration(Context &context)
{
    QString ret;
    QTextStream out(&ret);

    if(this->comment !="")
        out << comment << "\n";
    if(this->kind == Enum)
    {
        out << "enum " << this->name << "\n{\n";
        for(int i=0; i<this->fields.count(); ++i)
        {
            out << fields[i].name;
            if(i < fields.count() -1)
                out << ",\n";
        }
        out << this->embed;

        out << "\n};\n\n";
        context.generatedTypes.insert(this->name);
        return ret;
    }
    out << "class " << this->name << " ";
    if(this->parentName.count() != 0)
    {
        out << " :  ";
        for(int p=0; p< this->parentName.count(); ++p)
        {
            QString parent = this->parentName[p];
            if(parent == "AST" || parent == "IScopeIntroducer"  || parent == "PrettyPrintable"
                    || context.generatedTypes.contains(parent))
            {
                out << " public " << parent;
                if(p < this->parentName.count() - 1)
                    out << ", ";

            }
            else
            {
                wcerr << "Cannot generate class " << this->name.toStdWString() << " before it's parent " << parent.toStdWString() << endl;
            }
        }
    }
    context.generatedTypes.insert(this->name);

    if(context.dependencies.contains(this->name))
    {
        for(int i=0; i<context.dependencies[this->name].count(); ++i)
        {
            QString dep = context.dependencies[this->name][i];
            if(!context.generatedTypes.contains(dep))
            {
                context.neededDeclarations.insert(dep);
                if(dep =="")
                    cout << "break!";
            }
        }
    }
    out << "\n{\npublic:\n";

    QList<Param> constructorParams;
    QList<Param> baseConstructorParams;
    for(int p=0; p< this->parentName.count(); ++p)
    {
        baseConstructorParams.append(context.constructorParamsForClass[this->parentName[p]]);
    }


    constructorParams.append(baseConstructorParams);

    // Now generate field declarations, while collecting our constructor params
    for(int i=0; i<this->fields.count(); i++)
    {
        FieldInfo &fi = fields[i];

        // rename a field like "name" to "_name" if we need to generate a "name() {...}" function later on
        if((fi.kind == NormalField || (fi.kind == VectorField && fi.hasGenWholeVector))
                && !fi.name.startsWith("_"))
        {
            fi.name = "_" + fi.name;
        }

        if(fi.comment != "")
        {
            out << fi.comment << endl;
        }

        out << fi.cppType(context) << " " << fi.name <<";\n";
        if(fi.hasConstructorArgument)
        {
            Param p;
            p.name = fi.name;
            p.type = fi.cppType(context);
            constructorParams.append(p);
        }
    }

    context.constructorParamsForClass[this->name] = constructorParams;

    // Now we're ready to generate our constructor
    out << "public:\n\n";

    if(autoGenerateConstructor)
    {
        out << this->name << "(";
        for(int i=0; i<constructorParams.count(); ++i)
        {
            out << constructorParams[i].type << " " << constructorParams[i].name;
            if(i != constructorParams.count()-1)
                out << ",\n";
        }
        out << ");\n";
    }
    if(constructorParams.count() !=0 && this->generateEmptyConstructor)
    {
        // Seems we have to generate another constructor
        out << this->name << "() {}\n";
    }
    // Let's generate methods for fields
    for(int i=0; i<this->fields.count(); i++)
    {
        FieldInfo &fi = fields[i];
        if(fi.kind == NormalField)
        {
            out << fi.cppType(context) << " " << noStartWithUnderScore(fi.name) <<"() { return " << fi.name <<"; }\n";
            if(fi.hasGenMutator)
            {
                out << "void set" << upCaseFirstChar(fi.name) <<"("<< fi.cppType(context) <<" val) { " << fi.name <<" = val; }\n";
            }
        }
        else if(fi.kind == VectorField)
        {
            QString singleName;
            if(fi.notplural)
                singleName = fi.name;
            else
                singleName = singularForm(fi.name);

            out << "int " << noStartWithUnderScore(singleName) <<"Count() { return " << fi.name <<".count(); }\n";

            out << fi.cppVectorParam(context) << " " << noStartWithUnderScore(singleName) <<"(int index) { return " << fi.name <<"[index]; }\n";

            if(fi.hasGenWholeVector)
            {
                QString funcPrefix = fi.notplural? " get" : " ";
                out << fi.cppType(context) << funcPrefix <<

                       (fi.notplural? upCaseFirstChar(fi.name) : noStartWithUnderScore(fi.name)) <<"() { return " << fi.name <<"; }\n";
            }
            if(fi.hasAddFunction)
            {
                out << "void add" << upCaseFirstChar(singleName) <<"(" << fi.cppVectorParam(context) << " arg) { " << fi.name <<".append(arg);}\n";
            }

        }
        else if(fi.kind == SetField)
        {
            QString singleName;
            if(fi.notplural)
                singleName = fi.name;
            else
                singleName = singularForm(fi.name);

            QString etype = fi.cppVectorParam(context);


            out << "int " << noStartWithUnderScore(singleName) <<"Count() { return " << fi.name <<".count(); }\n";

            if(fi.hasAddFunction)
            {

                out << "void insert" << upCaseFirstChar(singleName) <<"(" << etype << " a) { " << fi.name <<".insert(a);}\n";
            }

            if(fi.hasContains)
            {

                out << "bool contains" << upCaseFirstChar(singleName) <<"(" << etype << " a) { return " << fi.name <<".contains(a);}\n";
            }
        }
        else if(fi.kind == MapField)
        {
            QString singleName;
            if(fi.notplural)
                singleName = fi.name;
            else
                singleName = singularForm(fi.name);

            QStringList parts = fi.jsonType.split("/", QString::SkipEmptyParts);
            QString keyType = fi.cppType(parts[0], context);
            QString valType = fi.cppType(parts[1], context);


            out << "int " << noStartWithUnderScore(singleName) <<"Count() { return " << fi.name <<".count(); }\n";

            out << valType << " " << noStartWithUnderScore(singleName) <<"(" << keyType <<" index) { return " << fi.name <<"[index]; }\n";


            if(fi.hasAddFunction)
            {

                out << "void insert" << upCaseFirstChar(singleName) <<"(" << keyType << " a, " << valType << "b) { " << fi.name <<"[a]=b;}\n";
            }

            if(fi.hasContains)
            {

                out << "bool contains" << upCaseFirstChar(singleName) <<"(" << keyType << " a) { return " << fi.name <<".contains(a);}\n";
            }
        }

        if(fi.hasConstructorArgument)
        {
            Param p;
            p.name = fi.name;
            p.type = fi.cppType(context);
            constructorParams.append(p);
        }
    }

    // If the class is not abstract, it has to implement prettyPrint & toString
    if(needSharedPtr(this->name, context))
    {
        if(!this->isAbstact)
        {
            if(!this->suppressToString)
                out << "virtual QString toString();\n";

            out << "virtual void prettyPrint(CodeFormatter *f);\n\n";
        }
        else
        {
            if(!this->suppressToString)
                out << "virtual QString toString()=0;\n";
            out << "virtual void prettyPrint(CodeFormatter *f)=0;\n\n";

        }
    }
    if(needSharedPtr(this->name, context))
    {
        if(!this->isAbstact)
        {
            out << "virtual void traverse(shared_ptr<PrettyPrintable> p, Traverser *tv);\n\n";
            out << "virtual void traverseChildren(Traverser *tv);\n\n";
            out << "virtual QString childrenToString();\n";
        }
        else
        {
            out << "virtual void traverse(shared_ptr<PrettyPrintable> p, Traverser *tv)=0;\n\n";
            out << "virtual void traverseChildren(Traverser *tv);\n\n";
            out << "virtual QString childrenToString();\n";
        }
    }

    out << this->embed;

    out << "};\n\n";
    return ret;
}

QString TypeInfo::generateImplementation(Context &context)
{
    if(kind == Enum)
        return "";
    return QString("%1\n%2\n%3\n%4")
            .arg(generateConstructorImpl(context))
            .arg(generateToStringImpl(context))
            .arg(generateChildrenToString(context))
            .arg(generateTraverseImpl(context));
}

QString TypeInfo::generateConstructorImpl(Context &context)
{
    QString ret;
    QTextStream out(&ret);


    if(autoGenerateConstructor)
    {
        out << this->name <<"::" << this->name << "(";
        const QList<Param> &constructorParams = context.constructorParamsForClass[name];
        for(int i=0; i<constructorParams.count(); ++i)
        {
            out << constructorParams[i].type << " " << constructorParams[i].name;
            if(i != constructorParams.count()-1)
                out << ",\n";
        }
        out << ")";
        if(parentName.count() > 0 || constructorParams.count() > 0)
            out << ": ";
        out << "\n";
        int k=0;
        for(int i=0; i<parentName.count(); i++)
        {
            QString parent = parentName[i];
            QList<Param> parentParams = context.constructorParamsForClass[parent];
            out << parent << "(";
            for(int j=0; j<parentParams.count(); j++)
            {
                out << context.constructorParamsForClass[name][k].name;
                if(j < parentParams.count() -1)
                    out << ",";
                k++;
            }
            out << ")";
            if(i < parentName.count() -1)
                out <<", \n";
        }
        if(parentName.count() > 0 && constructorParams.count() > k)
            out << ",";

        for(int i=k; i<constructorParams.count(); ++i)
        {
            out << constructorParams[i].name << "(" << constructorParams[i].name << ")";
            if(i != constructorParams.count()-1)
                out << ",\n";
        }

        out << "\n{\n";
        out << constructorCode;
        out << "}\n";
    }
    return ret;
}

QString TypeInfo::generateToStringImpl(Context &context)
{
    if(isAbstact)
        return "";
    if(!needSharedPtr(this->name, context))
        return "";

    if(this->suppressToString)
        return "";
    QString ret;
    QTextStream out(&ret);

    out << "QString " << name << "::toString()\n{\n";
    out << "QString ret;\n";
    out << "QTextStream out(&ret);\n";
    out << "out << _ws(L\"" << this->arabicName << "(\");\n";

    for(int i=0; i<parentName.count(); ++i)
    {
        TypeInfo &ti = context.TypeInfos[parentName[i]];
        if(ti.name == "")
            continue; // todo: why does that happen??
        if(!needSharedPtr(ti.name, context))
            continue;
        out << "out << " << ti.name << "::childrenToString();\n";
        if(i < parentName.count()-1)
            out << "out <<\", \";\n";
    }

    out << "out << \", \" << childrenToString();\n";

    out << "out <<\")\";\n";
    out << "return ret;\n";
    out << "}\n";

    return ret;
}

QString TypeInfo::generateChildrenToString(Context &context)
{
    if(!needSharedPtr(name, context))
        return "";
    QString ret;
    QTextStream out(&ret);
    out << "QString " << name << "::childrenToString()\n{\n";
    out << "QString ret;\n";
    out << "QTextStream out(&ret);\n";

    for(int i=0; i<fields.count(); ++i)
    {
        FieldInfo &fi = fields[i];
        if(fi.hasToString)
            out << fi.generateToString(context);
    }
    out << "return ret;\n";
    out << "}\n";
    return ret;
}

QString TypeInfo::generateTraverseImpl(Context &context)
{
    if(!needSharedPtr(this->name, context))
        return "";

    QString ret;
    QTextStream out(&ret);

    if(!this->isAbstact)
    {
        out << "void " << this->name << "::traverse(shared_ptr<PrettyPrintable> p, Traverser *tv)\n{\n";
        out << "tv->visit(p);\n";
        for(int i=0; i< this->parentName.count(); ++i)
        {
            QString pname = parentName[i];

            if(context.TypeInfos[pname].isa(context, "KalimatAst"))
            {
                out << pname << "::traverseChildren(tv);\n";
            }
        }
        out << "this->traverseChildren(tv);\n";
        out << "tv->exit(p);\n";
        out <<"}\n";
    }
    out << "void " << this->name << "::traverseChildren(Traverser *tv)\n{\n";
    for(int i=0; i<fields.count(); i++)
    {
        out << fields[i].generateTraverse(context);
    }
    out <<"}\n";

    return ret;

}

bool TypeInfo::isa(Context &context, QString typeName)
{
    if(this->name == typeName)
        return true;
    for(int p=0; p< this->parentName.count(); ++p)
    {
        QString parent = this->parentName[p];
        if(parent == "PrettyPrintable")
            return typeName == "PrettyPrintable";

        if(context.TypeInfos[parent].isa(context, typeName))
            return true;
    }
    return false;
}

QString FieldInfo::cppType(Context &context)
{
    if(this->kind == MapField)
    {
        QStringList strs = jsonType.split("/", QString::SkipEmptyParts);
        return QString("QMap<%1, %2 >")
                .arg(cppType(strs[0], context))
                .arg(cppType(strs[1], context));
    }
    else if(this->kind == VectorField)
    {
        return QString("QVector<%1 >").arg(cppType(jsonType, context));
    }
    else if(this->kind == SetField)
    {
        return QString("QSet<%1 >").arg(cppType(jsonType, context));
    }

    return cppType(jsonType, context);
}

QString FieldInfo::cppType(QString type, Context &context)
{
    if(needSharedPtr(type, context))
    {
        return QString("shared_ptr<%1>").arg(type);
    }
    return type;
}

QString FieldInfo::cppVectorParam(Context &context)
{
    return cppType(jsonType, context);
}

bool needSharedPtr(QString type, Context &context)
{
    if(context.TypeInfos[type].isa(context, "KalimatAst") ||
            context.TypeInfos[type].isa(context, "PrettyPrintable"))
    {
        return true;
    }
    return false;
}

QString FieldInfo::generateToString(Context &context)
{
    if(kind == NormalField)
    {
        if(needSharedPtr(jsonType, context))
        {
            return "if(" + name + ") \n { \n out << " + noStartWithUnderScore(name) + "()->toString() << \", \" ;\n}\n";
        }
        else
        {
            return "out << " + name + " << \", \" ;\n";
        }
    }
    else if(kind == VectorField)
    {

        QString ret = "out << \"[\";\n";
        ret += "for(int i=0; i<"+ name + ".count(); i++)\n{\n";

        if(needSharedPtr(jsonType, context))
        {
            ret += "if(" + name + "[i]) \n { \n out << " + name + "[i]->toString() << \", \" ;\n}\n";
        }
        else
        {
            ret += "out << " + name + "[i] << \", \" ;\n";
        }
        ret +="}\n";
        ret += "out << \"]\";\n";
        return ret;
    }
    else if(kind == SetField)
    {
        bool shared = needSharedPtr(jsonType, context);
        QString ret = "out << \"{\";\n";
        QString valType = shared? "shared_ptr<"+jsonType+"> ": jsonType;
        ret += "for(QSet<"+ valType + " >::const_iterator i="+name+".begin(); i!="+ name + ".end(); ++i)\n{\n";

        if(shared)
        {
            ret += "if(*i) \n { \n out << (*i)->toString() << \", \" ;\n}\n";
        }
        else
        {
            ret += "out << *i << \", \" ;\n";
        }
        ret +="}\n";
        ret += "out << \"}\";\n";
        return ret;
    }
    else if(kind == MapField)
    {
        QString mapTo = jsonType.split("/", QString::SkipEmptyParts)[1];
        bool shared = needSharedPtr(mapTo, context);
        QString ret = "out << \"{\";\n";
        QString valType = shared? "shared_ptr<"+mapTo+"> ": mapTo;
        ret += "for(QMap<QString, "+ valType + " >::const_iterator i="+name+".begin(); i!="+ name + ".end(); ++i)\n{\n";

        if(shared)
        {
            ret += "out << i.key();\n";
            ret += "if(i.value()) \n { \n out << i.value()->toString() << \", \" ;\n}\n";
        }
        else
        {
            ret += "out << i.value().toString() << \", \" ;\n";
        }
        ret +="}\n";
        ret += "out << \"}\";\n";
        return ret;
    }
    return "errrooorrrrr!!!";
}

QString FieldInfo::generateTraverse(Context &context)
{
    if(!this->traverse)
        return "";
    if(!needSharedPtr(jsonType, context))
        return "";

    QString ret;
    QTextStream out(&ret);

    if(kind == NormalField)
    {
        // todo: generate 'if' only if field is nullable
        out << "if(" << this->name << ")\n{\n";
        out << this->name << "->traverse(" << this->name << ", tv);\n";
        out << "}\n\n";
    }
    else if(kind == VectorField)
    {
        // todo: generate 'if' only if field is nullable
        out << "for(int i=0; i<" << this->name << ".count(); ++i)\n{\n";
        out << "if(" << this->name <<"[i])\n{\n";
        out << this->name << "[i]->traverse(" << this->name << "[i], tv);\n";
        out << "}\n\n";
        out << "}\n\n";
    }
    else if(kind == MapField)
    {
        // todo: generate 'if' only if field is nullable
        QString mapTo = jsonType.split("/", QString::SkipEmptyParts)[1];
        bool shared = needSharedPtr(mapTo, context);
        QString valType = shared? "shared_ptr<"+mapTo+"> ": mapTo;
        out << "for(QMap<QString, "+ valType + " >::const_iterator i="+name+".begin(); i!="+ name + ".end(); ++i)\n{\n";
        if(shared)
        {
            out << "if(i.value())\n{\n";
            out << "i.value()->traverse(i.value(), tv);\n";
            out << "}\n";
        }
        out << "}\n";
    }
    else if(kind == SetField)
    {
        // todo: generate 'if' only if field is nullable
        bool shared = needSharedPtr(jsonType, context);
        QString valType = shared? "shared_ptr<"+jsonType+"> ": jsonType;
        out << "for(QSet< "+ valType + " >::const_iterator i="+name+".begin(); i!="+ name + ".end(); ++i)\n{\n";
        if(shared)
        {
            out << "if(*i)\n{\n";
            out << "(*i)->traverse(*i, tv);\n";
            out << "}\n";
        }
        out << "}\n";
    }

    return ret;
}
