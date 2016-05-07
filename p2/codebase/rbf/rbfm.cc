#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <string>
#include <algorithm>
#include "rbfm.h"

RecordBasedFileManager* RecordBasedFileManager::_rbf_manager = NULL;
PagedFileManager *RecordBasedFileManager::_pf_manager = NULL;

RecordBasedFileManager* RecordBasedFileManager::instance()
{
    if(!_rbf_manager)
        _rbf_manager = new RecordBasedFileManager();

    return _rbf_manager;
}

RecordBasedFileManager::RecordBasedFileManager()
{
    // Initialize the internal PagedFileManager instance
    _pf_manager = PagedFileManager::instance();
}

RecordBasedFileManager::~RecordBasedFileManager()
{
}

RC RecordBasedFileManager::createFile(const string &fileName) 
{
    // Creating a new paged file.
    if (_pf_manager->createFile(fileName))
        return RBFM_CREATE_FAILED;

    // Setting up the first page.
    void * firstPageData = calloc(PAGE_SIZE, 1);
    if (firstPageData == NULL)
        return RBFM_MALLOC_FAILED;
    newRecordBasedPage(firstPageData);

    // Adds the first record based page.
    FileHandle handle;
    if (_pf_manager->openFile(fileName.c_str(), handle))
        return RBFM_OPEN_FAILED;
    if (handle.appendPage(firstPageData))
        return RBFM_APPEND_FAILED;
    _pf_manager->closeFile(handle);

    free(firstPageData);

    return SUCCESS;
}

RC RecordBasedFileManager::destroyFile(const string &fileName) 
{
    return _pf_manager->destroyFile(fileName);
}

RC RecordBasedFileManager::openFile(const string &fileName, FileHandle &fileHandle) 
{
    return _pf_manager->openFile(fileName.c_str(), fileHandle);
}

RC RecordBasedFileManager::closeFile(FileHandle &fileHandle) 
{
    return _pf_manager->closeFile(fileHandle);
}

RC RecordBasedFileManager::insertRecord(FileHandle &fileHandle, const vector<Attribute> &recordDescriptor, const void *data, RID &rid) 
{
    // Gets the size of the record.
    unsigned recordSize = getRecordSize(recordDescriptor, data);

    // Cycles through pages looking for enough free space for the new entry.
    void *pageData = malloc(PAGE_SIZE);
    if (pageData == NULL)
        return RBFM_MALLOC_FAILED;
    bool pageFound = false;
    unsigned i;
    unsigned numPages = fileHandle.getNumberOfPages();
    for (i = 0; i < numPages; i++)
    {
        if (fileHandle.readPage(i, pageData))
            return RBFM_READ_FAILED;

        // When we find a page with enough space (accounting also for the size that will be added to the slot directory), we stop the loop.
        if (getPageFreeSpaceSize(pageData) >= sizeof(SlotDirectoryRecordEntry) + recordSize)
        {
            pageFound = true;
            break;
        }
    }

    // If we can't find a page with enough space, we create a new one
    if(!pageFound)
    {
        newRecordBasedPage(pageData);
    }

    SlotDirectoryHeader slotHeader = getSlotDirectoryHeader(pageData);

    // Setting the return RID.
    rid.pageNum = i;
    rid.slotNum = slotHeader.recordEntriesNumber;

    // Adding the new record reference in the slot directory.
    SlotDirectoryRecordEntry newRecordEntry;
    newRecordEntry.length = recordSize;
    newRecordEntry.offset = slotHeader.freeSpaceOffset - recordSize;
    setSlotDirectoryRecordEntry(pageData, rid.slotNum, newRecordEntry);

    // Updating the slot directory header.
    slotHeader.freeSpaceOffset = newRecordEntry.offset;
    slotHeader.recordEntriesNumber += 1;
    setSlotDirectoryHeader(pageData, slotHeader);

    // Adding the record data.
    setRecordAtOffset (pageData, newRecordEntry.offset, recordDescriptor, data);

    // Writing the page to disk.
    if (pageFound)
    {
        if (fileHandle.writePage(i, pageData))
            return RBFM_WRITE_FAILED;
    }
    else
    {
        if (fileHandle.appendPage(pageData))
            return RBFM_APPEND_FAILED;
    }

    free(pageData);
    return SUCCESS;
}

RC RecordBasedFileManager::readRecord(FileHandle &fileHandle, const vector<Attribute> &recordDescriptor, const RID &rid, void *data) 
{
    // Retrieve the specific page
    void * pageData = malloc(PAGE_SIZE);
    if (fileHandle.readPage(rid.pageNum, pageData))
        return RBFM_READ_FAILED;

    // Checks if the specific slot id exists in the page
    SlotDirectoryHeader slotHeader = getSlotDirectoryHeader(pageData);
    if(slotHeader.recordEntriesNumber < rid.slotNum)
        return RBFM_SLOT_DN_EXIST;

    // Gets the slot directory record entry data
    SlotDirectoryRecordEntry recordEntry = getSlotDirectoryRecordEntry(pageData, rid.slotNum);

    // Retrieve the actual entry data
    getRecordAtOffset(pageData, recordEntry.offset, recordDescriptor, data);

    free(pageData);
    return SUCCESS;
}

RC RecordBasedFileManager::printRecord(const vector<Attribute> &recordDescriptor, const void *data) 
{
    // Parse the null indicator into an array
    int nullIndicatorSize = getNullIndicatorSize(recordDescriptor.size());
    char nullIndicator[nullIndicatorSize];
    memset(nullIndicator, 0, nullIndicatorSize);
    memcpy(nullIndicator, data, nullIndicatorSize);
    
    // We've read in the null indicator, so we can skip past it now
    unsigned offset = nullIndicatorSize;

    cout << "----" << endl;
    for (unsigned i = 0; i < (unsigned) recordDescriptor.size(); i++)
    {
        cout << setw(10) << left << recordDescriptor[i].name << ": ";
        // If the field is null, don't print it
        bool isNull = fieldIsNull(nullIndicator, i);
        if (isNull)
        {
            cout << "NULL" << endl;
            continue;
        }
        switch (recordDescriptor[i].type)
        {
            case TypeInt:
                uint32_t data_integer;
                memcpy(&data_integer, ((char*) data + offset), INT_SIZE);
                offset += INT_SIZE;

                cout << "" << data_integer << endl;
            break;
            case TypeReal:
                float data_real;
                memcpy(&data_real, ((char*) data + offset), REAL_SIZE);
                offset += REAL_SIZE;

                cout << "" << data_real << endl;
            break;
            case TypeVarChar:
                // First VARCHAR_LENGTH_SIZE bytes describe the varchar length
                uint32_t varcharSize;
                memcpy(&varcharSize, ((char*) data + offset), VARCHAR_LENGTH_SIZE);
                offset += VARCHAR_LENGTH_SIZE;

                // Gets the actual string.
                char *data_string = (char*) malloc(varcharSize + 1);
                if (data_string == NULL)
                    return RBFM_MALLOC_FAILED;
                memcpy(data_string, ((char*) data + offset), varcharSize);

                // Adds the string terminator.
                data_string[varcharSize] = '\0';
                offset += varcharSize;

                cout << data_string << endl;
                free(data_string);
            break;
        }
    }
    cout << "----" << endl;

    return SUCCESS;
}

// Private helper methods

// Configures a new record based page, and puts it in "page".
void RecordBasedFileManager::newRecordBasedPage(void * page)
{
    memset(page, 0, PAGE_SIZE);
    // Writes the slot directory header.
    SlotDirectoryHeader slotHeader;
    slotHeader.freeSpaceOffset = PAGE_SIZE;
    slotHeader.recordEntriesNumber = 0;
    setSlotDirectoryHeader(page, slotHeader);
}

SlotDirectoryHeader RecordBasedFileManager::getSlotDirectoryHeader(void * page)
{
    // Getting the slot directory header.
    SlotDirectoryHeader slotHeader;
    memcpy (&slotHeader, page, sizeof(SlotDirectoryHeader));
    return slotHeader;
}

void RecordBasedFileManager::setSlotDirectoryHeader(void * page, SlotDirectoryHeader slotHeader)
{
    // Setting the slot directory header.
    memcpy (page, &slotHeader, sizeof(SlotDirectoryHeader));
}

SlotDirectoryRecordEntry RecordBasedFileManager::getSlotDirectoryRecordEntry(void * page, unsigned recordEntryNumber)
{
    // Getting the slot directory entry data.
    SlotDirectoryRecordEntry recordEntry;
    memcpy  (
            &recordEntry,
            ((char*) page + sizeof(SlotDirectoryHeader) + recordEntryNumber * sizeof(SlotDirectoryRecordEntry)),
            sizeof(SlotDirectoryRecordEntry)
            );

    return recordEntry;
}

void RecordBasedFileManager::setSlotDirectoryRecordEntry(void * page, unsigned recordEntryNumber, SlotDirectoryRecordEntry recordEntry)
{
    // Setting the slot directory entry data.
    memcpy  (
            ((char*) page + sizeof(SlotDirectoryHeader) + recordEntryNumber * sizeof(SlotDirectoryRecordEntry)),
            &recordEntry,
            sizeof(SlotDirectoryRecordEntry)
            );
}

// Computes the free space of a page (function of the free space pointer and the slot directory size).
unsigned RecordBasedFileManager::getPageFreeSpaceSize(void * page) 
{
    SlotDirectoryHeader slotHeader = getSlotDirectoryHeader(page);
    return slotHeader.freeSpaceOffset - slotHeader.recordEntriesNumber * sizeof(SlotDirectoryRecordEntry) - sizeof(SlotDirectoryHeader);
}

unsigned RecordBasedFileManager::getRecordSize(const vector<Attribute> &recordDescriptor, const void *data) 
{
    // Read in the null indicator
    int nullIndicatorSize = getNullIndicatorSize(recordDescriptor.size());
    char nullIndicator[nullIndicatorSize];
    memset(nullIndicator, 0, nullIndicatorSize);
    memcpy(nullIndicator, (char*) data, nullIndicatorSize);

    // Offset into *data. Start just after null indicator
    unsigned offset = nullIndicatorSize;
    // Running count of size. Initialize to size of header
    unsigned size = sizeof (RecordLength) + (recordDescriptor.size()) * sizeof(ColumnOffset) + nullIndicatorSize;

    for (unsigned i = 0; i < (unsigned) recordDescriptor.size(); i++)
    {
        // Skip null fields
        if (fieldIsNull(nullIndicator, i))
            continue;
        switch (recordDescriptor[i].type)
        {
            case TypeInt:
                size += INT_SIZE;
                offset += INT_SIZE;
            break;
            case TypeReal:
                size += REAL_SIZE;
                offset += REAL_SIZE;
            break;
            case TypeVarChar:
                uint32_t varcharSize;
                // We have to get the size of the VarChar field by reading the integer that precedes the string value itself
                memcpy(&varcharSize, (char*) data + offset, VARCHAR_LENGTH_SIZE);
                size += varcharSize;
                offset += varcharSize + VARCHAR_LENGTH_SIZE;
            break;
        }
    }

    return size;
}

// Calculate actual bytes for nulls-indicator for the given field counts
int RecordBasedFileManager::getNullIndicatorSize(int fieldCount) 
{
    return int(ceil((double) fieldCount / CHAR_BIT));
}

bool RecordBasedFileManager::fieldIsNull(char *nullIndicator, int i)
{
    int indicatorIndex = i / CHAR_BIT;
    int indicatorMask  = 1 << (CHAR_BIT - 1 - (i % CHAR_BIT));
    return (nullIndicator[indicatorIndex] & indicatorMask) != 0;
}

void RecordBasedFileManager::setRecordAtOffset(void *page, unsigned offset, const vector<Attribute> &recordDescriptor, const void *data)
{
    // Read in the null indicator
    int nullIndicatorSize = getNullIndicatorSize(recordDescriptor.size());
    char nullIndicator[nullIndicatorSize];
    memset (nullIndicator, 0, nullIndicatorSize);
    memcpy(nullIndicator, (char*) data, nullIndicatorSize);

    // Points to start of record
    char *start = (char*) page + offset;

    // Offset into *data
    unsigned data_offset = nullIndicatorSize;
    // Offset into page header
    unsigned header_offset = 0;

    RecordLength len = recordDescriptor.size();
    memcpy(start + header_offset, &len, sizeof(len));
    header_offset += sizeof(len);

    memcpy(start + header_offset, nullIndicator, nullIndicatorSize);
    header_offset += nullIndicatorSize;

    // Keeps track of the offset of each record
    // Offset is relative to the start of the record and points to the END of a field
    ColumnOffset rec_offset = header_offset + (recordDescriptor.size()) * sizeof(ColumnOffset);

    unsigned i = 0;
    for (i = 0; i < recordDescriptor.size(); i++)
    {
        if (!fieldIsNull(nullIndicator, i))
        {
            // Points to current position in *data
            char *data_start = (char*) data + data_offset;

            // Read in the data for the next column, point rec_offset to end of newly inserted data
            switch (recordDescriptor[i].type)
            {
                case TypeInt:
                    memcpy (start + rec_offset, data_start, INT_SIZE);
                    rec_offset += INT_SIZE;
                    data_offset += INT_SIZE;
                break;
                case TypeReal:
                    memcpy (start + rec_offset, data_start, REAL_SIZE);
                    rec_offset += REAL_SIZE;
                    data_offset += REAL_SIZE;
                break;
                case TypeVarChar:
                    unsigned varcharSize;
                    // We have to get the size of the VarChar field by reading the integer that precedes the string value itself
                    memcpy(&varcharSize, data_start, VARCHAR_LENGTH_SIZE);
                    memcpy(start + rec_offset, data_start + VARCHAR_LENGTH_SIZE, varcharSize);
                    // We also have to account for the overhead given by that integer.
                    rec_offset += varcharSize;
                    data_offset += VARCHAR_LENGTH_SIZE + varcharSize;
                break;
            }
        }
        // Copy offset into record header
        // Offset is relative to the start of the record and points to END of field
        memcpy(start + header_offset, &rec_offset, sizeof(ColumnOffset));
        header_offset += sizeof(ColumnOffset);
    }
}

// Support header size and null indicator. If size is less than recordDescriptor size, then trailing records are null
// Memset null indicator as 1?
void RecordBasedFileManager::getRecordAtOffset(void *page, unsigned offset, const vector<Attribute> &recordDescriptor, void *data)
{
    // Pointer to start of record
    char *start = (char*) page + offset;

    // Allocate space for null indicator. The returned null indicator may be larger than
    // the null indicator in the table has had fields added to it
    int nullIndicatorSize = getNullIndicatorSize(recordDescriptor.size());
    char nullIndicator[nullIndicatorSize];
    memset(nullIndicator, 0, nullIndicatorSize);

    // Get number of columns and size of the null indicator for this record
    RecordLength len = 0;
    memcpy (&len, start, sizeof(RecordLength));
    int recordNullIndicatorSize = getNullIndicatorSize(len);

    // Read in the existing null indicator
    memcpy (nullIndicator, start + sizeof(RecordLength), recordNullIndicatorSize);

    // If this new recordDescriptor has had fields added to it, we set all of the new fields to null
    for (unsigned i = len; i < recordDescriptor.size(); i++)
    {
        int indicatorIndex = (i+1) / CHAR_BIT;
        int indicatorMask  = 1 << (CHAR_BIT - 1 - (i % CHAR_BIT));
        nullIndicator[indicatorIndex] |= indicatorMask;
    }
    // Write out null indicator
    memcpy(data, nullIndicator, nullIndicatorSize);

    // Initialize some offsets
    // rec_offset: points to data in the record. We move this forward as we read data from our record
    unsigned rec_offset = sizeof(RecordLength) + recordNullIndicatorSize + len * sizeof(ColumnOffset);
    // data_offset: points to our current place in the output data. We move this forward as we write data to data.
    unsigned data_offset = nullIndicatorSize;
    // directory_base: points to the start of our directory of indices
    char *directory_base = start + sizeof(RecordLength) + recordNullIndicatorSize;
    
    for (unsigned i = 0; i < recordDescriptor.size(); i++)
    {
        if (fieldIsNull(nullIndicator, i))
            continue;
        
        // Grab pointer to end of this column
        ColumnOffset endPointer;
        memcpy(&endPointer, directory_base + i * sizeof(ColumnOffset), sizeof(ColumnOffset));

        // rec_offset keeps track of start of column, so end-start = total size
        uint32_t fieldSize = endPointer - rec_offset;

        // Special case for varchar, we must give data the size of varchar first
        if (recordDescriptor[i].type == TypeVarChar)
        {
            memcpy((char*) data + data_offset, &fieldSize, VARCHAR_LENGTH_SIZE);
            data_offset += VARCHAR_LENGTH_SIZE;
        }
        // Next we copy bytes equal to the size of the field and increase our offsets
        memcpy((char*) data + data_offset, start + rec_offset, fieldSize);
        rec_offset += fieldSize;
        data_offset += fieldSize;
    }
}


RC RecordBasedFileManager::deleteRecord(FileHandle &fileHandle, const vector<Attribute> &recordDescriptor, const RID &rid){
   void * page = malloc(PAGE_SIZE);
   int SoF = fileHandle.readPage(rid.pageNum, page);
   if(SoF != 0){
      return 1;
   }
   SlotDirectoryRecordEntry empty_replacement;
   setSlotDirectoryRecordEntry( page, rid.slotNum, empty_replacement);  
    
   free(page);
   return 0;
}

RC RecordBasedFileManager::updateRecord(FileHandle &fileHandle, const vector<Attribute> &recordDecriptor, const void * data , const RID &rid){
  int i = deleteRecord(fileHandle,recordDecriptor,rid);
  if(i != 0) return 1;
  RID rid2;
  insertRecord(fileHandle, recordDecriptor, data, rid2);

  if(rid2.slotNum != rid.slotNum || rid2.pageNum != rid.pageNum){

   void * page = malloc(PAGE_SIZE);
   fileHandle.readPage(rid.pageNum, page);

   SlotDirectoryRecordEntry SDRE;
   setSlotDirectoryRecordEntry(page, rid.slotNum, SDRE);
   fileHandle.writePage(rid.pageNum, page);
   free(page);
   return 0;
  }
  else  return 0;
}

RC RecordBasedFileManager::readAttribute(FileHandle &fileHandle,const vector<Attribute> &recordDescriptor, const RID &rid, const string &attributeName, void * data){
   int k = 0;
   int len = 0;
   void * page = malloc(PAGE_SIZE);
   int j = recordDescriptor.size();
   int buffer = 0;
   
   for(int i = 0;i < j;i++){
    
       if(recordDescriptor.at(i).type == TypeReal){
         if(recordDescriptor.at(i).name != attributeName){ buffer += 4; }         
           memcpy(data,((char *) page + buffer) ,4 ); 
           k++;
       } else if(recordDescriptor.at(i).type == TypeInt){
         if(recordDescriptor.at(i).name != attributeName){ buffer += 4; }
          memcpy(data,( ( char * ) page + buffer), 4);
          k++;
       } else if(recordDescriptor.at(i).type == TypeVarChar){
          memcpy(&len, ((char *) page + buffer), len + VARCHAR_LENGTH_SIZE);
          if(recordDescriptor.at(i).name != attributeName){ buffer += len + VARCHAR_LENGTH_SIZE; }
            memcpy(data, ((char *) page + buffer), len + VARCHAR_LENGTH_SIZE);
            k++;
       }
       
       if(recordDescriptor.at(i).name == attributeName){ break; }
      // cout<< k << endl;
   }
   free(page);
   return 0;
}

bool str_scan(CompOp co,const void * value, char * str);
bool int_scan(CompOp co,const  void * value, int number);
bool float_scan(CompOp co,const void* value, float fl);


RC RecordBasedFileManager::scan(FileHandle &fileHandle, const vector<Attribute> &recordDescriptor ,const string &conditionAttribute,const CompOp compOp, const void * value, const vector<string> &attributeNames, RBFM_ScanIterator &rbfm_ScanIterator){
  void * page;
  void * OP;
  unsigned len;
  void * page2 = malloc(PAGE_SIZE);
  vector<RID> rid_vec;
  vector<int> int_vec;
  vector<void*> mem_vec;
  SlotDirectoryHeader H;
  SlotDirectoryRecordEntry RE;
  float floater;
  int buffer1 = 0;
  int buffer2 = 0;
  bool qualifies = false;
  bool doesnt_qualify = false;
  char * str_block;
  int int_block;
  float float_block;

  for(int i = 0;i < fileHandle.getNumberOfPages();i++){
     H = getSlotDirectoryHeader(page);
     for(int k = 0;k < H.recordEntriesNumber;k++){

       RE = getSlotDirectoryRecordEntry(page,k);

       if(RE.status == 1){
          floater++;
          page = malloc(RE.length);
          buffer1 = 0;
          memcpy((char *) page, ((char *) page2 + RE.offset), RE.length);
          OP = malloc(RE.length);
          buffer2 = 0;
          for(int j = 0;j < recordDescriptor.size();j++){

           if(doesnt_qualify){break;}
             qualifies = (find(attributeNames.begin(), attributeNames.end(),recordDescriptor.at(j).name)!= attributeNames.end()) ;
///// swap  
             bool got_it = recordDescriptor.at(j).name.compare(conditionAttribute) == 0;

             if(recordDescriptor.at(k).type == TypeInt){

              memcpy(&int_block, ((char *) page + buffer1), 4 );
              doesnt_qualify = got_it && !int_scan(compOp,value,int_block);
              if(qualifies ==  true){
                 memcpy(((char *) OP + buffer2),((char *) page + buffer1), 4 );
                 buffer2 += 4;
              }
              buffer1 += 4;
              
              
              }else if(recordDescriptor.at(k).type == TypeVarChar){
             	
                memcpy(&len, ((char *) page + buffer1), 4);
                str_block = (char *) malloc(len + 1);                

                memcpy(str_block, (char *) page + buffer1 + 4, len);

                str_block[len] = '\0';

                doesnt_qualify = got_it && !str_scan(compOp,value,str_block);
                if(qualifies == true){
                  memcpy(((char *) OP + buffer2), ((char *) page + buffer1),len + 4);
                  buffer2 += len + 4;
                }
                buffer1 += 4 + len;
                
             } else if(recordDescriptor.at(k).type == TypeReal){
                memcpy(&float_block,((char *)page + buffer1),4);
                doesnt_qualify = got_it && !float_scan(compOp,value,float_block);
                if(qualifies == true){
                  memcpy(((char *) OP + buffer2), ((char *) page + buffer1),4);
                  buffer2 += 4;
                }
                buffer1+= 4;
             }

            }

           if(!doesnt_qualify){
             RID rep;
             rep.slotNum = i;
             rep.pageNum = k;
             rid_vec.push_back(rep);
             mem_vec.push_back(OP);
             int_vec.push_back(buffer2);
             
           }else {

              free(OP);

            }

         free(page);


       }

     }


  }
  
  rbfm_ScanIterator.mem_vec = mem_vec;
  rbfm_ScanIterator.int_vec = int_vec;
  rbfm_ScanIterator.rid_vec = rid_vec;

  free(page2);


  return 0;
}

bool str_scan(CompOp co,const void * value, char * str){
	
	if(co == EQ_OP) return strcmp(str, (char*) value) == 0;

	else if(co == LT_OP) return strcmp(str, (char*) value) < 0;

	else if(co == GT_OP) return strcmp(str, (char*) value) > 0;

	else if(co == LE_OP) return strcmp(str, (char*) value) <= 0;

	else if( co ==GE_OP) return strcmp(str, (char*) value) >= 0;

	else if( co ==NE_OP) return strcmp(str, (char*) value) != 0;

	else{} return true;
}

bool int_scan(CompOp co,const  void * value, int number){

	int output;
	memcpy (&output, value, 4);
	
	if(co == EQ_OP) return number == output;
		
	else if(co ==LT_OP) return number < output;
		
	else if(co ==GT_OP) return number > output;
		
	else if(co ==LE_OP) return number <= output;
		
	else if(co ==GE_OP) return number >= output;
		
	else if(co ==NE_OP) return number != output;
		
	else if(co ==NO_OP){}
         return true;

    
}

bool float_scan(CompOp co,const void* value, float fl){
    float output;
	memcpy (&output, value, 4);

	if(co == EQ_OP) return fl == output;
		
	else if(co ==LT_OP) return fl < output;
		
	else if(co ==GT_OP) return fl > output;
		
	else if(co ==LE_OP) return fl <= output;
		
	else if(co ==GE_OP) return fl >= output;
		
	else if(co ==NE_OP) return fl != output;
		
	else if(co ==NO_OP){}  return true;
    
}







