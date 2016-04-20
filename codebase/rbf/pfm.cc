#include "pfm.h"
#include <fstream>

PagedFileManager* PagedFileManager::_pf_manager = 0;

PagedFileManager* PagedFileManager::instance()
{
    if(!_pf_manager)
        _pf_manager = new PagedFileManager();

    return _pf_manager;
}


PagedFileManager::PagedFileManager()
{
}


PagedFileManager::~PagedFileManager()
{
}


RC PagedFileManager::createFile(const string &fileName)
{
   if (ifstream(fileName)){
	return -1;
   }
   ofstream file(fileName);
   return 0;
}


RC PagedFileManager::destroyFile(const string &fileName)
{
    const char* f = fileName.c_str();
    if(remove(f) != 0){
	 perror("Error deleting file");
	 return -1;
    }
    puts("File successfully deleted");
    return 0;
}


RC PagedFileManager::openFile(const string &fileName, FileHandle &fileHandle)
{
    const char* f = fileName.c_str();
    FILE * file = fopen (f, "r");
    if (file == NULL) {
       return -1;
    }
    fileHandle.setFile(file);
    return 0;
}


RC PagedFileManager::closeFile(FileHandle &fileHandle)
{
    FILE * f = fileHandle.getFile();
    if(fclose(f) != 0) { 
   	 return -1;
    }
    return 0;
}


FileHandle::FileHandle()
{
	readPageCounter = 0;
	writePageCounter = 0;
	appendPageCounter = 0;
        file = NULL;
}


FileHandle::~FileHandle()
{
}


FILE * FileHandle::getFile(){
	return file;
}
  
RC FileHandle::setFile(FILE *f){
	file = f;
	if(file == NULL){
		return -1;
	}
	return 0;
}

RC FileHandle::readPage(PageNum pageNum, void *data)
{
    readPageCounter = readPageCounter + 1;
    return 0;
}


RC FileHandle::writePage(PageNum pageNum, const void *data)
{
    writePageCounter = writePageCounter + 1;
    return 0;
}


RC FileHandle::appendPage(const void *data)
{
    appendPageCounter = appendPageCounter + 1;
    return 0;
}


unsigned FileHandle::getNumberOfPages()
{
    return appendPageCounter;
}


RC FileHandle::collectCounterValues(unsigned &readPageCount, unsigned &writePageCount, unsigned &appendPageCount)
{
	return -1;
}
