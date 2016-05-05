
#include "rm.h"

RelationManager* RelationManager::_rm = 0;

RelationManager* RelationManager::instance()
{
    if(!_rm)
        _rm = new RelationManager();

    return _rm;
}

RelationManager::RelationManager()
{
}

RelationManager::~RelationManager()
{
}

vector<Attribute> getTableAttr(){
    Attribute table1, table2, table3;
    table1.name = "table-id";
    table1.type = TypeInt;
    table1.length = 4;
    table2.name = "table-name";
    table2.type = TypeVarChar;
    table2.length = 50;
    table3.name = "file-name";
    table3.type = TypeVarChar;
    table3.length = 50;

    std::vector<Attribute> table;
    table.push_back(table1);
    table.push_back(table2);
    table.push_back(table3);

    return table;
}

vector<Attribute> getColumAttr(){   
    Attribute col1, col2, col3, col4, col5;
    col1.name = "column-id";
    col1.type = TypeInt;
    col1.length = 4;
    col2.name = "column-name";
    col2.type = TypeVarChar;
    col2.length = 50;
    col3.name = "column-type";
    col3.type = TypeInt;
    col3.length = 4;
    col4.name = "column-length";
    col4.type = TypeInt;
    col4.length = 4;
    col5.name = "column-position";
    col5.type = TypeInt;
    col5.length = 4;

    std::vector<Attribute> column;
    column.push_back(col1);
    column.push_back(col2);
    column.push_back(col3);
    column.push_back(col4);
    column.push_back(col5);
    
    return column;
}

RC RelationManager::createCatalog()
{
    vector<Attribute> table = getTableAttr();
    vector<Attribute> colum = getColumAttr();   
    
    return 0;
}

RC RelationManager::deleteCatalog()
{
    if(deleteTable("Columns") != 0){
	return -1;
    }
    if(deleteTable("Tables") != 0){
	return -1;
    }
    return 0;
}

RC RelationManager::createTable(const string &tableName, const vector<Attribute> &attrs)
{
    RecordBasedFileManager *r = RecordBasedFileManager::instance();
    FileHandle f; 

    if(r->createFile(tableName) != 0){
    	return -1;
    }

    if(r->openFile(tableName, f) != 0){
	return -1;
    }
    
    const vector<Attribute> columAttr = getColumAttr();
    const vector<Attribute> tableAttr = getTableAttr();
    
    const void *colData = malloc(PAGE_SIZE);

    


    return 0;
}

RC RelationManager::deleteTable(const string &tableName)
{
    
    return -1;
}

RC RelationManager::getAttributes(const string &tableName, vector<Attribute> &attrs)
{
    return -1;
}

RC RelationManager::insertTuple(const string &tableName, const void *data, RID &rid)
{
    return -1;
}

RC RelationManager::deleteTuple(const string &tableName, const RID &rid)
{
    return -1;
}

RC RelationManager::updateTuple(const string &tableName, const void *data, const RID &rid)
{
    return -1;
}

RC RelationManager::readTuple(const string &tableName, const RID &rid, void *data)
{
    return -1;
}

RC RelationManager::printTuple(const vector<Attribute> &attrs, const void *data)
{
	return -1;
}

RC RelationManager::readAttribute(const string &tableName, const RID &rid, const string &attributeName, void *data)
{
    return -1;
}

RC RelationManager::scan(const string &tableName,
      const string &conditionAttribute,
      const CompOp compOp,                  
      const void *value,                    
      const vector<string> &attributeNames,
      RM_ScanIterator &rm_ScanIterator)
{
    return -1;
}



