#ifndef PROGRAMDATABASE_H
#define PROGRAMDATABASE_H

#include <QtSql/QSqlDatabase>
#include "../smallvm/codedocument.h"
#include "Lexer/token.h"
#include "Parser/KalimatAst/kalimat_ast_gen.h"

namespace ProgDb
{
    const int Cu_Program = 0;
    const int Cu_Module = 1;

    const int Cu_FuncDef = 0;
    const int Cu_ClassDef = 1;
    const int Cu_RulesDef = 2;
    const int Cu_GlobalDef = 3;

    const int Cu_IsProcedure = 0;
    const int Cu_IsFunction = 1;
}

class ProgramDatabase
{
    QSqlDatabase db;
    bool _isOpen;
public:
    ProgramDatabase();
    bool open();
    void createTables(bool actually);
    void close();
    bool isOpen() { return _isOpen; }

    bool hasModule(QString filename);
    void updateModule(QString filename, CodeDocument *doc, shared_ptr<CompilationUnit> module);
    void updateModuleTokens(QString filename, const QVector<Token> &tokens);
    void updateImportsOfModule(QString filename, QVector<QString> imports);
    void updateModuleDefinitions(QString filename, shared_ptr<CompilationUnit> module);
    void addDefinitionToModule(int id, QString filename, shared_ptr<Declaration>);

    void addClassDef(int defId, QString filename, shared_ptr<ClassDecl>);
    void addFunctionDef(int defId, QString filename, shared_ptr<ProceduralDecl>);

    QSqlQuery q(QString query, QVariant);
    QSqlQuery q(QString query, QVariant arg1, QVariant arg2);
    QSqlQuery q(QString query, QVariant arg1, QVariant arg2, QVariant arg3);
private:
    bool exec(QString query);
    bool exec(QString query, QVariant);
    bool exec(QString query, QVariant, QVariant);
    bool exec(QString query, QVariant, QVariant, QVariant);
    bool exec(QString query, QVariant, QVariant, QVariant, QVariant);
    bool exec(QString query, QVariant, QVariant, QVariant, QVariant, QVariant);
    bool exec(QString query, QVariant, QVariant, QVariant, QVariant, QVariant, QVariant);



};

#endif // PROGRAMDATABASE_H
