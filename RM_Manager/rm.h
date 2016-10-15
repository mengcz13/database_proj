#ifndef RM_H
#define RM_H
#include <cstring>
#include <cmath>
#include "bufmanager/BufPageManager.h"
#include "fileio/FileManager.h"
#include "utils/pagedef.h"
#include "utils/utils.h"

// My Bit Set Implementation
typedef char Unit;
#define LBIT(x, u) (1 << ((sizeof(u) << 3) - 1 - x))
#define BITOF(x) (sizeof(x) << 3)
struct MyBitSet {
    Unit* _data;
    SlotNum size;
    size_t _datasize;

    MyBitSet(SlotNum s) {
        _datasize = (s - 1 + BITOF(Unit)) / BITOF(Unit);
        _data = new Unit[_datasize];
        memset(_data, 0, sizeof(Unit) * _datasize);
        size = s;
    }

    ~MyBitSet() { delete []_data; }

    Unit* getPos(SlotNum n) const { return _data + n / BITOF(Unit); }

    void set(SlotNum n) {   // set to 1
        (*(getPos(n))) |= LBIT(n % BITOF(Unit), Unit);
    }

    void clear(SlotNum n) { // set to 0
        (*(getPos(n))) &= (~LBIT(n % BITOF(Unit), Unit));
    }

    void clearAll() {
        memset(_data, 0, sizeof(Unit) * _datasize);
    }

    bool check(SlotNum n) const { // check if is 1
        Unit t = *(getPos(n));
        return (t&=LBIT(n % BITOF(Unit), Unit));
    }

    SlotNum getSlotNum() const { return size; }

    size_t getDataSize() const { return sizeof(Unit) * _datasize; }
    static size_t getDataSize(SlotNum s) {
        return sizeof(Unit) * ((s - 1 + BITOF(Unit)) / BITOF(Unit));
    }

    void toCharArray(char* target) const {
        memcpy(target, _data, sizeof(Unit) * _datasize);
    }

    void fromCharArray(char* source) {
        memcpy(_data, source, sizeof(Unit) * _datasize);
    }

};

// Header of page1, 2, .. 
struct PageHeader {
    FP nextFree;
    SlotNum currentSlotNum;
    MyBitSet myBitSet;
    
    PageHeader(SlotNum s): currentSlotNum(0), myBitSet(s) {}
    static size_t getFixedSize() { return sizeof(FP) + sizeof(SlotNum); }
    static size_t getSize(int slotNum) {
        return getFixedSize() + MyBitSet::getDataSize(slotNum);
    }
    size_t getSize() { return getFixedSize() + myBitSet.getDataSize(); }
    void fromCharArray(char* source) {
        nextFree = *((FP*)source);
        source += sizeof(FP);
        currentSlotNum = *((SlotNum*)source);
        source += sizeof(SlotNum);
        myBitSet.fromCharArray(source);
    }
    void toCharArray(char* target) const {
        *((FP*)target) = nextFree;
        target += sizeof(FP);
        *((SlotNum*)target) = currentSlotNum;
        target += sizeof(SlotNum);
        myBitSet.toCharArray(target);
    }
    void insertAt(SlotNum s) {
        ++currentSlotNum;
        myBitSet.set(s);
    }
    void deleteAt(SlotNum s) {
        --currentSlotNum;
        myBitSet.clear(s);
    }
    bool checkAt(SlotNum s) const {
        return myBitSet.check(s);
    }
    void reset() {
        currentSlotNum = 0;
        myBitSet.clearAll();
    }
};

// Header of File, stored at page0
struct FileHeader {
    PageNum pagen;  // number of pages in a file, including page0!
    SlotNum slotn;  // number of slots in a page
    int slotSize;   // size of a slot, recordSize
    FP firstFree;   // point to the first free page, if == pagen then no free page

    FileHeader(PageNum p, SlotNum sn, int s, FP f): pagen(p), slotn(sn), slotSize(s), firstFree(f) {}
    FileHeader(int s): pagen(1), slotSize(s), firstFree(1) {
        slotn = ((PAGE_SIZE - PageHeader::getFixedSize() - 1) << 3) / ((slotSize << 3) + 1);
    }
    FileHeader(char* p) {
        fromCharArray(p);
    }
    FileHeader() {}
    void fromCharArray(char* p) {
        (*this) = *((FileHeader*)p);
    }
    void toCharArray(char* p) const {
        *((FileHeader*)p) = (*this);
    }
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

// private:
    PageNum pageNum;
    SlotNum slotNum;
};

class RM_Record {
public:
    RM_Record() {
        _data = NULL;
    }
    RM_Record(int recordSize) {
        _data = new char[recordSize];
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
    RC setData(const char* pData, int len) {
        memcpy(_data, pData, len);
    }
    RC setRID(const RID& rid) {
        _rid.copyFrom(rid);
    }

private:
    char* _data;
    RID _rid;
};

class RM_FileHandle {
public:
    RM_FileHandle(BufPageManager& bpm): bufPageManager(bpm) {
        pPageHeader = NULL;
        fileHeaderModified = false;
    }
    ~RM_FileHandle() {
        if (pPageHeader)
            delete pPageHeader;
    }
    
    RC getRec(const RID& rid, RM_Record& rec) const;
    RC insertRec(const char* pData, RID& rid);
    RC deleteRec(const RID& rid);
    RC updateRec(const RM_Record& rec);
    RC forcePages() {
        writeFileHeader();
        bufPageManager.close();
    }
    int getFileID() const { return fileID; }
    void setFileID(int id) { fileID = id; }
    void setBufPageManager(BufPageManager& bpm) { bufPageManager = bpm; }
    void setFileHeader(FileHeader& fh) { 
        fileHeader = fh;
        if (pPageHeader)
            delete pPageHeader;
        pPageHeader = new PageHeader(fileHeader.slotn); 
    }
    void writeFileHeader() {
        if (fileHeaderModified) {
            int index = 0;
            char* p = (char*)bufPageManager.getPage(fileID, 0, index);
            fileHeader.toCharArray(p);
            bufPageManager.markDirty(index);
        }
    }
    size_t getPSlotOffSet(SlotNum slotID) const {
        return pPageHeader->getSize() + slotID * fileHeader.slotSize;
    }

    friend class RM_FileScan;

private:
    int fileID;
    BufPageManager& bufPageManager;
    FileHeader fileHeader;
    PageHeader* pPageHeader;
    bool fileHeaderModified;
};

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

struct Compare {
    AttrType attrType;
    CompOp compOp;

    Compare() {}
    Compare(AttrType at, CompOp co): attrType(at), compOp(co) {}

    bool comp(void* pi, void* vi, int attrLength) {
        if (compOp == NO_OP) {
            return (!pi);
        }
        else if (!pi || !vi)
            return false;
        int p = 0, v = 0;
        double p2 = 0, v2 = 0;
        char* p3 = NULL; char* v3 = NULL;
        switch (attrType) {
            case INTEGER:
                p = *(int*)pi;
                v = *(int*)vi;
                switch (compOp) {
                    case EQ_OP:
                        return (p == v);
                    case LT_OP:
                        return (p < v);
                    case GT_OP:
                        return (p > v);
                    case LE_OP:
                        return (p <= v);
                    case GE_OP:
                        return (p >= v);
                    case NE_OP:
                        return (p != v);
                    default:
                        return false;
                }
                break;
            case FLOAT:
                p2 = *(double*)pi;
                v2 = *(double*)vi;
                switch (compOp) {
                    case EQ_OP:
                        return (fabs(p2 - v2) < EPS);
                    case LT_OP:
                        return (p2 <= (v2 - EPS));
                    case GT_OP:
                        return (p2 >= (v2 + EPS));
                    case LE_OP:
                        return (p2 < (v2 + EPS));
                    case GE_OP:
                        return (p2 > (v2 - EPS));
                    case NE_OP:
                        return (fabs(p2 - v2) >= EPS);
                    default:
                        return false;
                }
                break;
            case STRING:
                p3 = (char*)pi;
                v3 = (char*)vi;
                switch (compOp) {
                    case EQ_OP:
                        return (strncmp(p3, v3, attrLength) == 0);
                    case NE_OP:
                        return (strncmp(p3, v3, attrLength));
                    case LT_OP:
                    case GT_OP:
                    case LE_OP:
                    case GE_OP:
                    default:
                        return false;
                }
                break;
            default:
                return false;
        }
        return false;
    }
};

class RM_FileScan {
public:
    RM_FileScan(const RM_FileHandle& fH): fileHandle(fH) {
        currRID = RID(1, 0);
        vi = NULL;
    }
    ~RM_FileScan() {}

    RC openScan(AttrType attrType,
                int attrLength,
                int attrOffset,
                CompOp compOp,
                void* value);
    RC getNextRec(RM_Record& rec);
    RC closeScan();

private:
    RID currRID;
    int attrOffset;
    int attrLength;
    Compare comp;
    void* vi;
    const RM_FileHandle& fileHandle;
};


#endif