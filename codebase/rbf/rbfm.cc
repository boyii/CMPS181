#include "rbfm.h"
#include <iostream>
#include <fstream>
#include <math.h>
#include <stdlib.h>
#include <cstring>
#include <string>

RecordBasedFileManager* RecordBasedFileManager::_rbf_manager = 0;

RecordBasedFileManager* RecordBasedFileManager::instance()
{
    if(!_rbf_manager)
        _rbf_manager = new RecordBasedFileManager();

    return _rbf_manager;
}

RecordBasedFileManager::RecordBasedFileManager()
{
}

RecordBasedFileManager::~RecordBasedFileManager()
{
}

RC RecordBasedFileManager::createFile(const string &fileName) {
   	PagedFileManager PFM = PagedFileManager();

	int i = PFM.createFile(fileName);
	if(i == 0){
		return 0;
	}

	return -1;
}

RC RecordBasedFileManager::destroyFile(const string &fileName) {
	PagedFileManager PFM = PagedFileManager();
	int i = PFM.destroyFile(fileName);
	if (i == 0) {
		return 0;
	}
	return -1;
}

RC RecordBasedFileManager::openFile(const string &fileName, FileHandle &fileHandle) {
	PagedFileManager PFM = PagedFileManager();
	ifstream infile(fileName);
	if (infile.good()) {
		int i = PFM.openFile(fileName, fileHandle);
		if (i == 0) return 0;
	}

	return -1;
	
}

RC RecordBasedFileManager::closeFile(FileHandle &fileHandle) {
	PagedFileManager PFM = PagedFileManager();
	int i = PFM.closeFile(fileHandle);
	return i;
}

int RecordBasedFileManager::get_space_we_need(const vector<Attribute> &recordDescriptor, const void *data) {

	unsigned vc = 0;

	for (auto i = 0; i < recordDescriptor.size(); i++)
		if (recordDescriptor[i].type == 0)
		{
			PI.space_we_need += 4;
		}
		else if (recordDescriptor[i].type == 1) {
			PI.space_we_need += 4;
		}
		else if (recordDescriptor[i].type == 2) {
			memcpy(&vc, (char*)data + PI.space_we_need, 4);
			
			PI.space_we_need += 4;
			PI.space_we_need += vc;
		}
				
	return PI.space_we_need;
}

bool RecordBasedFileManager::endOfPage() {
	if (PAGE_SIZE - PI.where_to_start_writing - PI.space_we_need > 0) {
		return false;
	}
	return true;
}

RC RecordBasedFileManager::insertRecord(FileHandle &fileHandle, const vector<Attribute> &recordDescriptor, const void *data, RID &rid) {
	PagedFileManager PFM = PagedFileManager();
	FILE * F = fileHandle.getFile();
	void * page_space = malloc(PAGE_SIZE);

	PI.space_we_need = get_space_we_need(recordDescriptor,data);

	if (PI.pages == 0) {
		rid.pageNum = 1;
		rid.slotNum = 1;
		PI.pages++;
		PI.slot = 1;
		PI.where_to_start_writing += PI.space_we_need;

		memcpy(((char*) page_space + PI.where_to_start_writing), data, PI.space_we_need);
		fileHandle.writePage(1, page_space);
	}
	else {
		if (endOfPage())
		{
			PI.pages++;
			PI.slot = 1;
			PI.where_to_start_writing += PI.space_we_need;

			memcpy(((char*)page_space + PI.where_to_start_writing), data, PI.space_we_need);
			fileHandle.appendPage(page_space);
			// create new page // 
		}
		else {
			rid.pageNum = PI.pages;
			rid.slotNum = PI.slot++;
			memcpy(((char*)page_space + PI.where_to_start_writing), data, PI.space_we_need);
			fileHandle.writePage(PI.pages, page_space);

		}
	}
	
	

	return -1;
}


RC RecordBasedFileManager::readRecord(FileHandle &fileHandle, const vector<Attribute> &recordDescriptor, const RID &rid, void *data) {
    return -1;
}

RC RecordBasedFileManager::printRecord(const vector<Attribute> &recordDescriptor, const void *data) {

        int field_num = ceil(recordDescriptor.size()/8);
        int buffer = 0;
        int int_type;
        int string_type;
        float float_type;
        int int_bytes = 4;
        char * final_op;

        for (auto j = 0;j < recordDescriptor.size();++j) {

                if (recordDescriptor.at(j).type == 0) {
                        memcpy(&int_type,((char *) data + buffer), 4); // copy int from data to int_type with byte size 4
                        buffer += 4; //make buffer room


                        cout << recordDescriptor[j].name << ": " << int_type << endl;
                        // int

                }
                else if (recordDescriptor.at(j).type == 1) {
                        memcpy(&float_type,((char *) data + buffer ),4);
                        buffer = buffer + 4;
                        cout << recordDescriptor[j].name << ": " << float_type << endl;

                        // real
                }
                else if (recordDescriptor.at(j).type == 2) {
                        memcpy(&string_type,((char *) data + buffer),4);
                        buffer = buffer + 4;
                        final_op = (char *) malloc(string_type + 1);
                        memcpy((void*) final_op, ((char *)data + buffer), string_type);
                        final_op[string_type] = '\0';
                        buffer = buffer + string_type;

                        cout << recordDescriptor[j].name << ": " << final_op << endl;
                        free(final_op);
                        // varchar
                }
        }
        return 0;

}
