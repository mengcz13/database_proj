#include "rm.h"

RC RM_FileScan::openScan(AttrType attrType,
                int attrLength,
                int attrOffset,
                CompOp compOp,
                void* value) {

    this->currRID = RID(1, -1);
    this->attrOffset = attrOffset;
    this->attrLength = attrLength;
    this->comp = Compare(attrType, compOp);
    this->vi = value;
}

RC RM_FileScan::getNextRec(RM_Record& rec) {
    int pagen = fileHandle.fileHeader.pagen;
    int slotn = fileHandle.fileHeader.slotn;
    int pn = 0, sn = 0;
    for (pn = currRID.pageNum; pn < pagen; ++pn) {
        int index = 0;
        char* pp = (char*)fileHandle.bufPageManager.getPage(fileHandle.fileID, pn, index);
        fileHandle.pPageHeader->fromCharArray(pp);
        fileHandle.bufPageManager.access(index);
        if (pn == currRID.pageNum)
            sn = currRID.slotNum + 1;
        else
            sn = 0;
        for (;sn < slotn; ++sn) {
            if (fileHandle.pPageHeader->checkAt(sn)) {
                char* data = pp + fileHandle.getPSlotOffSet(sn);
                char* pi = data + attrOffset;
                if (comp.comp(pi, vi, attrLength)) {
                    currRID = RID(pn, sn);
                    rec.setRID(currRID);
                    rec.setData(data, fileHandle.fileHeader.slotSize);
                    return OK;
                }
            }
        }
    }
    return NOTFOUND;
}

RC RM_FileScan::closeScan() {
    vi = NULL;
}