#include "ix.h"

IndexManager* IndexManager::_index_manager = 0;
PagedFileManager* IndexManager::_p = 0;

IndexManager* IndexManager::instance()
{
    if(!_index_manager)
        _index_manager = new IndexManager();

    return _index_manager;
}

IndexManager::IndexManager()
{
    _p = PagedFileManager::instance();
}

IndexManager::~IndexManager()
{
}

RC IndexManager::createFile(const string &fileName)
{
    if(_p->createFile(fileName.c_str()) != 0){
	return -1;
    }

    IXFileHandle ixFHandle;
    if(_p->openFile(fileName.c_str(), ixFHandle.f) != 0){
	return -1;
    }

    // Append a first page as meta data 
    void *page = malloc(PAGE_SIZE);
    ixFHandle.appendPage(page);
    int i = 0;
    memcpy(page, &i, INT_SIZE);

    // set up root page
    void *rootPage = malloc(PAGE_SIZE);
    ixFHandle.appendPage(rootPage);
    int j = 1;
    memcpy(rootPage, &j, INT_SIZE);

    // set up first leaf page
    void *leafPage = malloc(PAGE_SIZE);
    ixFHandle.appendPage(leafPage);
    int k = 2;
    memcpy(leafPage, &k, INT_SIZE);

    return 0;
}

RC IndexManager::destroyFile(const string &fileName)
{
    if(_p->destroyFile(fileName) != 0){
	return -1;
    } 

    return 0;
}

RC IndexManager::openFile(const string &fileName, IXFileHandle &ixfileHandle)
{
    if(_p->openFile(fileName, ixfileHandle.f) != 0){
	return -1;
    }

    return 0;
}

RC IndexManager::closeFile(IXFileHandle &ixfileHandle)
{
    if(_p->closeFile(ixfileHandle.f) != 0){
	return -1;
    }

    return 0;
}

RC IndexManager::insertEntry(IXFileHandle &ixfileHandle, const Attribute &attribute, const void *key, const RID &rid)
{
    return -1;
}

RC IndexManager::deleteEntry(IXFileHandle &ixfileHandle, const Attribute &attribute, const void *key, const RID &rid)
{
    return -1;
}


RC IndexManager::scan(IXFileHandle &ixfileHandle,
        const Attribute &attribute,
        const void      *lowKey,
        const void      *highKey,
        bool			lowKeyInclusive,
        bool        	highKeyInclusive,
        IX_ScanIterator &ix_ScanIterator)
{
    return -1;
}

void IndexManager::printBtree(IXFileHandle &ixfileHandle, const Attribute &attribute) const {
}

IX_ScanIterator::IX_ScanIterator()
{
}

IX_ScanIterator::~IX_ScanIterator()
{
}

RC IX_ScanIterator::getNextEntry(RID &rid, void *key)
{
    return -1;
}

RC IX_ScanIterator::close()
{
    return -1;
}


IXFileHandle::IXFileHandle()
{
    ixReadPageCounter = 0;
    ixWritePageCounter = 0;
    ixAppendPageCounter = 0;
}

IXFileHandle::~IXFileHandle()
{
}

RC IXFileHandle::readPage(PageNum pageNum, void *data)
{
    if(f.readPage(pageNum, data) != 0){
	return -1;
    }

    ixReadPageCounter++;
    return SUCCESS;
}


RC IXFileHandle::writePage(PageNum pageNum, const void *data)
{
    if(f.writePage(pageNum, data) != 0){
	return -1;
    }

    ixWritePageCounter++; 
    return SUCCESS;
}


RC IXFileHandle::appendPage(const void *data)
{
   if(f.appendPage(data) != 0){
	return -1;
   }

   ixAppendPageCounter++;
   return SUCCESS;    
}

RC IXFileHandle::collectCounterValues(unsigned &readPageCount, unsigned &writePageCount, unsigned &appendPageCount)
{
    readPageCount = ixReadPageCounter;
    writePageCount = ixWritePageCounter;
    appendPageCount = ixAppendPageCounter;
    return SUCCESS;
}
