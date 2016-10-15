#include "rm.h"

RC RM_FileHandle::getRec(const RID& rid, RM_Record& rec) const {
    int index = 0;
    PageNum pageID = 0;
    SlotNum slotID = 0;
    rid.getPageID(pageID);
    rid.getSlotID(slotID);
    char* p = (char*)bufPageManager.getPage(fileID, pageID, index);
    char* prec = p + getPSlotOffSet(slotID);
    rec.setRID(rid);
    char* _pdata = NULL;
    rec.getData(_pdata);
    memcpy(_pdata, prec, fileHeader.slotSize);
    bufPageManager.access(index);
    return OK;
}

RC RM_FileHandle::insertRec(const char* pData, RID& rid) {
    int index = 0, pn = 0, sn = 0;
    char* pPage = NULL;
    PageNum firstFreePage = fileHeader.firstFree;
    for (; firstFreePage < fileHeader.pagen; ) {
        pPage = (char*)bufPageManager.getPage(fileID, firstFreePage, index);
        pPageHeader->fromCharArray(pPage);
        bufPageManager.access(index);
        if (pPageHeader->currentSlotNum < fileHeader.slotn) {
            for (sn = 0; sn < fileHeader.slotn; ++sn)
                if (!pPageHeader->checkAt(sn))
                    break;
            break;
        }
        firstFreePage = pPageHeader->nextFree;
    }
    pn = firstFreePage;
    char* prec = NULL;
    if (pn == fileHeader.pagen) {
        sn = 0;
        pPageHeader->reset();
        ++fileHeader.pagen;
        pPageHeader->nextFree = fileHeader.pagen;
        fileHeader.firstFree = pn;
        pPage = (char*)bufPageManager.getPage(fileID, pn, index);
        pPageHeader->toCharArray(pPage);
        bufPageManager.markDirty(index);
    }
    pPage = (char*)bufPageManager.getPage(fileID, pn, index);
    prec = pPage + getPSlotOffSet(sn);
    memcpy(prec, pData, fileHeader.slotSize);
    pPageHeader->insertAt(sn);
    if (pPageHeader->currentSlotNum == fileHeader.slotn) {
        fileHeader.firstFree = pPageHeader->nextFree;
    }
    pPageHeader->toCharArray(pPage);
    bufPageManager.markDirty(index);
    fileHeaderModified = true;
    // int index2 = 0;
    // char* pFH = (char*)bufPageManager.getPage(fileID, 0, index2);
    // fileHeader.toCharArray(pFH);
    // bufPageManager.markDirty(index2);
    rid.copyFrom(RID(pn, sn));
    return OK;
}

RC RM_FileHandle::deleteRec(const RID& rid) {
    int index = 0, pn = 0, sn = 0;
    rid.getPageID(pn);
    rid.getSlotID(sn);
    char* p = (char*)bufPageManager.getPage(fileID, pn, index);
    pPageHeader->fromCharArray(p);
    pPageHeader->deleteAt(sn);
    if (pPageHeader->currentSlotNum == fileHeader.slotn - 1) {
        pPageHeader->nextFree = fileHeader.firstFree;
        fileHeader.firstFree = pn;
    }
    pPageHeader->toCharArray(p);
    bufPageManager.markDirty(index);
    fileHeaderModified = true;
    // p = (char*)bufPageManager.getPage(fileID, 0, index);
    // fileHeader.toCharArray(p);
    // bufPageManager.markDirty(index);
    return OK;
}

RC RM_FileHandle::updateRec(const RM_Record& rec) {
    int index = 0, pn = 0, sn = 0;
    RID rid;
    rec.getRid(rid);
    rid.getPageID(pn);
    rid.getSlotID(sn);
    char* p = (char*)bufPageManager.getPage(fileID, pn, index);
    char* prec = p + getPSlotOffSet(sn);
    char* data = NULL;
    rec.getData(data);
    memcpy(prec, data, fileHeader.slotSize);
    bufPageManager.markDirty(index);
    return OK;
}