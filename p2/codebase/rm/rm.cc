#include <cmath>
#include <string>
#include <iostream>
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
}

RelationManager::~RelationManager(){}

vector<Attribute> getTableAttr(){

    //make table attributes
    vector<Attribute> table;

    //make an attribute for each of the table elements
    Attribute tab1, tab2, tab3;    
        //id information
	tab1.name = TABLE_ID;
	tab1.type = TypeInt;
	tab1.length = (AttrLength)INT_SIZE;

        //name information
	tab2.name = TABLE_NAME;
	tab2.type = TypeVarChar;
	tab2.length = (AttrLength)40;

        //file information
	tab3.name = TABLE_FILE;
	tab3.type = TypeVarChar;
	tab3.length = (AttrLength)40;

	table.push_back(tab1);
	table.push_back(tab2);
	table.push_back(tab3);
	return table;
}

vector<Attribute> getColumAttr(){   
    Attribute col1, col2, col3, col4, col5;
	 col1.name = "column-id";
	 col1.type = TypeInt;
	 col1.length = INT_SIZE;
	    col2.name = "column-name";
	    col2.type = TypeVarChar;
	    col2.length = VARCHAR_LENGTH_SIZE;
	    col3.name = "column-type";
	    col3.type = TypeInt;
	    col3.length = INT_SIZE;
	    col4.name = "column-length";
	    col4.type = TypeInt;
	    col4.length = INT_SIZE;
	    col5.name = "column-position";
	    col5.type = TypeInt;
	    col5.length = INT_SIZE;

	    std::vector<Attribute> column;
	    column.push_back(col1);
	    column.push_back(col2);
	    column.push_back(col3);
	    column.push_back(col4);
	    column.push_back(col5);
	    
	    return column;
}


/****************************************
 * Creates the Tables.tbl and Columns.clm
 * with the basic records provided in the 
 * documentation.
 */
RC RelationManager::createCatalog()
{
    vector<Attribute> table = getTableAttr();
    vector<Attribute> colum = getColumAttr();   

    //creates Tables.tbl to hold all of the registered tables
    if(rbfm->createFile("Tables.tbl") != 0){
	return -1;	
    }

    FileHandle f2;
    if(rbfm->openFile("Tables.tbl", f2) != 0){
    	return -1; //failed to open
    }    
  
    int tableDataSize = INT_SIZE + 80;
    void * tableData = malloc(tableDataSize);

    vector<Attribute> tableAttr = getTableAttr();
    if(tableData == NULL) return RM_MALLOC_FAILED;

    RID rid;   
    
    //establish Tables record data for insertion -- seperate function
    if(setTableData("Tables", tableData, TABLE_NUMBER_ID) != 0){
	return -1;
    }

    //inserts the Tables record into the new Tables.tbl
    rbfm->insertRecord(f2, getTableAttr(), tableData, rid); 

    //establish Columns record data for insertion
    if(setTableData("Columns", tableData, COLUMN_ID) != 0){
	return -1;
    }
    //inserts the Columns reocrd into the new Tables.tbl
    rbfm->insertRecord(f2, tableAttr, tableData, rid);
    rbfm->closeFile(f2);


    //creates Columns.clm
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
    RID colRID;

    //for each attribute that makes up a column record 
    //it inserts one record containing the information
    while(count < columAttr.size()){
	setColumnData("Columns", columAttr, colData, COLUMN_ID);
        rbfm->insertRecord(f2, columAttr, colData, colRID);
	count++;
    }

    //for each attribute that makes up a table record 
    //it inserts one record containing the information
    RID tabRID;
    while(count < tableAttr.size()){
	setColumnData("Tables", tableAttr, tabData, TABLE_NUMBER_ID);
        rbfm->insertRecord(f2, tableAttr, tabData, tabRID);
	count++;
    }

    rbfm->closeFile(f2);
    return 0;
}

RC RelationManager::deleteCatalog()
{
    //utilize deleteTable with "Tables.tbl"
    //it will catch the fileName and treat it like a catalog
    if(deleteTable("Tables.tbl")!= 0){
	return -1; 
    }
    return 0;        
}

RC RelationManager::setTableData(const string &tableName, const void * data, int table_id){

    //kind of a hack but for now skips the null indicator
    //TODO - set up null indicator, so it works better with other code
    int offset = int(ceil((double) 2 / CHAR_BIT));

    //set table id
    memcpy((char*) data + offset, &table_id, INT_SIZE);
    offset += INT_SIZE;

    int32_t length = tableName.length();
    //set the value of table name length in front of the actual string in data
    memcpy ((char*) data + offset, &(length), VARCHAR_LENGTH_SIZE);
    offset += VARCHAR_LENGTH_SIZE;
    //set up table name into data
    memcpy((char*) data + offset, tableName.c_str(), length);
    offset += length;

    string fileName = tableName + TABLE_FILE_END;
    int32_t file_length = fileName.length();
     
    //set the value of file name length in front of the actual string in data
    memcpy ((char*) data + offset, &(file_length), VARCHAR_LENGTH_SIZE);    
    offset += VARCHAR_LENGTH_SIZE;

    //set the tableName value
    memcpy((char*) data + offset, tableName.c_str(), file_length);
    offset += file_length;
  

    return 0;
}

RC RelationManager::setColumnData(const string &tableName, vector<Attribute> columAttr, const void * data, int id){

    int offset = int(ceil((double)5  / CHAR_BIT));
    int realId = id;

    memcpy ((char*) data + offset, &realId, INT_SIZE);
    offset += INT_SIZE;

    string name = columAttr[id].name.c_str();
    int name_length = name.length();

    memcpy ((char*) data + offset, &name_length, VARCHAR_LENGTH_SIZE); 
    offset += VARCHAR_LENGTH_SIZE;
    memcpy ((char*) data + offset, columAttr[id].name.c_str(), name_length);
    offset += name_length;

    memcpy ((char*) data + offset, &(columAttr[id].type), INT_SIZE);
    offset += INT_SIZE;

    memcpy ((char*) data + offset, &(columAttr[id].length), INT_SIZE);
    offset += INT_SIZE;

    return 0;
}

RC RelationManager::createTable(const string &tableName, const vector<Attribute> &attrs)
{
    FileHandle f2;
    string tblName = tableName + ".tbl";
    
    //create and open the table being made
    if(rbfm->createFile(tblName) != 0){
    	return -1;
    }
    if(rbfm->openFile(tblName, f2) != 0){
	return -1;
    }
 
    //hardcoded because scan isn't reliable
    //need to scan to last element and add 1
    int index = 3;
    const vector<Attribute> tableAttr = getTableAttr();

    //set up new empty page
    void * data = malloc(PAGE_SIZE);
    if (data == NULL) return RM_MALLOC_FAILED;
    RID rid;

    //morphs data into right format for insertion
    if(setTableData(tableName, data, index) != 0){
	return -1;
    }
   
    //inserts the table record into Tables.tbl file
    rbfm->insertRecord(f2, tableAttr, &data, rid);
    rbfm->closeFile(f2);

    const vector<Attribute> columAttr = getColumAttr();
    void * colData = malloc(PAGE_SIZE);
    if (colData == NULL) return RM_MALLOC_FAILED;
    RID colRid;

    //
    if(rbfm->openFile("Columns.clm", f2) != 0){
	return -1;
    }
    
    for(unsigned int i = 0; (unsigned) i < columAttr.size(); i++){
    	if(setColumnData(tableName, columAttr, colData, i) != 0){
		return -1;
        }
    	rbfm->insertRecord(f2, columAttr, colData, colRid);
    }
    rbfm->closeFile(f2);
    free(data);
    free(colData);
    
    return 0;
}

RC RelationManager::deleteTable(const string &tableName)
{
    //to check whether or not it is a catalog
    bool catalog = false;
    
    //statments catch it if catalog and change bool
    if(tableName.compare("Tables.tbl") == 0){
         	catalog = true; 
    }

    if(tableName.compare("Columns.clm") == 0){
        	return -1;
    }

    //grab the index of the file to be deleted
    unsigned index = getTableIndex(tableName);
    FileHandle f;
    string table;

    if(!catalog){
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
    }else{
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

	     if(rbfm->destroyFile("Columns.clm") != 0){
		return -1;
	    }if(rbfm->destroyFile("Tables.tbl") != 0){
		return -1;
	    }

    }

    return 0;
}

int RelationManager::getTableIndex(const string &tableName){
    const vector<Attribute> tableAttr = getTableAttr();

    FileHandle f;
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
// get attributes from scan method which retieves data from table
// then memcpy to vector which holds output
RC RelationManager::getAttributes(const string &tableName, vector<Attribute> &attrs)
{
    int index = getTableIndex(tableName);

    const vector<Attribute> columAttr = getColumAttr();
    vector<string> projAttr;
	projAttr.push_back(COLUMN_NAME);
	projAttr.push_back(COLUMN_TYPE);
	projAttr.push_back(COLUMN_LENGTH);

    FileHandle f2;
    RBFM_ScanIterator s;
    RID rid;
    rbfm->openFile("Columns.clm", f2);
    //scan function gets record with table-id 
    //passes in equals condition
    rbfm->scan(f2, getColumAttr(), "table-id", EQ_OP,(char*) index, projAttr, s);

    void * data = malloc(PAGE_SIZE);
    while(s.getNextRecord(rid, data) != RBFM_EOF){
        // attribute is stored in string format 
        // thus we must find length of the string 
        // from memory and then memcpy that size 
        // into a final output string
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
        // next attr is int 
	memcpy(&(temp.type), (char*) data + offset, INT_SIZE);
	offset += INT_SIZE;
        
	memcpy(&(temp.length), (char*) data + offset, INT_SIZE);
        offset += INT_SIZE;

	attrs.push_back(temp);
    }

    s.close();
    if(rbfm->closeFile(f2) != 0){
	return -1;
    }
    return 0;
}

RC RelationManager::insertTuple(const string &tableName, const void *data, RID &rid)
{ 
    if(tableName.compare("Tables.tbl") == 0 || tableName.compare("Columns.clm") == 0){
	return -1;
    } 
  
    FileHandle f; 
    vector<Attribute> tableRecordDescriptor;
    getAttributes(tableName, tableRecordDescriptor);

    string table = tableName + ".tbl";
    if(rbfm->openFile(table, f) != 0){
	return -1;
    }

    int res = rbfm->insertRecord(f, tableRecordDescriptor, data, rid);
    rbfm->closeFile(f);
 
    return res;
}

RC RelationManager::deleteTuple(const string &tableName, const RID &rid)
{
    if(tableName.compare("Tables.tbl") == 0 || tableName.compare("Columns.clm") == 0){
	return -1;
    } 
    
    FileHandle f;
    string table = tableName + ".tbl";

    vector<Attribute> tableRecordDescriptor;
    getAttributes(tableName, tableRecordDescriptor);

    if(rbfm->openFile(table, f) != 0){
	return -1;
    }

    int res = rbfm->deleteRecord(f,tableRecordDescriptor, rid);
    rbfm->closeFile(f);
 
    return -1;
}

RC RelationManager::updateTuple(const string &tableName, const void *data, const RID &rid)
{
    if(tableName.compare("Tables.tbl") == 0 || tableName.compare("Columns.clm") == 0){
	return -1;
    }
    FileHandle f; 
    string table = tableName + ".tbl";
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
    FileHandle f;
    string table = tableName + ".tbl";
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
    if(rbfm->printRecord(attrs, data) != 0) return -1;
    return 0;
}

RC RelationManager::readAttribute(const string &tableName, const RID &rid, const string &attributeName, void *data)
{
    FileHandle f;
    string table = tableName + "tbl";
    if(rbfm->openFile(table, f) != 0){
	return -1;
    }
    vector<Attribute> tablerecordDescriptor;
    getAttributes(tableName, tablerecordDescriptor);

    int res = rbfm->readAttribute(f, tablerecordDescriptor, rid, attributeName, data);
    rbfm->closeFile(f);

    return 0;
}

RC RelationManager::scan(const string &tableName,
      const string &conditionAttribute,
      const CompOp compOp,                  
      const void *value,                    
      const vector<string> &attributeNames,
      RM_ScanIterator &rm_ScanIterator)
{
    FileHandle f;
    string table = tableName + ".tbl";
    if(rbfm->openFile(table, f)){
	return -1;
    }

    vector<Attribute> tablerecordDescriptor;
    getAttributes(tableName, tablerecordDescriptor);

    RBFM_ScanIterator s;
    rbfm->scan(f, tablerecordDescriptor, conditionAttribute, compOp, value, attributeNames, s); 

    if(rbfm->closeFile(f) != 0){ 
 	return -1;
    }

    return 0;
}



