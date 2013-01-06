#include "programdatabase.h"
#include "../smallvm/utils.h"
#include <QtSql/QSqlQuery>
#include <QDebug>
#include <QtSql/QSqlError>

ProgramDatabase::ProgramDatabase()
{
    _isOpen = false;
}

bool ProgramDatabase::hasModule(QString filename)
{
    QSqlQuery q(db);
    q.prepare("SELECT filename from modules WHERE filename=?;");
    q.addBindValue(filename);
    if(!q.exec())
    {
        qDebug() << q.lastError().databaseText() << "/" << q.lastError().driverText();
        return false;
    }
    return q.next();
}

void ProgramDatabase::updateModule(QString filename, CodeDocument *doc, shared_ptr<CompilationUnit> module)
{
    int type = ProgDb::Cu_Module;
    shared_ptr<Program> prog = dynamic_pointer_cast<Program>(module);
    if(prog)
    {
        type = ProgDb::Cu_Program;
    }

    if(!hasModule(filename))
    {
        exec("INSERT INTO modules(filename, type, code_doc) VALUES (?,?,?);",
             filename,type, (uint) doc);
    }
    else
    {
        exec("UPDATE modules SET type=?, code_doc=? WHERE filename=?;",
             type,
             (uint) doc,
             filename);
    }
}

void ProgramDatabase::updateModuleTokens(QString filename, const QVector<Token> &tokens)
{
    exec("DELETE FROM tokens WHERE module_filename=?;",
         filename);
    for(int i=0; i<tokens.count(); ++i)
    {
        const Token &t = tokens[i];
        exec("INSERT INTO tokens(module_filename, pos, line, col, lexeme, length) VALUES (?,?,?,?,?,?);",
             filename, t.Pos, t.Line, t.Column, t.Lexeme, t.Lexeme.length());
    }
}

void ProgramDatabase::updateImportsOfModule(QString filename, QVector<QString> imports)
{
    exec("DELETE FROM module_imports WHERE importer=?;",
         filename);

    for(int i=0; i<imports.count(); ++i)
    {
        QString imported = imports[i];
        exec("INSERT INTO module_imports(importer, imported) VALUES(?,?)", filename, imported);
    }
}

void ProgramDatabase::updateModuleDefinitions(QString filename, shared_ptr<CompilationUnit> module)
{
    //exec("DELETE FROM function_definitions WHERE def_id in (select id from definitions where module_filename=?);",
    //     filename);

    exec("DELETE FROM definitions WHERE module_filename=?;",
         filename);

    shared_ptr<Program> prog = dynamic_pointer_cast<Program>(module);
    if(prog)
    {
        for(int i=0; i<prog->originalElementCount(); ++i)
        {
            shared_ptr<Declaration> decl = dynamic_pointer_cast<Declaration>(prog->originalElement(i));
            if(decl)
            {
                //qDebug() << "Found:" << decl->toString();
                addDefinitionToModule(i, filename, decl);
            }
        }
    }
    else
    {
        shared_ptr<Module> mod = dynamic_pointer_cast<Module>(module);

        for(int i=0; i<mod->declarationCount(); ++i)
        {
            shared_ptr<Declaration> decl = dynamic_pointer_cast<Declaration>(mod->declaration(i));
            addDefinitionToModule(i, filename, decl);
        }
    }
}

void ProgramDatabase::addDefinitionToModule(int id, QString filename, shared_ptr<Declaration> decl)
{
    shared_ptr<ClassDecl> cd = dynamic_pointer_cast<ClassDecl>(decl);
    if(cd)
    {
        exec("INSERT INTO definitions(id, module_filename, name, type) VALUES(?,?,?,?);",
             id,
             filename,
             cd->name()->name(),
             ProgDb::Cu_ClassDef);

        addClassDef(id, filename, cd);
    }
    else
    {
        shared_ptr<ProceduralDecl> pd = dynamic_pointer_cast<ProceduralDecl>(decl);
        if(pd)
        {
            exec("INSERT INTO definitions(id, module_filename, name, type) VALUES(?,?,?,?);",
                 id,
                 filename,
                 pd->procName()->name(),
                 ProgDb::Cu_FuncDef);
            //qDebug() << "Recognized: " << pd->procName()->name();
            addFunctionDef(id, filename, pd);
        }
        else
        {
            //qDebug() << "Can't recognize:" << decl->toString();
        }
    }
}

void ProgramDatabase::addFunctionDef(int defId, QString filename, shared_ptr<ProceduralDecl> pd)
{
    int numRets = 0;

    shared_ptr<FunctionDecl> fd = dynamic_pointer_cast<FunctionDecl>(pd);
    if(fd)
    {
        numRets = 1;
    }

    exec("INSERT INTO function_definitions(def_id, module_filename, arity, num_return_values, defining_token_pos, ending_token_pos) "
         "VALUES(?,?,?,?,?,?);",
         defId,
         filename,
         pd->formalCount(),
         numRets,
         pd->getPos().Pos,
         pd->endingToken().Pos);
}

void ProgramDatabase::addClassDef(int defId, QString filename, shared_ptr<ClassDecl>)
{

}

bool ProgramDatabase::open()
{
    db = QSqlDatabase::addDatabase("QSQLITE", "pdb");
    db.setDatabaseName(":memory:");
    //db.setDatabaseName("/home/samy/code/kalimat/progdata.sqlite2");

    if(!db.open())
    {
        return false;
    }
    QSqlQuery pragma(db);
    pragma.exec("PRAGMA foreign_keys = ON;");

    createTables(true);

    _isOpen = true;
    return true;
}

void ProgramDatabase::createTables(bool actually)
{
    QString ddl_query = Utils::readResourceTextFile(":/prog_ddl.sql").readAll();
    QSqlQuery q(db);
    QStringList tableCreationStrings = ddl_query.split(";", QString::SkipEmptyParts);
    for(int i=0; i<tableCreationStrings.count(); ++i)
    {
        QString tcs = tableCreationStrings[i].trimmed();
        if(tcs == "")
            continue;
        bool ret = q.exec(tcs);
        if(!ret)
        {
           qDebug() << q.lastError().databaseText() << "/" << q.lastError().driverText();
        }
    }
}

void ProgramDatabase::close()
{
    _isOpen = false;
    db.close();
}

bool ProgramDatabase::exec(QString query)
{
    QSqlQuery q(db);
    q.prepare(query);
    bool result = q.exec();
    if(!result)
    {
        qDebug() << "Error in query:" << query << ":" << q.lastError().databaseText() << "/" << q.lastError().driverText();
    }
    return result;
}

bool ProgramDatabase::exec(QString query, QVariant arg1)
{
    QSqlQuery q(db);
    if(!q.prepare(query))
    {
        qDebug() << "Error in query:" << query << ":" << q.lastError().databaseText() << "/" << q.lastError().driverText();
        return false;
    }
    q.addBindValue(arg1);
    bool result = q.exec();
    if(!result)
    {
        qDebug() << "Error in query:" << query << ":" << q.lastError().databaseText() << "/" << q.lastError().driverText();
    }
    return result;
}

bool ProgramDatabase::exec(QString query, QVariant arg1, QVariant arg2)
{
    QSqlQuery q(db);
    if(!q.prepare(query))
    {
        qDebug() << "Error in query:" << query << ":" << q.lastError().databaseText() << "/" << q.lastError().driverText();
        return false;
    }
    q.addBindValue(arg1);
    q.addBindValue(arg2);
    bool result = q.exec();
    if(!result)
    {
        qDebug() << "Error in query:" << query << ":" << q.lastError().databaseText() << "/" << q.lastError().driverText();
    }
    return result;
}

bool ProgramDatabase::exec(QString query, QVariant arg1, QVariant arg2, QVariant arg3)
{
    QSqlQuery q(db);
    if(!q.prepare(query))
    {
        qDebug() << "Error in query:" << query << ":" << q.lastError().databaseText() << "/" << q.lastError().driverText();
        return false;
    }
    q.addBindValue(arg1);
    q.addBindValue(arg2);
    q.addBindValue(arg3);

    bool result = q.exec();
    if(!result)
    {
        qDebug() << "Error in query:" << query << ":" << q.lastError().databaseText() << "/" << q.lastError().driverText();
    }
    return result;
}

bool ProgramDatabase::exec(QString query, QVariant arg1, QVariant arg2, QVariant arg3, QVariant arg4)
{
    QSqlQuery q(db);
    if(!q.prepare(query))
    {
        qDebug() << "Error in query:" << query << ":" << q.lastError().databaseText() << "/" << q.lastError().driverText();
        return false;
    }
    q.addBindValue(arg1);
    q.addBindValue(arg2);
    q.addBindValue(arg3);
    q.addBindValue(arg4);

    bool result = q.exec();
    if(!result)
    {
        qDebug() << "Error in query:" << query << ":" << q.lastError().databaseText() << "/" << q.lastError().driverText();
    }
    return result;
}

bool ProgramDatabase::exec(QString query, QVariant arg1, QVariant arg2, QVariant arg3, QVariant arg4, QVariant arg5)
{
    QSqlQuery q(db);
    if(!q.prepare(query))
    {
        qDebug() << "Error in query:" << query << ":" << q.lastError().databaseText() << "/" << q.lastError().driverText();
        return false;
    }
    q.addBindValue(arg1);
    q.addBindValue(arg2);
    q.addBindValue(arg3);
    q.addBindValue(arg4);
    q.addBindValue(arg5);

    bool result = q.exec();
    if(!result)
    {
        qDebug() << "Error in query:" << query << ":" << q.lastError().databaseText() << "/" << q.lastError().driverText();
    }
    return result;
}

bool ProgramDatabase::exec(QString query, QVariant arg1, QVariant arg2, QVariant arg3,
                           QVariant arg4, QVariant arg5, QVariant arg6)
{
    QSqlQuery q(db);
    if(!q.prepare(query))
    {
        qDebug() << "Error in query:" << query << ":" << q.lastError().databaseText() << "/" << q.lastError().driverText();
        return false;
    }
    q.addBindValue(arg1);
    q.addBindValue(arg2);
    q.addBindValue(arg3);
    q.addBindValue(arg4);
    q.addBindValue(arg5);
    q.addBindValue(arg6);

    bool result = q.exec();
    if(!result)
    {
        qDebug() << "Error in query:" << query << ":" << q.lastError().databaseText() << "/" << q.lastError().driverText();
    }
    return result;
}

QSqlQuery ProgramDatabase::q(QString query, QVariant arg1)
{
    QSqlQuery q(db);
    if(!q.prepare(query))
    {
        qDebug() << "Error in query:" << query << ":" << q.lastError().databaseText() << "/" << q.lastError().driverText();
        return q;
    }
    q.addBindValue(arg1);
    bool result = q.exec();
    if(!result)
    {
        qDebug() << "Error in query:" << query << ":" << q.lastError().databaseText() << "/" << q.lastError().driverText();
    }
    return q;
}

QSqlQuery ProgramDatabase::q(QString query, QVariant arg1, QVariant arg2)
{
    QSqlQuery q(db);
    if(!q.prepare(query))
    {
        qDebug() << "Error in query:" << query << ":" << q.lastError().databaseText() << "/" << q.lastError().driverText();
        return q;
    }
    q.addBindValue(arg1);
    q.addBindValue(arg2);
    bool result = q.exec();
    if(!result)
    {
        qDebug() << "Error in query:" << query << ":" << q.lastError().databaseText() << "/" << q.lastError().driverText();
    }
    return q;
}

QSqlQuery ProgramDatabase::q(QString query, QVariant arg1, QVariant arg2, QVariant arg3)
{
    QSqlQuery q(db);
    if(!q.prepare(query))
    {
        qDebug() << "Error in query:" << query << ":" << q.lastError().databaseText() << "/" << q.lastError().driverText();
        return q;
    }
    q.addBindValue(arg1);
    q.addBindValue(arg2);
    q.addBindValue(arg3);
    bool result = q.exec();
    if(!result)
    {
        qDebug() << "Error in query:" << query << ":" << q.lastError().databaseText() << "/" << q.lastError().driverText();
    }
    return q;
}
