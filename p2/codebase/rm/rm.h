
#ifndef _rm_h_
#define _rm_h_

#include <string>
#include <string.h>
#include <vector>

#include "../rbf/rbfm.h"

using namespace std;

# define RM_EOF (-1)  // end of a scan operator
# define RM_MALLOC_FAILED (2)

# define COLUMN_ID     2
# define COLUMN_NAME   "column-name"
# define COLUMN_TYPE   "column-type"
# define COLUMN_LENGTH   "column-length"

# define TABLE_NUMBER_ID     1 
# define TABLE_NAME   "table-name"
# define TABLE_ID   "table-id"
# define TABLE_FILE   "file-name"

// RM_ScanIterator is an iteratr to go through tuples
class RM_ScanIterator {
public:
  RM_ScanIterator() {};
  ~RM_ScanIterator() {};

  // "data" follows the same format as RelationManager::insertTuple()
  RC getNextTuple(RID &rid, void *data) { return RM_EOF; };
  RC close() { return -1; };
};


// Relation Manager
class RelationManager
{
public:
  static RelationManager* instance();
  
  RC createCatalog();

  RC deleteCatalog();

  RC createTable(const string &tableName, const vector<Attribute> &attrs);

  RC deleteTable(const string &tableName);

  int getTableIndex(const string &tableName);

  RC setColumnData(const string &tableName, vector<Attribute> columAttr, const void * data, int id, int offset);

  RC setTableData(const string &tableName, const void * data, int id, int offset);

  RC getAttributes(const string &tableName, vector<Attribute> &attrs);

  RC insertTuple(const string &tableName, const void *data, RID &rid);

  RC deleteTuple(const string &tableName, const RID &rid);

  RC updateTuple(const string &tableName, const void *data, const RID &rid);

  RC readTuple(const string &tableName, const RID &rid, void *data);

  // Print a tuple that is passed to this utility method.
  // The format is the same as printRecord().
  RC printTuple(const vector<Attribute> &attrs, const void *data);

  RC readAttribute(const string &tableName, const RID &rid, const string &attributeName, void *data);

  // Scan returns an iterator to allow the caller to go through the results one by one.
  // Do not store entire results in the scan iterator.
  RC scan(const string &tableName,
      const string &conditionAttribute,
      const CompOp compOp,                  // comparison type such as "<" and "="
      const void *value,                    // used in the comparison
      const vector<string> &attributeNames, // a list of projected attributes
      RM_ScanIterator &rm_ScanIterator);

protected:
  RelationManager();
  ~RelationManager();

private:
  static RelationManager *_rm;
  static RecordBasedFileManager* rbfm; 
  unsigned int index;
  FileHandle f;
  const string* tables_filename;
  const string* columns_filename;
  };

  vector<Attribute> getTableAttr();
  
  vector<Attribute> getColumAttr(); 


#endif
