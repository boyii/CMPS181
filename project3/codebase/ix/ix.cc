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
    int i = 0;
    memcpy(page, &i, INT_SIZE);
    ixFHandle.appendPage(page);


    // set up root page
    void *rootPage = malloc(PAGE_SIZE);
    int j = 1;
    memcpy(rootPage, &j, INT_SIZE);
    j = 0;
    // add leaf boolean
    memcpy((char *) rootPage + INT_SIZE, &j,INT_SIZE);
    // add number of children
    j = 0;
    memcpy((char *) rootPage + 8, &j,INT_SIZE);
    ixFHandle.appendPage(rootPage);

    // set up first leaf page
    void *leafPage = malloc(PAGE_SIZE);
    int k = 2;
    memcpy(leafPage, &k, INT_SIZE);
    ixFHandle.appendPage(leafPage);

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

int MinInd(vector<int> arr){
    int index = 0;
    float min = (float) arr[0];
    for(unsigned i = 0;i < arr.size();i++){
        if(arr[i] < min){ min = arr[i]; index = i; }
        else continue;
    }
    return index;

}

int MinInd2(vector<float> arr){
    int index = 0;
    float min = (float) arr[0];
    for(unsigned i = 0;i < arr.size();i++){
        if(arr[i] < min){ min = arr[i]; index = i; }
        else continue;
    }
    return index;

}



int MinInd3(vector<char *> arr){
    int index = 0;
    char * min = arr[0];
    for(unsigned i = 0;i < arr.size();i++){
        if(strcmp(min, arr[i]) > 0){ min = arr[i]; index = i; }
        else continue;
    }
    return index;


}


void IndexManager::sortPage1(IXFileHandle &ixfileHandle, void * page){
    int * pgN = (int * ) malloc(4);
    int * entries = (int *) malloc(4);
    memcpy(entries,(char *) page + 8,4);
    memcpy(pgN, page,4);
    int buff = 32;

    vector<int> arr;
    vector<void *> mem;

    for(int i = 1;i <= *entries;i++){
        int * indexx = ( int *) malloc(4);
        void * block = malloc(20);
        memcpy(block,(char *) page + buff, 20);
        memcpy(indexx,(char *) page + buff,4);
        arr.push_back(*indexx);
        mem.push_back(block);
        buff+=20;
        free(indexx);
    }

    vector<int> arr2;
    vector<void*> mem2;
    // now, a highly ineffeccient sorting alg.
    void * mem_max = mem[0];
    int place = arr.size();
    for(int j = 0;j < place;j++){
        int min = MinInd(arr);
        void * newBlock = malloc(20);
        memcpy(newBlock, mem[min], 20);
        arr2.push_back(arr[min]);
        mem2.push_back(newBlock);

        free(mem[min]);
        arr.erase(arr.begin() + min);
        mem.erase(mem.begin()+ min);

    }

    int buff2 = 32;
    // we finally write it back to the page 
    for(int k = 0;k < mem2.size();k++){
        memcpy((char *) page + buff2, mem2[k],20);
        free(mem2[k]);
        buff2+=20;
    }

}

void IndexManager::sortPage2(IXFileHandle &ixfileHandle, void * page){
    int * pgN = (int * ) malloc(4);
    int * entries = (int *) malloc(4);

    memcpy(entries,(char *) page + 8,4);
    memcpy(pgN, page,4);
    int buff = 32;

    vector<float> arr;
    vector<void *> mem;

    for(int i = 1;i <= *entries;i++){
        float * indexx = ( float *) malloc(4);
        void * block = malloc(20);
        memcpy(block,(char *) page + buff, 20);
        memcpy(indexx,(char *) page + buff,4);
        arr.push_back(*indexx);
        mem.push_back(block);
        buff+=20;
        free(indexx);
    }

    vector<float> arr2;
    vector<void*> mem2;
    // now, a highly ineffeccient sorting alg.
    void * mem_max = mem[0];
    int place = arr.size();
    for(int j = 0;j < place;j++){
        int min = MinInd2(arr);
        void * newBlock = malloc(20);
        memcpy(newBlock, mem[min], 20);
        arr2.push_back(arr[min]);
        mem2.push_back(newBlock);

        free(mem[min]);
        arr.erase(arr.begin() + min);
        mem.erase(mem.begin()+ min);

    }

    int buff2 = 32;
    // we finally write it back to the page 
    for(int k = 0;k < mem2.size();k++){
        memcpy((char *) page + buff2, mem2[k],20);
        free(mem2[k]);
        buff2+=20;
    }


}


void IndexManager::sortPage3(IXFileHandle &ixfileHandle, void * page){
    int * pgN = (int * ) malloc(4);
    int * entries = (int *) malloc(4);

    memcpy(entries,(char *) page + 8,4);
    memcpy(pgN, page,4);
    int buff = 32;

    vector<char *> arr;
    vector<void *> mem;
    unsigned length;
    memcpy(&length,(char *) page + buff,4);
    buff += length;
    for(int i = 1;i <= *entries;i++){
        char * indexx = ( char *) malloc(length+1);
        void * block = malloc(16+ length + 1);
        memcpy(block,(char *) page + buff,length+ 17 );
        memcpy(indexx,(char *) page + buff,length + 1);
        indexx[length] = '\0';
        arr.push_back(indexx);
        mem.push_back(block);
        buff+=20;
    }

    vector<char *> arr2;
    vector<void*> mem2;
    // now, a highly ineffeccient sorting alg.
    void * mem_max = mem[0];
    int place = arr.size();
    for(int j = 0;j < place;j++){
        int min = MinInd3(arr); // we need to change this so it fits var char
        void * newBlock = malloc(17 + length);
        memcpy(newBlock, mem[min], 17 + length);
        arr2.push_back(arr[min]);
        mem2.push_back(newBlock);

        free(mem[min]);
        arr.erase(arr.begin() + min);
        mem.erase(mem.begin()+ min);

    }

    int buff2 = 32 + length;
    // we finally write it back to the page 
    for(int k = 0;k < mem2.size();k++){
        memcpy((char *) page + buff2, mem2[k],16 + length + 1);
        free(mem2[k]);
        buff2+= (17+length);
    }



}




void IndexManager::sortPage(IXFileHandle &ixfileHandle, unsigned pageNum){ // used to sort the page entries to make it easier to direct traffic down to leaf nodes
    void * page = malloc(PAGE_SIZE);
    ixfileHandle.readPage(pageNum,page);
    int * type = (int *) malloc(4);
    memcpy(type,(char *) page + 28 ,4); // might need to change 28

    switch(*type){
        case 0:
            cout << "err: entry not written to" << endl;
            return;
        case 1:
            sortPage1(ixfileHandle, page); //if key == int
            break;
        case 2:
            sortPage2(ixfileHandle, page); // if key == real
            break;
        case 3:
            sortPage3(ixfileHandle, page); // if key == VarChar
            break;
    }


    ixfileHandle.writePage(pageNum,page);
    free(page);
}






RC IndexManager::insertEntry(IXFileHandle &ixfileHandle, const Attribute &attribute, const void *key, const RID &rid)
{
    // first we deal with the case where there is only 
    void * meta = malloc(PAGE_SIZE);
    ixfileHandle.readPage(0,meta);

    int * pageNumbers = (int *) malloc(INT_SIZE);
    memcpy(pageNumbers, (char *) meta + 8, INT_SIZE); //now we know how mny pagess we have

    int * root_number = (int *) malloc();
    memcpy(root_number, (char * ) meta + 4, INT_SIZE);// get out root page number

    if(pageNumbers < 1){ // means we havent filled in the key for the root node
        void * rootPage = malloc(PAGE_SIZE);
        ixfileHandle.readPage(root_number, rootPage);
        int t = 0;
        switch(attribute.type){
            case TypeInt:
                t = 1;
                break;
            case TypeReal:
                t = 2;
                break;
            case TypeVarChar:
                t = 3;
                break;
             default:
              t = 0;
                 break;
        }

        memcpy((char *) rootPage + 24, &t,INT_SIZE); // now we know what type of attr we have

        // we do something with the rid, i forgot what
        //

        memcpy((char *) rootPage + 32, key, sizeof(key)); // now we copy over the data 
        int update = *pageNumbers;
        update++;
        memcpy((char *) meta + 8, &update,INT_SIZE);    // increment the page total on meta page

        ixfileHandle.writePage(*root_number, rootPage); // write the updated page to memory
        ixfileHandle.writePage(0,meta); // wrte the updated meta page to memory

        free(rootPage);
        
        return 0;
    }

	

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
	
    unsigned desiredPageId;
    if(treeVersal(attr, ixFileHandle, rootPage, lowKey, desiredPageId) != 0){
	return -1;
    }
    free(rootPage);

   
    return 0;
}

RC IndexManager::treeVersal(const Attribute attr, IXFileHandle &ixFHandle, unsigned pageId, unsigned &desiredPageId){

    void *pageData = malloc(PAGE_SIZE);

    if(ixFileHandle.readpage(pageId, pageData) != 0){
	return -1;
    }

    uint32_t type;
    memcpy(&type, pageData, INT_SIZE);
    if(type == 1){
	desiredPageId = pageId;
	free(pageData);
	return 0;	
    }

    return treeVersal(attr, ixFHandle, key, getCorrectChildID(key, pageData), desiredPageId);
}

unsigned IndexManager::getCorrectChildID(const void * key, void * pageData){
	unsigned offset = 32;

	bool nextEntry = true; 
	unsigned childPageID;
	unsigned type;
	memcpy(&type, pageData + 28, INT_SIZE);

	while(nextEntry){
		if(compareKey(attribute, key, (char*) pageData + offset) < 0){
			memcpy(&childPageID, (char *) pageData + offset, sizeof(unsigned));
			nextEntry = false;
		}else{
			if(type == 0 || type == 1) offset += 20;
			else{
				unsigned varcharLength;
				memcpy(&varcharLength, (char *) pageData + offset, VARCHAR_LENGTH_SIZE);
				offset += varcharLength;
				offset += VARCHAR_LENGTH_SIZE;
			}
		}		
	}

	return childPageID;
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
