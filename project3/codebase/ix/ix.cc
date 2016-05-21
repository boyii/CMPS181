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
//    if(_p->createFile(fileName.c_str()) != 0){
//	return -1;
//    }
//
//    IXFileHandle ixFHandle;
//    if(_p->openFile(fileName.c_str(), ixFHandle.f) != 0){
//	return -1;
//    }
//
//    // Append a first page as meta data 
//    void *page = malloc(PAGE_SIZE);
//    int i = 0;
//    memcpy(page, &i, INT_SIZE);
//    ixFHandle.appendPage(page);
//
//
//    // set up root page
//    void *rootPage = malloc(PAGE_SIZE);
//    int j = 1;
//    memcpy(rootPage, &j, INT_SIZE);
//    j = 0;
//    // add leaf boolean
//    memcpy((char *) rootPage + INT_SIZE, &j,INT_SIZE);
//    // add number of children
//    j = 0;
//    memcpy((char *) rootPage + 8, &j,INT_SIZE);
//    ixFHandle.appendPage(rootPage);
//
//    // set up first leaf page
//    void *leafPage = malloc(PAGE_SIZE);
//    int k = 2;
//    memcpy(leafPage, &k, INT_SIZE);
//    ixFHandle.appendPage(leafPage);

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
//    // first we deal with the case where there is only 
//    void * meta = malloc(PAGE_SIZE);
//    ixfileHandle.readPage(0,meta);
//
//    int * pageNumbers = (int *) malloc(INT_SIZE);
//    memcpy(pageNumbers, (char *) meta + 8, INT_SIZE); //now we know how mny pagess we have
//
//    int * root_number = (int *) malloc();
//    memcpy(root_number, (char * ) meta + 4, INT_SIZE);// get out root page number
//
//    if(pageNumbers < 1){ // means we havent filled in the key for the root node
//        void * rootPage = malloc(PAGE_SIZE);
//        ixfileHandle.readPage(root_number, rootPage);
//        int t = 0;
//        switch(attribute.type){
//            case TypeInt:
//                t = 1;
//                break;
//            case TypeReal:
//                t = 2;
//                break;
//            case TypeVarChar:
//                t = 3;
//                break;
//             default:
//              t = 0;
//                 break;
//        }
//
//        memcpy((char *) rootPage + 24, &t,INT_SIZE); // now we know what type of attr we have
//
//        // we do something with the rid, i forgot what
//        //
//
//        memcpy((char *) rootPage + 32, key, sizeof(key)); // now we copy over the data 
//        int update = *pageNumbers;
//        update++;
//        memcpy((char *) meta + 8, &update,INT_SIZE);    // increment the page total on meta page
//
//        ixfileHandle.writePage(*root_number, rootPage); // write the updated page to memory
//        ixfileHandle.writePage(0,meta); // wrte the updated meta page to memory
//
//        free(rootPage);
//        
//        return 0;
//    }
//
	

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
    bool LowUnlimited = (lowKey == null);    
    bool HighUnlimited = (highKey == null);  

    void * rootPage = malloc(PAGE_SIZE);
    unsigned rootId;   

    if(ixfileHandle.readPage(0, rootPage) != 0){
	return -1;
    }
    memcpy(&rootId, rootPage, INT_SIZE);
	
    free(rootPage);
   
    return 0;
}

RC IndexManager::treeVersal(const Attribute attr, IXFileHandle &ixFHandle, unsigned pageId, unsigned &nextPageId){

    void *pageData = malloc(PAGE_SIZE);

    if(ixFileHandle.readpage(pageId, pageData) != 0){
	return -1;
    }

    PageType type;
    memcpy(&type, pageData, INT_SIZE);
    if(type == 1){
	returnPageId = pageId;
	free(pageData);
	return 0;	
    }

    return treeVersal(attr, ixFHandle, pageId, nextPageId);
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
    return _rbfm_ScanIterator->getNextRecord(rid, key);
}

RC IX_ScanIterator::close()
{
    return _rbfm_ScanIterator->close();
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
