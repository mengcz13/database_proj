#include "rm.h"

RC RM_FileHandle::getRec(const RID& rid, RM_Record& rec) const {
    int index = 0;
    PageNum pageID = 0;
    SlotNum slotID = 0;
    rid.getPageID(pageID);
    rid.getSlotID(slotID);
    char* p = bufPageManager.getPage(fileID, pageID, index);
    char* prec = p + getPSlotOffSet(slotID);
    rec.setRID(rid);
    rec.setData(prec);
    bufPageManager.access(index);
    return OK;
}

RC RM_FileHandle::updateRec(const RM_Record& rec) {
    int index = 0, pn = 0, sn = 0;
    RID rid;
    rec.getRid(rid);
    rid.getPageID(pn);
    rid.getSlotID(sn);
    char* p = bufPageManager.getPage(fileID, pn, index);
    char* prec = p + getPSlotOffSet(slotID);
    rec.getData(prec);
    bufPageManager.markDirty(index);
    return OK;
}