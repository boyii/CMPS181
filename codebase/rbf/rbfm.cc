#include "rbfm.h"
#include <iostream>
#include <fstream>

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

RC RecordBasedFileManager::insertRecord(FileHandle &fileHandle, const vector<Attribute> &recordDescriptor, const void *data, RID &rid) {
    return -1;
}

RC RecordBasedFileManager::readRecord(FileHandle &fileHandle, const vector<Attribute> &recordDescriptor, const RID &rid, void *data) {
    return -1;
}

RC RecordBasedFileManager::printRecord(const vector<Attribute> &recordDescriptor, const void *data) {
    return -1;
}
