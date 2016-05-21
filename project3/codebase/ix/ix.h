#ifndef _ix_h_
#define _ix_h_

#include <vector>
#include <string>

#include "../rbf/rbfm.h"

# define IX_EOF (-1)  // end of the index scan

class IX_ScanIterator;
class IXFileHandle;

class IndexManager {

    public:
        static IndexManager* instance();

        // Create an index file.
        RC createFile(const string &fileName);

        // Delete an index file.
        RC destroyFile(const string &fileName);

        // Open an index and return an ixfileHandle.
        RC openFile(const string &fileName, IXFileHandle &ixfileHandle);

        // Close an ixfileHandle for an index.
        RC closeFile(IXFileHandle &ixfileHandle);

        // Insert an entry into the given index that is indicated by the given ixfileHandle.
        RC insertEntry(IXFileHandle &ixfileHandle, const Attribute &attribute, const void *key, const RID &rid);

        // Delete an entry from the given index that is indicated by the given ixfileHandle.
        RC deleteEntry(IXFileHandle &ixfileHandle, const Attribute &attribute, const void *key, const RID &rid);

        unsigned getCorrectChildID(const void * key, void * pageData);
        RC treeVersal(IXFileHandle &ixFHandle, unsigned pageId, unsigned &desiredPageId, const void * lowkey);
        // Initialize and IX_ScanIterator to support a range search
        RC scan(IXFileHandle &ixfileHandle,
                const Attribute &attribute,
                const void *lowKey,
                const void *highKey,
                bool lowKeyInclusive,
                bool highKeyInclusive,
                IX_ScanIterator &ix_ScanIterator);

        // Print the B+ tree in pre-order (in a JSON record format)
        void printBtree(IXFileHandle &ixfileHandle, const Attribute &attribute) const;

    protected:
        IndexManager();
        ~IndexManager();

    private:
        void RecursiveInsert(IXFileHandle IXFH, void * starting_page, void * key, int keytype, int entry_number);
        int compareKeys(void * entry, const void * key, unsigned type);
        void sortPage1(IXFileHandle &ixfileHandle, void * page);
        void sortPage2(IXFileHandle &ixfileHandle, void * page);
        void sortPage3(IXFileHandle &ixfileHandle, void * page);
        void sortPage(IXFileHandle &ixfileHandle, unsigned pageNum);
        static IndexManager *_index_manager;
        static PagedFileManager *_p;

};


class IX_ScanIterator {
    friend class RBFM_ScanIterator;
    public:

        // Constructor
        IX_ScanIterator();

        // Destructor
        ~IX_ScanIterator();

        vector<void*> getEntries();
        RC setEntriesValues(vector<void*> data);
        // Get next matching entry
        RC getNextEntry(RID &rid, void *key);

        // Terminate index scan
        RC close();

    private:
        RBFM_ScanIterator *_rbfm_ScanIterator;
};



class IXFileHandle {
    friend class FileHandle;
    public:

    // variables to keep counter for each operation
    unsigned ixReadPageCounter;
    unsigned ixWritePageCounter;
    unsigned ixAppendPageCounter;
    FileHandle f;

    // Constructor
    IXFileHandle();

    // Destructor
    ~IXFileHandle();

        RC readPage(PageNum pageNum, void *data);
        RC writePage(PageNum pageNum, const void *data);
        RC appendPage(const void *data);

	// Put the current counter values of associated PF FileHandles into variables
	RC collectCounterValues(unsigned &readPageCount, unsigned &writePageCount, unsigned &appendPageCount);

};

#endif
