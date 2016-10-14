#ifndef RM_H
#define RM_H
#include <cstring>
#include "bufmanager/BufPageManager.h"
#include "fileio/FileManager.h"
#include "utils/pagedef.h"
#include "utils/utils.h"

class RM_Manager {
public:
    RM_Manager(BufPageManager& bpm);
    ~RM_Manager();
    
    RC createFile(const char* fileName, int recordSize);
    RC destroyFile(const char* fileName);
    RC openFile(const char* fileName, RM_FileHandle& fileHandle);
    RC closeFile(RM_FileHandle& fileHandle);

private:
    BufPageManager& bufPageManager;
    FileManager& fileManager;
};

class RM_FileHandle {
public:
    RM_FileHandle() {
        pPageHeader = NULL;
    }
    ~RM_FileHandle() {
        if (pPageHeader)
            delete pPageHeader;
    }
    
    RC getRec(const RID& rid, RM_Record& rec) const;
    RC insertRec(const char* pData, RID& rid);
    RC deleteRec(const RID& rid);
    RC updateRec(const RM_Record& rec);
    RC forcePages();
    int getFileID() const { return fileID; }
    void setFileID(int id) { fileID = id; }
    void setBufPageManager(BufPageManager& bpm) { bufPageManager = bpm; }
    void setFileHeader(FileHeader& fh) { 
        fileHeader = fh;
        if (pPageHeader)
            delete pPageHeader;
        pPageHeader = new PageHeader(fileHeader.slotn); 
    }
    size_t getPSlotOffSet(SlotNum slotID) {
        return pPageHeader->getSize() + slotID * fileHeader.slotSize;
    }

private:
    int fileID;
    BufPageManager& bufPageManager;
    FileHeader fileHeader;
    PageHeader* pPageHeader;
};

class RM_FileScan {
public:
    RM_FileScan();
    ~RM_FileScan();

    RC openScan(const RM_FileHandle& fileHandle,
                AttrType attrType,
                int attrLength,
                int attrOffset,
                CompOp compOp,
                void* value);
    RC getNextRec(RM_Record& rec);
    RC closeScan();
};

class RM_Record {
public:
    RM_Record() {
        _data = NULL;
    }
    ~RM_Record() {
        if (_data)
            delete []_data;
    }

    RC getData(char*& pData) const {
        pData = _data;
    }
    RC getRid(RID& rid) const {
        rid.copyFrom(_rid);
    }
    RC setData(const char* pData);
    RC setRID(const RID& rid) {
        _rid.copyFrom(rid);
    }

private:
    char* _data;
    RID _rid;
};

class RID {
public:
    RID() {}
    ~RID() {}
    RID(PageNum pageID, SlotNum slotID): pageNum(pageID), slotNum(slotID) {

    }

    RC getPageID(PageNum& pageID) const {
        pageID = pageNum;
        return OK;
    }
    RC getSlotID(SlotNum& slotID) const {
        slotID = slotNum;
        return OK;
    }

    void copyFrom(const RID& rid) {
        pageNum = rid.pageNum;
        slotNum = rid.slotNum;
    }

private:
    PageNum pageNum;
    SlotNum slotNum;
};

// Header of File, stored at page0
struct FileHeader {
    PageNum pagen;  // number of pages in a file, including page0!
    SlotNum slotn;  // number of slots in a page
    int slotSize;   // size of a slot, recordSize
    FP firstFree;   // point to the first free page, if == pagen then no free page

    FileHeader(PageNum p, SlotNum sn, int s, FP f): pagen(p), slotn(sn), slotSize(s), firstFree(f) {}
    FileHeader(int s): pagen(1), slotSize(s), firstFree(1) {
        slotn = (PAGE_SIZE - PageHeader::getFixedSize()) / (slotSize + 1);
    }
    FileHeader(char* p) {
        fromCharArray(p);
    }
    FileHeader() {}
    void fromCharArray(char* p) {
        (*this) = *((FileHeader*)p);
    }
    void toCharArray(char* p) {
        *((FileHeader*)p) = (*this);
    }
};

// Header of page1, 2, .. 
struct PageHeader {
    FP nextFree;
    bool isFree;
    MyBitSet myBitSet;
    
    PageHeader(SlotNum s): myBitSet(s) {}
    static size_t getFixedSize() { return sizeof(FP) + sizeof(bool); }
    static size_t getSize(int slotNum) {
        return getFixedSize() + MyBitSet::getDataSize(slotNum);
    }
    size_t getSize() { return getFixedSize() + myBitSet.getDataSize(); }
    void fromCharArray(char* source) {
        nextFree = *((FP*)source);
        source += sizeof(FP);
        isFree = *((bool*)source);
        source += sizeof(bool);
        myBitSet.fromCharArray(source);
    }
    void toCharArray(char* target) {
        *((FP*)target) = nextFree;
        target += sizeof(FP);
        *((bool*)target) = isFree;
        target += sizeof(bool);
        myBitSet.toCharArray(target);
    }
};

// My Bit Set Implementation
typedef char Unit;
#define LBIT(x, u) (1 << ((sizeof(u) << 3) - 1 - x))
struct MyBitSet {
    Unit* _data;
    SlotNum size;
    size_t _datasize;

    MyBitSet(SlotNum s) {
        _datasize = s - 1 + sizeof(Unit) / sizeof(Unit);
        _data = new Unit[_datasize];
        memset(_data, 0, sizeof(Unit) * _datasize);
        size = s;
    }

    ~MyBitSet() { delete []_data; }

    Unit* getPos(SlotNum n) { return _data + n / sizeof(Unit); }

    void set(SlotNum n) {   // set to 1
        (*(getPos(n))) |= LBIT(n % sizeof(Unit), Unit);
    }

    void clear(SlotNum n) { // set to 0
        (*(getPos(n))) &= (~LBIT(n % sizeof(Unit), Unit));
    }

    bool check(SlotNum n) { // check if is 1
        Unit t = *(getPos(n));
        return (t&=LBIT(n % sizeof(Unit), Unit));
    }

    SlotNum getSlotNum() { return size; }

    size_t getDataSize() { return sizeof(Unit) * _datasize; }
    static size_t getDataSize(SlotNum s) {
        return sizeof(Unit) * (s - 1 + sizeof(Unit) / sizeof(Unit));
    }

    void toCharArray(char* target) {
        memcpy(target, _data, sizeof(Unit) * _datasize);
    }

    void fromCharArray(char* source) {
        memcpy(_data, source, sizeof(Unit) * _datasize);
    }

};

#endif