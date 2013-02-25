#include <QtCore/QCoreApplication>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QStringList>
#include <iostream>
#include "json.h"
#include "typeinfo.h"

using namespace std;


void setupFields(TypeInfo &type, QList<QVariant> nodes, Context &context);

QSet<QString> possibleNodeAttributes;


void processType(QMap<QString, QVariant> type, Context &context)
{
    TypeInfo ti;

    ti.name = type["type"].toString();
    ti.arabicName = type["arabic"].toString();
    if(type.contains("generateEmptyConstructor"))
        ti.generateEmptyConstructor = type["generateEmptyConstructor"].toBool();
    else
        ti.generateEmptyConstructor = false;
    if(type["struct"].toBool())
    {
        ti.kind = Struct;
    }
    else if(type["enum"].toBool())
    {
        ti.kind = Enum;
        setupFields(ti, type["members"].toList(), context);
    }
    else
    {
        ti.kind = Class;
    }
    if(ti.kind == Class || ti.kind == Struct)
    {
        if(type.contains("parent"))
        {
            if(type["parent"].type() == QVariant::String)
            {
                QString parent = type["parent"].toString();
                ti.parentName.append(parent);
                context.dependencies[ti.name].append(parent);
            }
            else
            {
                QList<QVariant> parents = type["parent"].toList();
                for(int i=0; i<parents.count(); ++i)
                {
                    ti.parentName.append(parents[i].toString());
                    context.dependencies[ti.name].append(parents[i].toString());
                }
            }

        }
        if(type.contains("abstract"))
            ti.isAbstact = type["abstract"].toBool();
        else
            ti.isAbstact = type["nodes"].toList().empty();

        if(type.contains("suppressToString"))
            ti.suppressToString = type["suppressToString"].toBool();
        else
            ti.suppressToString = false;

        if(type.contains("autoGenerateConstructor"))
            ti.autoGenerateConstructor = type["autoGenerateConstructor"].toBool();
        else
            ti.autoGenerateConstructor = true;


        if(type.contains("embed"))
            ti.embed = type["embed"].toString();

        if(type.contains("comment"))
            ti.comment = type["comment"].toString();

        if(type.contains("constructorCode"))
            ti.constructorCode = type["constructorCode"].toString();

        setupFields(ti, type["nodes"].toList(), context);
    }
    context.allTypes.append(ti);
}

void setupFields(TypeInfo &type, QList<QVariant> nodes, Context &context)
{
    for(QList<QVariant>::const_iterator i=nodes.begin(); i!=nodes.end(); ++i)
    {
        FieldInfo field;
        QMap<QString, QVariant> obj = (*i).toMap();

        field.name = obj["name"].toString();

        field.kind = NormalField;
        if(obj.contains("vector") && obj["vector"].toBool())
            field.kind = VectorField;
        else if(obj.contains("map") && obj["map"].toBool())
            field.kind = MapField;
        else if(obj.contains("set") && obj["set"].toBool())
            field.kind = SetField;


        field.arabicName = obj["arabic"].toString();
        if(obj.contains("comment"))
            field.comment = obj["comment"].toString();
        field.jsonType = obj["type"].toString();

        if(obj.contains("genGetWholeVector"))
            field.hasGenWholeVector = obj["genGetWholeVector"].toBool();
        else
            field.hasGenWholeVector = false;

        if(obj.contains("hasAddFunction"))
            field.hasAddFunction = obj["hasAddFunction"].toBool();
        else
            field.hasAddFunction = false;

        if(obj.contains("constructorArg"))
            field.hasConstructorArgument = obj["constructorArg"].toBool();
        else
            field.hasConstructorArgument = true;

        if(obj.contains("generateContains"))
            field.hasContains = obj["generateContains"].toBool();
        else
            field.hasContains = true;

        if(obj.contains("notplural"))
            field.notplural = obj["notplural"].toBool();
        else
            field.notplural = false;

        if(obj.contains("toString"))
            field.hasToString = obj["toString"].toBool();
        else
            field.hasToString = true;

        if(obj.contains("nullable"))
            field.nullable = obj["nullable"].toBool();
        else
            field.nullable = false;
        if(obj.contains("traverse"))
            field.traverse = obj["traverse"].toBool();
        else
            field.traverse = true;

        if(obj.contains("generateMutator"))
            field.hasGenMutator = obj["generateMutator"].toBool();
        else
            field.hasGenMutator = false;

        type.fields.append(field);

        if(field.kind != MapField)
        {
            if(context.allTypeNames.contains(field.jsonType))
            {
                context.dependencies[type.name].append(field.jsonType);
            }
        }
        else
        {
            QStringList componentTypes = field.jsonType.split("/", QString::SkipEmptyParts);

            QString dep1 = componentTypes[0];
            QString dep2 = componentTypes[1];

            if(context.allTypeNames.contains(dep1))
            {
                context.dependencies[type.name].append(dep1);
            }

            if(context.allTypeNames.contains(dep2))
            {
                context.dependencies[type.name].append(dep2);
            }
        }

    }
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QFile f("../kalimat-ast-gen/astdata.json");
    f.open(QIODevice::ReadOnly | QIODevice::Text);
    QTextStream in(&f);
    in.setCodec("UTF-8");
    QString data = in.readAll();
    f.close();

    QtJson::Json js;
    bool success;
    QVariant result;
    result = js.parse(data, success);
    if(!success)
    {
        cout << "Couldn't parse json data";
        exit(-1);
    }

    Context context;

    QList<QVariant> types = result.toList();

    for(int i=0; i<types.count(); i++)
    {
        context.allTypeNames.insert(types[i].toMap()["type"].toString());
    }

    for(int i=0; i<types.count(); i++)
    {
        QMap<QString, QVariant> type = types[i].toMap();
        processType(type, context);

    }

    for(int i=0; i<context.allTypes.count(); ++i)
    {
        context.TypeInfos[context.allTypes[i].name] = context.allTypes[i];
     //   if(!context.allTypes[i].isAbstact)
     //       wcout << context.allTypes[i].name.toStdWString() <<", ";
    }
    //wcout << endl;

    QStringList generated;



    for(int i=0; i<context.allTypes.count(); ++i)
        generated.append(context.allTypes[i].generateDeclaration(context) + "\n");


    for(QSet<QString>::const_iterator i=context.neededDeclarations.begin(); i!=context.neededDeclarations.end(); ++i)
        generated.prepend(" class " + *i + ";\n");

    generated.prepend("#include \"kalimatast.h\"\n");

    generated.prepend("#ifndef KALIMAT_AST_GEN_H\n#define KALIMAT_AST_GEN_H\n\n");
    generated.append("#endif\n");


    QFile f2("../kalimat_ast_gen.h");
    f2.open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream header(&f2);
    header << generated.join("");
    f2.close();
    generated.clear();
    wcout << "wrote " << QFileInfo(f2).absoluteFilePath().toStdWString() << endl;


    for(int i=0; i<context.allTypes.count(); ++i)
        generated.append(context.allTypes[i].generateImplementation(context) + "\n");



    generated.prepend("#include \"../../../smallvm/utils.h\"\n");
    generated.prepend("#include \"kalimat_ast_gen.h\"\n");

    QFile f3("../kalimat_ast_gen.cpp");
    f3.open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream cpp(&f3);
    cpp << generated.join("");
    f3.close();
    wcout << "wrote " << QFileInfo(f3).absoluteFilePath().toStdWString() << endl;
    generated.clear();

    //wcout << generated.join("").toStdWString();
    //wcout.flush();
    //return a.exec();
    return 0;
}
