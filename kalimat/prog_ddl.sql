drop table modules;
CREATE TABLE "modules" (
    "filename" TEXT NOT NULL primary key,
    "type" INTEGER NOT NULL,
    "code_doc" INTEGER NOT NULL,
    "timestamp" INTEGER
);
drop table tokens;
CREATE TABLE "tokens" (
    "module_filename" TEXT NOT NULL  references modules(filename),
    "pos" INTEGER,
    "line" INTEGER,
    "col" INTEGER,
    "lexeme" TEXT,
    "length" INTEGER,
    primary key(module_filename, pos)
);
drop table definitions;
CREATE TABLE "definitions" (
    "id" INTEGER,
    "module_filename" TEXT NOT NULL references modules(filename),
    "name" TEXT NOT NULL,
    "type" INTEGER NOT NULL,
	primary key (id, module_filename)
);

drop table function_definitions;
CREATE TABLE function_definitions (
    "def_id" integer not null ,
    "module_filename" TEXT NOT NULL ,
    "arity" INTEGER NOT NULL,
    "num_return_values" INTEGER NOT NULL,
    "defining_token_pos" INTEGER,
    "ending_token_pos" INTEGER,
    primary key (def_id, module_filename),
    foreign key (def_id, module_filename) references definitions(id, module_filename) on delete cascade
);

drop table module_imports;
CREATE TABLE "module_imports" (
    "importer" TEXT NOT NULL , 
    "imported" TEXT NOT NULL,
    constraint module_imports_pk primary key ("importer", imported),
    foreign key (importer) REFERENCES modules(filename)
    
);
;
