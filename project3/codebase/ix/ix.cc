#include "ix.h"
#include <iostream>
#include <string.h>

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



int IndexManager::compareKeys(void * entry, const void * key, unsigned type){ // this function comapare
    int * e;
    int * k;
    float * kk;
    float * ee;
    int * length;
    int * length2;
    char * str;
    char * keystr;
    switch(type){
        case 1:{ // its an int
            k = (int *) malloc(4);
            e = (int *) malloc(4);
            memcpy(e,entry,4);
            memcpy(k,key,4);
            if(*e < *k){
                return -1;
            } else if (*e > *k){
                return 1;
            } else{
                return 0;
            }

            free(e);
            free(k);
            break;
        }
        case 2:{ // its a real
            kk = (float*) malloc(4);
            ee = (float *) malloc(4);
            memcpy(ee,entry,4);
            memcpy(kk,key,4);
            if(*ee < *kk){
                return -1;
            } else if (*ee > *kk){
                return 1;
            } else {
                return 0;
            }
            free(ee);
            free(kk);
            break;
        }
        case 3:{ // its a var char
            length =(int *) malloc(4);
            memcpy(length,entry,4);
            length2 = (int *) malloc(4);
            memcpy(length2,key,4);
            str =(char *) malloc(*length+1);
            keystr = (char *) malloc(*length2+1);
            str[*length]  ='\0';
            keystr[*length2] = '\0';
            int result = strcmp(str,keystr);
            free(str);
            free(length);
            return result;
            break;
        }
        default:{
            break;
        }
    }

    return -100;
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

RC IndexManager::insertEntry(IXFileHandle &ixfileHandle, const Attribute &attribute, const void *key, const RID &rid){

    // first we deal with the case where there is only 
    void * meta = malloc(PAGE_SIZE);
    ixfileHandle.readPage(0,meta);

    int * pageNumbers = (int *) malloc(INT_SIZE);
    memcpy(pageNumbers, (char *) meta + 8, INT_SIZE); //now we know how mny pagess we have

    int * root_number = (int *) malloc(4);
    memcpy(root_number, (char * ) meta + 4, INT_SIZE);// get out root page number

    if(*pageNumbers < 1){ // means we havent filled in the key for the root node
        void * rootPage = malloc(PAGE_SIZE);
        ixfileHandle.readPage(*root_number, rootPage);
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

        memcpy((char *) rootPage + 28, &t,INT_SIZE); // now we know what type of attr we have

        // we do something with the rid, i forgot what
        //

        memcpy((char *) rootPage + 32, key, sizeof(key)); // now we copy over the data 
        int update = *pageNumbers;
        update++;
        memcpy((char *) meta + 8, &update,INT_SIZE);    // increment the page total on meta page


        void * newLeaf = malloc(PAGE_SIZE);
        int * leaf_page_number = (int *) malloc(4);
        memcpy(leaf_page_number,(char *) meta + 8,4);
        int i = *leaf_page_number;
        i++;
        memcpy(newLeaf,&i,4);
        i = 1;
        memcpy((char *) newLeaf + 4,&i,4);
        i = 1;
        memcpy((char *) newLeaf + 8, &i,4);

        int VarCharSize; // figure this out later
        if(t == 3){
            VarCharSize =  sizeof(key)/4;
        } else{ VarCharSize = 4;  }


        memcpy((char * ) newLeaf + 28, key, VarCharSize);
        // left right child are null, buffer 8 bytes

        memcpy((char *) newLeaf + (36 + VarCharSize), &(rid.pageNum),4);
        memcpy((char *) newLeaf + (40 + VarCharSize), &(rid.slotNum),4);
        i = 40 + VarCharSize;
        memcpy((char *) newLeaf + 16, &i,4);

        memcpy((char *) rootPage + (32 + VarCharSize), root_number, 4);

        ixfileHandle.writePage(*root_number, rootPage); // write the updated page to memory
        ixfileHandle.writePage(0,meta);
        free(newLeaf);
        free(rootPage);
        free(leaf_page_number);
        return 0;

    }

    void * entry;
    int t;


    // we now search for a place to put our new entry 
    bool inserted = false;
    void * our_page = malloc(PAGE_SIZE);
    ixfileHandle.readPage(*root_number,our_page); // start at the root

    bool get_next = true; // we find which road to the leaf we take when this is finally false
   // at this point i was supposed to use recursive insert but i couldnt get it to work :( 

    return -1;
}






void IndexManager::RecursiveInsert(IXFileHandle IXFH, void * starting_page, void * key, int keytype, int entry_number){
    //decide whether to go left or right
    // insert if we reach leaf

    void * entry;
    int buffer = 32;
    int in_case;
    bool found = false;


// extract our record entry
while(!found){
    switch(keytype){
        case 1:{
            buffer = (entry_number - 1) * 20;
            entry = malloc(20);
            memcpy(entry,(char * ) starting_page + buffer,20);
        }
        case 2:{
            buffer = (entry_number - 1) * 20;
            entry = malloc(20);
            memcpy(entry,(char * ) starting_page + buffer,20);
        }
        case 3:{
            for(int i = 0;i < entry_number;i++){
                memcpy(&in_case,(char *) starting_page + buffer,4);
                if((i-1) == entry_number){ entry = malloc(20 + in_case); memcpy(entry,(char *) starting_page + buffer, (20 + in_case)); break;   }
                buffer =  buffer + in_case + 20;
            }

        }

    }

    int i2 = compareKeys(entry,key,keytype);
    if(i2 == 1 || i2 == 0) found == true;
}
    int next_page_number;

    int i = compareKeys(entry,key,keytype);
    if(i == -1){
        // going left    
        if(keytype == 1 || keytype == 2) memcpy(&next_page_number,(char *) entry + 4, 4);
        else{
            int buffer3;
            memcpy(&buffer3,entry, 4);
            memcpy(&next_page_number,(char*) entry + (4+buffer3),4);

        }
    } else{
        // going right
        if(keytype == 1 || keytype == 2) memcpy(&next_page_number,(char *) entry + 8, 4);
        else{
            int buffer3;
            memcpy(&buffer3,entry, 4);
            memcpy(&next_page_number,(char*) entry + (8+buffer3),4);

        }



    }

    entry_number = 1;
    IXFH.readPage(next_page_number,starting_page);
    RecursiveInsert(IXFH,starting_page,key,keytype, entry_number);

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
    bool LowUnlimited = (lowKey == NULL);    
    bool HighUnlimited = (highKey == NULL);  

    void * rootPage = malloc(PAGE_SIZE);
    unsigned rootId;   

    //open up the meta page to find the root
    if(ixfileHandle.readPage(0, rootPage) != 0){
	return -1;
    }

    //input root id findings from the meta page to the rootid
    memcpy(&rootId, rootPage, INT_SIZE);
	
    //run the recursive treeVersal to find the leaf id with the lowest
    //possible leaf value
    unsigned desiredPageId;
    if(!LowUnlimited){
    	if(treeVersal(ixfileHandle, rootId, desiredPageId, lowKey) != 0){
		return -1;
    	}
    }else{
	if(treeVersal(ixfileHandle, rootId, desiredPageId, 0) != 0){
		return -1;
    	}
    }

    void * lowestLeafPage = malloc(PAGE_SIZE);

    if(ixfileHandle.readPage(desiredPageId, lowestLeafPage) != 0){
	return -1;
    }
    
    bool nextEntry = true;
    vector<void*> validEntries;

    //since we've gotten to the lowest valid leaf, we must walk through each
    //entries' right child until it reaches the highKey!
    while(nextEntry){
	RID rid;
	void * nextEntryData = malloc(PAGE_SIZE);

	//walking through
	if(ix_ScanIterator.getNextEntry(rid, nextEntryData) != 0){
		return -1;
	}

	//figure out the type of entry (varchar, int, real)
	unsigned type;
	memcpy(&type, nextEntryData, INT_SIZE);

	if(highKeyInclusive){
		if(compareKeys(nextEntryData, highKey, type) > -1){
			validEntries.push_back(nextEntryData);
		}else{
			nextEntry = false;
		}
	}else{
		//if its not inclusive on the high level, then all 
		if(compareKeys(nextEntryData, highKey, type) > 0){
			validEntries.push_back(nextEntryData);
		}else{	
			nextEntry = false;
		}
	}
    } 

    //set the entries values to the scan iterator as the docs request
    ix_ScanIterator.setEntriesValues(validEntries);

    return 0;
}

RC IndexManager::treeVersal(IXFileHandle &ixFHandle, unsigned pageId, unsigned &desiredPageId, const void * lowkey){

    void *pageData = malloc(PAGE_SIZE);

    //opens the page passed in - first case this is the root
    if(ixFHandle.readPage(pageId, pageData) != 0){
	return -1;
    }

    //in the fourth byte slot (4-8) there is an integer that describes what type
    //of entry it is.
    uint32_t type;
    memcpy(&type, pageData + 4, INT_SIZE);

    //type == 1 implies leaf page, and in this case we found our target. So reset
    //the desiredPageId to the current page to call in the parent function scan
    if(type == 1){
	desiredPageId = pageId;
	free(pageData);
	return 0;	
    }

    //recursive call to traverse to the next child.
    return treeVersal(ixFHandle, getCorrectChildID(lowkey, pageData), desiredPageId,lowkey);
}

//chooses which child to choose when traversing from the root to the leaves
//key is the comparison key for each entry
unsigned IndexManager::getCorrectChildID(const void * key, void * pageData){
	//in hindsight, getNextRecord would've worked in this function.

	//the header ends after 32 bits
        unsigned offset = 32;

	bool nextEntry = true; 
	unsigned childPageID;
	unsigned type;
	memcpy(&type, pageData + 28, INT_SIZE);

	//test if it is 0 then it is the left most child to say less than all children
	if(key == 0){
	   memcpy(&childPageID, (char *) pageData + offset, sizeof(unsigned));
	   return childPageID;	
        } 
   
	while(nextEntry){
		if(compareKeys((char*)pageData + offset, key, type) < 0){
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
    if(_rbfm_ScanIterator->getNextRecord(rid, key) != 0){
	return IX_EOF;
    }
    return 0;
}

RC IX_ScanIterator::setEntriesValues(vector<void*> data){
	_rbfm_ScanIterator->setEntries(data);
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
