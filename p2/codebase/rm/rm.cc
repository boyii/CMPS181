
#include "rm.h"

RelationManager* RelationManager::_rm = 0;
RecordBasedFileManager* RelationManager::rbfm = 0;

RelationManager* RelationManager::instance()
{
    if(!_rm)
        _rm = new RelationManager();

    return _rm;
}

RelationManager::RelationManager()
{
    rbfm = RecordBasedFileManager::instance();
    index = 0;
    tables_filename = NULL;
    columns_filename = NULL;
}

RelationManager::~RelationManager(){}

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

    if(getTableIndex("Tables") == 0){
	return -1;
    }

    if(rbfm->createFile("Tables.tbl") != 0){
	return -1;	
    }

    FileHandle f2;
    if(rbfm->openFile("Tables.tbl", f2) != 0){
    	return -1;
    }    
   
    void * tableData = malloc(PAGE_SIZE);
    vector<Attribute> tableAttr = getTableAttr();
    if(tableData == NULL) return RM_MALLOC_FAILED;
    RID rid;   
    int offset = 0;
    if(setTableData("Tables", tableData, TABLE_NUMBER_ID, offset) != 0){
	return -1;
    }
    rbfm->insertRecord(f, tableAttr, tableData, rid); 
    if(setTableData("Columns", tableData, COLUMN_ID, offset) != 0){
	return -1;
    }
    rbfm->insertRecord(f, tableAttr, tableData, rid);
    rbfm->closeFile(f2);

    if(rbfm->createFile("Columns.clm") != 0){
	return -1;	
    }

    if(rbfm->openFile("Columns.clm", f2) != 0){
    	return -1;
    }    
    
    vector<Attribute> columAttr = getColumAttr();
    int count = 0;
    void * colData = malloc(PAGE_SIZE);
    void * tabData = malloc(PAGE_SIZE);
    int offset2 = 0;
    RID colRID;
    while(count < columAttr.size()){
	setColumnData("Columns", columAttr, colData, COLUMN_ID, offset2);
        rbfm->insertRecord(f2, columAttr, colData, colRID);
	count++;
    }

    int offset3;
    RID tabRID;
    while(count < tableAttr.size()){
	setColumnData("Tables", tableAttr, tabData, TABLE_NUMBER_ID, offset3);
        rbfm->insertRecord(f2, tableAttr, tabData, tabRID);
	count++;
    }

    rbfm->closeFile(f);
    return 0;
}

RC RelationManager::deleteCatalog()
{
      
   return -1;
}

RC RelationManager::setTableData(const string &tableName, const void * data, int id, int offset){

    //set table id
    memcpy((char*) data + offset, &index, INT_SIZE);
    offset += INT_SIZE;
    index += 1;

    //set table name
    memcpy((char*) data + offset, &tableName, VARCHAR_LENGTH_SIZE);
    offset += 50;

    //set file name
    std::string fileName = tableName;
    fileName.append(".tbl");
    memcpy((char*) data + offset, &fileName, VARCHAR_LENGTH_SIZE);
    offset += 50;

    return 0;
}

RC RelationManager::setColumnData(const string &tableName, vector<Attribute> columAttr, const void * data, int id, int offset){

    int realId = id + 1;
    memcpy ((char*) data + offset, &realId, INT_SIZE);
    offset += INT_SIZE;

    memcpy ((char*) data + offset, columAttr[id].name.c_str(), VARCHAR_LENGTH_SIZE);
    offset += VARCHAR_LENGTH_SIZE;

    memcpy ((char*) data + offset, &(columAttr[id].type), INT_SIZE);
    offset += INT_SIZE;

    memcpy ((char*) data + offset, &(columAttr[id].length), INT_SIZE);
    offset += INT_SIZE;

    return 0;
}
RC RelationManager::createTable(const string &tableName, const vector<Attribute> &attrs)
{
    std::string tblName = tableName + ".tbl";
    if(rbfm->createFile(tblName) != 0){
    	return -1;
    }
    if(rbfm->openFile(tblName, f) != 0){
	return -1;
    }
 
    int index = getTableIndex(tableName);
    const vector<Attribute> tableAttr = getTableAttr();

    void * data = malloc(PAGE_SIZE);
    if (data == NULL) return RM_MALLOC_FAILED;
    RID rid;

    int offset = 0;
    if(setTableData(tableName, data, index, offset) != 0){
	return -1;
    }
   
    rbfm->insertRecord(f, tableAttr, &data, rid);
    rbfm->closeFile(f);


    const vector<Attribute> columAttr = getColumAttr();
    void * colData = malloc(PAGE_SIZE);
    if (colData == NULL) return RM_MALLOC_FAILED;
    RID colRid;

    int colOffset = 0;

    if(rbfm->openFile("Columns.clm", f) != 0){
	return -1;
    }
    
    for(unsigned int i = 0; (unsigned) i < columAttr.size(); i++){
    	if(setColumnData(tableName, columAttr, colData, i, colOffset) != 0){
		return -1;
        }
    	rbfm->insertRecord(f, columAttr, colData, colRid);
    }
    rbfm->closeFile(f);
    free(data);
    free(colData);
    
    return 0;
}

RC RelationManager::deleteTable(const string &tableName)
{
    if(tableName.compare("Columns.clm") == 0 || 
       tableName.compare("Tables.tbl") == 0)
    {
        	return -1;
    }

    unsigned index = getTableIndex(tableName);

    std::string table;
	table = tableName + ".tbl";
    if(rbfm->destroyFile(table) != 0){
	return -1;
    }

    if(rbfm->openFile("Tables.tbl", f) != 0){
	return -1;
    }
    
    vector<string> projAttr;
    	projAttr.push_back(TABLE_ID);
    RBFM_ScanIterator s;
    if(rbfm->scan(f, getTableAttr(), TABLE_ID, EQ_OP, &index, projAttr, s) != 0){
	return -4;
    }

    void * data = malloc(INT_SIZE);
    RID r; 
    s.getNextRecord(r, data);
    rbfm->deleteRecord(f, getTableAttr(), r);
    rbfm->closeFile(f);
    s.close();

 
    if(rbfm->openFile("Columns.clm", f) != 0) return -2;
    RBFM_ScanIterator s2;
    if(rbfm->scan(f, getColumAttr(), TABLE_ID, EQ_OP, &index, projAttr, s2) != 0){
	return 4;
    }
	

    RID rid2;
    void * data2 = malloc(INT_SIZE);
    while(s2.getNextRecord(rid2, data2) != RBFM_EOF){
	rbfm->deleteRecord(f, getColumAttr(), rid2);
    }
    rbfm->closeFile(f);
    s2.close();

    return 0;
}

int RelationManager::getTableIndex(const string &tableName){
    const vector<Attribute> tableAttr = getTableAttr();
    if(rbfm->openFile("Tables.tbl", f) != 0){
    	return -1;
    }
    
    vector<string> projAttr;
	projAttr.push_back(TABLE_ID);
	projAttr.push_back(TABLE_FILE);
	projAttr.push_back(TABLE_NAME);

    RBFM_ScanIterator s;    
    void * data = malloc(INT_SIZE);
    RID r;
    if(rbfm->scan(f, getTableAttr(), "table-name", EQ_OP, tableName.c_str(), projAttr, s) != 0){
	return -1;
    } 

    if(s.getNextRecord(r, data) != 0){
	return 2;
    }
    int id;
    memcpy(&id, data, INT_SIZE);

    s.close();
    if(rbfm->closeFile(f) != 0){
	return -1;;
    }
    
    return id;
}

RC RelationManager::getAttributes(const string &tableName, vector<Attribute> &attrs)
{
    int index = getTableIndex(tableName);

    const vector<Attribute> columAttr = getColumAttr();
    vector<string> projAttr;
	projAttr.push_back(COLUMN_NAME);
	projAttr.push_back(COLUMN_TYPE);
	projAttr.push_back(COLUMN_LENGTH);

    FileHandle f;
    RBFM_ScanIterator s;
    RID rid;
    rbfm->openFile("Columns.clm", f);
    rbfm->scan(f, getColumAttr(), "table-id", EQ_OP,(char*) index, projAttr, s);

    void * data = malloc(PAGE_SIZE);
    while(s.getNextRecord(rid, data) != RBFM_EOF){
        int offset = 0;
	Attribute temp;
	
	char * name;	
	unsigned name_length;
	memcpy(&name_length, (char*) data + offset, VARCHAR_LENGTH_SIZE);
	offset += VARCHAR_LENGTH_SIZE;

	name = (char *) malloc(name_length + 1);
	memcpy(name, (char*) data + offset, name_length);
	temp.name = name;
	offset += name_length;

	memcpy(&(temp.type), (char*) data + offset, INT_SIZE);
	offset += INT_SIZE;
        
	memcpy(&(temp.length), (char*) data + offset, INT_SIZE);
        offset += INT_SIZE;

	attrs.push_back(temp);
    }

    s.close();
    if(rbfm->closeFile(f) != 0){
	return -1;
    }
    return 0;
}

RC RelationManager::insertTuple(const string &tableName, const void *data, RID &rid)
{ 
    if(tableName.compare("Tables.tbl") == 0 || tableName.compare("Columns.clm") == 0){
	return -1;
    } 
   
    vector<Attribute> tableRecordDescriptor;
    getAttributes(tableName, tableRecordDescriptor);

    std::string table = tableName + ".tbl";
    if(rbfm->openFile(table, f) != 0){
	return -1;
    }

    int res = rbfm->insertRecord(f, tableRecordDescriptor, data, rid);
    rbfm->closeFile(f);
 
    return res;
}

RC RelationManager::deleteTuple(const string &tableName, const RID &rid)
{
//    if(tableName.compare() == 0 || tableName.compare() == 0){
//	return -1;
//    }    

    std::string table = tableName + ".tbl";
    if(rbfm->openFile(table, f) != 0){
	return -1;
    }

   /// int res = rbfm->deleteRecords(f);
    rbfm->closeFile(f);
 
    return -1;
}

RC RelationManager::updateTuple(const string &tableName, const void *data, const RID &rid)
{
//    if(tableName.compare() == 0 || tableName.compare() == 0){
//	return -1;
//    }    

    std::string table = tableName + ".tbl";
    if(rbfm->openFile(table, f) != 0){
	return -1;
    }

    vector<Attribute> recordDescriptor;
    getAttributes(tableName, recordDescriptor);

    int res = rbfm->updateRecord(f, recordDescriptor, data, rid);
    rbfm->closeFile(f);
 
    return res;
}

RC RelationManager::readTuple(const string &tableName, const RID &rid, void *data)
{
    std::string table = tableName + ".tbl";
    if(rbfm->openFile(table, f) != 0){
	return -1;
    }

    vector<Attribute> tupleAttributes;
    getAttributes(tableName, tupleAttributes);
    if(rbfm->readRecord(f, tupleAttributes, rid, data) != 0){
	return -1;
    }
    rbfm->closeFile(f);
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



