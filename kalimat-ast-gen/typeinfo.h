#ifndef TYPEINFO_H
#define TYPEINFO_H

#include <QMap>
#include <QSet>

enum Kind { Class, Struct, Enum};
enum FieldKind { NormalField, VectorField, MapField, SetField};

// json attributes:
// type: string, name: string, arabic:string,  vector: bool, map: bool, comment: string, hasAddFunction: bool
// genGetWholeVector: bool, constructorArg: bool

struct Context;
struct FieldInfo
{
    FieldKind kind;
    QString name;
    QString arabicName;
    QString jsonType;
    QString comment;
    bool hasAddFunction;
    bool hasContains;
    bool hasConstructorArgument;
    bool hasGenWholeVector;
    bool hasGenMutator;
    bool hasToString;
    bool nullable;
    bool traverse;
    bool notplural;

    QString cppType(Context &context);
    QString cppType(QString type, Context &context);
    QString cppVectorParam(Context &context); // e.g if it's QVector<shared_ptr<Expression> >, return shared_ptr<Expression>

    QString generateToString(Context &context);
    QString generateTraverse(Context &context);
};



struct TypeInfo
{
    Kind kind;
    QString name;
    QList<QString> parentName;
    QString arabicName;
    bool isAbstact;
    bool generateEmptyConstructor;
    bool autoGenerateConstructor;
    QList<FieldInfo> fields;
    QString embed;
    QString comment;
    QString constructorCode;
    bool suppressToString;

    QString generateDeclaration(Context &context);

    QString generateImplementation(Context &context);
    QString generateConstructorImpl(Context &context);
    QString generateToStringImpl(Context &context);
    QString generateChildrenToString(Context &context);
    QString generateTraverseImpl(Context &context);

    bool isa(Context &context, QString typeName);
};

struct Param
{
    QString name;
    QString type;
};

struct Context
{
    QSet<QString> generatedTypes;
    QSet<QString> neededDeclarations;
    QMap<QString, QList<Param> > constructorParamsForClass;
    QMap<QString, TypeInfo> TypeInfos;
    QMap<QString, QList<QString> > dependencies;
    QList<TypeInfo> allTypes;
    QSet<QString> allTypeNames;

};

bool needSharedPtr(QString type, Context &context);

#endif // TYPEINFO_H
