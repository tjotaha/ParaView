/*=========================================================================

Program:   Visualization Toolkit
Module:    $RCSfile$

Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
All rights reserved.
See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
/*-------------------------------------------------------------------------
  Copyright 2008 Sandia Corporation.
  Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
  the U.S. Government retains certain rights in this software.
-------------------------------------------------------------------------*/
// .NAME vtkSQLDatabaseSchema - create a SQL database schema
//
// .SECTION Description
//  A class to create a SQL database schema
//
// .SECTION Thanks
// Thanks to Philippe Pebay and David Thompson from Sandia National 
// Laboratories for implementing this class.
//
// .SECTION See Also
// vtkSQLSQLDatabase

#ifndef __vtkSQLDatabaseSchema_h
#define __vtkSQLDatabaseSchema_h

#include "vtkObject.h"

#include <cstdarg> // Because one method has a variable list of arguments 

// This is a list of known supported VTK SQL backend classes. 
// A particular SQL backend does not have to be listed here to be supported, but
// these macros allow for the specification of SQL backend-specific database schema items.
#define VTK_SQL_ALLBACKENDS      "*" // works for all backends
#define VTK_SQL_MYSQL            "vtkMySQLDatabase"
#define VTK_SQL_POSTGRESQL       "vtkPostgreSQLDatabase"
#define VTK_SQL_SQLITE           "vtkSQLiteDatabase"

class vtkSQLDatabaseSchemaInternals;

class VTK_IO_EXPORT vtkSQLDatabaseSchema : public vtkObject
{
 public:
  vtkTypeRevisionMacro(vtkSQLDatabaseSchema, vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);
  static vtkSQLDatabaseSchema *New();

  //BTX
  // Description:
  // Basic data types for database columns
  enum DatabaseColumnType
    {
      SERIAL    = 0, // specifying the indices explicitly to prevent bad compiler mishaps
      SMALLINT  = 1,
      INTEGER   = 2,
      BIGINT    = 3,
      VARCHAR   = 4,
      TEXT      = 5,
      REAL      = 6,
      DOUBLE    = 7,
      BLOB      = 8,
      TIME      = 9,
      DATE      = 10,
      TIMESTAMP = 11
    };

  // Description:
  // Types of indices that can be generated for database tables
  enum DatabaseIndexType
    {
      INDEX       = 0, // Non-unique index of values in named columns
      UNIQUE      = 1, // Index of values in named columns required to have at most one entry per pair of valid values.
      PRIMARY_KEY = 2 // Like UNIQUE but additionally this serves as the primary key for the table to speed up insertions.
    };

  // Description:
  // Events where database triggers can be registered.
  enum DatabaseTriggerType
    {
      BEFORE_INSERT = 0, // Just before a row is inserted
      AFTER_INSERT  = 1,  // Just after a row is inserted
      BEFORE_UPDATE = 2, // Just before a row's values are changed
      AFTER_UPDATE  = 3,  // Just after a row's values are changed
      BEFORE_DELETE = 4, // Just before a row is deleted
      AFTER_DELETE  = 5  // Just after a row is deleted
    };
  //ETX

  // Description:
  // Add a preamble to the schema
  // This can be used, in particular, to create functions and/or
  // load languages in a backend-specific manner.
  // Example usage:
  // vtkSQLDatabaseSchema* schema = vtkSQLDatabaseSchema::New();
  // schema->SetName( "Example" );
  // schema->AddPreamble( "dropPLPGSQL", "DROP LANGUAGE IF EXISTS PLPGSQL CASCADE", VTK_SQL_POSTGRESQL );
  // schema->AddPreamble( "loadPLPGSQL", "CREATE LANGUAGE PLPGSQL", VTK_SQL_POSTGRESQL );
  // schema->AddPreamble( "createsomefunction", 
  // "CREATE OR REPLACE FUNCTION somefunction() RETURNS TRIGGER AS $btable$ "
  // "BEGIN "
  // "INSERT INTO btable (somevalue) VALUES (NEW.somenmbr); "
  // "RETURN NEW; "
  // "END; $btable$ LANGUAGE PLPGSQL", 
  //  VTK_SQL_POSTGRESQL );

  virtual int AddPreamble( const char* preName, 
                           const char* preAction,
                           const char* preBackend = VTK_SQL_ALLBACKENDS );

  // Description:
  // Add a table to the schema 
  virtual int AddTable( const char* tblName );

  virtual int AddColumnToTable( int tblHandle, 
                                int colType, 
                                const char* colName,
                                int colSize, 
                                const char* colAttribs );

  virtual int AddColumnToTable( const char* tblName, 
                                int colType, 
                                const char* colName,
                                int colSize, 
                                const char* colAttribs )
  {
    return this->AddColumnToTable( this->GetTableHandleFromName( tblName ),
                                   colType, 
                                   colName, 
                                   colSize, 
                                   colAttribs );
  }

  virtual int AddIndexToTable( int tblHandle, 
                               int idxType, 
                               const char* idxName );

  virtual int AddIndexToTable( const char* tblName, 
                               int idxType, 
                               const char* idxName )
  {
    return this->AddIndexToTable( this->GetTableHandleFromName( tblName ), 
                                  idxType, 
                                  idxName );
  }

  virtual int AddColumnToIndex( int tblHandle, 
                                int idxHandle, 
                                int colHandle );

  virtual int AddColumnToIndex( const char* tblName, 
                                const char* idxName, 
                                const char* colName )
  {
    int tblHandle = this->GetTableHandleFromName( tblName );
    return this->AddColumnToIndex( tblHandle,
                                   this->GetIndexHandleFromName( tblName, idxName ),
                                   this->GetColumnHandleFromName( tblName, colName ) );
  }

  virtual int AddTriggerToTable( int tblHandle,
                                 int trgType, 
                                 const char* trgName, 
                                 const char* trgAction,
                                 const char* trgBackend = VTK_SQL_ALLBACKENDS );

  virtual int AddTriggerToTable( const char* tblName,
                                 int trgType, 
                                 const char* trgName, 
                                 const char* trgAction,
                                 const char* trgBackend = VTK_SQL_ALLBACKENDS )
  {
    return this->AddTriggerToTable( this->GetTableHandleFromName( tblName ),
                                    trgType, 
                                    trgName, 
                                    trgAction,
                                    trgBackend );
  }

  // Description:
  // Given a preamble name, get its handle.
  int GetPreambleHandleFromName( const char* preName );

  // Description:
  // Given a preamble handle, get its name.
  const char* GetPreambleNameFromHandle( int preHandle );

  // Description:
  // Given a preamble handle, get its action.
  const char* GetPreambleActionFromHandle( int preHandle );

  // Description:
  // Given a preamble handle, get its backend.
  const char* GetPreambleBackendFromHandle( int preHandle );

  // Description:
  // Given a table name, get its handle.
  int GetTableHandleFromName( const char* tblName );

  // Description:
  // Given a table hanlde, get its name.
  const char* GetTableNameFromHandle( int tblHandle );

  // Description:
  // Given the names of a table and an index, get the handle of the index in this table.
  int GetIndexHandleFromName( const char* tblName, 
                              const char* idxName );

  // Description:
  // Given the handles of a table and an index, get the name of the index.
  const char* GetIndexNameFromHandle( int tblHandle, 
                                      int idxHandle );

  // Description:
  // Given the handles of a table and an index, get the type of the index.
  int GetIndexTypeFromHandle( int tblHandle, 
                              int idxHandle );

  // Description:
  // Given the handles of a table, an index, and a column name, get the column name.
  const char* GetIndexColumnNameFromHandle( int tblHandle, 
                                            int idxHandle,
                                            int cnmHandle );

  // Description:
  // Given the names of a table and a column, get the handle of the column in this table.
  int GetColumnHandleFromName( const char* tblName, 
                               const char* colName );

  // Description:
  // Given the handles of a table and a column, get the name of the column.
  const char* GetColumnNameFromHandle( int tblHandle, 
                                       int colHandle );

  // Description:
  // Given the handles of a table and a column, get the type of the column.
  int GetColumnTypeFromHandle( int tblHandle, 
                               int colHandle );

  // Description:
  // Given the handles of a table and a column, get the size of the column.
  int GetColumnSizeFromHandle( int tblHandle, 
                               int colHandle );

  // Description:
  // Given the handles of a table and a column, get the attributes of the column.
  const char* GetColumnAttributesFromHandle( int tblHandle, 
                                             int colHandle );

  // Description:
  // Given the names of a trigger and a table, get the handle of the trigger in this table.
  int GetTriggerHandleFromName( const char* tblName, 
                                const char* trgName );

  // Description:
  // Given the handles of a table and a trigger, get the name of the trigger.
  const char* GetTriggerNameFromHandle( int tblHandle, 
                                        int trgHandle );

  // Description:
  // Given the handles of a table and a trigger, get the type of the trigger.
  int GetTriggerTypeFromHandle( int tblHandle, 
                                int trgHandle );

  // Description:
  // Given the handles of a table and a trigger, get the action of the trigger.
  const char* GetTriggerActionFromHandle( int tblHandle, 
                                          int trgHandle );

  // Description:
  // Given the handles of a table and a trigger, get the backend of the trigger.
  const char* GetTriggerBackendFromHandle( int tblHandle, 
                                           int trgHandle );

  // Description:
  // Reset the schema to its initial, empty state.
  void Reset();

  // Description:
  // Get the number of preambles.
  int GetNumberOfPreambles();

  // Description:
  // Get the number of tables.
  int GetNumberOfTables();

  // Description:
  // Get the number of columns in a particular table .
  int GetNumberOfColumnsInTable( int tblHandle );

  // Description:
  // Get the number of indices in a particular table .
  int GetNumberOfIndicesInTable( int tblHandle );

  // Description:
  // Get the number of column names associated to a particular index in a particular table .
  int GetNumberOfColumnNamesInIndex( int tblHandle, 
                                     int idxHandle );

  // Description:
  // Get the number of trigger in a particular table .
  int GetNumberOfTriggersInTable( int tblHandle );

  // Description:
  // Set/Get the name of the schema.
  vtkSetStringMacro(Name);
  vtkGetStringMacro(Name);

  //BTX
  // Tokens passed to AddTable to indicate the type of data that follows. Random integers chosen to prevent mishaps.
  enum VarargTokens
    {
      COLUMN_TOKEN       = 58,
      INDEX_TOKEN        = 63,
      INDEX_COLUMN_TOKEN = 65,
      END_INDEX_TOKEN    = 75,
      TRIGGER_TOKEN      = 81,
      END_TABLE_TOKEN    = 99
    };

  // Description:
  // An unwrappable but useful routine to construct built-in schema.
  // Example usage:
  // int main()
  // {
  // vtkSQLDatabaseSchema* schema = vtkSQLDatabaseSchema::New();
  // schema->SetName( "Example" );
  // schema->AddTableMultipleArguments( "atable",
  // vtkSQLDatabaseSchema::COLUMN_TOKEN, vtkSQLDatabaseSchema::INTEGER, "tablekey",  0, "",
  // vtkSQLDatabaseSchema::COLUMN_TOKEN, vtkSQLDatabaseSchema::VARCHAR, "somename", 11, "NOT NULL",
  // vtkSQLDatabaseSchema::COLUMN_TOKEN, vtkSQLDatabaseSchema::BIGINT,  "somenmbr", 17, "DEFAULT 0",
  // vtkSQLDatabaseSchema::INDEX_TOKEN, vtkSQLDatabaseSchema::PRIMARY_KEY, "bigkey",
  // vtkSQLDatabaseSchema::INDEX_COLUMN_TOKEN, "tablekey",
  // vtkSQLDatabaseSchema::END_INDEX_TOKEN,
  // vtkSQLDatabaseSchema::INDEX_TOKEN,  vtkSQLDatabaseSchema::UNIQUE, "reverselookup",
  // vtkSQLDatabaseSchema::INDEX_COLUMN_TOKEN, "somename",
  // vtkSQLDatabaseSchema::INDEX_COLUMN_TOKEN, "somenmbr",
  // vtkSQLDatabaseSchema::END_INDEX_TOKEN,
  // vtkSQLDatabaseSchema::TRIGGER_TOKEN,  vtkSQLDatabaseSchema::AFTER_INSERT,
  //  "InsertTrigger", "DO NOTHING", 
  //   VTK_SQL_SQLITE,
  // vtkSQLDatabaseSchema::TRIGGER_TOKEN,  vtkSQLDatabaseSchema::AFTER_INSERT,
  //  "InsertTrigger", "FOR EACH ROW EXECUTE PROCEDURE somefunction ()", 
  //   VTK_SQL_POSTGRESQL,
  // vtkSQLDatabaseSchema::TRIGGER_TOKEN,  vtkSQLDatabaseSchema::AFTER_INSERT,
  //   "InsertTrigger", "FOR EACH ROW INSERT INTO btable SET SomeValue = NEW.SomeNmbr", 
  //   VTK_SQL_MYSQL,
  // vtkSQLDatabaseSchema::END_TABLE_TOKEN
  // );
  // return 0;
  // }
  int AddTableMultipleArguments( const char* tblName, ... );
  //ETX

 protected:
  vtkSQLDatabaseSchema();
  ~vtkSQLDatabaseSchema();

  char* Name;
//BTX
  class vtkSQLDatabaseSchemaInternals* Internals;
//ETX

 private:
  vtkSQLDatabaseSchema(const vtkSQLDatabaseSchema &); // Not implemented.
  void operator=(const vtkSQLDatabaseSchema &); // Not implemented.
};

#endif // __vtkSQLDatabaseSchema_h
